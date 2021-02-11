#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <string>
#include <stdexcept>


#include "ProgramParams.hxx"

#include "CreateFitsFile.hxx"
#include "SCI_RAW_SubArray.hxx"
#include "SCI_CAL_SubArray.hxx"

using namespace boost;
using namespace boost::unit_test;


/** ****************************************************************************
 *  @brief Required struct by the boost unit test system
 */
struct Fixture{
   Fixture() {}
   ~Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( TestTimeKeywords, Fixture )


BOOST_AUTO_TEST_CASE( Create )
{
   unlink("result/CH_PR201111_TG002004_TU2015-04-04T14-16-18_SCI_RAW_SubArray_V0000.fits");
   unlink("result/CH_PR201111_TG002004_TU2015-04-04T14-16-18_SCI_CAL_SubArray_V0000.fits");

   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   VisitId visitId(20, 1111, 20, 4);
   PassId  passId(2015, 6, 5, 16);
   UTC     utc("2015-04-04T14:16:18");

    SciRawSubarray * sciRawSubarray = createFitsFile<SciRawSubarray>("result",
                                      utc, visitId,  {1,2,3});
    SciCalSubarray * sciCalSubarray = createFitsFile<SciCalSubarray>("result",
                                      utc, visitId, {1,2,3});

    sciRawSubarray->setKeyTStrtO(15);
    sciRawSubarray->setKeyTStopO(123456789012);
   sciRawSubarray->setKeyTStopU(UTC("2015-04-06T13:58:40.502"));
   sciRawSubarray->setKeyTStopM(MJD(23473.1523));
   sciRawSubarray->setKeyPassId(passId);
   sciCalSubarray->setKeyTStopB(BJD(23473.1523));

   // the data classes have to be deleted to write the data into the FITS file.
   delete sciCalSubarray;
   delete sciRawSubarray;
}

BOOST_AUTO_TEST_CASE( Read )
{
   CheopsInit(framework::master_test_suite().argc,
              framework::master_test_suite().argv  );

   SciRawSubarray * sciRawSubarray = new SciRawSubarray("result/CH_PR201111_TG002004_TU2015-04-04T14-16-18_SCI_RAW_SubArray_V0000.fits");
   SciCalSubarray * sciCalSubarray = new SciCalSubarray("result/CH_PR201111_TG002004_TU2015-04-04T14-16-18_SCI_CAL_SubArray_V0000.fits");

   OBT tStrtO =  sciRawSubarray->getKeyTStrtO();
   OBT tStopO =  sciRawSubarray->getKeyTStopO();

   BOOST_CHECK_EQUAL((int64_t)tStrtO, 15);
   BOOST_CHECK_EQUAL((int64_t)tStopO, 123456789012);

   UTC tStrtU =  sciRawSubarray->getKeyTStrtU();
   UTC tStopU =  sciRawSubarray->getKeyTStopU();

   BOOST_CHECK_EQUAL(tStrtU.empty(), true);
   BOOST_CHECK_EQUAL((std::string)tStopU, std::string("2015-04-06T13:58:40.502000"));

   MJD tStrtM =  sciRawSubarray->getKeyTStrtM();
   MJD tStopM =  sciRawSubarray->getKeyTStopM();

   BOOST_CHECK_EQUAL((double)tStrtM, 0 );
   BOOST_CHECK_EQUAL((double)tStopM, 23473.1523);

   PassId passid = sciRawSubarray->getKeyPassId();
   BOOST_CHECK_EQUAL(passid.getYear(), 15 );
   BOOST_CHECK_EQUAL(passid.getMonth(), 6 );
   BOOST_CHECK_EQUAL(passid.getDay(),   5 );
   BOOST_CHECK_EQUAL(passid.getHour(), 16 );


   BJD tStrtB =  sciCalSubarray->getKeyTStrtB();
   BJD tStopB =  sciCalSubarray->getKeyTStopB();

   BOOST_CHECK_EQUAL((double)tStrtB, 0 );
   BOOST_CHECK_EQUAL((double)tStopB, 23473.1523);


   // the data classes have to be deleted to write the data into the FITS file.
   delete sciCalSubarray;
   delete sciRawSubarray;
}


BOOST_AUTO_TEST_SUITE_END()
