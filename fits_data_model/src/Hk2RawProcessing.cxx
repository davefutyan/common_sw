/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDataModel
 *  @brief Implementation of the Hk2RawProcessing class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 13.0  2020-08-28 MBE      #22243 CHEOPS and Gaia magnitudes in the output data
 *  @version 12.0  2019-10-23 RRO      #19772 Implement the decoding of TC(196,1)
 *  @version 11.4.3 2019-09-03 RRO     #19468 new HK table : SCI_RAW_HkAsy30767
 *  @version 9.1.4 2018-04=11 RRO      change class name from ColTimeData<OBT> to ColObtData
 *  @version 9.0 2017-11-24 RRO        new Hk data structure: HkAsy30759
 *  @version 7.3 2017-05-18 RRO        #12838 New header keywords: MAG_V, MAG_VERR, MAG_CHPS, MAG_CERR
 *  @version 6.3.1 2016-11-07 RRO      #11946 New header keyword: OBS_CAT, PRP_VST1 and PRP_VSTN
 *  @version 6.3 2016-10-21 RRO        New header keyword: PI_UID
 *  @version 6.3 2016-09-23 RRO        enum values are not continuous values any more
 *  @version 5.2 2016-06-03 RRO        Header keywords CHEOPSID, TG_TYPE and OBS_MODE
 *                                     does not exist any more.
 *                                     New header keyword: SPECTYPE
 *  @version 4.1 2015-07-31 RRO        first released version
 */

#include "CreateFitsFile.hxx"
#include "SCI_RAW_HkDefault.hxx"
#include "SCI_RAW_HkExtended.hxx"
#include "SCI_RAW_HkIfsw.hxx"
#include "SCI_RAW_HkIaswDg.hxx"
#include "SCI_RAW_HkIaswPar.hxx"
#include "SCI_RAW_HkIbswDg.hxx"
#include "SCI_RAW_HkIbswPar.hxx"
#include "SCI_RAW_HkAsy30759.hxx"
#include "SCI_RAW_HkAsy30767.hxx"
#include "SCI_RAW_HkCentroid.hxx"
#include "SCI_RAW_HkOperationParameter.hxx"
#include "REF_APP_HkEnumConversion.hxx"

#include "Hk2RawProcessing.hxx"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
HkRawFitsTable::HkRawFitsTable(FitsDalTable * fitsDalTable,
                               const std::string hkStructName,
                               std::map<std::string, ColMetaData*> & colData) :
                 m_fitsDalTable(fitsDalTable) , m_hkStructName(hkStructName) {

   // re-assign variables to the columns.
   // The variables are located in m_colData

   map<string, ColMetaData*>::iterator i_colData    = colData.begin();
   map<string, ColMetaData*>::iterator i_colDataEnd = colData.end();

   while (i_colData != i_colDataEnd) {
      i_colData->second->reAssign(m_fitsDalTable);

      i_colData++;
   }

}

////////////////////////////////////////////////////////////////////////////////
void HkRawFitsTable::writeRow(const UTC & utc,
                              std::map<std::string, ColMetaData*> & colData) {

   if (m_firstUtc == UTC()) {
      m_firstUtc = utc;
   }
   else if (utc < m_currentUtc || utc == m_currentUtc)
      throw runtime_error("UTC of next row (" + utc.getUtc() +
                          ") is less or equal than previous row (" +
                          m_currentUtc.getUtc() + ") of table " +
                          m_hkStructName);
   m_currentUtc = utc;

   // set random values if value was not set by uesr
   map<string, ColMetaData*>::iterator i_colData    = colData.begin();
   map<string, ColMetaData*>::iterator i_colDataEnd = colData.end();

   while (i_colData != i_colDataEnd) {
      i_colData->second->setRandomDiff();
      i_colData++;
   }

   // set the time columns
   i_colData = colData.find("UTC_TIME");
   dynamic_cast<ColTimeData<UTC> * >(i_colData->second)->SetTime(utc);

   i_colData = colData.find("OBT_TIME");
   dynamic_cast<ColOBTData * >(i_colData->second)->SetTime(utc.getObt());

   i_colData = colData.find("MJD_TIME");
   dynamic_cast<ColTimeData<MJD> * >(i_colData->second)->SetTime(utc.getMjd());


   m_fitsDalTable->WriteRow();

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Hk2RawProcessing::Hk2RawProcessing(const std::string & hkConversionFilename,
                                   const std::string & outDir,
                                   const VisitId & visitId) :
         HkProcessing(false, true), m_outDir(outDir), m_visitId(visitId)  {

   RefAppHkenumconversion hkConversionTable(hkConversionFilename);

   // map<ColName,vector<enum_text>>
   map<string, vector<string>> hkConversion;
   while (hkConversionTable.ReadRow()) {
      hkConversion[hkConversionTable.getCellCalibName()].push_back(hkConversionTable.getCellText());
   }

   // set the conversion in the columns. The column knows which conversion
   // it has to take
   map<string, map<string, ColMetaData *>>::iterator i_colData = m_colData.begin();
   while (i_colData != m_colData.end()) {

      map<string, ColMetaData *>::iterator ii_colData = i_colData->second.begin();
      while (ii_colData != i_colData->second.end()) {

         // The columns copy its conversion data from this hkConversion.
         // Therefore we do not need to keep this hkConversino alive.
         ii_colData->second->setHkConversion(hkConversion);

         ii_colData++;
      }
      i_colData++;
   }
}

////////////////////////////////////////////////////////////////////////////////
Hk2RawProcessing::~Hk2RawProcessing() {

   map<string, HkRawFitsTable *>::iterator  i_hkRawFitsTables;
   i_hkRawFitsTables = m_hkRawFitsTables.begin();
   while (i_hkRawFitsTables != m_hkRawFitsTables.end()) {

      // set the header attributes provided by the application
      i_hkRawFitsTables->second->setKeyProcChn       (m_keyProcChn);
      i_hkRawFitsTables->second->setKeyTargname      (m_keyTargname);
      i_hkRawFitsTables->second->setKeySpectype      (m_keySpectype);
      i_hkRawFitsTables->second->setKeyMagG          (m_keyMagG);
      i_hkRawFitsTables->second->setKeyMagGerr       (m_keyMagGerr);
      i_hkRawFitsTables->second->setKeyMagChps       (m_keyMagChps);
      i_hkRawFitsTables->second->setKeyMagCerr       (m_keyMagCerr);
      i_hkRawFitsTables->second->setKeyPiName        (m_keyPiName);
      i_hkRawFitsTables->second->setKeyPiUid         (m_keyPiUid);
      i_hkRawFitsTables->second->setKeyObsid         (m_keyObsid);
      i_hkRawFitsTables->second->setKeyRaTarg        (m_keyRaTarg);
      i_hkRawFitsTables->second->setKeyDecTarg       (m_keyDecTarg);
      i_hkRawFitsTables->second->setKeyObsCat        (m_keyObsCat);
      i_hkRawFitsTables->second->setKeyPrpVst1       (m_keyPrpVst1);
      i_hkRawFitsTables->second->setKeyPrpVstn       (m_keyPrpVstn);


      delete i_hkRawFitsTables->second;
      ++i_hkRawFitsTables;
   }
}

////////////////////////////////////////////////////////////////////////////////
void Hk2RawProcessing::writeRow(const UTC & utc, const std::string & structName) {

   map<std::string, HkRawFitsTable *>::iterator i_hkRawFitsTable;

   i_hkRawFitsTable = m_hkRawFitsTables.find(structName);
   if (i_hkRawFitsTable == m_hkRawFitsTables.end()) {
      // this is the first time
      // do we know this structName?
      std::map<std::string, std::map<std::string, ColMetaData*>>::iterator i_mapColData;
      i_mapColData = m_colData.find(structName);
      if (i_mapColData == m_colData.end()) {
         throw runtime_error("In Hk2RawProcessing::writeRow: Unknown data structure Name: " +
                             structName);
      }

      HkRawFitsTable * hkRawFitsTable = nullptr;

      if (structName == "SCI_RAW_HkDefault") {
          SciRawHkdefault * sciRawHkdefault = createFitsFile<SciRawHkdefault> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkdefault>(sciRawHkdefault, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkExtended") {
          SciRawHkextended * sciRawHkextended = createFitsFile<SciRawHkextended> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkextended>(sciRawHkextended, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkIfsw") {
          SciRawHkifsw * sciRawHkifsw = createFitsFile<SciRawHkifsw> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkifsw>(sciRawHkifsw, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkIbswPar") {
          SciRawHkibswpar * sciRawHkibswpar = createFitsFile<SciRawHkibswpar> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkibswpar>(sciRawHkibswpar, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkIbswDg") {
          SciRawHkibswdg * sciRawHkibswdg = createFitsFile<SciRawHkibswdg> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkibswdg>(sciRawHkibswdg, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkIaswPar") {
          SciRawHkiaswpar * sciRawHkiaswpar = createFitsFile<SciRawHkiaswpar> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkiaswpar>(sciRawHkiaswpar, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkIaswDg") {
          SciRawHkiaswdg * sciRawHkiaswdg = createFitsFile<SciRawHkiaswdg> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkiaswdg>(sciRawHkiaswdg, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkAsy30759") {
          SciRawHkasy30759 * sciRawHkasy30759 = createFitsFile<SciRawHkasy30759> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkasy30759>(sciRawHkasy30759, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkAsy30767") {
          SciRawHkasy30767 * sciRawHkasy30767 = createFitsFile<SciRawHkasy30767> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkasy30767>(sciRawHkasy30767, i_mapColData->second);
       }

      else if (structName == "SCI_RAW_HkCentroid") {
         SciRawHkcentroid * sciRawHkCentroid = createFitsFile<SciRawHkcentroid> (
                m_outDir, utc, m_visitId);
          hkRawFitsTable = new HkRawFitsTableChild<SciRawHkcentroid>(sciRawHkCentroid, i_mapColData->second);
       }

     if (hkRawFitsTable == nullptr) {
         // this only can happen if we have a new HK data sturcture and have
         // not updated the "else if" structure above.
         return;
      }

      i_hkRawFitsTable =  m_hkRawFitsTables.insert(
            pair<string, HkRawFitsTable *>(structName, hkRawFitsTable) ).first;

   }

   // either this is not the first time, or the new table was just created
   // now we can add a row
   i_hkRawFitsTable->second->writeRow(utc, m_colData[structName]);


}




