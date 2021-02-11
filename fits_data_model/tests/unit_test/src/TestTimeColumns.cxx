#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <string>
#include <stdexcept>


#include "ProgramParams.hxx"

#include "CreateFitsFile.hxx"
#include "SCI_RAW_ImageMetadata.hxx"
#include "SCI_CAL_ImageMetadata.hxx"

using namespace boost;
using namespace boost::unit_test;


/** ****************************************************************************
 *  @brief Required struct by the boost unit test system
 */
struct Fixture{
   Fixture() {}
   ~Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( TestTimeColumns, Fixture )


BOOST_AUTO_TEST_CASE( Create )
{
   unlink("result/CH_PR101111_TG002004_TU2015-04-04T14-16-18_SCI_RAW_ImageMetadata_V0000.fits");
   unlink("result/CH_PR101111_TG002004_TU2015-04-04T14-16-18_SCI_CAL_ImageMetadata_V0000.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   VisitId visitId(10, 1111, 20, 4);
   UTC     utc("2015-04-04T14:16:18");

   SciRawImagemetadata * sciRawImagemetadata = createFitsFile<SciRawImagemetadata>("result",
                                     utc, visitId);
   SciCalImagemetadata * sciCalImagemetadata = createFitsFile<SciCalImagemetadata>("result",
                                     utc, visitId);

   // write default values
   sciRawImagemetadata->WriteRow();
   sciCalImagemetadata->WriteRow();

   // write specific value
   OBT obt(9876543210987);
   sciRawImagemetadata->setCellObtTime(obt);
   UTC colUtc("2015-04-06T14:14:59.984");
   sciRawImagemetadata->setCellUtcTime(colUtc);
   MJD colMjd(542.45);
   sciRawImagemetadata->setCellMjdTime(colMjd);
   BJD colBjd(542.45);
   sciCalImagemetadata->setCellBjdTime(colBjd);
   sciRawImagemetadata->WriteRow();
   sciCalImagemetadata->WriteRow();

   // set NULL value
   sciRawImagemetadata->setNullObtTime();
   sciRawImagemetadata->setNullUtcTime();
   sciRawImagemetadata->setNullMjdTime();
   sciCalImagemetadata->setNullBjdTime();
   sciRawImagemetadata->WriteRow();
   sciCalImagemetadata->WriteRow();

   // the data classes have to be deleted to write the data into the FITS file.
   delete sciRawImagemetadata;
   delete sciCalImagemetadata;
}


BOOST_AUTO_TEST_SUITE_END()
