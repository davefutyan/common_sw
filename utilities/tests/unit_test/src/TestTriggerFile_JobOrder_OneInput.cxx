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
  TriggerFileFixture() {
    m_inputTriggerFileName = TriggerFile::WriteTriggerFile(
        m_outputPath,
        m_processorName,
        m_passId, m_programmeType, m_programmeId, m_requestId,
        m_visitCounter,
        m_validityStart, m_validityStop,
        m_inputProcessingNumber, m_outputProcessingNumber,
        m_obsid,
        "test_input",
        m_creationDate,
        m_prevPassId);
   }

   ~TriggerFileFixture() {
     unlink(m_inputTriggerFileName.c_str());
   }

   std::string m_outputPath = "results";
   std::string m_inputProcessorName = "test_input";
   std::string m_processorName = "example_processor";
   std::string m_dataName = "test";
   std::string m_inputTriggerFileName = "";

   PassId m_passId = PassId("18010112");
   PassId m_prevPassId = PassId("18010106");
   uint8_t m_programmeType = 10;
   uint16_t m_programmeId = 1;
   uint16_t m_requestId = 1;
   uint8_t m_visitCounter = 1;
   VisitId m_visitId = VisitId(m_programmeType, m_programmeId, m_requestId, m_visitCounter);
   UTC m_creationDate = UTC("2018-01-01T00:00:00");
   UTC m_validityStart = UTC("2020-01-01T01:01:01");
   UTC m_validityStop = UTC("2022-01-01T01:01:01");
   uint16_t m_inputProcessingNumber = 0;
   uint16_t m_outputProcessingNumber = 1;
   uint32_t m_obsid = 1;
};

BOOST_FIXTURE_TEST_SUITE( TestReadWriteTriggerFile_ProgramParams_OneTriggerFile, TriggerFileFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestReadWriteTriggerFile )
{
  ParamsPtr params = CheopsInit(framework::master_test_suite().argc,
                                framework::master_test_suite().argv);

  std::string processorName = params->GetAsString("Processor_Name");

  BOOST_CHECK( !TriggerFile::Empty() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), processorName );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputDirectory(), m_outputPath );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)m_passId );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPrevPassId(), (std::string)m_prevPassId );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeType(), m_programmeType);
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeId(), m_programmeId);
  BOOST_CHECK_EQUAL( TriggerFile::GetRequestId(), m_requestId);
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitCounter(), m_visitCounter);
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), m_validityStop.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), m_inputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), m_outputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), m_obsid);
  BOOST_CHECK_EQUAL( TriggerFile::GetCreationDate().getUtc(), m_creationDate.getUtc());

  std::string outputTriggerFileName = m_outputPath + "/CH_PR100001_TG000101_PS18010112_TU2020-01-01T01-01-01_TRIGGER-" + processorName + "_V0001.xml";
  unlink(outputTriggerFileName.c_str());

  // Writes the output trigger file
  CheopsExit();
  BOOST_CHECK( boost::filesystem::exists(outputTriggerFileName) );

  // Check the values of the output file
  TriggerFile::ReadTriggerFile(outputTriggerFileName);
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), processorName );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputDirectory(), m_outputPath );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)m_passId );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPrevPassId(), (std::string)m_prevPassId );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeType(), m_programmeType);
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeId(), m_programmeId);
  BOOST_CHECK_EQUAL( TriggerFile::GetRequestId(), m_requestId);
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitCounter(), m_visitCounter);
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), m_validityStop.getUtc() );
  // Check that both the input and output processing number of the output
  // TRIGGER file equals the output processing number of the input TRIGGER file
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), m_outputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), m_outputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), m_obsid);
  BOOST_CHECK_EQUAL( TriggerFile::GetCreationDate().getUtc(), m_creationDate.getUtc());

  unlink(outputTriggerFileName.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
