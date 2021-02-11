/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the VisitId class
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 8.2   2017-11-07 ABE first version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include <stdexcept>
#include <string>

#include "ProgramParams.hxx"
#include "VisitId.hxx"

using namespace boost::unit_test;

struct VisitIdFixture {

  VisitIdFixture() {}

   ~VisitIdFixture() {}
};

BOOST_FIXTURE_TEST_SUITE( testVisitId, VisitIdFixture )

BOOST_AUTO_TEST_CASE( testVisitId )
{

  // Create an invalid visit id
  VisitId invalidVisitId;
  BOOST_CHECK( !invalidVisitId.isValid());
  BOOST_CHECK_EQUAL( invalidVisitId.getProgramType(), 0 );
  BOOST_CHECK_EQUAL( invalidVisitId.getProgramId(), 0 );
  BOOST_CHECK_EQUAL( invalidVisitId.getRequestId(), 0 );
  BOOST_CHECK_EQUAL( invalidVisitId.getVisitCounter(), 1 );

  // Passing an invalid programme type shall fail
  BOOST_CHECK_THROW( VisitId(1, 0, 0, 1), std::runtime_error );
  BOOST_CHECK_THROW( VisitId(9, 0, 0, 1), std::runtime_error );
  BOOST_CHECK_THROW( VisitId(51, 0, 0, 1), std::runtime_error );

  // Create a valid visit id
  VisitId validVisitId(10, 0, 0, 1);
  BOOST_CHECK( validVisitId.isValid() );
  BOOST_CHECK_EQUAL( validVisitId.getProgramType(), 10 );
  BOOST_CHECK_EQUAL( validVisitId.getProgramId(), 0 );
  BOOST_CHECK_EQUAL( validVisitId.getRequestId(), 0 );
  BOOST_CHECK_EQUAL( validVisitId.getVisitCounter(), 1 );

  // Create a visit id by passing a string to the constructor
  BOOST_CHECK( VisitId(validVisitId.getFileNamePattern()) == validVisitId );

  // Compare visit ids
  BOOST_CHECK( VisitId() == VisitId() );
  BOOST_CHECK( VisitId() < VisitId(10, 0, 0, 1) );
  BOOST_CHECK( VisitId() != VisitId(10, 0, 0, 1) );

  BOOST_CHECK( VisitId(10, 0, 0, 1) == VisitId(10, 0, 0, 1) );
  BOOST_CHECK( VisitId(10, 0, 0, 1) < VisitId(20, 0, 0, 1) );
  BOOST_CHECK( VisitId(10, 0, 0, 1) != VisitId(20, 0, 0, 1) );

}


BOOST_AUTO_TEST_SUITE_END()
