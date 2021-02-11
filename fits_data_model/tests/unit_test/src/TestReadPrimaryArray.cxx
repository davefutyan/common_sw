#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"


#include <string>

#include "FitsDalHeader.hxx"
#include "ProgramParams.hxx"

#include "SCI_RAW_SubArray.hxx"

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

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   FitsDalHeader * image = new FitsDalHeader("result/image.fits[0]");

   int32_t naxis = image->GetAttr<int32_t>("NAXIS");

   BOOST_CHECK_EQUAL(naxis, 0);

   delete image;


}




BOOST_AUTO_TEST_SUITE_END()
