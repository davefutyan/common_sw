/** ****************************************************************************
 *  @file
 *
 *  @ingroup bookkeeping
 *  @brief Implementation of the Bookkeeping class
 *
 *  @author Reiner Rohlfs, UGE
 *  
 *  @version 12.0.1 2019-11-14 ABE #20111 Handle 32 bit unsigned integers
 *  @version  9.0   2018-02-01 RRO        UTC can be used as bookkeeping number
 *  @version  8.0   2017-08-03 ABE        bug fix: the current directory was
 *                                        always prepended to the bookkeeping
 *                                        path.
 *
 *  @version 7.0  2017-01-20 RRO first version
 */

#include <fstream>

#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/range/algorithm/count.hpp"

#include "Bookkeeping.hxx"

using namespace std;

Bookkeeping::Bookkeeping(const std::string & path,
		const std::string & name,
		std::vector<BKNumber> & masterList) : m_master(masterList) {

	// Check that the name is not an empty string
	if (name == "") {
		throw runtime_error("Bookkeeping constructor: The bookkeeping group name is an empty string");
	}

	// Check that some BKNumbers are defined for the new bookkeeping group
	if (masterList.size() == 0) {
		throw runtime_error("Bookkeeping constructor: The list of BKNumbers is empty");
	}

	// Define the full path for the directory for the bookkeeping group
    string fullpath(path);
    if (fullpath.at(0) != '/') fullpath = string(getenv("PWD"))+"/"+fullpath;
	m_groupDir = fullpath+"/"+name;

	// Check that the path is valid
	if (!boost::filesystem::exists(fullpath)) {
		throw runtime_error("Bookkeeping constructor: Directory does not exist: "+fullpath);
	}

	// Check if the bookkeeping group already exists
	if (boost::filesystem::exists(m_groupDir)) {

		// Check if the new instance of the bookkeeping group has the same master as existing group
		bool master_found = false;
		// Loop over files in the existing group directory
		boost::filesystem::directory_iterator it(m_groupDir), eod;
		for (;it != eod; ++it) {

			if (is_regular_file(it->path())) {

				// Split the filename into substrings, using underscore as the separator
				vector<string> substrings;
				boost::split(substrings,it->path().filename().string(),boost::is_any_of("_"));

				// Identify the master list file
				if (substrings[0] == "master") {

					master_found = true;

					// Determine how many substrings there should be based on the master list
					unsigned numberOfSubstrings = masterList.size()*3 + 1;
					// Take into account that names can include an underscore, resulting in additional substrings
					vector<BKNumber>::const_iterator masterNum = masterList.begin();
					while (masterNum != masterList.end()) {
						numberOfSubstrings += boost::count(masterNum->m_name, '_');
						masterNum++;
					}

					// Require that the number of substrings is as expected
					if (substrings.size() != numberOfSubstrings) {
						throw runtime_error("Error in Bookkeeping constructor: master list does not match master list of existing group: "
								+it->path().filename().string());
					}

					// Extract the list of BKNumbers from the filename and compare to the master list
					unsigned istr = 1; // Skip the "master_" prefix
					masterNum = masterList.begin();
					while (masterNum != masterList.end()) {

						string name = substrings[istr++];
						// Take into account that names can include an underscore, requiring concatenation of substrings
						// since underscore is used as the substring separator
						unsigned count = boost::count(masterNum->m_name, '_');
						for (unsigned j=0; j<count; j++) {
							name += "_";
							name += substrings[istr++];
						}

						bool key = (substrings[istr++] == "true");

						BK_DATA_TYPE type;
						string type_str = substrings[istr++];
						if (type_str == "uint8") {
							type = bk_uint8;
						} else if (type_str == "uint16") {
							type = bk_uint16;
            } else if (type_str == "UTC") {
						   type = bk_UTC;
						} else if (type_str == "uint32") {
              type = bk_uint32;
						} else {
							throw runtime_error("Error in Bookkeeping::Insert: invalid BKDataType: "+type);
						}

						BKNumber bknum = {name,key,type};
						if (!(bknum == *masterNum)) {
							throw runtime_error("Error in Bookkeeping constructor: master list does not match master list of existing group: "
									+it->path().filename().string());
						}
						masterNum++;

					}

				}

			}
		}

		if (!master_found) {
			throw runtime_error("Bookkeeping constructor: Bookkeeping group "+name+" already exists, but there is no master list file for the existing group");
		}

	} else {

		// Create a sub-directory for the new bookkeeping group
		boost::filesystem::create_directory(m_groupDir);

		// Create a file to define the master list for the group
		string filename = "master_";

		// Loop over the BKNumbers
		vector<BKNumber>::const_iterator masterNum = masterList.begin();
		while (masterNum != masterList.end()) {

			// Append the name, key and data type for each BKNumber to the filename, separated by an underscore
			filename += masterNum->m_name;
			filename += masterNum->m_key ? "_true_" : "_false_";
			if (masterNum->m_bkDataType == bk_uint8) {
				filename += "uint8_";
			} else if (masterNum->m_bkDataType == bk_uint16) {
				filename += "uint16_";
      } else if (masterNum->m_bkDataType == bk_UTC) {
        filename += "UTC_";
			} else if (masterNum->m_bkDataType == bk_uint32) {
				filename += "uint32_";
			} else {
				throw runtime_error("Error in Bookkeeping::Insert: invalid BKDataType: "+masterNum->m_bkDataType);
			}

			masterNum++;
		}
		filename.pop_back(); // Remove the last trailing underscore

		ofstream ofs(m_groupDir+"/"+filename);
		ofs.close();

	}

}

void Bookkeeping::Insert(const BKItem & bkItem) {

	// First part of filename is the start and stop UTC times
	string filename = "TU";
	filename += bkItem.m_start.getUtc(true);
	filename += "_TU";
	filename += bkItem.m_stop.getUtc(true);
	filename += "_";
	boost::replace_all(filename, ":", "-");
	//boost::erase_all(filename, ":");
	//boost::erase_all(filename, "-");

	// Check that the number of BKNumbers is the same as for the master
	if (bkItem.m_bkNumbers.size() != m_master.size()) {
		throw runtime_error("Error in Bookkeeping::Insert: number of BKNumbers in inserted item not consistent with the bookkeeping group master list");
	}

	// Loop over the BKNumbers
	vector<BKNumber>::const_iterator masterNum = m_master.begin();
	vector<BKNumber>::const_iterator bknum = bkItem.m_bkNumbers.begin();
	while (bknum != bkItem.m_bkNumbers.end()) {

		// Check that the sequence of BKNumbers is the same as for the master
		if (!(*bknum == *masterNum)) {
			throw runtime_error("Error in Bookkeeping::Insert: name, key or data type for BKNumber "
					+bknum->m_name+" does not match the corresponding BKNumber of the bookkeeping group master list");
		}

		// Append the name and value for each BKNumber to the filename, separated by an underscore
		filename += bknum->m_name;
		filename += "_";
		if (bknum->m_bkDataType == bk_uint8) {
			filename += to_string(bknum->m_uint8Number);
		} else if (bknum->m_bkDataType == bk_uint16) {
			filename += to_string(bknum->m_uint16Number);
    } else if (bknum->m_bkDataType == bk_UTC) {
      filename += bknum->m_utc.getUtc(true);
		} else if (bknum->m_bkDataType == bk_uint32) {
			filename += to_string(bknum->m_uint32Number);
		} else {
			throw runtime_error("Error in Bookkeeping::Insert: invalid BKDataType: "+bknum->m_bkDataType);
		}
		filename += "_";

		masterNum++;
		bknum++;
	}
	filename.pop_back(); // Remove the last trailing underscore

	// Loop over existing items in the bookkeeping group
	boost::filesystem::directory_iterator it(m_groupDir), eod;
	for (;it != eod; ++it) {
		if (is_regular_file(it->path())) {

			// Check if an identical booking item already exists and exit without doing anything if it does
			if (filename == it->path().filename().string()) return;

			// Split the filename into substrings, using underscore as the separator
			vector<string> substrings;
			boost::split(substrings,it->path().filename().string(),boost::is_any_of("_"));

			// Skip the master list file
			if (substrings[0] == "master") continue;

			// Extract the start and stop UTC times from the filename
			UTC start = UTC(substrings[0]);
			UTC stop = UTC(substrings[1]);

			// Check that the time interval of the item to be inserted does not overlap with that of an existing item
			if (start<bkItem.m_stop && stop>bkItem.m_start) {
				throw runtime_error("Error in Bookkeeping::Insert: The time interval for the item to be inserted ("
						+bkItem.m_start.getUtc(true)+" - "+bkItem.m_stop.getUtc(true)
						+") overlaps with the time interval of the following existing item: "
						+it->path().filename().string());
			}

		}
	}

	// Create the bookkeeping item file and close it
	ofstream ofs(m_groupDir+"/"+filename);
	ofs.close();

}

std::list<BKItem> Bookkeeping::Find(const BKItem & bkItem) {

	// Check that the number of BKNumbers is the same as for the master
	if (bkItem.m_bkNumbers.size() != m_master.size()) {
		throw runtime_error("Error in Bookkeeping::Insert: number of BKNumbers in inserted item not consistent with the bookkeeping group master list");
	}

	// Check that the sequence of BKNumbers is the same as for the master
	vector<BKNumber>::const_iterator masterNum = m_master.begin();
	vector<BKNumber>::const_iterator bknum = bkItem.m_bkNumbers.begin();
	while (bknum != bkItem.m_bkNumbers.end()) {
		if (!(*bknum == *masterNum)) {
			throw runtime_error("Error in Bookkeeping::Find: name, key or data type for BKNumber "
					+bknum->m_name+" does not match the corresponding BKNumber of the bookkeeping group master list");
		}
		masterNum++;
		bknum++;
	}

	// List of matching  BKItems to be returned
	std::list<BKItem> bkItems;

	// Loop over existing items in the bookkeeping group
	boost::filesystem::directory_iterator it(m_groupDir), eod;
	for (;it != eod; ++it) {
		if (is_regular_file(it->path())) {

			BKItem item;

			// Split the filename into substrings, using underscore as the separator
			vector<string> substrings;
			boost::split(substrings,it->path().filename().string(),boost::is_any_of("_"));

			// Skip the master list file
			if (substrings[0] == "master") continue;

			// Extract the start and stop UTC times from the filename
			item.m_start = UTC(substrings[0]);
			item.m_stop = UTC(substrings[1]);

			// Extract the BKNumber names and values from the filename
			vector<string> names,values;
			unsigned istr = 2; //Skip the start and stop times from the filename
			for (unsigned i=0; i<m_master.size(); i++) {
				string name = substrings[istr++];
				// Take into account that names can include an underscore, requiring concatenation of substrings
				// since underscore is used as the substring separator
				unsigned count = boost::count(m_master[i].m_name, '_');
				for (unsigned j=0; j<count; j++) {
					name += "_";
					name += substrings[istr++];
				}
				names.push_back(name);
				values.push_back(substrings[istr++]);
			}
//			for (unsigned i=0; i<m_master.size(); i++) {
//				cout << m_master[i].m_name << " " << names[i] << " " << values[i] << endl;
//			}

			// Create a vector of BKNumbers corresponding to the filename
			vector<BKNumber> numbers;
			for (unsigned i=0; i<names.size(); i++) {
				// Check that the ordering of the BKNumber names names matches the master
				if (names[i] != m_master[i].m_name) {
					throw runtime_error("Error in Bookkeeping::Find: ordering of BKNumber names does not match master list for BKItem "
							+it->path().filename().string());
				}
				// Make a copy of the master BKNumber and assign the value
				BKNumber number = m_master[i];
				if (number.m_bkDataType == bk_uint8) {
					number.m_uint8Number = static_cast<uint8_t>(stoi(values[i]));
				} else if (number.m_bkDataType == bk_uint16) {
					number.m_uint16Number = static_cast<uint16_t>(stoi(values[i]));
        } else if (number.m_bkDataType == bk_UTC) {
          number.m_utc = UTC(values[i]);
				} else if (number.m_bkDataType == bk_uint32) {
					number.m_uint32Number = static_cast<uint32_t>(stoi(values[i]));
				} else {
					throw runtime_error("Error in Bookkeeping::Find: invalid BKDataType: "+number.m_bkDataType);
				}
				numbers.push_back(number);
			}
			item.m_bkNumbers = numbers;

			// Check if the set of BKNumber values for the current item matches those of the item being searched for
			bool match = true;
			for (unsigned i=0; i<bkItem.m_bkNumbers.size(); i++) {
				if (bkItem.m_bkNumbers[i].m_key) { // Only consider BKNumbers for which the key is true
					if (bkItem.m_bkNumbers[i].m_bkDataType == bk_uint8) {
						if (item.m_bkNumbers[i].m_uint8Number != bkItem.m_bkNumbers[i].m_uint8Number) match = false;
					} else if (bkItem.m_bkNumbers[i].m_bkDataType == bk_uint16) {
						if (item.m_bkNumbers[i].m_uint16Number != bkItem.m_bkNumbers[i].m_uint16Number) match = false;
          } else if (bkItem.m_bkNumbers[i].m_bkDataType == bk_UTC) {
            if (item.m_bkNumbers[i].m_utc != bkItem.m_bkNumbers[i].m_utc) match = false;
					} else if (bkItem.m_bkNumbers[i].m_bkDataType == bk_uint32) {
						if (item.m_bkNumbers[i].m_uint32Number != bkItem.m_bkNumbers[i].m_uint32Number) match = false;
					} else {
						throw runtime_error("Error in Bookkeeping::Find: invalid BKDataType: "+bkItem.m_bkNumbers[i].m_bkDataType);
					}
				}
			}
			if (match) bkItems.push_back(item);

		}
	}

	return bkItems;
}
