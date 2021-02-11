/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief   unit_test of reading a FITS table
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 9.0.1 2018-01-31 RRO #15378: new test TSCALE_AS_DOUBLE
 *  @version 9.0   2018-01-04 RRO #15057: new tests of GetColUnit()
 *  @versino 5.2   2016-05-05 RRO #10580 test random access of rows in FITS table
 *  @version 4.3   2015-10-13 RRO #9304: values of not available columns are
 *                                       set to their default value
 *  @version 3.0   2014-12-17 RRO #7054: Support NULL values of columns
 *  @version 1.0   2014-06-26 RRO first released version
 *
 */

#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

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

BOOST_FIXTURE_TEST_SUITE( testReadTable, FitsDalFixture )

////////////////////////////////////////////////////////////////////////////////
// Reads one row of a test table, which was created by the TestCreateTable
// program
BOOST_AUTO_TEST_CASE( AllDataTypesTable )
{

   FitsDalTable * table = new FitsDalTable(
           "results/testTable.fits[TST-TBL-ALLDATATYPES]");

   bool boolVal[3];
   std::string stringVal;
   uint8_t     ucharVal;
   int8_t      charVal;
   uint16_t    ushortVal;
   int16_t     shortVal;
   int16_t     notDefNull = 24;
   uint32_t    uintVal;
   int32_t     intVal;
   uint64_t    ulongVal;
   uint64_t    ulongNull;
   int64_t     longVal;
   float       floatVal[5];
   double      doubleVal[3];
   int32_t     doesNotExistVal = 246;

   floatVal[4] = 44.;
   doubleVal[2] = 2.2;

   table->Assign("BoolCol",           boolVal, 3, "no comment", "other unit");
   table->Assign("stringcolumn",      &stringVal, 6);
   table->Assign("Unsigned Char Col", &ucharVal);
   table->Assign("SignedCharCol",     &charVal);
   table->Assign("ushortcol",         &ushortVal);
   //the NULL value is not defined. The varaible should not be overwritten
   table->Assign("shortCol",          &shortVal, 1, "","", &notDefNull);
   table->Assign("uintcol",           &uintVal);
   table->Assign("intCol",            &intVal);
   table->Assign("ulongcol",          &ulongVal, 1, "", "", &ulongNull);
   table->Assign("longCol",           &longVal);
   // the floatCol has 5 bins per cell. The fifth should not be overwritten
   // the previously assign value (44) should be kept.
   table->Assign("floatCol",          floatVal, 4);
   // the coubleCol has 2 bins per cell. Trying to read 3 should not cause
   // an error, but the third value will not be updated.
   table->Assign("doubleCol",         doubleVal, 3);
   // this column does not exist.  doesNotExistVal shall be set to int32_t() == 0
   table->Assign("does not exist",    &doesNotExistVal);

   BOOST_CHECK_EQUAL(table->GetColUnit("BoolCol"), "boolUnit");
   BOOST_CHECK_EQUAL(table->GetColUnit("shortCol"), "");
   BOOST_CHECK_EQUAL(table->GetColUnit("notExisitngCol"), "");

   BOOST_CHECK_EQUAL(true, table->ReadRow());

   BOOST_CHECK_EQUAL(true, boolVal[0]);
   BOOST_CHECK_EQUAL(false, boolVal[1]);
   BOOST_CHECK_EQUAL(true, boolVal[2]);
   BOOST_CHECK_EQUAL("test s", stringVal);
   BOOST_CHECK_EQUAL(255,           ucharVal);
   BOOST_CHECK_EQUAL(-126,          charVal);
   BOOST_CHECK_EQUAL(65535,         ushortVal);
   BOOST_CHECK_EQUAL(-32768,        shortVal);
   BOOST_CHECK_EQUAL(24,            notDefNull);
   BOOST_CHECK_EQUAL(1234567U,      uintVal);
   BOOST_CHECK_EQUAL(1234567,       intVal);
   BOOST_CHECK_EQUAL(123456789012UL,  ulongVal);
   BOOST_CHECK_EQUAL(10223372036854775808ULL,  ulongNull);
   BOOST_CHECK_EQUAL(-123456789012, longVal);
   BOOST_CHECK_CLOSE(1.1,      floatVal[0],  0.001);
   BOOST_CHECK_CLOSE(2.2,      floatVal[1],  0.001);
   BOOST_CHECK_CLOSE(3.3,      floatVal[2],  0.001);
   BOOST_CHECK_CLOSE(4.4,      floatVal[3],  0.001);
   BOOST_CHECK_CLOSE(44.,      floatVal[4],  0.001);
   BOOST_CHECK_CLOSE(-4.E-100, doubleVal[0], 0.00001);
   BOOST_CHECK_CLOSE(5.5E120,  doubleVal[1], 0.00001);
   BOOST_CHECK_CLOSE(2.2,      doubleVal[2], 0.00001);
   BOOST_CHECK_EQUAL(0,        doesNotExistVal);

   // read next row = row number 2.
   BOOST_CHECK_EQUAL(true, table->ReadRow());
   BOOST_CHECK_EQUAL(2,       intVal);

   // read again first row
   BOOST_CHECK_EQUAL(true, table->ReadRow(1));
   BOOST_CHECK_EQUAL(1234567,       intVal);

   // read 2nd row
   BOOST_CHECK_EQUAL(true, table->ReadRow(2));
   BOOST_CHECK_EQUAL(2,       intVal);

   //table has only 3 rows, the next attempt to read data should return false.
   BOOST_CHECK_EQUAL(false, table->ReadRow(4));

   BOOST_CHECK_EQUAL(true, table->ReadRow(3));
   BOOST_CHECK_EQUAL(3,       intVal);

   //table has only 3 rows, the next attempt to read data should return false.
   BOOST_CHECK_EQUAL(false, table->ReadRow());

   // rewind to the beginning of the table
   BOOST_CHECK_EQUAL(true, table->SetReadRow(1));
   BOOST_CHECK_EQUAL(true, table->ReadRow());
   BOOST_CHECK_EQUAL(1234567, intVal);

   // start reading from a row after the end of the table
   BOOST_CHECK_EQUAL(false, table->SetReadRow(10));

   // reassign a column
   int32_t     newIntVal;
   table->ReAssign("intCol", &newIntVal);
   BOOST_CHECK_EQUAL(true, table->ReadRow());
   BOOST_CHECK_EQUAL(2, newIntVal);

   // read fits column metadata
   std::list<FitsColMetaData> metadata = table->GetFitsColMetaData();
   // there shall be one object per column
   BOOST_CHECK_EQUAL(std::list<FitsColMetaData>::size_type(12), metadata.size());

   BOOST_CHECK_EQUAL("FitsDalTable", table->getClassName());

   delete table;
}

////////////////////////////////////////////////////////////////////////////////
// Reads columns where the TSCALE and TZERO are defined as double values
// instead of integer values
BOOST_AUTO_TEST_CASE( TSCALE_AS_DOUBLE )
{

   FitsDalTable * table = new FitsDalTable(
           "resources/CH_PR300001_TG000401_TU2019-01-01T11-59-50_MPS_PRE_Visits_V0000_merged.fits+1");

   uint8_t     programme_type;
   uint16_t    programme_id;
   uint32_t    obsid;

   table->Assign("PROGRAMME_TYPE",   &programme_type);
   table->Assign("PROGRAMME_ID",     &programme_id);
   table->Assign("OBSID",            &obsid);


   BOOST_CHECK_EQUAL(true, table->ReadRow());

   BOOST_CHECK_EQUAL(30U,          programme_type);
   BOOST_CHECK_EQUAL(1U,           programme_id);
   BOOST_CHECK_EQUAL(1000041U,     obsid);


}

BOOST_AUTO_TEST_SUITE_END()
