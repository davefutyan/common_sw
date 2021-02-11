#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"


#include <string>

#include "ProgramParams.hxx"

#include "SCI_COR_Lightcurve.hxx"

using namespace boost;
using namespace boost::unit_test;


/** ****************************************************************************
 *  @brief Required struct by the boost unit test system
 */
struct Fixture{
   Fixture() {}
   ~Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( FitsTableSuite, Fixture )


BOOST_AUTO_TEST_CASE( testTableCreation )
{

   unlink("result/new_table.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   SciCorLightcurve * table = new SciCorLightcurve("result/new_table.fits", "CREATE");

   BOOST_CHECK_EQUAL(table->getUnitOfCellFlux(), "electrons");
   BOOST_CHECK_EQUAL(table->getUnitOfCellStatus(), "");


   table->WriteRow();
   table->WriteRow();

   delete table;


}




BOOST_AUTO_TEST_SUITE_END()
