/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief   unit_test of creating and updating a FITS table
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 6.3   2016-10-27 RRO first released version
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

BOOST_FIXTURE_TEST_SUITE( testUpdateTable, FitsDalFixture )

////////////////////////////////////////////////////////////////////////////////
// Create a table with one column per supported data type
BOOST_AUTO_TEST_CASE( UpdateColumns )
{
   unlink("results/testUpdateColumns.fits");

   FitsDalTable * table = new FitsDalTable(
           "results/testUpdateColumns.fits", "CREATE");

   table->SetAttr("EXTNAME", std::string("TST-TBL-UpdateColumns"));

   int32_t     col1, col2;

   table->Assign("COL_1",      &col1);
   table->Assign("COL_2",      &col2);

   col1 = 1;
   table->WriteRow();

   col1 = 2;
   table->WriteRow();

   col1 = 3;
   table->WriteRow();

   table->PrepareWriteRow(2);
   col2 = 20;
   table->WriteRow();

   col2 = 30;
   table->WriteRow();

   col2 = 40;
   table->WriteRow();

   delete table;

   // open the table again an verify that the correct values are stored in the table

   table = new FitsDalTable("results/testUpdateColumns.fits");
   table->Assign("COL_1",      &col1);
   table->Assign("COL_2",      &col2);

   table->ReadRow();
   BOOST_CHECK_EQUAL(col1, 1);

   table->ReadRow();
   BOOST_CHECK_EQUAL(col1, 2);
   BOOST_CHECK_EQUAL(col2, 20);

   table->ReadRow();
   BOOST_CHECK_EQUAL(col1, 3);
   BOOST_CHECK_EQUAL(col2, 30);

   table->ReadRow();
   BOOST_CHECK_EQUAL(col1, 3);
   BOOST_CHECK_EQUAL(col2, 40);

   delete table;


}


BOOST_AUTO_TEST_SUITE_END()
