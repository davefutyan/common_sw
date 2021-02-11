/** ****************************************************************************
 *  @file
 *  @ingroup utilities
 *  @brief Declaration of class ValidRefFile.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 10.0.0 2018-08-02 RRO #16836 Update of the documentation of the method
 *                                        ValidRefFile::getFile().
 *  @version 8.1.1 2017-10-30 RRO #14878 comparison of filename to open a next
 *                                       reference file compared filename with and
 *                                       without the extension in square brackets.
 *  @version 8.1 2017-10-22 RRO, change provided by Gabor
 *                              #13055 provide different pointer by getFile()
 *                                     if REF file has changed.
 *  @version 7.4 2017-06-01 ABE #14081 Order files by archive revision and
 *                                     processing number instead of by creation
 *                                     time.
 *  @version 4.0 2015-07-21 ABE        Check that ref files have the same extname.
 *  @version 3.2 2015-04-14 ABE        First version
 */

#ifndef VALIDREFFILE_HXX_
#define VALIDREFFILE_HXX_

#include <list>
#include <map>
#include <set>

#include <boost/range/adaptor/reversed.hpp>

#include <FitsDalHeader.hxx>
#include <Utc.hxx>


/** **************************************************************************
 *  @example ValidRefFile.cxx
 *  @ingroup utilities
 *  @author Anja Bekkelien UGE
 *
 *  @brief Usage of the ValidRefFile class
 *
 *  This code snippet shows how to use the ValidRefFile class to retrieve
 *  always the correct reference file for a specific UTC time.
 *
 *  Note: for the time range of one visit or one pass more than one reference
 *  file of the same type may be valid. The ValidRefFile class determines and
 *  returns the valid reference file for a specific UTC time.
 */


/**
 *  @brief This class models a validity period consisting of a validity start
 *         and stop time.
 *  @ingroup utilities
 *  @author Anja Bekkelien UGE
 *
 *  Objects of this class are intended to be used as elements of a container
 *  capable of ordering its elements, such as a set.
 *
 *  The class overrides the ==, != and < operators such that containers will
 *  order objects of this class by the archive revision number and the
 *  processing number. The archive revision number always takes precedence, so
 *  when comparing two objects, the one with the highest archive revision number
 *  will be considered greater even if its processing number is lower than that
 *  of the other object.
 *
 *  In other words, objects are ordered by the archive revision number (lowest
 *  first), and if the archive revision number is the same they are ordered by
 *  the processing number.
 *
 *  There should never be multiple files with identical archive revision
 *  numbers and processing numbers!
 *
 *  To find the applicable validity period for a given time it suffices to
 *  iterate backwards over the container and take the first element encountered
 *  that contains that time.
 */
class ValidityPeriod {

 private:

  UTC m_validityStart;    ///< The start of the validity period.
  UTC m_validityStop;     ///< The end of the validity period.
  int32_t m_archRev;      ///< The archive revision number
  int32_t m_procNum;      ///< The processing number
  std::string m_fileName; ///< The name of the reference file, including the extension name.
  std::string m_baseFileName; ///< The name of the reference file without the extension name

 public:

  /**
   *  @brief Constructs a validity period using UTC times.
   *
   *  @param validityStart The start of the validity period.
   *  @param validityStop  The end of the validity period.
   *  @param archRev       The archive revision number
   *  @param procNum       The processing number
   *  @param fileName      The name of the reference file, including the extension
   *  @param baseFileName  The base of the reference file without extension
   */
  ValidityPeriod(UTC validityStart, UTC validityStop, int32_t archRev, int32_t procNum,
                 std::string fileName, std::string baseFileName);

  /**
   *  @brief Constructs a validity period using UTC times defined in strings.
   *
   *  @param validityStart The start of the validity period.
   *  @param validityStop  The end of the validity period.
   *  @param archRev       The archive revision number
   *  @param procNum       The processing number
   *  @param fileName      The name of the reference file, including the extension
   *  @param baseFileName  The base of the reference file without extension
   */
  ValidityPeriod(std::string validityStart, std::string validityStop, int32_t archRev, int32_t procNum,
                 std::string fileName, std::string baseFileName);

  /**
   *  @brief Compares this validity periods to another by the archive revision
   *         and processing number, to determine whether or not this object has
   *         a lower version than the other object.
   *
   *  @param validityPeriod the validity period to compare with.
   *  @return true if the archive revision and processing number of this object
   *          are lower than those of the other object.
   */
  bool operator < (const ValidityPeriod & validityPeriod) const;

  /**
   *  @brief Compares this validity periods to another by the archive revision
   *         and processing number, to determine whether or not the two objects
   *         have the same version.
   *
   *  @param validityPeriod the validity period to compare with.
   *  @return true if the archive revision and processing number of this object
   *          are the same as those of the other object.
   */
  bool operator == (const ValidityPeriod & validityPeriod) const;

  /**
   *  @brief Compares this validity periods to another by the archive revision
   *         and processing number, to determine whether or not the two objects
   *         have different versions.
   *
   *  @param validityPeriod the validity period to compare with.
   *  @return true if the archive revision and processing number of this object
   *          are different from those of the other object.
   */
  bool operator != (const ValidityPeriod & validityPeriod) const;

  /**
   *  @brief Returns true if the utc time is contained within the start and stop
   *         time (inclusive) of this validity period.
   *  @param utc a UTC time.
   *  @return true if the utc time is contained within the start and stop time
   *          (inclusive) of this validity period.
   */
  bool contains(const UTC utc) const;

  /**
   *  @brief Returns the file name of the reference file that defines this
   *         validity period.
   *  @return file name.
   */
  std::string getFileName() const { return m_fileName; }

  /**
   *  @brief Returns the base file name of the reference file that defines this
   *         validity period.
   *  @return file name without extension name.
   */
  std::string getBaseFileName() const { return m_baseFileName; }


  /**
   *  Returns the start time of this validity period.
   *
   *  @return validity start as UTC.
   */
  UTC getValidityStart() const { return m_validityStart; }

  /**
   *  Returns the stop time of this validity period.
   *
   *  @return validity stop as UTC.
   */
  UTC getValidityStop() const { return m_validityStop; }

  /**
   *  @brief Returns the archive revision number of the reference file that
   *         defines this validity period.
   *
   *  @return the archive revision number.
   */
  int32_t getArchRev() const { return m_archRev; }

  /**
   *  @brief Returns the processing number of the reference file that
   *         defines this validity period.
   *
   *  @return the processing number.
   */
  int32_t getProcNum() const { return m_procNum; }

};


/** **************************************************************************
 *  @ingroup utilities
 *  @brief This class is used to get the reference file that is applicable for a
 *         given UTC time.
 *  @author  Anja Bekkelien UGE
 *
 *  The class takes a list of reference file names in the constructor. The
 *  class type specifies the data structure type of the files. The method
 *  @b getFile will return, among those files, the one that is applicable for a
 *  specific UTC time.
 *
 */
template <class HDU>
class ValidRefFile {

 private:

  /**
   *  @brief A set containing all available validity periods.
   *
   *  The validity periods are ordered by creation date.
   */
  std::set<ValidityPeriod> m_validityPeriods;

  /**
   *  @brief The currently open reference file.
   *
   *  When a reference file for a specific data time is requested, this class
   *  opens the file and stores a pointer to it in @b m_hdu.
   *
   *  The file is kept open as long as subsequent requests are within its
   *  validity period.
   *
   *  If a reference file for a data time that is not within its validity period
   *  is requested, the file pointed to by @b m_hdu is closed, the new file is
   *  opened and is assigned to @b m_hdu.
   */
  HDU * m_hdu = nullptr;

 public:

  /**
   *  @brief Constructor that takes a list of reference file names as input.
   *
   *  @param fileNames A list of names of reference files. All files must be of
   *                   the type specified by this template class' type.
   */
  ValidRefFile(const std::list<std::string> & fileNames) {

    std::string structName;
    FitsDalHeader * header = nullptr;
    std::pair<std::set<ValidityPeriod>::iterator, bool> returnValue;

    for (auto & fileName : fileNames) {
      // First iteration, do not check extname
      if (header == nullptr) {
        header = new FitsDalHeader(fileName);
      }
      // Subsequent iterations, check that the current reference file's extname
      // is the same as those of the previous reference files
      else {
        header = new FitsDalHeader(fileName);

        if (structName != header->GetAttr<std::string>("EXTNAME")) {
          throw std::runtime_error("Found reference files of different types: ["
                                   + structName + "], ["
                                   + header->GetAttr<std::string>("EXTNAME") + "]");
        }
      }

      structName = header->GetAttr<std::string>("EXTNAME");

      ValidityPeriod vp(header->GetAttr<std::string>("V_STRT_U"),
                        header->GetAttr<std::string>("V_STOP_U"),
                        header->GetAttr<int32_t>("ARCH_REV"),
                        header->GetAttr<int32_t>("PROC_NUM"),
                        fileName,
                        header->GetFileName());

      returnValue = m_validityPeriods.insert(vp);
      // The second evaluates to false if the insertion replaced an existing
      // item, meaning a validity time with identical archive revision number
      // and processing number had already been inserted. This is an error.
      if (!returnValue.second) {
        std::string errorMsg = "Found multiple " + structName + " files with ARCH_REV=";
        errorMsg.append(std::to_string(vp.getArchRev()));
        errorMsg.append(" and PROC_NUM=");
        errorMsg.append(std::to_string(vp.getProcNum()));
        errorMsg.append(" (" + returnValue.first->getFileName() + ", " + fileName + ")");
        throw std::runtime_error(errorMsg);
      }

      delete header;
    }

  }

  /**
   *  @brief Closes the reference file if it is open.
   */
  ~ValidRefFile() {

    if (m_hdu != nullptr) {
      delete m_hdu;
    }
  }

  /** ****************************************************************************
   *  @brief Returns the HDU of the reference file that is applicable for the UTC
   *         time given as method argument.
   *
   *  The returned pointer must not be deleted by the calling function. It will be
   *  deleted by this class.  The returned pointer is at least valid
   *  as long as this class is not deleted and until the next call of this
   *  getFile() method. The returned pointer is not valid any more, i. e.
   *  will be deleted by this class, if at the next
   *  call to this method a HDU pointer, pointing to an other data structure, is
   *  returned. In this case the pointer itself will be different than for the
   *  previous file. This can be used by the calling function to
   *  identify that the file has changed, if needed.
   *
   *  The file is not updated, for example to set the row number of the next ReadRow
   *  if the method is called and the same file pointer is returned as in the previous call.
   *
   *  @param dataTime The UTC time for which the returned reference file must be
   *                  valid.
   *  @return the HDU of the reference file that is valid at the specified UTC time.
   *
   *  @throws runtime_error if no reference file is valid for the given UTC @b dataTime.
   */
  HDU * getFile(const UTC & dataTime) {

    // Iterate over the reference files' validity periods, starting with the
    // most recently created file, to find the file containing the specific data time.
    for (auto validityPeriod : boost::adaptors::reverse(m_validityPeriods)) {

      if (!validityPeriod.contains(dataTime)) {
        continue;
      }
      // Found the right validity period

      // First time opening an HDU after this object was created
      if (m_hdu == nullptr) {
        m_hdu = new HDU(validityPeriod.getFileName());
      }
      // Need to switch to another HDU ?
      else if (m_hdu->GetFileName() != validityPeriod.getBaseFileName()) {
        HDU * old_hdu = m_hdu;
        m_hdu = new HDU(validityPeriod.getFileName());
        delete old_hdu;
      }

      return m_hdu;
    }

    // No reference file found
    throw std::runtime_error("No applicable reference file of class " + HDU::getClassName() + " was found for UTC time " + dataTime.getUtc());
  }

};


#endif /* VALIDREFFILE_HXX_ */
