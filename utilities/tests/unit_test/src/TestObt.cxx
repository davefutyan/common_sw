/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the OBT class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 4.3   2015-10-27 RRO #9541 synchronization bit changed from MSB to LSB
 *  @version 4.3   2015-10-27 RRO #9302 testing new operators
 *  @version 3.2   2015-04-03 RRO first version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <stdexcept>
#include <string>

#include "ProgramParams.hxx"
#include "Obt.hxx"
#include "Utc.hxx"
#include "DeltaTime.hxx"
#include "LeapSeconds.hxx"

using namespace boost::unit_test;

struct ObtFixture {
   ObtFixture() {
      m_params = CheopsInit(framework::master_test_suite().argc,
                            framework::master_test_suite().argv);

   }

   ~ObtFixture() {
   }

   ParamsPtr m_params;
};

BOOST_FIXTURE_TEST_SUITE( testObt, ObtFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( constructor )
{

//   OBT::SetResetCounterFileName("resources/CH_TU2010-01-01T00-00-00_REF_APP_ObtReset_V001.fits");
//   std::list< std::string > leapSeconds = {"resources/CH_TU1972-01-01T00-00-00_REF_APP_LeapSeconds_V001.fits"};
//  LeapSeconds::setLeapSecondsFileNames(leapSeconds);

   OBT obt1;
   // using the address operator
   BOOST_CHECK_EQUAL(*&obt1, 0);

   OBT obt2(20, true);
   // using the getObt() method
   BOOST_CHECK_EQUAL(obt2.getObt(), 21);

   OBT obt3(20, false);
   // using the cast operator
   BOOST_CHECK_EQUAL((int64_t)obt3, 20);

   // obt4 and obt5 are identical, but should have different reset counter
   OBT obt4(4000000, UTC(2015, 1, 1, 10, 20, 30));
   BOOST_CHECK_EQUAL(obt4.getResetCounter(), 0);

   OBT obt5(4000000, UTC(2025, 1, 1, 10, 20, 30));
   BOOST_CHECK_EQUAL(obt5.getResetCounter(), 1);
   BOOST_CHECK_EQUAL(obt5.getObt(), 4000000 + 0x1000000000000LL);

   // a OBT just before a reset, but the UTC is already after the reset
   OBT obt6(1000000000000LL - 10000, UTC(2020, 1, 1, 18, 23, 54));
   BOOST_CHECK_EQUAL(obt6.getResetCounter(), 0);

   // a OBT just after a reset, but the UTC is still before the reset
   OBT obt7(1000000 + 1000, UTC(2019, 12, 30, 18, 23, 54));
   BOOST_CHECK_EQUAL(obt7.getResetCounter(), 1);


}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( comparison )
{
   OBT obt1(102);
   OBT obt2(100, true);
   OBT obt3(100);

   BOOST_CHECK_EQUAL(obt2 < obt1, true);

   BOOST_CHECK_EQUAL(obt2 == obt3, false);

   BOOST_CHECK_EQUAL(obt1.isSynchronized(), false);
   BOOST_CHECK_EQUAL(obt2.isSynchronized(), true);

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( deltaTime )
{
   OBT obt(0x50000, true);     // 5 sec + synchronization bit.
   DeltaTime delta(1);   // 1 sec

   BOOST_CHECK_EQUAL((obt + delta).getObt(), 0x60001);
   BOOST_CHECK_EQUAL((obt - delta).getObt(), 0x40001);

   obt += delta;
   BOOST_CHECK_EQUAL(obt.getObt(), 0x60001);

   obt -= delta;
   BOOST_CHECK_EQUAL(obt.getObt(), 0x50001);

   // difference of two OBTs which belong to different OBT resets
   OBT obt1(0);
   OBT obt2(0x1000000000100LL);

   DeltaTime obtDiff = obt2 - obt1;

   // the OBT difference between the two OBT resets are
   // 1000000000000 = 15258789.0625 seconds
   // 0x100 OBT = 256 OBT = 0.00390625 sec
   // 15258789.0625 + 0.00390625 = 15258789.06640625
   BOOST_CHECK_EQUAL(obtDiff.getSeconds(), 15258789.06640625);

   OBT obtPassDown(2000000, UTC(2020, 4, 4, 0, 0, 0));
   DeltaTime deltaPass(double(1500000) / double(0x10000));

   obtPassDown -= deltaPass;
   BOOST_CHECK_EQUAL(obtPassDown.getObt(), 1000000500000LL);

   OBT obtPassUp(1000000700000);
   obtPassUp += deltaPass;
   BOOST_CHECK_EQUAL(obtPassUp.getObt(), 2200000 + 0x1000000000000);


}


BOOST_AUTO_TEST_SUITE_END()
