/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the PassId class
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 6.0   2016-07-01 ABE first version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include <stdexcept>
#include <string>

#include "ProgramParams.hxx"
#include "PassId.hxx"

using namespace boost::unit_test;

struct PassIdFixture {

  PassIdFixture() {}

   ~PassIdFixture() {}
};

BOOST_FIXTURE_TEST_SUITE( testPassId, PassIdFixture )

BOOST_AUTO_TEST_CASE( testPassId )
{

  // Create an empty pass id
  PassId emptyPassId;
  BOOST_CHECK( !emptyPassId.isValid());
  BOOST_CHECK_EQUAL( std::string(emptyPassId), "" );
  BOOST_CHECK_EQUAL( emptyPassId.getYear(), 10 );
  BOOST_CHECK_EQUAL( emptyPassId.getMonth(), 1 );
  BOOST_CHECK_EQUAL( emptyPassId.getDay(), 1 );
  BOOST_CHECK_EQUAL( emptyPassId.getHour(), 0 );

  // Copy an empty pass id
  PassId emptyPassIdCopy(emptyPassId);
  BOOST_CHECK( !emptyPassIdCopy.isValid());
  BOOST_CHECK_EQUAL( std::string(emptyPassIdCopy), "" );
  BOOST_CHECK_EQUAL( emptyPassIdCopy.getYear(), 10 );
  BOOST_CHECK_EQUAL( emptyPassIdCopy.getMonth(), 1 );
  BOOST_CHECK_EQUAL( emptyPassIdCopy.getDay(), 1 );
  BOOST_CHECK_EQUAL( emptyPassIdCopy.getHour(), 0 );


  // Create pass id with valid integer constructor arguments
  uint8_t year = 20;
  uint8_t month = 2;
  uint8_t day = 2;
  uint8_t hour = 2;

  PassId validPassId(year, month, day, hour);
  BOOST_CHECK( validPassId.isValid());
  BOOST_CHECK_EQUAL( std::string(validPassId), "20020202" );
  BOOST_CHECK_EQUAL( validPassId.getYear(), year );
  BOOST_CHECK_EQUAL( validPassId.getMonth(), month );
  BOOST_CHECK_EQUAL( validPassId.getDay(), day );
  BOOST_CHECK_EQUAL( validPassId.getHour(), hour );

  // Copy pass id with valid integer constructor arguments
  PassId validCopy(validPassId);
  BOOST_CHECK( validCopy.isValid());
  BOOST_CHECK_EQUAL( std::string(validCopy), "20020202" );
  BOOST_CHECK_EQUAL( validCopy.getYear(), year );
  BOOST_CHECK_EQUAL( validCopy.getMonth(), month );
  BOOST_CHECK_EQUAL( validCopy.getDay(), day );
  BOOST_CHECK_EQUAL( validCopy.getHour(), hour );

  // Create invalid pass id with empty string constructor argument
  PassId invalidStringPassId("");
  BOOST_CHECK( !invalidStringPassId.isValid());
  BOOST_CHECK_EQUAL( std::string(invalidStringPassId), "" );
  BOOST_CHECK_EQUAL( invalidStringPassId.getYear(), 10 );
  BOOST_CHECK_EQUAL( invalidStringPassId.getMonth(), 1 );
  BOOST_CHECK_EQUAL( invalidStringPassId.getDay(), 1 );
  BOOST_CHECK_EQUAL( invalidStringPassId.getHour(), 0 );

  // Create invalid pass id with invalid string constructor argument
  PassId invalidStringPassId2("test");
  BOOST_CHECK( !invalidStringPassId2.isValid());
  BOOST_CHECK_EQUAL( std::string(invalidStringPassId2), "" );
  BOOST_CHECK_EQUAL( invalidStringPassId2.getYear(), 10 );
  BOOST_CHECK_EQUAL( invalidStringPassId2.getMonth(), 1 );
  BOOST_CHECK_EQUAL( invalidStringPassId2.getDay(), 1 );
  BOOST_CHECK_EQUAL( invalidStringPassId2.getHour(), 0 );

  // Create valid pass id with valid string constructor argument
  PassId validStringPassId("20020202");
  BOOST_CHECK( validStringPassId.isValid());
  BOOST_CHECK_EQUAL( std::string(validStringPassId), "20020202" );
  BOOST_CHECK_EQUAL( validStringPassId.getYear(), year );
  BOOST_CHECK_EQUAL( validStringPassId.getMonth(), month );
  BOOST_CHECK_EQUAL( validStringPassId.getDay(), day );
  BOOST_CHECK_EQUAL( validStringPassId.getHour(), hour );

}

BOOST_AUTO_TEST_CASE( testEqualOperator )
{

  PassId p1;
  PassId p2;
  BOOST_CHECK( p1 == p1 );
  BOOST_CHECK( p1 == p2 );

  p1 = PassId("20010100");
  BOOST_CHECK( p1 != p2 );

  p2 = PassId("20010100");
  BOOST_CHECK( p1 == p2 );

  p2 = PassId("20010101");
  BOOST_CHECK( p1 != p2 );
}

BOOST_AUTO_TEST_SUITE_END()
