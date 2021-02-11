/*
 * test_parameters.cxx
 *
 *  Created on: May 8, 2014
 *      Author: rohlfs
 */

#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <stdint.h>

#include "ProgramParams.hxx"
#include "FitsDalTable.hxx"
#include "FitsDalImage.hxx"

using namespace boost::unit_test;

struct FitsDalFixture {
   FitsDalFixture() {
      m_params = CheopsInit(framework::master_test_suite().argc,
                            framework::master_test_suite().argv);


   }

   ~FitsDalFixture() {
   }

   ParamsPtr m_params;
};

BOOST_FIXTURE_TEST_SUITE( testWriteCurrentStatus, FitsDalFixture )


////////////////////////////////////////////////////////////////////////////////
// Create a table with one column per supported data type
BOOST_AUTO_TEST_CASE( CopyTable )
{
   unlink("results/originalTable.fits");
   unlink("results/copyTable.fits");

   FitsDalTable * table = new FitsDalTable(
           "results/originalTable.fits", "CREATE");

   table->SetAttr("EXTNAME", std::string("TST-TBL-ALLDATATYPES"));

   int64_t     longVal   = -123456789012;
   float       floatVal[5]  {1.1, 2.2, 3.3, 4.4, 5.5};

   table->Assign("longCol",           &longVal);
   table->Assign("floatCol",          floatVal, 5);

   table->WriteRow();
   longVal = 2;
   table->WriteRow();
   longVal = 3;
   floatVal[3] = 33;
   table->WriteRow();

   table->WriteCurrentStatus("results/copyTable.fits");

   delete table;
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( CopyImage )
{
   unlink("results/originalImage.fits");
   unlink("results/copyImage.fits");

   FitsDalImage<uint16_t> * image = new FitsDalImage<uint16_t>(
           "results/originalImage.fits", "CREATE", {2, 3});

   image->SetAttr("EXTNAME", std::string("TST-IMA-SIMPLE"));

   uint16_t pixelValue = 0;
   for (int32_t y = 0; y < 3; y++) {
      for (int32_t x = 0; x < 2; x++) {
         (*image)[y][x] = ++pixelValue;
      }
   }

   image->WriteCurrentStatus("results/copyImage.fits");

   delete image;
}


BOOST_AUTO_TEST_SUITE_END()
