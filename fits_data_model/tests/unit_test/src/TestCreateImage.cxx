#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <string>

#include "ProgramParams.hxx"

#include "SCI_PRW_SubArray.hxx"

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

   unlink("result/image.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   SciPrwSubarray * image = new SciPrwSubarray("result/image.fits", "CREATE",
                                               {2,3,4});


   BOOST_CHECK_EQUAL(true, image->GetNull().second);

   (*image)[1][1][1] = image->GetNull().first;
   (*image)[1][1][0] = 24;

   delete image;


}




BOOST_AUTO_TEST_SUITE_END()
