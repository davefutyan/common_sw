/*
 * test_parameters.cxx
 *
 *  Created on: May 8, 2014
 *      Author: rohlfs
 */

#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>

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

BOOST_FIXTURE_TEST_SUITE( testJobOrder, ParamsFixture )


////////////////////////////////////////////////////////////////////////////////
// Get the input data structures and file names
BOOST_AUTO_TEST_CASE( InputFiles )
{

   const std::set<std::string> inputStructNames = m_params->GetInputStructNames();
   BOOST_CHECK_EQUAL(inputStructNames.size(), 3);

   BOOST_CHECK_EQUAL(inputStructNames.count("SCI_PRW_FullArray"), 1);
   BOOST_CHECK_EQUAL(inputStructNames.count("SCI_PRW_SubArray"), 1);
   BOOST_CHECK_EQUAL(inputStructNames.count("XML"), 1);

   const std::list<std::string> & sciPrwFullArrays = m_params->GetInputFiles("SCI_PRW_FullArray");
   BOOST_CHECK_EQUAL(sciPrwFullArrays.size(), 10);
   BOOST_CHECK_EQUAL(sciPrwFullArrays.front(), "resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000000_V000.fits[SCI_PRW_FullArray]");
   BOOST_CHECK_EQUAL(sciPrwFullArrays.back(),  "resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000009_V000.fits[SCI_PRW_FullArray]");

   // Check that files that are not fits files does not have an extension name
   const std::list<std::string> & xmlFiles = m_params->GetInputFiles("XML");
   BOOST_CHECK_EQUAL(xmlFiles.size(), 1);
   BOOST_CHECK_EQUAL(xmlFiles.front(), "resources/CH_PR990001_TG000001_TU2018-01-01T12-00-30_Metadata_V000.xml");

   // Check that an empty list is returned for structs that have no files
   BOOST_CHECK_NO_THROW(m_params->GetInputFiles("SCI_PRW_SubArray"));
   const std::list<std::string> & sciPrwSubArrays = m_params->GetInputFiles("SCI_PRW_SubArray");
   BOOST_CHECK_EQUAL(sciPrwSubArrays.size(), 0);

   BOOST_CHECK_THROW(m_params->GetInputFiles("DOES_NOT_EXIST"), std::runtime_error);
}

////////////////////////////////////////////////////////////////////////////////
// Get the output directories
BOOST_AUTO_TEST_CASE( OutputDirs )
{
   BOOST_CHECK_EQUAL(m_params->GetOutDir(), "results");
   BOOST_CHECK_EQUAL(m_params->GetTmpDir(), "results");
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type string
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsString )
{
   BOOST_CHECK_EQUAL( m_params->GetAsString("Processor_Name"), "example_ipf" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Stdout_Log_Level"), "INFO" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Stderr_Log_Level"), "ERROR" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Test"), "1" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Breakpoint_Enable"), "0" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Processing_Station"), "TEST" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Start"), "2014-08-01T00:00:00.000000" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Stop"), "2014-08-02T00:00:00.000000" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Configuration_Baseline"), "000000" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Version"), "01.00" );
   BOOST_CHECK_EQUAL( m_params->GetAsString("Configuration_Dir"), "resources/config" );

   BOOST_CHECK_THROW( m_params->GetAsString("Does_not_exist"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type int
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsInt )
{
   BOOST_CHECK_EQUAL( m_params->GetAsInt("Int_Value"), 2 );
   BOOST_CHECK_THROW( m_params->GetAsInt("Int_Value_Out_Of_Range"), std::runtime_error );
   BOOST_CHECK_THROW( m_params->GetAsInt("Log_File"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type double
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsDouble )
{
   BOOST_CHECK_EQUAL( m_params->GetAsDouble("Double_Value"), 1.5 );
   BOOST_CHECK_THROW( m_params->GetAsDouble("Log_File"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type double
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsBool )
{
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_Yes"), true );
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_No"), false );
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_On"), true );
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_Off"), false );
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_True"), true );
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_False"), false );
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_1"), true );
   BOOST_CHECK_EQUAL( m_params->GetAsBool("Bool_Value_0"), false );
   BOOST_CHECK_THROW( m_params->GetAsBool("Log_File"), std::runtime_error );
   BOOST_CHECK_THROW( m_params->GetAsBool("Double_Value"), std::runtime_error );
   BOOST_CHECK_THROW( m_params->GetAsBool("Int_Value"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type vector<string>
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsStringVector )
{
   std::vector<std::string> expected = {"This is", "a", "quoted string", "test"};
   std::vector<std::string> value = m_params->GetAsStringVector("String_Vector_Value_Space_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   value = m_params->GetAsStringVector("String_Vector_Value_Comma_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   expected = { m_params->GetAsStringVector("Log_File") };
   value = m_params->GetAsStringVector("Log_File");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   BOOST_CHECK_THROW( m_params->GetAsStringVector("String_Vector_Value_Invalid"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type vector<int>
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsIntVector )
{
   std::vector<int> expected = {1, 20000,  30};
   std::vector<int> value = m_params->GetAsIntVector("Int_Vector_Value_Space_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   value = m_params->GetAsIntVector("Int_Vector_Value_Comma_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   expected = { m_params->GetAsIntVector("Int_Value") };
   value = m_params->GetAsIntVector("Int_Value");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   BOOST_CHECK_THROW( m_params->GetAsIntVector("Log_File"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type vector<double>
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsDoubleVector )
{
   std::vector<double> expected = {1.5, 20000.5,  30.5};
   std::vector<double> value = m_params->GetAsDoubleVector("Double_Vector_Value_Space_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   value = m_params->GetAsDoubleVector("Double_Vector_Value_Comma_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   expected = { m_params->GetAsDoubleVector("Double_Value") };
   value = m_params->GetAsDoubleVector("Double_Value");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   BOOST_CHECK_THROW( m_params->GetAsDoubleVector("Log_File"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Get the job order parameters of type vector<bool>
BOOST_AUTO_TEST_CASE( JobOrderParams_GetAsBoolVector )
{
   std::vector<bool> expected = { true, false, true, false, true, false, true, false, };

   std::vector<bool> value = m_params->GetAsBoolVector("Bool_Vector_Value_Space_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   value = m_params->GetAsBoolVector("Bool_Vector_Value_Comma_Sep");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   expected = { m_params->GetAsBoolVector("Bool_Value_Yes") };
   value = m_params->GetAsBoolVector("Bool_Value_Yes");
   BOOST_CHECK_EQUAL_COLLECTIONS( value.begin(), value.end(), expected.begin(), expected.end() );

   BOOST_CHECK_THROW( m_params->GetAsBoolVector("Log_File"), std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
// Test CheopsExit
BOOST_AUTO_TEST_CASE( TestCheopsExit )
{

   CheopsExit(false);

}

BOOST_AUTO_TEST_SUITE_END()
