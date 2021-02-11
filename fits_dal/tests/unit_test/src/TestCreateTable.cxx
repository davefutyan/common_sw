/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief   unit_test of creating a FITS table
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 9.0   2018-01-04 RRO #15057: new tests of GetColUnit()
 *  @version 3.0   2014-12-17 RRO #7054: Support NULL values of columns
 *  @version 1.0   2014-06-26 RRO first released version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <stdint.h>

#include "ProgramParams.hxx"
#include "FitsDalTable.hxx"

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

BOOST_FIXTURE_TEST_SUITE( testCreateTable, FitsDalFixture )

////////////////////////////////////////////////////////////////////////////////
// Create a table with one column per supported data type
BOOST_AUTO_TEST_CASE( AllDataTypesTable )
{
   unlink("results/testTable.fits");

   FitsDalTable * table = new FitsDalTable(
           "results/testTable.fits", "CREATE");

   table->SetAttr("EXTNAME", std::string("TST-TBL-ALLDATATYPES"));
   table->SetAttr("EXT_VER", std::string("1"));

   bool boolVal[3] {true, false, true};
   std::string stringVal = "test string";
   uint8_t     ucharVal  = 255;
   int8_t      charVal   = -126;
   uint16_t    ushortVal = 65535;
   int16_t     shortVal  = -32768;
   uint32_t    uintVal   = 1234567;
   int32_t     intVal    = 1234567;
   uint64_t    ulongVal  = 123456789012;
   uint64_t    ulongNull = 10223372036854775808ULL;
   int64_t     longVal   = -123456789012;
   float       floatVal[5]  {1.1, 2.2, 3.3, 4.4, 5.5};
   double      doubleVal[2] {-4.E-100, 5.5E120};

   table->Assign("BoolCol",           boolVal, 3, "bool comment", "boolUnit");
   table->Assign("stringcolumn",      &stringVal, 6, "", "string unit");
   table->Assign("Unsigned Char Col", &ucharVal);
   table->Assign("SignedCharCol",     &charVal);
   table->Assign("ushortcol",         &ushortVal);
   table->Assign("shortCol",          &shortVal);
   table->Assign("uintcol",           &uintVal);
   table->Assign("intCol",            &intVal);
   table->Assign("ulongcol",          &ulongVal, 1, "", "", &ulongNull);
   table->Assign("longCol",           &longVal);
   table->Assign("floatCol",          floatVal, 5);
   table->Assign("doubleCol",         doubleVal, 2);

   table->WriteRow();
   intVal = 2;
   table->WriteRow();
   intVal = 3;
   table->WriteRow();

   BOOST_CHECK_EQUAL(table->GetColUnit("BoolCol"), "boolUnit");
   BOOST_CHECK_EQUAL(table->GetColUnit("shortCol"), "");
   BOOST_CHECK_EQUAL(table->GetColUnit("notExisitngCol"), "");


   delete table;
}


BOOST_AUTO_TEST_SUITE_END()
