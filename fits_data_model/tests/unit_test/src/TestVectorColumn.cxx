#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"


#include <string>

#include "ProgramParams.hxx"

#include "../../../include/REF_APP_Limits.hxx"

using namespace boost;
using namespace boost::unit_test;


/** ****************************************************************************
 *  @brief Required struct by the boost unit test system
 */
struct Fixture{
   Fixture() {}
   ~Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( VectorColumneSuite, Fixture )


BOOST_AUTO_TEST_CASE( testVectorColumn )
{

   std::string fileName = "result/test_vector_column_REF_APP_Limits.fits";
   unlink(fileName.c_str());

   RefAppLimits * refAppLimits = new RefAppLimits(fileName, "CREATE");

   // Check that all elements in the vector column cell have NULL value
   for(int i = 0; i < refAppLimits->getSizeUpperLimit(); i++) {
     BOOST_CHECK( refAppLimits->isNullUpperLimit(i) );
   }

   std::vector<double> upperLimitVector = refAppLimits->getCellVectorUpperLimit();
   // Check that the returned vector has the correct size
   BOOST_CHECK_EQUAL( upperLimitVector.size(), refAppLimits->getSizeUpperLimit() );

   // Update using vector argument
   std::vector<double> upperLimitVectorNew = std::vector<double>(refAppLimits->getSizeUpperLimit(), 1);
   refAppLimits->setCellVectorUpperLimit(upperLimitVectorNew);
   upperLimitVector = refAppLimits->getCellVectorUpperLimit();
   BOOST_CHECK_EQUAL_COLLECTIONS(upperLimitVectorNew.begin(), upperLimitVectorNew.end(),
                                 upperLimitVector.begin(), upperLimitVector.end() );

   refAppLimits->WriteRow();

   // Add vector that is too short
   upperLimitVectorNew = std::vector<double>(refAppLimits->getSizeUpperLimit()-1, 2);
   refAppLimits->setCellVectorUpperLimit(upperLimitVectorNew);
   upperLimitVector = refAppLimits->getCellVectorUpperLimit();

   BOOST_CHECK_EQUAL_COLLECTIONS(upperLimitVectorNew.begin(), upperLimitVectorNew.end(),
                                 upperLimitVector.begin(), upperLimitVector.begin()+upperLimitVectorNew.size() );
   // Check that the last element has not changed value
   BOOST_CHECK_EQUAL( upperLimitVector[upperLimitVector.size()-1], 1 );
   refAppLimits->WriteRow();

   // Add vector that is too long
   upperLimitVectorNew = std::vector<double>(refAppLimits->getSizeUpperLimit()+1, 3);
   refAppLimits->setCellVectorUpperLimit(upperLimitVectorNew);
   upperLimitVector = refAppLimits->getCellVectorUpperLimit();

   BOOST_CHECK_EQUAL_COLLECTIONS(upperLimitVectorNew.begin(), upperLimitVectorNew.begin()+upperLimitVector.size(),
                                 upperLimitVector.begin(), upperLimitVector.end() );
   refAppLimits->WriteRow();

   delete refAppLimits;

}


BOOST_AUTO_TEST_SUITE_END()
