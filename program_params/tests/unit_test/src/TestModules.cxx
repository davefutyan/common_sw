/*
 * test_parameters.cxx
 *
 *  Created on: May 8, 2014
 *      Author: rohlfs
 */

#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include "Logger.hxx"
#include "ProgramParams.hxx"

using namespace boost::unit_test;

struct ParamsFixture {
   ParamsFixture() {
      // Prevent duplication of log output
      boost::log::core::get()->remove_all_sinks();

      m_params = CheopsInit(framework::master_test_suite().argc,
                          framework::master_test_suite().argv);
   }

   ~ParamsFixture() {
   }

   ParamsPtr m_params;
};

BOOST_FIXTURE_TEST_SUITE( testAccessModules, ParamsFixture )


////////////////////////////////////////////////////////////////////////////////
// Asking for a parameters with their default values and original data type
BOOST_AUTO_TEST_CASE( VIA_MODUEL )
{

   const ModuleParams moduleParams = m_params->Module("TestModule");

   std::string testString = moduleParams.GetAsString("StringParam");
   int32_t     testInt    = moduleParams.GetAsInt   ("IntParam");
   double      testDouble = moduleParams.GetAsDouble("DoubleParam");
   bool        testBool   = moduleParams.GetAsBool  ("Level2.BoolParam");

   BOOST_CHECK_EQUAL(testString, "test string");
   BOOST_CHECK_EQUAL(testInt,    456);
   BOOST_CHECK_EQUAL(testDouble, -0.000165);
   BOOST_CHECK_EQUAL(testBool,   true);
}


////////////////////////////////////////////////////////////////////////////////
// Asking for a parameters with their default values and original data type
BOOST_AUTO_TEST_CASE( VIA_PARAMS )
{

   std::string testString = m_params->GetAsString("TestModule.StringParam");
   int32_t     testInt    = m_params->GetAsInt   ("TestModule.IntParam");
   double      testDouble = m_params->GetAsDouble("TestModule.DoubleParam");
   bool        testBool   = m_params->GetAsBool  ("TestModule.Level2.BoolParam");

   BOOST_CHECK_EQUAL(testString, "test string");
   BOOST_CHECK_EQUAL(testInt,    456);
   BOOST_CHECK_EQUAL(testDouble, -0.000165);
   BOOST_CHECK_EQUAL(testBool,   true);
}

BOOST_AUTO_TEST_SUITE_END()
