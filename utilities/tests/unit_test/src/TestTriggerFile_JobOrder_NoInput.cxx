/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the TriggerFile class
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 10.2   2018-10-11 ABE first version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"
#include <boost/filesystem.hpp>

#include <stdexcept>
#include <string>

#include "ProgramParams.hxx"

#include "TriggerFile.hxx"

using namespace boost::unit_test;

struct TriggerFileFixture {
  TriggerFileFixture() {}

   ~TriggerFileFixture() {}

   std::string m_outputPath = "results";
   std::string m_inputProcessorName = "test_input";
   std::string m_processorName = "example_processor";
   std::string m_dataName = "test";
   std::string m_inputTriggerFileName = "";

   PassId m_passId = PassId();
   PassId m_prevPassId = PassId();
   VisitId m_visitId = VisitId();
   UTC m_creationDate = UTC("2018-01-01T00:00:00");
   UTC m_validityStart = UTC("2020-01-01T01:01:01");
   UTC m_validityStop = UTC("2022-01-01T01:01:01");
   uint16_t m_inputProcessingNumber = 0;
   uint16_t m_outputProcessingNumber = 1;
   uint32_t m_obsid = 1;
};

BOOST_FIXTURE_TEST_SUITE( TestReadWriteTriggerFile_ProgramParams_NoTriggerFile, TriggerFileFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestReadWriteTriggerFile )
{
  ParamsPtr params = CheopsInit(framework::master_test_suite().argc,
                                framework::master_test_suite().argv);

  std::string processorName = params->GetAsString("Processor_Name");
  const std::list<std::string> triggerFiles = {};
  UTC jobOrderStartTime = params->GetAsString("Start");
  UTC jobOrderStopTime = params->GetAsString("Stop");

  BOOST_CHECK( TriggerFile::Empty() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), processorName );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputDirectory(), m_outputPath );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPrevPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitId().getFileNamePattern(), m_visitId.getFileNamePattern() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), jobOrderStartTime.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), jobOrderStopTime.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetCreationDate().getUtc(), UTC().getUtc());

  std::string outputTriggerFileName = m_outputPath + "/CH_TU2020-08-01T00-00-00_TRIGGER-" + processorName + "_V0000.xml";
  unlink(outputTriggerFileName.c_str());

  // Writes the output trigger file
  CheopsExit();
  BOOST_CHECK( boost::filesystem::exists(outputTriggerFileName) );

  // Check the values of the output file
  TriggerFile::ReadTriggerFile(outputTriggerFileName);
  BOOST_CHECK( !TriggerFile::Empty() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), processorName );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputDirectory(), m_outputPath );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPrevPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitId().getFileNamePattern(), m_visitId.getFileNamePattern() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), jobOrderStartTime.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), jobOrderStopTime.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), 0 );

  unlink(outputTriggerFileName.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
