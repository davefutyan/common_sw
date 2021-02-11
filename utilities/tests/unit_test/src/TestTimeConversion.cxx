/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the UTC class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 3.0   2015-01-24 RRO first version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include <stdexcept>
#include <string>

#include "ProgramParams.hxx"
#include "DeltaTime.hxx"
#include "Obt.hxx"
#include "Utc.hxx"
#include "Mjd.hxx"

using namespace boost::unit_test;

struct TimeConversionFixture {
   TimeConversionFixture() {
      m_params = CheopsInit(framework::master_test_suite().argc,
                            framework::master_test_suite().argv);

   }

   ~TimeConversionFixture() {
   }

   ParamsPtr m_params;
};

BOOST_FIXTURE_TEST_SUITE( testTimeConverson, TimeConversionFixture )

BOOST_AUTO_TEST_CASE( year1970 )
{
   // the DeltaTime is the number of seconds between 1970 and 2000,
   // including leap seconds
   UTC utc = UTC("1970-01-01T00:00:00.000000") + DeltaTime(946684864.184);

   BOOST_CHECK_EQUAL(utc.getUtc().c_str(), "2000-01-01T00:00:00.000000");


}
////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( OBT2UTC )
{
   OBT obt(1000001000000-1);
   UTC utc = obt.getUtc();

   BOOST_CHECK_EQUAL(utc.getUtc().c_str(), "2019-12-31T23:59:49.999959");

   // a conversion after a OBT clock reset
   OBT obt1(2000000 + 0x1000000000000);
   UTC utc1 = obt1.getUtc();

   BOOST_CHECK_EQUAL(utc1.getUtc().c_str(), "2020-01-01T00:00:15.258788");

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( UTC2OBT )
{
   UTC utc("2019-12-31T23:59:49.999959");
   OBT obt = utc.getObt();

   BOOST_CHECK_EQUAL(obt.getObt(), 1000000999998);

   // a conversion after a OBT clock reset
   UTC utc1("2020-01-01T00:00:15.258788");
   OBT obt1 = utc1.getObt();

   BOOST_CHECK_EQUAL(obt1.getObt(), 281474978710656);

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( UTC2MJd )
{
   UTC utc("2015-05-18T00:00:00.003052");
   MJD mjd = utc.getMjd();

   BOOST_CHECK_EQUAL(mjd.getMjd(), 57160.000777627916);
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( MJD2UTC )
{

   MJD mjd(57160.000777627916);
   UTC utc = mjd.getUtc();

   BOOST_CHECK_EQUAL(utc.getUtc().c_str(), "2015-05-18T00:00:00.003052");
}


BOOST_AUTO_TEST_SUITE_END()
