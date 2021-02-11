/** ****************************************************************************
 *  @file
 *  @ingroup FitsDataModel
 *
 *  @brief  Declaration of the functions of create new FITS files with correct
 *          filename
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 6.3.1 2016-11-07 RRO #11946: set keyword ARCH_REV and PROC_NUM
 *  @version 3.0   2015-01-14 RRO #7156: first version
 *
 */


#ifndef CREATE_FITS_FILE_HXX_
#define CREATE_FITS_FILE_HXX_

#include <initializer_list>
#include <string>
#include <vector>
#include <typeinfo>

#include <VisitId.hxx>
#include <PassId.hxx>
#include <Obt.hxx>
#include <Utc.hxx>

#include <FitsDalHeader.hxx>


/** *************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Creates a CHEOPS compatible file-name, depending on the input
 *         parameters
 *
 *  The format is:
 *  path/CH_visitId_passId_utc_structName-dataName_V0000.fits
 *  visitId, passId and dataName are optional and will be part of the filename
 *  if their data structure is valid (visitId, passId) or not empty (dataName).
 */
std::string buildFileName(const std::string & path, const UTC & utc,
                          const VisitId & visitId, const PassId & passId,
                          const std::string & dataName,
                          const std::string & structName);



 /** **************************************************************************
  *  @example CreateFitsFile.cxx
  *  @ingroup FitsDataModel
  *  @author Reiner Rohlfs UGE
  *
  *  @brief Create a FITS files
  *
  *  This code snippet shows how to use the function CreateFitsFile() to
  *  create CHEOPS compatible FITS files with the correct file-names.
  *
  *  A correct file-name looks like:\n
  *  CH_PRppnnnn_TGttttnn_PSyymmhhdd_TUyyyy-mm-ddThh-mm-ss_DataStructureDefinition-DataName_Vnnn.fits
  *
  *  A CHEOPS compatible FITS file is used to determine the VisitId, and
  *  the UTC time.
  */


  /** *************************************************************************
   *  @ingroup FitsDataModel
   *  @author Reiner Rohlfs UGE
   *
   *  @brief  This function is intended to be used to create FITS files with an
   *          image as first extension.
   *
   *  The filename is build up, following the definition in the Data Product
   *  Definition Document:\n
   *  path/CH_PRppnnnn_TGttttnn_PSyymmhhdd_TUyyyy-mm-ddThh-mm-ss_DataStructureDefinition-DataName_Vnnn.fits
   *  - PRppnnnn_TGttttnn (optional) is derived from the @b visitId.
   *  - PSyymmhhdd (optional) is derived from the @b passId
   *  - TUyyyy-mm-ddThh-mm-ss is the UTC of the first data in the file and is
   *    derived from the parameter @b utc.
   *  - DataStructureDefinition is the name of the data structure, created
   *    by this function. If is derived by the static member function
   *    HDU::getExtName()
   *  - DataName (optional) is the name of the data to distinguish data for the same time
   *    period in the same DataStructureDefinition.
   *  - Vnnn is the version of the file. It is the next higher version of the
   *    same file that is already in the CHEOPS archive. It will be set to V000
   *    if there is not yet a file of the filename in the archive. The version
   *    will be set to V000 if the program creating this file is
   *    running outside the CHEOPS system, i.e. there is no CHEOSP archive
   *    available, for example as unit_tests.
   *
   *    The header keywords in the created HDU, defining the visit id
   *    are set by this function.
   *
   *    See also example CreateFitsFile.cxx
   *
   *  @param [in] path     directory path where the FITS file will be
   *                       created. It should be the path returned by the
   *                       ProgramParams::GetOutputDir() method.
   *  @param [in] utc      The UTC of the first data in the new FITS
   *                       file.
   *  @param [in] visitId  The visitID as it was assigned to the data by the
   *                       Mission Planning System.
   *                       The program type (pp), the program number
   *                       (nnnn), the target number (tttt) and the
   *                       visit counter (nn) of the new filename are
   *                       derived from this visitId. The visitId will not be
   *                       part of the filename if @b visitId is pass as
   *                       default constructor ( VisitId() ) to this
   *                       function.
   *  @param [in] imgSize  defines the size of the image. imgSize[0] == size of
   *                       x-axis, imgSize[1] == size of y-axis, ... The size of a specific
   *                       dimension will be ignored, if the size of that dimension
   *                       is already defined in the fsd-file of the data structure.
   *  @param [in] passId   (optional) The passId the data belong to. Shall not be set for
   *                       data to be processed by Data Reduction. The default
   *                       ( PassId() ) can be used in this case.
   *  @param [in] dataName (optional) An additional identifier of the data. Shall be used
   *                       if several data for the same time period, i.e. the same
   *                       VisitId and / or the same PassId shall be created in identical
   *                       data structures. The parameter is ignored if the string
   *                       is empty (the default).
   *
   *  @tparam HDU         defines the data structure type, that is create
   *                      by this function. It has to be one of the data
   *                      classes of the FITS Data Model.
   *
   *  @return  An object of the requested data type, see @b Template
   *           @b Parameters.\n
   *           <b> The returned objects have to be deleted by
   *           the application program.</b>
   */
   template <class HDU> inline
   HDU * createFitsFile(const std::string & path, const UTC & utc,
                        const VisitId & visitId, std::vector<long> imgSize,
                        const PassId & passId = PassId(),
                        const std::string & dataName = std::string() )  {
      HDU * hdu = new HDU(buildFileName(path, utc, visitId, passId, dataName, HDU::getExtName() ),
                           "CREATE", imgSize);
      hdu->setKeyVisitId(visitId);
      hdu->setKeyArchRev(FitsDalHeader::getDataVersion() / 100);
      hdu->setKeyProcNum(FitsDalHeader::getDataVersion() % 100);
      return hdu;
      }

   /** *************************************************************************
    *  @ingroup FitsDataModel
    *  @author Reiner Rohlfs UGE
    *
    *  @brief  This function is intended to be used to create FITS files with a
    *          table as first extension or with an image for which the size
    *          is already defined in the fsd-file.
    *
    *  The filename is build up, following the definition in the Data Product
    *  Definition Document:\n
    *  path/CH_PRppnnnn_TGttttnn_PSyymmhhdd_TUyyyy-mm-ddThh-mm-ss_DataStructureDefinition-DataName_Vnnn.fits
    *  - PRppnnnn_TGttttnn (optional) is derived from the @b visitId.
    *  - PSyymmhhdd (optional) is derived from the @b passId
    *  - TUyyyy-mm-ddThh-mm-ss is the UTC of the first data in the file and is
    *    derived from the parameter @b utc.
    *  - DataStructureDefinition is the name of the data structure, created
    *    by this function. If is derived by the static member function
    *    HDU::getExtName()
    *  - DataName (optional) is the name of the data to distinguish data for the same time
    *    period in the same DataStructureDefinition.
    *  - Vnnn is the version of the file. It is the next higher version of the
    *    same file that is already in the CHEOPS archive. It will be set to V000
    *    if there is not yet a file of the filename in the archive. The version
    *    will be set to V000 if the program creating this file is
    *    running outside the CHEOPS system, i.e. there is no CHEOSP archive
    *    available, for example as unit_tests.
    *
    *    The header keywords in the created HDU, defining the visit id
    *    are set by this function.
    *
    *    See also example CreateFitsFile.cxx
    *
    *  @param [in] path     directory path where the FITS file will be
    *                       created. It should be the path returned by the
    *                       ProgramParams::GetOutputDir() method.
    *  @param [in] utc      The UTC of the first data in the new FITS
    *                       file.
    *  @param [in] visitId  The visitID as it was assigned to the data by the
    *                       Mission Planning System.
    *                       The program type (pp), the program number
    *                       (nnnn), the target number (tttt) and the
    *                       visit counter (nn) of the new filename are
    *                       derived from this visitId. The visitId will not be
    *                       part of the filename if @b visitId is pass as
    *                       default constructor ( VisitId() ) to this
    *                       function.
    *  @param [in] passId   (optional) The passId the data belong to. Shall not be set for
    *                       data to be processed by Data Reduction. The default
    *                       ( PassId() ) can be used in this case.
    *  @param [in] dataName (optional) An additional identifier of the data. Shall be used
    *                       if several data for the same time period, i.e. the same
    *                       VisitId and / or the same PassId shall be created in identical
    *                       data structures. The parameter is ignored if the string
    *                       is empty (the default).
    *
    *  @tparam HDU         defines the data structure type, that is create
    *                      by this function. It has to be one of the data
    *                      classes of the FITS Data Model.
    *
    *  @return  An object of the requested data type, see @b Template
    *           @b Parameters.\n
    *           <b> The returned objects have to be deleted by
    *           the application program.</b>
    */
   template <class HDU> inline
   HDU * createFitsFile(const std::string & path, const UTC & utc,
                        const VisitId & visitId,
                        const PassId & passId = PassId(),
                        const std::string & dataName = std::string() )  {
      HDU * hdu = new HDU(buildFileName(path, utc, visitId, passId, dataName, HDU::getExtName() ),
                           "CREATE");
      hdu->setKeyVisitId(visitId);
      hdu->setKeyArchRev(FitsDalHeader::getDataVersion() / 100);
      hdu->setKeyProcNum(FitsDalHeader::getDataVersion() % 100);
      return hdu;
      }



#endif /* CREATE_FITS_FILE_HXX_ */
