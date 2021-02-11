/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief Implementations of the FitsDalHeader class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 9.3.1 2018-06-25 RRO #16553 add the const specifier to GetFileName()
 *  @version 9.2   2018-05-15 ABE #15807 Set PIPE_VER in data product headers
 *  @version 6.2   2016-09-02 RRO #11463: set version of data in every header
 *  @version 5.2   2016-06-05 RRO      new Method: WriteCurrentStatus()
 *  @version 4.4   2015-12-04 RRO      : GetFileName() returns the filename without
 *                                       extension.
 *  @version 4.2   2015-09-07 RRO #9181: set PROC_CHN keyword
 *  @version 3.0   2014-12-17 RRO #7054: support uint64_t as datatype for a
 *                                keyword value. It is needed to set TNULL for
 *                                a uint64_t column.
 *  @version 1.3   2014-07-31 RRO #6101: Grouping of header keywords
 *  @version 1.2   2014-07-30 RRO #6099: the order of keywords in the FITS header
 *                                is defined by the order in the fsd file.
 *  @version 1.2   2014-07-29 RRO #6096: new function:
 *                                CompareDataStructureVersion()
 *  @version 1.1   2014-07-24 RRO #6076: write keywords DATASUM and CHECKSUM when
 *                                a DHU is created / updated.
 *                                New method: FitsDalHeader::VerifyChecksum()
 *  @version 1.0   2013-12-14 RRO first released version
 */


#include <limits>
#include <string.h>

#include <iostream>

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/date_time.hpp"

#include "Logger.hxx"

#include "FitsDalHeader.hxx"

using namespace std;


std::string   FitsDalHeader::ms_programName;
std::string   FitsDalHeader::ms_programVersion;
std::string   FitsDalHeader::ms_svnRevision;
std::string   FitsDalHeader::ms_procChain;
uint16_t      FitsDalHeader::ms_revisionNumber = 0;
uint16_t      FitsDalHeader::ms_processingNumber = 0;
std::string   FitsDalHeader::ms_pipelineVersion;


FitsDalHeader::FitsDalHeader()
{
   m_fitsFile = nullptr;
   m_headerCards.reserve(100);

   boost::posix_time::ptime now( boost::posix_time::second_clock::universal_time());
   SetAttr("DATE", to_iso_extended_string(now) , "creation time of this HDU", "");
   SetAttr("STAMP",   ms_programName + " " + ms_programVersion, "program creating this HDU", "");
   SetAttr("SVN_REV", ms_svnRevision, "svn revision of the software creating this HDU", "");
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Opens a FITS data structure in read-only mode.
///
/// There is no access to the data of the HDU, i.e. to the image ore to the
/// table. Only the header keywords can be read.
///
/// @param [in] filename  filename with optional HDU definition.
///
/// @throw runtime_error if the extension cannot be opened.
FitsDalHeader::FitsDalHeader(const std::string & filename)
{
   m_update = false;
   m_headerCards.reserve(100);

   int status = 0;
   fits_open_data(&m_fitsFile, filename.c_str(), READONLY, &status);
   if (status != 0)
      throw runtime_error("Failed to open header of " + filename +
                          ", cfitsio error: " + to_string(status));

   VerifyChecksum();
   ReadAllKeywords();

}

/** *************************************************************************
*   If the extension was created or opened in update mode the keywords
*   DATASUM and CHECKSUM are created / updated. This is the very last step
*   before the extension is closed.
*/
FitsDalHeader::~FitsDalHeader()
{
	if (m_fitsFile)
	{
	   int status = 0;

		if (m_update)
		   {
		   // set the PROC_CHN keyword if it exist and is not yet set
		   try {
            if (GetAttr<string>("PROC_CHN").empty())
               SetAttr("PROC_CHN", ms_procChain);
		   }
		   catch (runtime_error &e) {
		      // we accept if the keyword does not exist and do nothing.
		   }

		   // Set the revision number, processing number and pipeline version.
		   /// The keywords should already exist from the fsd files.
		   SetAttr("ARCH_REV", ms_revisionNumber);
       SetAttr("PROC_NUM", ms_processingNumber);
       SetAttr("PIPE_VER", ms_pipelineVersion);

			WriteAllKeywords();
			fits_write_chksum(m_fitsFile, &status);
		   }

		status = 0;
	   fits_close_file(m_fitsFile, &status);

	}

}

///////////////////////////////////////////////////////////////////////////////
string FitsDalHeader::GetFileName() const {

   if (!m_fitsFile)
      return string();

   char rootName[FLEN_FILENAME];
   int status = 0;
   fits_parse_rootname(m_fitsFile->Fptr->filename, rootName, &status);

   return string(rootName);

}

///////////////////////////////////////////////////////////////////////////////
void FitsDalHeader::ReadAllKeywords()
{

	if (m_fitsFile == nullptr)
		return;

	int status = 0;
	char card[100];
	card[0] = 0;
	fits_read_record(m_fitsFile, 0, card, &status);
	while (status == 0) {
	   char inclist1[10];
	   char * inclist[1];

	   strcpy(inclist1, "*");
	   inclist[0] = inclist1;
	   fits_find_nextkey(m_fitsFile, inclist, 1, NULL, 0, card, &status);

	   if (status == 0) {
	      char name[80];
	      int  nameLength;
	      fits_get_keyname(card, name, &nameLength, &status);
	      if (nameLength > 0) {
	         m_headerCards.push_back(card);
	         m_keyNameIndex[name] = m_headerCards.size() -1;
	      }
	   }
	}

	if (status == KEY_NO_EXIST)
		// this is expected at end of list
	    status = 0;
	else
		throw runtime_error("Failed to read any keyword from : " +
				  GetFileName() + ", cfitsio error: " + to_string(status));

}

///////////////////////////////////////////////////////////////////////////////
void FitsDalHeader::WriteAllKeywords()
{

   int status = 0;

   vector<string>::iterator i_headerCard = m_headerCards.begin();
   while (i_headerCard != m_headerCards.end()) {

      char name[80];
      int  nameLength;
      char card[100];
      strcpy(card, i_headerCard->c_str());
      fits_get_keyname(card, name, &nameLength, &status);

      if (nameLength == 0)
         fits_write_record(m_fitsFile, i_headerCard->c_str(), &status);
      else
         fits_update_card(m_fitsFile, name, i_headerCard->c_str(), &status);
		if (status != 0)
			throw runtime_error("Failed to write keyword [" + *i_headerCard +
					            "] as [" + name + "] to file " +
								  GetFileName() + ", cfitsio error: " + to_string(status));
		++i_headerCard;
	}
}

/** *************************************************************************
*   This method should be called each time a HDU is opened. It verifies the
*   DATASUM and the CHECKSUM keywords.
*   If one of these keywords are not available a warning is issued via the
*   logger module. If one of these keywords are not correct a runtime_error
*   exception is thrown.
*
*   @throw runtime_error if one or both of the keywords DATASUM and CHECKSUM
*                        are not valid.
*/
void FitsDalHeader::VerifyChecksum()
{
   if (m_fitsFile == nullptr)
      return;

   int status = 0;
   int dataCheck = 1;
   int hduCheck = 1;

   fits_verify_chksum(m_fitsFile, &dataCheck, &hduCheck, &status);

   if (dataCheck == -1 || hduCheck == -1)
      {
      string keywords;
      if (dataCheck == -1 && hduCheck == -1)
         keywords = "s DATASUM and CHECKSUM.";
      else if (dataCheck == -1)
         keywords = " DATASUM.";
      else
         keywords = " CHECKSUM.";

      throw runtime_error("Checksum of HDU " + GetFileName() +
                           " is wrong. See keyword" + keywords);
      }

   if (dataCheck == 0 || hduCheck == 0)
      {
      string keywords;
      if (dataCheck == 0 && hduCheck == 0)
         keywords = "s DATASUM and CHECKSUM do";
      else if (dataCheck == -1)
         keywords = " DATASUM does";
      else
         keywords = " CHECKSUM does";

      logger << warn << "Keyword" << keywords << " not exist in " <<
                     GetFileName() << ". Cannot validate the checksum." <<
                     std::endl;
      }


}

/** *************************************************************************
 *  A warning message is raised if
 *  - the software version is not defined (empty string)
 *  - the version keyword (EXT_VER) is not present in the FITS header
 *  - the software version is different than the version int the FITS header
 */
void FitsDalHeader::CompareDataStructureVersion(const std::string & softwareVersion,
                                                const std::string & structName)
{
   // is software version defined?
   if (softwareVersion.length() == 0)
      {
      logger << warn << "The version of the data structure " << structName <<
                        " is not defined. This should never happen!!!" << std::endl;
      return;
      }

   // is the version defined in the HDU header?
   string fileVersion;
   try {
      fileVersion = GetAttr<std::string>("EXT_VER");
   }
   catch(exception) {
      logger << warn << "The version of the data structure (keyword EXT_VER) " <<
                        "is not defined in " << GetFileName() << std::endl;
      return;
   }

   // are both versions identical?
   if (softwareVersion != fileVersion) {
      logger << warn << "The data structure version of the structure [" <<
                         structName << "], that is used by the software [" <<
                        softwareVersion << "] and the version that used to create the FITS extension [" <<
                        fileVersion << "] in the file " << GetFileName() <<
                        ", are not identical." << std::endl;

   }
}


///////////////////////////////////////////////////////////////////////////////
void FitsDalHeader::GetValues(const string & name, char * valueStr,
		                      string * comment,  string * unit)
{
	map<string, uint32_t>::iterator i_keyNameIndex;

	i_keyNameIndex = m_keyNameIndex.find(name);
	if (i_keyNameIndex == m_keyNameIndex.end())
		throw runtime_error("Keyword " + name + " does not exist in header of " +
				GetFileName());

	int status = 0;
	char card[100];
	char charComment[100];
	strcpy(card, m_headerCards[i_keyNameIndex->second].c_str());
	fits_parse_value(card, valueStr, charComment, &status);

	if (status != 0)
		throw runtime_error("Failed to parse card of keyword " +
				            name + " in " + GetFileName() +
				            ". cfitsio error: " + to_string(status) );
	if (comment)
		*comment = charComment;

	if (unit)
		unit->clear();

	if (charComment[0] == '[')
	{
		char * end = strchr(charComment, ']');
		if (end)
		{
			*end = '\0';
			if (unit)
				*unit = charComment + 1;
			if (comment)
			   // skip a blank (' ') after ']', if it is a blank
				*comment = end + ((*(end+1)) == ' ' ? 2 : 1);
		}
	}


}

///////////////////////////////////////////////////////////////////////////////
void FitsDalHeader::SetValues(const string & name, const string & valueStr,
			                  const string & comment, const string & unit)
{

	int status = 0;

	string fitsComment;  // comment including the unit, as it is
		                 // written in the FITS header
	map<string, uint32_t>::iterator i_keyNameIndex;

	i_keyNameIndex = m_keyNameIndex.find(name);
	if (i_keyNameIndex == m_keyNameIndex.end())
	{
		// it is a new keyword. We add it to our list

		if (unit.length() > 0)
			fitsComment = "[" + unit + "] " + comment;
		else
			fitsComment = comment;

	   char charValueStr[100];
	   strcpy(charValueStr, valueStr.c_str());
	   char card[100];
	   fits_make_key(name.c_str(), charValueStr, fitsComment.c_str(),
	               card, &status);

	   m_headerCards.push_back(card);
      m_keyNameIndex[name] = m_headerCards.size() -1;
	}
	else
	{
		// it is an already known keyword. Just update the values
		char oldValueStr[100];
		string oldComment;
		string oldUnit;

		GetValues(name, oldValueStr, &oldComment, &oldUnit);

		if (unit.length() > 0)
			oldUnit = unit;
		if (comment.length() > 0)
			oldComment = comment;

		if (oldUnit.length() > 0)
			fitsComment = "[" + oldUnit + "] " + oldComment;
		else
			fitsComment = oldComment;

		char charValueStr[100];
		strcpy(charValueStr, valueStr.c_str());
		char card[100];
		fits_make_key(name.c_str(), charValueStr, fitsComment.c_str(),
			      card, &status);

		m_headerCards[i_keyNameIndex->second] = card;
	}
}

////////////////////////////////////////////////////////////////////////////////
void FitsDalHeader::SetProgram(const std::string & programName,
                               const std::string & programVersion,
                               const std::string & svnRevision,
                               const std::string & procChain,
                               uint16_t            revisionNumber,
                               uint16_t            processingNumber,
                               const std::string & pipelineVersion)
{
   ms_programName       = programName;
   ms_programVersion    = programVersion;
   ms_svnRevision       = svnRevision;
   ms_procChain         = procChain;
   ms_revisionNumber    = revisionNumber;
   ms_processingNumber  = processingNumber;
   ms_pipelineVersion   = pipelineVersion;
}


/** *************************************************************************
 *  @brief Converts @b valueStr to any integer data type
 *
 * This template function is used for all integer data types.
 * string, bool, double and float have their own implementation.
 *
 * @param [in] valueStr   the value as read from the FITS keyword
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <typename T>
	T StrtoDataType(const char * valueStr, const string & name, const string & fileName)
		{
		long long valueLong;
    	int status = 0;
    	ffc2j(valueStr, &valueLong, &status);
    	if (status != 0)
    		throw runtime_error("Failed to convert the value (" + string(valueStr) + ") of keyword "
    	       + name + " in " + fileName +
    	       " to data type long. cfitsio error: " + to_string(status) );

   		if (valueLong > numeric_limits<T>::max()  ||
        	valueLong < numeric_limits<T>::min()       )
    			throw runtime_error("overflow of the value " + to_string(valueLong) +
    	                         " of keyword " + name + " in " + fileName +
    	                         " while converting to requested data type"  );

    	return T (valueLong);
		}

/** *************************************************************************
 *  @brief Converts @b valueStr to a boolean data type
 *
 * @param [in] valueStr   the value as read from the FITS keyword
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <>
   uint64_t StrtoDataType<uint64_t>(const char * valueStr, const string & name, const string & fileName)
{
   try {
      return boost::lexical_cast<uint64_t>(valueStr);
   }
   catch (boost::bad_lexical_cast) {
   throw runtime_error("Failed to convert the value (" + string(valueStr) + ") of keyword "
        + name + " in " + fileName + " to data type uint64_t" );
   }
}


/** *************************************************************************
 *  @brief Converts @b valueStr to a boolean data type
 *
 * @param [in] valueStr   the value as read from the FITS keyword
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <>
	bool StrtoDataType<bool>(const char * valueStr, const string & name, const string & fileName)
		{
		int tmpVal;
    	int status = 0;
    	ffc2l(valueStr, &tmpVal, &status);
    	if (status != 0)
    		throw runtime_error("Failed to convert the value (" + string(valueStr) + ") of keyword "
    	       + name + " in " + fileName +
    	       " to data type bool. cfitsio error: " + to_string(status) );


    	return bool (tmpVal);
		}

/** *************************************************************************
 *  @brief Converts @b valueStr to a float data type
 *
 * @param [in] valueStr   the value as read from the FITS keyword
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <>
	float StrtoDataType<float>(const char * valueStr, const string & name, const string & fileName)
		{
		float value;
    	int status = 0;
    	ffc2r(valueStr, &value, &status);
    	if (status != 0)
    		throw runtime_error("Failed to convert the value (" + string(valueStr) + ") of keyword "
    	       + name + " in " + fileName +
    	       " to data type float. cfitsio error: " + to_string(status) );


    	return value;
		}

/** *************************************************************************
 *  @brief Converts @b valueStr to double data type
 *
 * @param [in] valueStr   the value as read from the FITS keyword
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <>
	double StrtoDataType<double>(const char * valueStr, const string & name, const string & fileName)
		{
		double value;
    	int status = 0;
    	ffc2d(valueStr, &value, &status);
    	if (status != 0)
    		throw runtime_error("Failed to convert the value (" + string(valueStr) + ") of keyword "
    	       + name + " in " + fileName +
    	       " to data type douoble. cfitsio error: " + to_string(status) );


    	return value;
		}

/** *************************************************************************
 *  @brief Converts @b valueStr to a std::stirng data type
 *
 * @param [in] valueStr   the value as read from the FITS keyword
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
 template<>
 	string StrtoDataType<string>(const char * valueStr, const string & name, const string & fileName)
		{
    	int status = 0;
    	char convStr[100];

    	ffc2s(valueStr, convStr, &status);
   	   	if (status != 0)
    		throw runtime_error("Failed to convert the value (" + string(valueStr) + ") of keyword "
    	       + name + " in " + fileName +
    	       " to data type char*. cfitsio error: " + to_string(status) );

    	return string (convStr);
		}


 /** *************************************************************************
  *  @brief Converts @b value to a string, ready to be written as the value
  *         in a FITS keyword
  *
  * This template function is used for all integer data types.
  * string, bool, double and float have their own implementation.
  *
  * @param [in] value      the value to be converted
  * @param [in] name       name of the keyword, used only in error messages
  * @param [in] fileName   filename + extension from where the keyword was read.
  *                        It is used only in error messages
  */
template <typename T>
string DataTypeToStr(T value, const string & name, const string & fileName)
{
	char strValue[40];
	int status = 0;

	ffi2c(value, strValue, &status);
   	if (status != 0)
    	throw runtime_error("Failed to convert the value (" + to_string(value) + ") for keyword "
    	       + name + " in " + fileName +
    	       " to a string. cfitsio error: " + to_string(status) );


	return string(strValue);
}

/** *************************************************************************
 *  @brief Converts @b value to a string, ready to be written as the value
 *         in a FITS keyword
 *
 * @param [in] value      the value to be converted
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <>
string DataTypeToStr(uint64_t value, const string & name, const string & fileName)
{
   try {
      return boost::lexical_cast<std::string>(value);
   }
   catch (boost::bad_lexical_cast) {
      throw runtime_error("Failed to convert the value (" + to_string(value) + ") for keyword "
             + name + " in " + fileName + " to a string." );

   }
}

/** *************************************************************************
 *  @brief Converts @b value to a string, ready to be written as the value
 *         in a FITS keyword
 *
 * @param [in] value      the value to be converted
 * @param [in] name       not used
 * @param [in] fileName   not used
 */
template <>
string DataTypeToStr(bool value, const string & name, const string & fileName)
{
	return value ?  string("T") : string("F");
}

/** *************************************************************************
 *  @brief Converts @b value to a string, ready to be written as the value
 *         in a FITS keyword
 *
 * @param [in] value      the value to be converted
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <>
string DataTypeToStr(float value, const string & name, const string & fileName)
{
	char strValue[40];
	int status = 0;

	ffr2e(value, -7, strValue, &status);
   	if (status != 0)
    	throw runtime_error("Failed to convert the value (" + to_string(value) + ") for keyword "
    	       + name + " in " + fileName +
    	       " to a string. cfitsio error: " + to_string(status) );


	return string(strValue);
}

/** *************************************************************************
 *  @brief Converts @b value to a string, ready to be written as the value
 *         in a FITS keyword
 *
 * @param [in] value      the value to be converted
 * @param [in] name       name of the keyword, used only in error messages
 * @param [in] fileName   filename + extension from where the keyword was read.
 *                        It is used only in error messages
 */
template <>
string DataTypeToStr(double value, const string & name, const string & fileName)
{
	char strValue[40];
	int status = 0;

	ffd2e(value, -15, strValue, &status);
   	if (status != 0)
    	throw runtime_error("Failed to convert the value (" + to_string(value) + ") for keyword "
    	       + name + " in " + fileName +
    	       " to a string. cfitsio error: " + to_string(status) );


	return string(strValue);
}

/** *************************************************************************
 *  @brief Converts @b value to a string, ready to be written as the value
 *         in a FITS keyword
 *
 * @param [in] value      the value to be converted
 * @param [in] name       not used
 * @param [in] fileName   not used
 */
template <>
string DataTypeToStr(string value, const string & name, const string & fileName)
{
	// output string will be at least 10 charachters.
	// singel quote charachers in the input string will be replaced
	// by two singel quotes charachters. e.g., o'brian ==> 'o''brian'
	char convValue[value.length() + 20];
	int status = 0;

	ffs2c(value.c_str(), convValue, &status);
	// this function never returns an error. Therefore we do not test the status.

	return string(convValue);

}

/** *************************************************************************
 *  @brief Converts @b value to a string, ready to be written as the value
 *         in a FITS keyword
 *
 * @param [in] value      the value to be converted
 * @param [in] name       not used
 * @param [in] fileName   not used
 */
template <>
string DataTypeToStr(const char * value, const string & name, const string & fileName)
{
   // output string will be at least 10 charachters.
   // singel quote charachers in the input string will be replaced
   // by two singel quotes charachters. e.g., o'brian ==> 'o''brian'
   char convValue[strlen(value) + 20];
   int status = 0;

   ffs2c(value, convValue, &status);
   // this function never returns an error. Therefore we do not test the status.

   return string(convValue);

}

/** *************************************************************************
  *  @param [in] name   name of the keyword for which the value, the comment
  *                     and the unit is requested
  *  @param [out] comment  the comment, without the unit, is copied into
  *                        this variable, if it is not a nullptr.
  *                        @b comment is set to an empty string if no comment is
  *                        available for the keyword.
  *  @param [out] unit     the unit is copied into
  *                        this variable, if it is not a nullptr. Note:
  *                        the unit in a FITS keyword is placed in square
  *                        brackets [] at the start of a comment string.
  *                        @b unit is set to an empty string if no unit is
  *                        available for the keyword.
  *  @return the value of the keyword in the requested data type. A conversion
  *          from the ASCII text in the FITS header is performed if possible.
  *          The value can be requested in following data formats:
  *  - int8_t
  *  - uint8_t
  *  - int16_t
  *  - uint16_t
  *  - int32_t
  *  - uint32_t
  *  - int64_t
  *  - float
  *  - double
  *  - bool
  *  - std::string
  *
  *  @throw runtime_error if the keyword @b name does not exist
  *  @throw runtime_error if the conversion to the requested data type cannot
  *                       be performed
  *  @throw runtime_error if the value in the FITS file is either too big or
  *                       to small for the requested data type.
  */
template <typename T>
	 T FitsDalHeader::GetAttr(const std::string & name,
	                                std::string * comment, std::string * unit )
{
   	char valueStr[80];
   	GetValues(name, valueStr, comment, unit);

    return StrtoDataType<T>(valueStr, name, GetFileName());
}



/** *************************************************************************
  *  If an attribute with with keyword = @b name exist already the value
  *  will be updated. If @b comment or @b unit are defined (not empty
  *  strings) they will be updated as well. A new attribute will be created
  *  if does not yet exist. Accepted data types for @b value are
  *  - int8_t
  *  - uint8_t
  *  - int16_t
  *  - uint16_t
  *  - int32_t
  *  - uint32_t
  *  - int64_t
  *  - uint64_t
  *  - float
  *  - double
  *  - bool
  *  - std::string
  *  - const char *
  */
template <typename T>
	void FitsDalHeader::SetAttr(const std::string & name, T value,
			                      const std::string & comment,
                               const std::string & unit)
{
	string valueStr = DataTypeToStr(value, name, GetFileName());
	SetValues(name, valueStr, comment, unit);
}

/** *************************************************************************
 *  A comment line is a line in the FITS header which has only a comment,
 *  no name and no value. It can even be an empty line.
 *
 *  @param [in] comment  The comment to be placed in one line of the
 *                       FITS header. It can be an empty string to place
 *                       and empty line in the FITS header.
 *                       If it is not empty a '/ ' will be placed in front
 *                       of the text to indicate it as a comment. The maximum
 *                       length is 68 characters. It will be truncated if it is
 *                       longer.
 */
void FitsDalHeader::InsertCommentLine(const std::string & comment) {

   if (comment.length() > 0) {
      string card("          / ");
      card += comment;
      if (card.length() > 80)
         card.resize(80);
      m_headerCards.push_back(card);
   }
   else
      m_headerCards.push_back(comment);
}

/** *************************************************************************
  */
void FitsDalHeader::WriteCurrentStatus(const std::string & filename) {

   if (!m_fitsFile)
      return;

   Flush();

   WriteAllKeywords();

   int status = 0;
   fitsfile  * newFitsFile;  ///< Handler of the new fits HDU

   fits_create_file(&newFitsFile, filename.c_str(), &status);
   if (status != 0)
      throw runtime_error("Failed to create " + filename +
                      ", cfitsio error: " + to_string(status));


   fits_copy_hdu(m_fitsFile, newFitsFile, 0, &status);
   if (status != 0)
      throw runtime_error("Failed to copy one extension from " + GetFileName() +
                          " to "+ filename + ", cfitsio error: " + to_string(status));

   fits_close_file(newFitsFile, &status);

}

// explicit instantiation
template void FitsDalHeader::SetAttr<int8_t>   (const string & name, int8_t value,   const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<uint8_t>  (const string & name, uint8_t value,  const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<int16_t>  (const string & name, int16_t value,  const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<uint16_t> (const string & name, uint16_t value, const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<int32_t>  (const string & name, int32_t value,  const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<uint32_t> (const string & name, uint32_t value, const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<int64_t>  (const string & name, int64_t value,  const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<uint64_t> (const string & name, uint64_t value, const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<bool>     (const string & name, bool value,     const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<float>    (const string & name, float value,    const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<double>   (const string & name, double value,   const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<string>   (const string & name, string value,   const string & comment, const string & unit);
template void FitsDalHeader::SetAttr<const char *>   (const string & name, const char * value,   const string & comment, const string & unit);

template int8_t   FitsDalHeader::GetAttr<int8_t>   (const string & name, string * comment, string * unit );
template uint8_t  FitsDalHeader::GetAttr<uint8_t>  (const string & name, string * comment, string * unit );
template int16_t  FitsDalHeader::GetAttr<int16_t>  (const string & name, string * comment, string * unit );
template uint16_t FitsDalHeader::GetAttr<uint16_t> (const string & name, string * comment, string * unit );
template int32_t  FitsDalHeader::GetAttr<int32_t>  (const string & name, string * comment, string * unit );
template uint32_t FitsDalHeader::GetAttr<uint32_t> (const string & name, string * comment, string * unit );
template int64_t  FitsDalHeader::GetAttr<int64_t>  (const string & name, string * comment, string * unit );
template uint64_t FitsDalHeader::GetAttr<uint64_t>  (const string & name, string * comment, string * unit );
template bool     FitsDalHeader::GetAttr<bool>     (const string & name, string * comment, string * unit );
template float    FitsDalHeader::GetAttr<float>    (const string & name, string * comment, string * unit );
template double   FitsDalHeader::GetAttr<double>   (const string & name, string * comment, string * unit );
template string   FitsDalHeader::GetAttr<string>   (const string & name, string * comment, string * unit );


