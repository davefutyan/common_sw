/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the UTC class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 5.1   2016-03-19 RRO the maximum year is now 2037
 *  @version 4.3   2015-10-27 RRO #9302 testing new operators
 *  @version 3.0   2015-01-24 RRO first version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include <stdexcept>
#include <string>

#include "ProgramParams.hxx"
#include "Utc.hxx"
#include "Mjd.hxx"
#include "DeltaTime.hxx"

using namespace boost::unit_test;

struct UtcFixture {
   UtcFixture() {
      m_params = CheopsInit(framework::master_test_suite().argc,
                            framework::master_test_suite().argv);

   }

   ~UtcFixture() {
   }

   ParamsPtr m_params;
};

BOOST_FIXTURE_TEST_SUITE( testUtc, UtcFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( NoSecFraction )
{
   // this should work
   UTC utc(2015, 01, 24, 13, 4, 3);
   BOOST_CHECK_EQUAL(utc.getUtc(), std::string("2015-01-24T13:04:03.000000"));


}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( WithSecFraction )
{
   UTC utc(2015, 01, 24, 13, 4, 3, 0.03);

   BOOST_CHECK_EQUAL(utc.getUtc(), std::string("2015-01-24T13:04:03.030000"));

 }

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( Wrong_Input_String )
{
   bool test;

   // wrong year
   test = false;
   try {
      UTC utc("15-3-12T23:04:23");
   }
   catch (std::runtime_error & error) {
      BOOST_CHECK_EQUAL(error.what(),
            std::string("The format of the UTC as string is not as expected."
                  "Expected: yyyy-mm-ddThh:mm:ss.ffffff, but found: 15-3-12T23:04:23") );
      test = true;
   }

   BOOST_CHECK_EQUAL(true, test);

   // wrong separator
   test = false;
   try {
      UTC utc("2015-03+12T23:04:23");
   }
   catch (std::runtime_error & error) {
      BOOST_CHECK_EQUAL(error.what(),
            std::string("The format of the UTC as string is not as expected."
                  "Expected: yyyy-mm-ddThh:mm:ss.ffffff, but found: 2015-03+12T23:04:23") );
      test = true;
   }

   BOOST_CHECK_EQUAL(true, test);

   // wrong year
   test = false;
  try {
       UTC utc2(215, 01, 24, 13, 4, 3);
    }
    catch (std::runtime_error & error) {
       BOOST_CHECK_EQUAL(error.what(),
             std::string("The year of a utc has to be in the range from 1970 to 2037."
                         " UTC - constructor found 215") );
      test = true;
   }
   BOOST_CHECK_EQUAL(true, test);

   // wrong month
   test = false;
   try {
      UTC utc2(2015, 00, 24, 13, 4, 3);
   }
   catch (std::runtime_error & error) {
      BOOST_CHECK_EQUAL(error.what(),
            std::string("The month of a utc has to be in the range from 1 to 12."
                        " UTC - constructor found 0") );
      test = true;
   }
   BOOST_CHECK_EQUAL(true, test);

   // wrong day
   test = false;
   try {
      UTC utc2(2015, 02, 30, 13, 4, 3);
   }
   catch (std::runtime_error & error) {
      BOOST_CHECK_EQUAL(error.what(),
            std::string("The day of a utc for month February has to be in the range from 1 to 29."
                        " UTC - constructor found 30") );
      test = true;
   }
   BOOST_CHECK_EQUAL(true, test);

   // wrong hour
   test = false;
   try {
      UTC utc2(2015, 01, 24, 24, 4, 3);
   }
   catch (std::runtime_error & error) {
      BOOST_CHECK_EQUAL(error.what(),
            std::string("The hour of a utc has to be in the range from 0 to 23."
                        " UTC - constructor found 24") );
      test = true;
   }
   BOOST_CHECK_EQUAL(true, test);

   // wrong minute
   test = false;
   try {
       UTC utc2(2015, 01, 24, 13, 70, 3);
    }
    catch (std::runtime_error & error) {
       BOOST_CHECK_EQUAL(error.what(),
             std::string("The minute of a utc has to be in the range from 0 to 59."
                         " UTC - constructor found 70") );
      test = true;
   }
   BOOST_CHECK_EQUAL(true, test);

   // wrong second
   test = false;
   try {
       UTC utc2(2015, 01, 24, 13, 7, 63);
    }
    catch (std::runtime_error & error) {
       BOOST_CHECK_EQUAL(error.what(),
             std::string("The second of a utc has to be in the range from 0 to 60."
                         " UTC - constructor found 63") );
      test = true;
   }
   BOOST_CHECK_EQUAL(true, test);

   // wrong fraction of a second
   test = false;
  try {
       UTC utc2(2015, 01, 24, 13, 4, 3, 1.4);
    }
    catch (std::runtime_error & error) {
       BOOST_CHECK_EQUAL(error.what(),
             std::string("The fraction of a second of a utc has to be less than 1.0."
                         " UTC - constructor found 1.400000") );
      test = true;
   }
   BOOST_CHECK_EQUAL(true, test);
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( Operators )
{
   UTC utc1("2015-04-06T13:02:17");
   UTC utc2("2015-04-06T13:02:17.040000");

   BOOST_CHECK_EQUAL(utc1 < utc2, true);
   BOOST_CHECK_EQUAL(utc2.getUtc(true), std::string("2015-04-06T13:02:17"));
   BOOST_CHECK_EQUAL(utc1.getUtc(),     std::string("2015-04-06T13:02:17.000000"));

   BOOST_CHECK_EQUAL(utc2.getFileNamePattern(), std::string("TU2015-04-06T13-02-17"));

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( Single_Item )
{
   UTC utc("2015-04-06T13:02:17.040");

   BOOST_CHECK_EQUAL(utc.getYear(),        2015);
   BOOST_CHECK_EQUAL(utc.getMonth(),       4);
   BOOST_CHECK_EQUAL(utc.getDay(),         6);
   BOOST_CHECK_EQUAL(utc.getHour(),        13);
   BOOST_CHECK_EQUAL(utc.getMinute(),      2);
   BOOST_CHECK_EQUAL(utc.getSecond(),      17);
   BOOST_CHECK_EQUAL(utc.getSecFraction(), 0.04);
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( deltaTime )
{
   UTC utc("2015-04-06T13:02:17.040");
   DeltaTime delta(61.5);   // 1 minute 1 sec 0.5 sec

   BOOST_CHECK_EQUAL((utc + delta).getUtc(), std::string("2015-04-06T13:03:18.540000"));
   BOOST_CHECK_EQUAL((utc - delta).getUtc(), std::string("2015-04-06T13:01:15.540000"));

   utc += delta;
   BOOST_CHECK_EQUAL(utc.getUtc(), std::string("2015-04-06T13:03:18.540000"));

   utc -= delta;
   BOOST_CHECK_EQUAL(utc.getUtc(), std::string("2015-04-06T13:02:17.040000"));

}
////////////////////////////////////////////////////////////////////////////////
//BOOST_AUTO_TEST_CASE( Utc2Mjd )
//{
//   UTC utc1("2010-01-01T00:00:00.000");
//   BOOST_CHECK_CLOSE((double)(utc1.getMjd()), 55197.0003725, 0.0000001);
//
//   UTC utc2("2010-01-01T12:00:00.000");
//   BOOST_CHECK_CLOSE((double)(utc2.getMjd()), 55197.5003725, 0.0000001);
//
//   UTC utc3("2010-01-01T00:01:00.000");
//   BOOST_CHECK_CLOSE((double)(utc3.getMjd()), 55197.001067, 0.0000001);
//
//}


BOOST_AUTO_TEST_SUITE_END()
