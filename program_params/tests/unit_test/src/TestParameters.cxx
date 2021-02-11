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

BOOST_FIXTURE_TEST_SUITE( testAccessParameters, ParamsFixture )

////////////////////////////////////////////////////////////////////////////////
// Asking for a parameter, which is not defined in the parameter file
// shall throw an exception.
BOOST_AUTO_TEST_CASE( NotDefinedParameter )
{
   BOOST_CHECK_THROW(m_params->GetAsInt("NotDefinedParameter"), std::exception);
}

////////////////////////////////////////////////////////////////////////////////
// Asking for a parameter, for which no value is defined
// shall throw an exception.
BOOST_AUTO_TEST_CASE( ParameterWithoutValue )
{
   try {
      m_params->GetAsString("MinDefParam");
   }
   catch (std::exception & e) {
      BOOST_CHECK_EQUAL(e.what(),
                        "Parameter [MinDefParam] itself or its value is not defined.");
      return;
    }

    // this line should never be executed
    BOOST_CHECK(0);
}

////////////////////////////////////////////////////////////////////////////////
// Asking for a parameter, for which the value is defined only on the
// command line
BOOST_AUTO_TEST_CASE( CommandLineValue )
{
   std::string commandLine = m_params->GetAsString("CommandLine");

   BOOST_CHECK_EQUAL(commandLine, "CommandLineValue");

}

////////////////////////////////////////////////////////////////////////////////
// Asking for a parameters with their default values and original data type
BOOST_AUTO_TEST_CASE( DefaultValues )
{
   std::string testString = m_params->GetAsString("StringParam");
   int32_t     testInt    = m_params->GetAsInt   ("IntParam");
   double      testDouble = m_params->GetAsDouble("DoubleParam");
   bool        testBool   = m_params->GetAsBool  ("BoolParam");

   BOOST_CHECK_EQUAL(testString, "test string");
   BOOST_CHECK_EQUAL(testInt,    456);
   BOOST_CHECK_EQUAL(testDouble, -0.000165);
   BOOST_CHECK_EQUAL(testBool, true);

   std::vector<std::string> testStringVector = m_params->GetAsStringVector("StringVectorParam");
   std::vector<int>         testIntVector    = m_params->GetAsIntVector   ("IntVectorParam");
   std::vector<double>      testDoubleVector = m_params->GetAsDoubleVector("DoubleVectorParam");
   std::vector<bool>        testBoolVector   = m_params->GetAsBoolVector  ("BoolVectorParam");

   BOOST_CHECK_EQUAL(testStringVector[0], "test string");

   BOOST_CHECK_EQUAL(testIntVector[0], 456);

}

////////////////////////////////////////////////////////////////////////////////
// Asking for a parameters with their default values but for a different
// data type
BOOST_AUTO_TEST_CASE( TypeConversion )
{
   try {
      m_params->GetAsDouble("IntParam");
   }
   catch (std::exception & e) {
      BOOST_CHECK_EQUAL(e.what(),
                        "Parameter [IntParam] cannot be read 'AsDouble'.");
      return;
    }

 // this line should never be executed
 BOOST_CHECK(0);
}


BOOST_AUTO_TEST_SUITE_END()
