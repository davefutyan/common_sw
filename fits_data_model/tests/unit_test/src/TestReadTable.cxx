#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <string>

#include "ProgramParams.hxx"

#include "REF_APP_HkEnumConversion.hxx"

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


BOOST_AUTO_TEST_CASE( testTableRead )
{

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   RefAppHkenumconversion * table = new RefAppHkenumconversion("resources/CH_TU2015-01-01T00-00-00_REF_APP_HkEnumConversion_V0007.fits");

   BOOST_CHECK_EQUAL(table->getUnitOfKeyVStopU(), "UTC");
   BOOST_CHECK_EQUAL(table->getUnitOfKeyArchRev(), "");
   BOOST_CHECK_EQUAL(table->getComOfKeyArchRev(), "Archive revision number");

   table->ReadRow();

   BOOST_CHECK_EQUAL(table->getCellEnum(), 0);
   BOOST_CHECK_EQUAL(table->getCellText(), "GROUND");

   delete table;
}




BOOST_AUTO_TEST_SUITE_END()
