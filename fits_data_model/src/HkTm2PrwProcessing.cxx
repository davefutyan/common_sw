/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDataModel
 *  @brief Implementation of the Hk processing from TM to PRW
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 12.1.4 2020-04-22 ABE #21297 Provide length of TM packet to
 *                                        HkPrwFitsTable::addTmPacket()
 *  @version 12.0.2 2020-02-21 RRO #20941 Write warning if a Extended or Default
 *                                        HK packet is discarded.
 *  @version 12.0   2019-10-23 RRO #19772 Implement the decoding of TC(196,1)
 *  @version 11.4.3 2019-09-03 RRO #19468 new HK table : SCI_RAW_HkAsy30767
 *  @version 11.4.3 2019-08-12 RRO #19272 Throw an obt_error if an OBT value
 *                                        in the data field of the TM packet is
 *                                        less than 2000000000000
 *  @version 11.0  2019-01-07 RRO #17849 Throw an obt_error if an OBT values is
 *                                       decreasing
 *  @version 9.3   2018-06-06 RRO        remove not used HK table: SCI_PRW_HkAsy30720
 *  @version 9.1.4 2018-04-11 RRO #15832 m_obtTmOffset shall not be used if its
 *                                       value < 0.
 *  @version 9.0   2017-11-24 RRO        new Hk data structure: HkAsy30759
 *  @version 6.4   2016-12-14 RRO #12249 new Methods of class HkPrwFitsTable:
 *                                       setKeyObsCat(), setKeyPrpVst1() and setKeyPrpVstn().
 *  @version 6.3   2016-10-27 RRO #11748 getHkPrwFitsTable() can return a HkAsy30720 Table
 *  @version 6.3   2016-10-21 RRO        remove deprecated method: setKeyMpsVisit()
 *  @version 6.2   2016-08-23 RRO #11468 new Method: HkPrwFitsTable::setKeyObsid()
 *  @version 4.3.1 2015-11-02 RRO #9608  endian was wrong. Data have to be swapped.
 *  @version 4.1   2015-07-22 RRO        first released version
 */

#include <string>

#include "Logger.hxx"
#include "Obt.hxx"

#include "CreateFitsFile.hxx"
#include "SCI_PRW_HkDefault.hxx"
#include "SCI_PRW_HkExtended.hxx"
#include "SCI_PRW_HkIfsw.hxx"
#include "SCI_PRW_HkIaswDg.hxx"
#include "SCI_PRW_HkIaswPar.hxx"
#include "SCI_PRW_HkIbswDg.hxx"
#include "SCI_PRW_HkIbswPar.hxx"
#include "SCI_PRW_HkAsy30759.hxx"
#include "SCI_PRW_HkAsy30767.hxx"
#include "SCI_PRW_HkCentroid.hxx"
#include "SCI_PRW_HkOperationParameter.hxx"


#include "HkTm2PrwProcessing.hxx"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

HkPrwFitsTable::HkPrwFitsTable(FitsDalTable * fitsDalTable,
                               const std::string hkStructName,
                               std::map<std::string, ColMetaData *> & colData,
                               int16_t obtTmOffset,
                               HkPrwFitsTable * hkPrwFitsTableChain)
        : m_fitsDalTable(fitsDalTable), m_hkStructName(hkStructName),
          m_colData(colData),  m_obtTmOffset(obtTmOffset),
          m_hkPrwFitsTableChain(hkPrwFitsTableChain)   {

   // re-assign variables to the columns.
   // The variables are located in m_colData

   map<string, ColMetaData*>::iterator i_colData    = m_colData.begin();
   map<string, ColMetaData*>::iterator i_colDataEnd = m_colData.end();

   while (i_colData != i_colDataEnd) {
      i_colData->second->reAssign(m_fitsDalTable);
      i_colData++;
   }

   // re-assign the m_currentObt to the OBT column (OBT_TIME)
   m_fitsDalTable->ReAssign("OBT_TIME", &m_currentObt);

}


////////////////////////////////////////////////////////////////////////////////
HkPrwFitsTable::~HkPrwFitsTable() {
   delete m_fitsDalTable;
   delete m_hkPrwFitsTableChain;
}

////////////////////////////////////////////////////////////////////////////////
void HkPrwFitsTable::setKeyPiName(const std::string & keyPiName)  {

   m_keyPiName = keyPiName;

   // pass the PI Name to the next table in the chain of FITS Tables
   if (m_hkPrwFitsTableChain != nullptr)  {
      m_hkPrwFitsTableChain->setKeyPiName(keyPiName);
   }

}

////////////////////////////////////////////////////////////////////////////////
void HkPrwFitsTable::setKeyPiUid(uint32_t keyPiUid)  {

   m_keyPiUid = keyPiUid;

   // pass the OBSID to the next table in the chain of FITS Tables
   if (m_hkPrwFitsTableChain != nullptr) {
      m_hkPrwFitsTableChain->setKeyPiUid(keyPiUid);
   }
}

////////////////////////////////////////////////////////////////////////////////
void HkPrwFitsTable::setKeyObsid(uint32_t keyObsid)  {

   m_keyObsid = keyObsid;

   // pass the OBSID to the next table in the chain of FITS Tables
   if (m_hkPrwFitsTableChain != nullptr) {
      m_hkPrwFitsTableChain->setKeyObsid(keyObsid);
   }
}

////////////////////////////////////////////////////////////////////////////////
void HkPrwFitsTable::setKeyObsCat(std::string keyObsCat) {

   m_keyObsCat = keyObsCat;

   // pass the ObsCat to the next table in the chain of FITS Tables
   if (m_hkPrwFitsTableChain != nullptr) {
      m_hkPrwFitsTableChain->setKeyObsCat(keyObsCat);
   }
}

////////////////////////////////////////////////////////////////////////////////
void HkPrwFitsTable::setKeyPrpVst1(uint32_t keyPrpVst1) {

   m_keyPrpVst1 = keyPrpVst1;

   // pass the PrpVst1 to the next table in the chain of FITS Tables
   if (m_hkPrwFitsTableChain != nullptr) {
      m_hkPrwFitsTableChain->setKeyPrpVst1(keyPrpVst1);
   }
}

////////////////////////////////////////////////////////////////////////////////
void HkPrwFitsTable::setKeyPrpVstn(uint32_t keyPrpVstn) {

   m_keyPrpVstn = keyPrpVstn;

   // pass the PrpVstn to the next table in the chain of FITS Tables
   if (m_hkPrwFitsTableChain != nullptr) {
      m_hkPrwFitsTableChain->setKeyPrpVstn(keyPrpVstn);
   }
}

////////////////////////////////////////////////////////////////////////////////
void HkPrwFitsTable::addTmPacket(const OBT & obt, const int8_t * tmPacketData, uint16_t maxLength)  {


   map<string, ColMetaData*>::iterator i_colData = m_colData.begin();
   map<string, ColMetaData*>::iterator i_colDataEnd = m_colData.end();

   while (i_colData != i_colDataEnd) {

      i_colData->second->setFromTm(tmPacketData, maxLength);

      i_colData++;
   }

   OBT newObt;
   // copy the OBT time, either from the input parameter OBT or from
   // a specific location in the TM data
   if (m_obtTmOffset < 0) {
      newObt = obt;
   }
   else {
      int64_t localObt = 0;
      memcpy (((int8_t*)&localObt) + 2, tmPacketData + m_obtTmOffset, 6);
      localObt = swap_endian(localObt);
      newObt = OBT(localObt, localObt & 0x01);
   }

   // test that the OBT is not too small #19272
   bool writeNextRow = true;
   if (m_obtTmOffset >= 0 && newObt.getObt() < 2000000000000)
      throw obt_error("The OBT of HK Table " + m_hkStructName +
                      " is " + to_string(newObt.getObt()) +
                      ", which is less than  2000000000000");

   else if (m_firstTmPacket) {
      // it is the first packet for this table
      m_firstTmPacket = false;
      m_firstUtc = newObt.getUtc();
   }
   else if (newObt == m_currentObt && m_obtTmOffset >= 0) {
      // this could happen if the data from the SEM are not updated since the
      // last TM packet. We do not add these data to the table
      writeNextRow = false;
      logger << warn << "  The OBT " << newObt
                        << " is identical as in previous TM packet for HK Table "
                        << m_hkStructName << ". Discarding this TM packet." << endl;
   }
   // test that the OBT is increasing
   else if (m_currentObt >= newObt) {
      // this should never happen
      throw obt_error("The OBT is decreasing from " +
                 to_string(m_currentObt.getObt()) + " to " +
                 to_string(newObt.getObt()) + " for HK Table " +
                 m_hkStructName);
   }

   // everything is OK, we can set m_currentObt and m_currentUtc
   m_currentObt = newObt;
   m_currentUtc = m_currentObt.getUtc();;

   if (writeNextRow) {
      // all values of the tmPacket are copied to the assigned variable
      // we can write the next row in the FITS table
      m_fitsDalTable->WriteRow();
   }

   // pass the Tm Packet to the next table in the chain of FITS Tables
   if (m_hkPrwFitsTableChain != nullptr)
      m_hkPrwFitsTableChain->addTmPacket(obt, tmPacketData, maxLength);

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
HkTm2PrwProcessing::HkTm2PrwProcessing(const std::string & outDir,
                                       const VisitId & visitId,
                                       const PassId & passId) :
          HkProcessing(true, false),
          m_outDir(outDir), m_visitId(visitId), m_passId(passId) {

}

////////////////////////////////////////////////////////////////////////////////
bool  HkTm2PrwProcessing::tmPacketIsAvailable(const TmPacketId & tmPacketId) {
   return m_prwTableInfo.find(tmPacketId) != m_prwTableInfo.end();
}

////////////////////////////////////////////////////////////////////////////////
HkPrwFitsTable * HkTm2PrwProcessing::getHkPrwFitsTable(const TmPacketId & tmPacketId,
                                                       const OBT & obt) {

   // get all data structures to which data of the tmPacketId has to be copied to.
   typedef multimap<TmPacketId, string>::iterator I_TableInfo;

   pair<I_TableInfo, I_TableInfo> i_tableInfoRange = m_prwTableInfo.equal_range(tmPacketId);
   if (i_tableInfoRange.first == i_tableInfoRange.second)
      throw runtime_error("TM packet with AppId " + to_string(tmPacketId.getAppId()) +
                          " SID " + to_string(tmPacketId.getSid()) +
                          " is not known." );

   HkPrwFitsTable * hkPrwFitsTable = nullptr;

   I_TableInfo i_tableInfo = i_tableInfoRange.first;
   while (i_tableInfo != i_tableInfoRange.second) {

      // SCI_PRW_HkDefault
      if (i_tableInfo->second == "SCI_PRW_HkDefault") {
         SciPrwHkdefault * sciPrwHkdefault = createFitsFile<SciPrwHkdefault> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkdefault>(sciPrwHkdefault,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkExtended
      else if (i_tableInfo->second == "SCI_PRW_HkExtended") {
         SciPrwHkextended * sciPrwHkextended = createFitsFile<SciPrwHkextended> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkextended>(sciPrwHkextended,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkIfsw
      else if (i_tableInfo->second == "SCI_PRW_HkIfsw") {
         SciPrwHkifsw * sciPrwHkifsw = createFitsFile<SciPrwHkifsw> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkifsw>(sciPrwHkifsw,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkIbswPar
      else if (i_tableInfo->second == "SCI_PRW_HkIbswPar") {
         SciPrwHkibswpar * sciPrwHkibswpar = createFitsFile<SciPrwHkibswpar> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkibswpar>(sciPrwHkibswpar,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkIbswDg
      else if (i_tableInfo->second == "SCI_PRW_HkIbswDg") {
         SciPrwHkibswdg * sciPrwHkibswdg = createFitsFile<SciPrwHkibswdg> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkibswdg>(sciPrwHkibswdg,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkIaswPar
      else if (i_tableInfo->second == "SCI_PRW_HkIaswPar") {
         SciPrwHkiaswpar * sciPrwHkiaswpar = createFitsFile<SciPrwHkiaswpar> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkiaswpar>(sciPrwHkiaswpar,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkIaswDg
      else if (i_tableInfo->second == "SCI_PRW_HkIaswDg") {
         SciPrwHkiaswdg * sciPrwHkiaswdg = createFitsFile<SciPrwHkiaswdg> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkiaswdg>(sciPrwHkiaswdg,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkAsy30759
      else if (i_tableInfo->second == "SCI_PRW_HkAsy30759") {
         SciPrwHkasy30759 * sciPrwHkasy30759 = createFitsFile<SciPrwHkasy30759> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkasy30759>(sciPrwHkasy30759,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkAsy30767
      else if (i_tableInfo->second == "SCI_PRW_HkAsy30767") {
         SciPrwHkasy30767 * sciPrwHkasy30767 = createFitsFile<SciPrwHkasy30767> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkasy30767>(sciPrwHkasy30767,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkCentroid
      else if (i_tableInfo->second == "SCI_PRW_HkCentroid") {
         SciPrwHkcentroid * sciPrwHkcentroid = createFitsFile<SciPrwHkcentroid> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkcentroid>(sciPrwHkcentroid,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      // SCI_PRW_HkOperationParameter
      else if (i_tableInfo->second == "SCI_PRW_HkOperationParameter") {
         SciPrwHkoperationparameter * sciPrwHkoperationparameter = createFitsFile<SciPrwHkoperationparameter> (
               m_outDir, obt.getUtc(), m_visitId, m_passId);
         hkPrwFitsTable = new HkPrwFitsTableChild<SciPrwHkoperationparameter>(sciPrwHkoperationparameter,
                                          m_passId,
                                          m_colData[i_tableInfo->second],
                                          m_obtTmOffset[i_tableInfo->second],
                                          hkPrwFitsTable);
      }

      i_tableInfo++;
   }

   return hkPrwFitsTable;

}
