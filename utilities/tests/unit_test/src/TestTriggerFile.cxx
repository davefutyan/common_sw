/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the TriggerFile class
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 5.0   2016-01-21 ABE first version
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

BOOST_FIXTURE_TEST_SUITE( testTriggerFile, TriggerFileFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestReadTriggerFileFromPath )
{

  // Before any values have been set in the TiggerFile class
  BOOST_CHECK( TriggerFile::Empty() );
  BOOST_CHECK( !TriggerFile::GetVisitId().isValid() );

  // Write a TRIGGER file using default values only
  std::string defaultTriggerFileName = TriggerFile::WriteTriggerFile(
      m_outputPath,
      TriggerFile::GetProcessorName(),
      TriggerFile::GetPassId(),
      TriggerFile::GetVisitId(),
      TriggerFile::GetValidityStart(),
      TriggerFile::GetValidityStop(),
      TriggerFile::GetOutputProcessingNumber(),
      TriggerFile::GetOutputProcessingNumber(),
      TriggerFile::GetObsid());

  BOOST_CHECK_EQUAL( defaultTriggerFileName,
                     m_outputPath + "/CH_TU1970-01-01T00-00-00_TRIGGER_V0000.xml" );
  unlink(defaultTriggerFileName.c_str());

  // Read a trigger file by passing the path
  TriggerFile::SetProcessorName(m_processorName);
  TriggerFile::SetOutputDirectory(m_outputPath);
  TriggerFile::ReadTriggerFile(m_inputTriggerFileName);

  BOOST_CHECK( !TriggerFile::Empty() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), m_processorName );
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

  VisitId visitId(m_programmeType, m_programmeId, m_requestId, m_visitCounter);
  if (TriggerFile::GetVisitId() != visitId) {
    throw std::runtime_error(TriggerFile::GetVisitId().getFileNamePattern() + " != " + visitId.getFileNamePattern());
  }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestInitialize_NoTriggerFile )
{
  // Test using one trigger file
  const std::list<std::string> & triggerFiles = {};

  BOOST_CHECK_THROW(
      TriggerFile::Initialize(
          m_outputPath,
          m_processorName,
          triggerFiles),
      std::runtime_error );

  TriggerFile::Initialize(
      m_outputPath,
      m_processorName,
      m_validityStart,
      m_validityStart);

  BOOST_CHECK( TriggerFile::Empty() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), m_processorName );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputDirectory(), m_outputPath );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPrevPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeType(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeId(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetRequestId(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitCounter(), 1 );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetCreationDate().getUtc(), UTC().getUtc());

  std::string outputTriggerFileName = TriggerFile::WriteTriggerFile(
      m_outputPath,
      TriggerFile::GetProcessorName(),
      TriggerFile::GetPassId(),
      TriggerFile::GetVisitId(),
      TriggerFile::GetValidityStart(),
      TriggerFile::GetValidityStop(),
      TriggerFile::GetOutputProcessingNumber(),
      TriggerFile::GetOutputProcessingNumber(),
      TriggerFile::GetObsid(),
      m_dataName,
      TriggerFile::GetCreationDate(),
      TriggerFile::GetPrevPassId());

  BOOST_CHECK( boost::filesystem::exists(outputTriggerFileName) );

  // Check the values of the output file
  TriggerFile::ReadTriggerFile(outputTriggerFileName);
  BOOST_CHECK( !TriggerFile::Empty() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), m_processorName );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputDirectory(), m_outputPath );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPrevPassId(), (std::string)PassId() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeType(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeId(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetRequestId(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitCounter(), 1 );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), 0 );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), 0 );

  unlink(outputTriggerFileName.c_str());
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestReadWriteTriggerFile_OneTriggerFile )
{
  const std::list<std::string> & triggerFiles = { m_inputTriggerFileName };
  std::cout << "1\n";
  TriggerFile::Initialize(
      m_outputPath,
      m_processorName,
      triggerFiles);

  std::cout << "2\n";
  BOOST_CHECK( !TriggerFile::Empty() );
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), m_processorName );
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

  std::cout << "3\n";
  std::string outputTriggerFileName = TriggerFile::WriteTriggerFile(
      m_outputPath,
      TriggerFile::GetProcessorName(),
      TriggerFile::GetPassId(),
      TriggerFile::GetVisitId(),
      TriggerFile::GetValidityStart(),
      TriggerFile::GetValidityStop(),
      TriggerFile::GetOutputProcessingNumber(),
      TriggerFile::GetOutputProcessingNumber(),
      TriggerFile::GetObsid(),
      m_dataName,
      TriggerFile::GetCreationDate(),
      TriggerFile::GetPrevPassId());

  BOOST_CHECK( boost::filesystem::exists(outputTriggerFileName) );

  // Check the values of the output file
  TriggerFile::ReadTriggerFile(outputTriggerFileName);
  BOOST_CHECK_EQUAL( TriggerFile::GetProcessorName(), m_processorName );
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

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestReadWriteTriggerFile_TwoTriggerFiles )
{
  // Test using one trigger file
  const std::list<std::string> & triggerFiles = {
      m_inputTriggerFileName,
      m_outputPath + "/CH_PS18010112_TU2020-01-01T01-01-01_TRIGGER-" + m_processorName + "-nonexistant_V0000.xml" };

  BOOST_CHECK_THROW(
      TriggerFile::Initialize(
          m_outputPath,
          m_processorName,
          triggerFiles),
      std::runtime_error );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestWriteTriggerFileToUndefinedDirectory )
{
  std::string outDir = "";
  std::string expectedTriggerFileName = "./CH_PR100001_TG000101_PS18010112_TU2020-01-01T01-01-01_TRIGGER-" + m_processorName + "_V0000.xml";
  unlink(expectedTriggerFileName.c_str());

  std::string triggerFileName = TriggerFile::WriteTriggerFile(
      outDir,
      m_processorName,
      m_passId,
      m_visitId,
      m_validityStart, m_validityStop,
      m_inputProcessingNumber,
      m_outputProcessingNumber,
      m_obsid,
      "");

  BOOST_CHECK_EQUAL( triggerFileName, expectedTriggerFileName );
  BOOST_CHECK( boost::filesystem::exists(triggerFileName) );

  // Check the values of the output file
  TriggerFile::ReadTriggerFile(triggerFileName);
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)m_passId );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeType(), m_programmeType);
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeId(), m_programmeId);
  BOOST_CHECK_EQUAL( TriggerFile::GetRequestId(), m_requestId);
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitCounter(), m_visitCounter);
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), m_validityStop.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), m_inputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), m_outputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), m_obsid);

  unlink(triggerFileName.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// Test using an invalid visit ID created using the empty VisitId constructor
BOOST_AUTO_TEST_CASE( TestWriteTriggerFileInvalidVisitId )
{
  std::string expectedTriggerFileName = m_outputPath + "/CH_PS18010112_TU2020-01-01T01-01-01_TRIGGER-" + m_processorName + "_V0000.xml";
  unlink(expectedTriggerFileName.c_str());

  VisitId visitId;
  BOOST_CHECK(visitId.isValid() == false);

  std::string triggerFileName = TriggerFile::WriteTriggerFile(
      m_outputPath,
      m_processorName,
      m_passId,
      visitId,
      m_validityStart, m_validityStop,
      m_inputProcessingNumber,
      m_outputProcessingNumber,
      m_obsid,
      "");

  BOOST_CHECK_EQUAL( triggerFileName, expectedTriggerFileName );
  BOOST_CHECK( boost::filesystem::exists(triggerFileName) );

  // Check the values of the output file
  TriggerFile::ReadTriggerFile(triggerFileName);
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)m_passId );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeType(), 0);
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeId(), 0);
  BOOST_CHECK_EQUAL( TriggerFile::GetRequestId(), 0);
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitCounter(), 1);
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), m_validityStop.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), m_inputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), m_outputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), m_obsid);

  unlink(triggerFileName.c_str());

  //////////////////////////////////////////////////////////////////////////////
  // Test using an invalid visit ID created by passing an invalid visit id to
  // the VisitId constructor via WriteTriggerFile.

  unlink(expectedTriggerFileName.c_str());

  BOOST_CHECK(visitId.isValid() == false);

  triggerFileName = TriggerFile::WriteTriggerFile(
      m_outputPath,
      m_processorName,
      m_passId,
      1, 0, 0, 1,
	    m_validityStart.getUtc(false), m_validityStop.getUtc(false),
      m_inputProcessingNumber,
      m_outputProcessingNumber,
      m_obsid,
      "");

  BOOST_CHECK_EQUAL( triggerFileName, expectedTriggerFileName );
  BOOST_CHECK( boost::filesystem::exists(triggerFileName) );

  // Check the values of the output file
  TriggerFile::ReadTriggerFile(triggerFileName);
  BOOST_CHECK_EQUAL( (std::string)TriggerFile::GetPassId(), (std::string)m_passId );
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeType(), 0);
  BOOST_CHECK_EQUAL( TriggerFile::GetProgrammeId(), 0);
  BOOST_CHECK_EQUAL( TriggerFile::GetRequestId(), 0);
  BOOST_CHECK_EQUAL( TriggerFile::GetVisitCounter(), 1);
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStart().getUtc(), m_validityStart.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetValidityStop().getUtc(), m_validityStop.getUtc() );
  BOOST_CHECK_EQUAL( TriggerFile::GetInputProcessingNumber(), m_inputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetOutputProcessingNumber(), m_outputProcessingNumber );
  BOOST_CHECK_EQUAL( TriggerFile::GetObsid(), m_obsid);

  unlink(triggerFileName.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
