/** ****************************************************************************
 *  @file
 *  @ingroup FitsDataModel
 *
 *  @brief  Implementation of the functions of create new FITS files with
 *          correct filename.
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 6.2   2016-09-02 RRO #11463: Get data version from FitsDalHeader class
 *  @version 3.0   2015-01-14 RRO #7156: first version
 *
 */

#include "CreateFitsFile.hxx"

using namespace std;



///////////////////////////////////////////////////////////////////////////////
std::string buildFileName(const std::string & path, const UTC & utc,
                          const VisitId & visitId, const PassId & passId,
                          const std::string & dataName,
                          const std::string & structName) {

   string fileName(path);
   fileName += "/CH_";

   // the program and the visit as PRppnnnn_TGttttnn_
   if (visitId.isValid())
      fileName += visitId.getFileNamePattern() + "_";

   // the pass ID if it is defined
   if (passId.isValid())
      fileName += passId.getFileNamePattern() + "_";

   // the time
   fileName += utc.getFileNamePattern() + "_";

   // the name of the data structure
   fileName += structName + (dataName.empty() ? "_" : "-");

   // the dataName if defined, i.e. not empty
   if (!dataName.empty())
      fileName += dataName + "_";

   // the file version
   char hstr[10];
   sprintf(hstr, "V%04hu", FitsDalHeader::getDataVersion());
   fileName += hstr;

   fileName += ".fits";

   return fileName;

}

