/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief   unit_test to set and read NULL values in a FITS table
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 3.0   2014-12-18 RRO #7054: Support NULL values of columns
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <limits>
#include <math.h>
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

BOOST_FIXTURE_TEST_SUITE( testNullValluesTable, FitsDalFixture )

////////////////////////////////////////////////////////////////////////////////
// Create a table with one column per supported data type all values are set
// to a NULL value
BOOST_AUTO_TEST_CASE( CreateNullTable )
{
   unlink("results/testNullTable.fits");

   FitsDalTable * table = new FitsDalTable(
           "results/testNullTable.fits", "CREATE");

   table->SetAttr("EXTNAME", std::string("TST-TBL-NULLVALUES"));

   std::string stringVal = "";
   uint8_t     ucharVal  = 5;
   int8_t      charVal   = 5;
   uint16_t    ushortVal = 5;
   int16_t     shortVal  = 5;
   uint32_t    uintVal   = 5;
   int32_t     intVal    = 5;
   uint64_t    ulongVal  = 5;
   int64_t     longVal   = 5;
   float       floatVal  = std::numeric_limits<float>::quiet_NaN();
   double      doubleVal = std::numeric_limits<double>::quiet_NaN();

   uint8_t     ucharNull  = 5;
   int8_t      charNull   = 5;
   uint16_t    ushortNull = 5;
   int16_t     shortNull  = 5;
   uint32_t    uintNull   = 5;
   int32_t     intNull    = 5;
   uint64_t    ulongNull  = 5;
   int64_t     longNull   = 5;


   table->Assign("stringcolumn",      &stringVal, 2, "", "string unit");
   table->Assign("Unsigned Char Col", &ucharVal,  1, "", "", &ucharNull);
   table->Assign("SignedCharCol",     &charVal,   1, "", "", &charNull);
   table->Assign("ushortcol",         &ushortVal, 1, "", "", &ushortNull);
   table->Assign("shortCol",          &shortVal,  1, "", "", &shortNull);
   table->Assign("uintcol",           &uintVal,   1, "", "", &uintNull);
   table->Assign("intCol",            &intVal,    1, "", "", &intNull);
   table->Assign("ulongcol",          &ulongVal,  1, "", "", &ulongNull);
   table->Assign("longCol",           &longVal,   1, "", "", &longNull);
   table->Assign("floatCol",          &floatVal);
   table->Assign("doubleCol",         &doubleVal);

   table->WriteRow();

   delete table;
}


////////////////////////////////////////////////////////////////////////////////
// Reads one row of a test table, which was created by the TestCreateTable
// program
BOOST_AUTO_TEST_CASE( readNullTable )
{

   FitsDalTable * table = new FitsDalTable(
           "results/testNullTable.fits[TST-TBL-NULLVALUES]");

   std::string stringVal;
   uint8_t     ucharVal ;
   int8_t      charVal  ;
   uint16_t    ushortVal;
   int16_t     shortVal ;
   uint32_t    uintVal  ;
   int32_t     intVal   ;
   uint64_t    ulongVal ;
   int64_t     longVal  ;
   float       floatVal ;
   double      doubleVal;

   uint8_t     ucharNull;
   int8_t      charNull ;
   uint16_t    ushortNull;
   int16_t     shortNull ;
   uint32_t    uintNull  ;
   int32_t     intNull   ;
   uint64_t    ulongNull ;
   int64_t     longNull  ;


   table->Assign("stringcolumn",      &stringVal, 2, "", "string unit");
   table->Assign("Unsigned Char Col", &ucharVal,  1, "", "", &ucharNull);
   table->Assign("SignedCharCol",     &charVal,   1, "", "", &charNull);
   table->Assign("ushortcol",         &ushortVal, 1, "", "", &ushortNull);
   table->Assign("shortCol",          &shortVal,  1, "", "", &shortNull);
   table->Assign("uintcol",           &uintVal,   1, "", "", &uintNull);
   table->Assign("intCol",            &intVal,    1, "", "", &intNull);
   table->Assign("ulongcol",          &ulongVal,  1, "", "", &ulongNull);
   table->Assign("longCol",           &longVal,   1, "", "", &longNull);
   table->Assign("floatCol",          &floatVal);
   table->Assign("doubleCol",         &doubleVal);


   BOOST_CHECK_EQUAL(true, table->ReadRow());

   BOOST_CHECK_EQUAL(stringVal.length(), 0U);
   BOOST_CHECK_EQUAL(ucharVal,   ucharNull);
   BOOST_CHECK_EQUAL(charVal,    charNull);
   BOOST_CHECK_EQUAL(ushortVal,  ushortNull);
   BOOST_CHECK_EQUAL(shortVal,   shortNull);
   BOOST_CHECK_EQUAL(uintVal,    uintNull);
   BOOST_CHECK_EQUAL(intVal,     intNull);
   BOOST_CHECK_EQUAL(ulongVal,   ulongNull);
   BOOST_CHECK_EQUAL(longVal,    longNull);
   BOOST_CHECK_EQUAL(std::isnan(floatVal), true);
   BOOST_CHECK_EQUAL(std::isnan(doubleVal), true);

   //table has only one row, the next attempt to read data should return false.
   BOOST_CHECK_EQUAL(false, table->ReadRow());

   delete table;
}


BOOST_AUTO_TEST_SUITE_END()
