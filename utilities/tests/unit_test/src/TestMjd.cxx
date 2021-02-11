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

#include <stdexcept>
#include <string>

#include "ProgramParams.hxx"
#include "Mjd.hxx"
#include "Bjd.hxx"
#include "BarycentricOffset.hxx"

using namespace boost::unit_test;

struct MjdFixture {
   MjdFixture() {
      m_params = CheopsInit(framework::master_test_suite().argc,
                            framework::master_test_suite().argv);

      BarycentricOffset::setEphemerisFileName("resources/CH_TU1949-12-14T00-00-00_EXT_APP_DE1_V0000.fits");

   }

   ~MjdFixture() {
   }

   ParamsPtr m_params;
};

BOOST_FIXTURE_TEST_SUITE( testMjd, MjdFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( Address_Assignment )
{
   MJD mjd(3456.345);

   double * mjdPointer = &mjd;

   BOOST_CHECK_EQUAL(*mjdPointer, 3456.345);
   mjd = 45.4;

   BOOST_CHECK_EQUAL(*mjdPointer, 45.4);

   *mjdPointer = 104.64;
   BOOST_CHECK_EQUAL(mjd.getMjd(), 104.64);

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( MJD2BJD )
{
   BarycentricOffset barycentricOffset(34.5, -23.5);
   MJD mjd(58228.677873); //2018 Vernal equinox (2018-04-20 16:15:00)

   BJD bjd = mjd + barycentricOffset;

   BOOST_CHECK_CLOSE(bjd.getBjd(), 2458229.173172, 0.000001);

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( BJD2MJD )
{
   BarycentricOffset barycentricOffset(34.5, -23.5);
   BJD bjd(2458229.173172);

   MJD mjd = bjd - barycentricOffset;

   BOOST_CHECK_CLOSE(mjd.getMjd(), 58228.677873, 0.000001);

}

BOOST_AUTO_TEST_SUITE_END()
