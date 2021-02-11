#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <string>
#include <stdexcept>


#include "ProgramParams.hxx"

#include "CreateFitsFile.hxx"
#include "SCI_PRW_SubArray.hxx"
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

BOOST_FIXTURE_TEST_SUITE( TestCreateFitsFile, Fixture )


BOOST_AUTO_TEST_CASE( UTC_Image )
{
   unlink("result/CH_TU2015-04-09T13-51-30_SCI_PRW_SubArray_V0000.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   UTC     utc(2015, 4, 9, 13, 51, 30, 0.45);

   SciPrwSubarray * sciPrwSubarray = createFitsFile<SciPrwSubarray>( "result",
                                       utc, VisitId(), {200, 200, 3});
   SciPrwImagemetadata * sciPrwImagemetadata = Append_SciPrwImagemetadata(sciPrwSubarray);
   sciPrwImagemetadata->setCellCeCounter(4);
   sciPrwImagemetadata->WriteRow();

   // the data classes have to be deleted to write the data into the FITS file.
   delete sciPrwSubarray;
   delete sciPrwImagemetadata;
}

BOOST_AUTO_TEST_CASE( All_Table )
{
   unlink("result/CH_PR990707_TG070755_PS15012009_TU2015-04-09T13-51-30_SCI_COR_Lightcurve-dataName_V0000.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   PassId  passId(15, 1, 20, 9);
   VisitId visitId(99, 707, 707, 55);
   UTC     utc(2015, 4, 9, 13, 51, 30, 0.45);

   SciCorLightcurve * sciCorLightcurve = createFitsFile<SciCorLightcurve>( "result",
                                            utc, visitId, passId, "dataName");


   // the data classes have to be deleted to write the data into the FITS file.
   delete sciCorLightcurve;
}



BOOST_AUTO_TEST_SUITE_END()
