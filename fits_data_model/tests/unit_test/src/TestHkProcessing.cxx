#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <stdexcept>
#include <string>

#include <Logger.hxx>
#include <LeapSeconds.hxx>
#include <ObtUtcCorrelation.hxx>
#include <DeltaTime.hxx>

#include "HkProcessing.hxx"
#include "HkTm2PrwProcessing.hxx"
#include "Hk2RawProcessing.hxx"


using namespace boost;
using namespace boost::unit_test;
using namespace std;

class TestHkProcessing : public HkProcessing {

public:
   TestHkProcessing() : HkProcessing(true, true) {};

   std::string  getPrwPacketId(const TmPacketId & tmPacketId) {
      std::map<TmPacketId, std::string>::iterator i_ = m_prwTableInfo.find(tmPacketId);
      if (i_ != m_prwTableInfo.end())
         return i_->second;
      else
         return std::string();
   }

   uint16_t     getObtOffset(std::string extName)  {
      return m_obtTmOffset[extName];   }

};

/** ****************************************************************************
 *  @brief Required struct by the boost unit test system
 */
struct Fixture{
   Fixture() {}
   ~Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( Hk_, Fixture )


BOOST_AUTO_TEST_CASE( ReadFsdFiles )
{

   TestHkProcessing  hkProc;

   std::string extName = hkProc.getPrwPacketId( TmPacketId(322, 1) );
   BOOST_CHECK_EQUAL(extName, "SCI_PRW_HkIfsw");

   uint16_t obtOffset = hkProc.getObtOffset("SCI_PRW_HkExtended");
   BOOST_CHECK_EQUAL(obtOffset, 6);

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( HkTm2Prw )
{

   unlink("result/CH_PR100345_TG000518_PS15072617_TU2015-01-01T00-02-32_SCI_PRW_HkDefault_V0000.fits");
   unlink("result/CH_PR100345_TG000518_PS15072617_TU2015-01-01T00-02-32_SCI_PRW_HkExtended_V0000.fits");
   unlink("result/CH_PR100345_TG000518_PS15072617_TU2015-01-01T00-02-32_SCI_PRW_HkIaswDg_V0000.fits");
   unlink("result/CH_PR100345_TG000518_PS15072617_TU2015-01-01T00-02-32_SCI_PRW_HkIfsw_V0000.fits");
   unlink("result/CH_PR100345_TG000518_PS15072617_TU2015-01-01T00-02-32_SCI_PRW_HkAsy30759_V0000.fits");
   unlink("result/CH_PR100345_TG000518_PS15072617_TU2015-01-01T00-02-32_SCI_PRW_HkCentroid_V0000.fits");
   // set the leap second file and the OBT2UTC correlation file
   list<string> leapSecondFile {"resources/CH_TU1972-01-01T00-00-00_SOC_APP_LeapSeconds_V0002.fits"};
   LeapSeconds::setLeapSecondsFileNames(leapSecondFile);

   list<string> obtUtcCorrelationFileName {"resources/CH_TU2015-01-01T00-00-00_AUX_RES_ObtUtcCorrelation_V000.fits"};
   OBTUTCCorrelation::SetCorrelationFileNames(obtUtcCorrelationFileName);


   HkTm2PrwProcessing hkTm2PrwProcessing("result/", VisitId(10, 345, 5, 18),
                                                    PassId(2015, 7, 26, 17));

   // test the tmPacketIsAvailable() method
   bool available1 = hkTm2PrwProcessing.tmPacketIsAvailable(TmPacketId(322, 1));
   BOOST_CHECK_EQUAL(available1, true);

   bool available2 = hkTm2PrwProcessing.tmPacketIsAvailable(TmPacketId(323, 1));
   BOOST_CHECK_EQUAL(available2, false);

   // test the exception of the getHkFitsTable() method
   bool exceptionCatched = false;
   try {
   hkTm2PrwProcessing.getHkPrwFitsTable(TmPacketId(30, 5), OBT(10000));
   }
   catch (runtime_error & err) {
      exceptionCatched = true;
      BOOST_CHECK_EQUAL(err.what(), "TM packet with AppId 30 SID 5 is not known.");
   }
   BOOST_CHECK_EQUAL(exceptionCatched, true);

   // simple case: one TM packet => one FITS table
   HkPrwFitsTable * hkPrwFitsTable = hkTm2PrwProcessing.getHkPrwFitsTable(TmPacketId(322, 1), OBT(10000000));


   hkPrwFitsTable->setKeyObsid(45);
   hkPrwFitsTable->setKeyPiName("Reiner Rohlfs");

   int8_t tmPacket[500];
   memset(tmPacket, 0, 500);
   tmPacket[9]  = 1;  //sibNFull  = 258
   tmPacket[10] = 2;
   tmPacket[20] = 21; // sdbStructState
   tmPacket[31] = 32; // sdu1State
   hkPrwFitsTable->addTmPacket(OBT(4), tmPacket);

   tmPacket[60] = 1;  // isSdsActive
   hkPrwFitsTable->addTmPacket(OBT(6), tmPacket);
   hkPrwFitsTable->addTmPacket(OBT(9), tmPacket);

   delete hkPrwFitsTable;

   // more complicated case: one TM packet => three FITS tables
   hkPrwFitsTable = hkTm2PrwProcessing.getHkPrwFitsTable(TmPacketId(322, 6), OBT(10000000));
   memset(tmPacket, 0, 500);

   hkPrwFitsTable->setKeyObsid(45);
   hkPrwFitsTable->setKeyPiName("Reiner Rohlfs");
   hkPrwFitsTable->setKeyPiUid(123456);
   hkPrwFitsTable->setKeyObsCat("QL Dark Frame");
   hkPrwFitsTable->setKeyPrpVst1(450);
   hkPrwFitsTable->setKeyPrpVstn(300);

   // set the OBTs
   tmPacket[0] = 2; // default
   tmPacket[4] = 2; // default
   tmPacket[5] = 1; // default

   tmPacket[6]  = 2; // extended
   tmPacket[10] = 2; // extended
   tmPacket[11] = 1; // extended


   tmPacket[13]   = 1; // default  STAT_MODE
   tmPacket[15]   = 2;
   tmPacket[17]   = 3;
   tmPacket[19]   = 4;
   tmPacket[21]   = 5;
   tmPacket[23]   = 6;
   tmPacket[25]   = 7;
   tmPacket[27]   = 8;
   tmPacket[29]   = 9;
   tmPacket[31]   = 10;
   tmPacket[33]   = 11;
   tmPacket[35]   = 12;

   tmPacket[36]   = 12;
   tmPacket[37]   = 12;
   tmPacket[38]   = 12;
   tmPacket[39]   = 12;

   tmPacket[43]   = 12;
   tmPacket[47]   = 12;
   tmPacket[51]   = 12;

   tmPacket[52] = 52; // extended TEMP_FEE_CCD


   hkPrwFitsTable->addTmPacket(OBT(5), tmPacket);

   tmPacket[4] = 3; // default
   tmPacket[5] = 1; // default


   tmPacket[10] = 3; // extended
   tmPacket[11] = 1; // extended


   tmPacket[15]   = 22;

   tmPacket[50] = 52; // last of default

   tmPacket[51] = 52; // last of default


   hkPrwFitsTable->addTmPacket(OBT(6), tmPacket);
   delete hkPrwFitsTable;

   //test a TM packet with an additional OBT parameter == Asy30759
   hkPrwFitsTable = hkTm2PrwProcessing.getHkPrwFitsTable(TmPacketId(180, 58), OBT(10000000));
   memset(tmPacket, 0, 500);
   tmPacket[5]  = 20;  // the OBT in the data field
   tmPacket[10] = 1;

   // this should be 12.345678 in PSE_quaternion_scal
   tmPacket[83] = 0x40;
   tmPacket[84] = 0x28;
   tmPacket[85] = 0xB0;
   tmPacket[86] = 0xFC;
   tmPacket[87] = 0xB4;
   tmPacket[88] = 0xF1;
   tmPacket[89] = 0xE4;
   tmPacket[90] = 0xB4;

   hkPrwFitsTable->addTmPacket(OBT(5), tmPacket);
   delete hkPrwFitsTable;


   //test a TM packet of the HKCentroid
   hkPrwFitsTable = hkTm2PrwProcessing.getHkPrwFitsTable(TmPacketId(323, 0), OBT(10000000));
   memset(tmPacket, 0, 500);

   tmPacket[0] = 0x01;
   tmPacket[1] = 0x02;
   tmPacket[2] = 0x03;
   tmPacket[3] = 0x00;
   tmPacket[4] = 0x00;
   tmPacket[5] = 0x00;
   tmPacket[6] = 0x00;
   tmPacket[7] = 0xc8;
   tmPacket[8] = 0x01;
   tmPacket[9] = 0x00;
   tmPacket[10] = 0xc8;
   tmPacket[11] = 0x00;
   tmPacket[12] = 0x00;
   tmPacket[13] = 0x01;
   tmPacket[14] = 0x44;
   tmPacket[15] = 0xdc;
   tmPacket[16] = 0x7b;
   tmPacket[17] = 0xe7;
   tmPacket[18] = 0x00;
   tmPacket[19] = 0x01;
   tmPacket[20] = 0x44;
   tmPacket[21] = 0xdd;
   tmPacket[22] = 0x7b;
   tmPacket[23] = 0xe7;
   tmPacket[24] = 0x17;
   tmPacket[25] = 0x98;
   tmPacket[26] = 0xfc;

   hkPrwFitsTable->addTmPacket(OBT(5), tmPacket);
   delete hkPrwFitsTable;


}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( Hk2Raw )
{
   unlink("result/CH_PR100345_TG000518_TU2015-07-31T15-51-22_SCI_RAW_HkDefault_V0000.fits");
   unlink("result/CH_PR100345_TG000518_TU2015-07-31T15-51-22_SCI_RAW_HkIfsw_V0000.fits");
   unlink("result/CH_PR100345_TG000518_TU2015-07-31T15-51-22_SCI_RAW_HkIaswPar_V0000.fits");
   unlink("result/CH_PR100345_TG000518_TU2015-07-31T15-51-22_SCI_RAW_HkCentroid_V0000.fits");

   Hk2RawProcessing hk2RawProcessing(
                "resources/CH_TU2015-01-01T00-00-00_REF_APP_HkEnumConversion_V0007.fits",
                "result", VisitId(10, 345, 5, 18));

   hk2RawProcessing.updateParameter("STAT_FLAGS",  456);
   hk2RawProcessing.updateParameter("sdu2State", "DOWN_TRANSFER");


   hk2RawProcessing.updateParameter("OFFSET_X", 22);
   hk2RawProcessing.updateParameter("OFFSET_Y", 33);
   hk2RawProcessing.updateParameter("LOCATION_X", 44);
   hk2RawProcessing.updateParameter("LOCATION_Y", 55);
   hk2RawProcessing.updateParameter("DATA_CADENCE", 2020);
   hk2RawProcessing.updateParameter("VALIDITY", 100);


   UTC utc("2015-07-31T15:51:22");
   DeltaTime deltaTime(20);


   for (int32_t loop = 0; loop < 500; loop++) {
      hk2RawProcessing.updateParameter("TEMP_SEM_SCU",  loop);
      hk2RawProcessing.updateParameter("TEMP_SEM_PCU",  loop + 1);

      hk2RawProcessing.updateParameter("OBT_START", OBT(loop));
      hk2RawProcessing.updateParameter("OBT_STOP",  OBT(loop));

      hk2RawProcessing.writeRow(utc, "SCI_RAW_HkDefault");

      hk2RawProcessing.writeRow(utc, "SCI_RAW_HkIfsw");
      hk2RawProcessing.writeRow(utc, "SCI_RAW_HkIaswPar");

      hk2RawProcessing.writeRow(utc, "SCI_RAW_HkCentroid");

      utc = utc + deltaTime;
   }

   hk2RawProcessing.setKeyCheopsid (4546);
   hk2RawProcessing.setKeyTargname ("new star");
   hk2RawProcessing.setKeyPiName   ("Reiner Rohlfs");
   hk2RawProcessing.setKeyObsid    (367235);
   hk2RawProcessing.setKeyRaTarg   (4.56);
   hk2RawProcessing.setKeyDecTarg  (-34.56);
   hk2RawProcessing.setKeyMagG     (12.0);
   hk2RawProcessing.setKeyMagGerr  (0.5);
   hk2RawProcessing.setKeyMagChps  (14.0);
   hk2RawProcessing.setKeyMagCerr  (1.5);

}
BOOST_AUTO_TEST_SUITE_END()
