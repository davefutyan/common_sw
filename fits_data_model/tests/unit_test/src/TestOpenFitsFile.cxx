#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <string>
#include <stdexcept>


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

BOOST_FIXTURE_TEST_SUITE( TestOpenFitsFile, Fixture )


BOOST_AUTO_TEST_CASE( Open_Extension )
{
   unlink("result/CH_TU2015-04-09T13-51-30_SCI_PRW_SubArray_V0000.fits[SCI_PRW_SubArray]");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   SciPrwSubarray * sciPrwSubarray = new SciPrwSubarray( "result/CH_TU2015-04-09T13-51-30_SCI_PRW_SubArray_V0000.fits");
   SciPrwImagemetadata * sciPrwImagemetadata = Open_SciPrwImagemetadata(sciPrwSubarray);
   sciPrwImagemetadata->ReadRow();
   int32_t status = sciPrwImagemetadata->getCellCeCounter();
   BOOST_CHECK_EQUAL(status, 4);

   SciPrwBlankreduced * sciPrwBlankreduced = Open_SciPrwBlankreduced(sciPrwSubarray);
   // sciPrwDarktopimage shall be nullptr as this extension does not exist in the file
   BOOST_CHECK_EQUAL(sciPrwBlankreduced == nullptr, true);


   // the data classes have to be deleted to write the data into the FITS file.
   delete sciPrwSubarray;
   delete sciPrwImagemetadata;
}

BOOST_AUTO_TEST_SUITE_END()
