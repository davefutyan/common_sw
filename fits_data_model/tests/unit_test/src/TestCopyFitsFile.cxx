#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <string>
#include <stdexcept>


#include "ProgramParams.hxx"

#include "EXT_PRE_StarCatalogue.hxx"
#include "REF_APP_HkParamConversion.hxx"

using namespace boost;
using namespace boost::unit_test;


/** ****************************************************************************
 *  @brief Required struct by the boost unit test system
 */
struct Fixture{
   Fixture() {}
   ~Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( TestCopyFitsFile, Fixture )


BOOST_AUTO_TEST_CASE( StarCatalogue )
{
   unlink("result/CH_PR310074_TG000300_TU2020-11-10T15-18-00_EXT_DRFT_StarCatalogue-OBSID0001277723_V0000.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   ExtPreStarcatalogue * extPreStarCat = new ExtPreStarcatalogue(
         std::string("result/CH_PR310074_TG000300_TU2020-11-10T15-18-00_EXT_DRFT_StarCatalogue-OBSID0001277723_V0000.fits"),
         std::string("resources/CH_PR110011_TG002701_TU2018-03-31T23-50-19_EXT_DRFT_StarCatalogue-OBSID0000006125_V0000.fits"));
   extPreStarCat->setKeyVisitctr(1);

   delete extPreStarCat;
}

BOOST_AUTO_TEST_CASE( DifferentTables )
{
   unlink("result/CH_TU2015-01-01T00-00-00_REF_APP_HkParamConversion_V0007.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   RefAppHkparamconversion * refAppHkparamconversion = new RefAppHkparamconversion(
         std::string("result/CH_TU2015-01-01T00-00-00_REF_APP_HkParamConversion_V0007.fits"),
         std::string("resources/CH_TU2015-01-01T00-00-00_REF_APP_HkEnumConversion_V0007.fits"));



   delete refAppHkparamconversion;
}

BOOST_AUTO_TEST_SUITE_END()
