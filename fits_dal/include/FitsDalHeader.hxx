/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief Declaration of the FitsDalHeader class
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
 *  @version 3.2   2015-04-23 RRO #8005: set destrutor to a virtual
 *  @version 1.3   2014-07-31 RRO #6101: Grouping of header keywords
 *  @version 1.2   2014-07-30 RRO #6099: the order of keywords in the FITS header
 *                                is defined by the order in the fsd file.
 *  @version 1.2   2014-07-29 RRO #6096: new function:
 *                                CompareDataStructureVersions()
 *  @version 1.1   2014-07-25 RRO #6076: new method:
 *                                FitsDalHeader::VerifyChecksum()
 *  @version 1.0   2013-12-14 RRO first released version
 *
 */

#ifndef _FITS_DAL_HEADER_HXX_
#define _FITS_DAL_HEADER_HXX_

#include <iostream>

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

#include "fitsio.h"
extern "C" {
#include <fitsio2.h>
}

/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Representing a FITS header with access functions to the attributes.
 *
 *  Most of the times this FitsDalHeader is used as the parent class of
 *  the FitsDalImage and FitsDalTable classes to access a FTIS extension.
 *  But it is also possible to open any FITS extension, using this class
 *  to read the header keyword. In this approach it is not possible to
 *  update the header keywords or to access the data of the HDU.
 */
class FitsDalHeader
{

protected:
   /** *************************************************************************
    *  @brief Default constructor used by the FitsDalImage and FitsDalTable
    *         classes
    */
	FitsDalHeader();

   /** *************************************************************************
    *  @brief Writes data to disk
    */
   virtual void Flush() {}


   /** *************************************************************************
    *  @brief Reads all header keywords and stores them in m_headerCards
    */
	void ReadAllKeywords();


   /** *************************************************************************
    *  @brief Writes all header keywords from m_headerCared to the FITS header
    *         on disk.
    */
	void WriteAllKeywords();


   /** *************************************************************************
    *  @brief Validates the DATASUM and the CHECKSUM keywords.
    */
	void VerifyChecksum();

   /** *************************************************************************
    *  @brief Compares the version of the data structure in the FITS file
    *         and the one of the software.
    */
   void CompareDataStructureVersion(const std::string & softwareVersion,
                                    const std::string & structName);

   fitsfile  * m_fitsFile;  ///< Handler of the fits extension
	bool        m_update;    ///< true if the extension was created or opened for update

private:
	/** *************************************************************************
    *  @brief Get the values of one keyword from m_headerCards
    */
	void GetValues(const std::string & name, char * valueStr, std::string * comment,
			          std::string * unit);

   /** *************************************************************************
    *  @brief Sets the values of one keyword in m_headerCards
    */
	void SetValues(const std::string & name, const std::string & valueStr,
			       const std::string & comment, const std::string & unit);


   /** *************************************************************************
	 *  @brief all header keyword as cfitsio cards
	 *
	 *  The order in this vector defines also the order in the FITS headder
	 */
	std::vector<std::string> m_headerCards;


   /** *************************************************************************
    *  @brief Defines the index of a keyword in m_headerCards
    *
    *  The key of the map is the name of a keyword card
    *  The value of the map is the index of that keyword in m_headerCards
    *  keywords without name, i.e. empty lines or lines with only a comment,
    *  are not stored in this map.
    */
	std::map<std::string, uint32_t > m_keyNameIndex;

	static std::string   ms_programName;      ///< Program name, creating the HDU
	static std::string   ms_programVersion;   ///< version number of program, creating the HDU
	static std::string   ms_svnRevision;      ///< SVN revision number of the module, creating the DHU
	static std::string   ms_procChain;        ///< name of the processing chain that creates this HDU
	static uint16_t      ms_revisionNumber;   ///< revision number of the archive (two digits)
	static uint16_t      ms_processingNumber; ///< processing number of the data processing (two digits)
	static std::string   ms_pipelineVersion;  ///< version of the pipeline

public:

   /** *************************************************************************
    *  @brief Constructor to open a FITS header for reading of the header
    *         keywords
    */
	FitsDalHeader(const std::string & filename);


   /** *************************************************************************
    *  @brief Writes all keywords in the FITS file, if in update mode and closes
    *         the file.
    */
	virtual ~FitsDalHeader();

	/** **************************************************************************
	 * @brief Returns the class name as a string.
	 *
	 * @return class name
	 */
	static std::string getClassName() { return "FitsDalHeader"; }

	/** *************************************************************************
    *  @brief These program specific values will be written in every header,
    *  that is create after calling this function.
    */
	static void SetProgram(const std::string & programName,
	                       const std::string & programVersion,
	                       const std::string & svnRevision,
	                       const std::string & procChain = std::string("undefined"),
	                       uint16_t            revisionNumber = 0,
	                       uint16_t            processingNumber = 0,
	                       const std::string & pipelineVersion = std::string("N/A"));


   /** *************************************************************************
    *  @brief Returns the data version of all files that are created
    *
    *  The DataVersion is 100 * "archive revision number" + "processing number"
    */
	static uint16_t getDataVersion()     {return ms_revisionNumber * 100 + ms_processingNumber;}

	/** *************************************************************************
    *  @brief Returns the file name or an empty string if this class is not
    *         yet associated with a FITS extension.
    */
	std::string GetFileName() const;


	/** *************************************************************************
    *  @brief Returns the value of an attribute in the requested data type
    */
	template <typename T>
		T GetAttr(const std::string & name, std::string * comment = nullptr,
	                                        std::string * unit = nullptr );

   /** *************************************************************************
    *  @brief Sets / updates the value, the comment and the unit of one
    *  attribute
    */
	template <typename T>
		void SetAttr(const std::string & name, T value,
				     const std::string & comment = std::string(),
	                 const std::string & unit = std::string()     );

   /** *************************************************************************
    *  @brief Insert one comment line in the FITS header
    */
	void InsertCommentLine(const std::string & comment);


   /** *************************************************************************
    *  @brief Writes the current status into a new FITS file
    *
    *  This method should be used only for debugging purpose.
    *  Only the FITS table or the FITS image can be copied into the new
    *  FITS file. The status of the header keywords is not defined.\n
    *  A FITS table in the saved FITS file may have more rows as already
    *  created in this source table. The values in the additional rows are random.\n
    *  A FITS image will be copied into the primary array of the new FITS file.
    *
    *  @param [in] filename of the file with the copied data. The file must not
    *              exist before calling this method.
    *
    *  @throw runtime_error if the new file cannot be created or when the
    *                       write operation fails.
    */
	void WriteCurrentStatus(const std::string & filename);
};


#endif /* _FITS_DAL_HEADER_HXX_ */
