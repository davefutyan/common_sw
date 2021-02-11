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
    m_inputTriggerFileName1 = TriggerFile::WriteTriggerFile(
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

    m_inputTriggerFileName2 = TriggerFile::WriteTriggerFile(
        m_outputPath,
        m_processorName,
        m_passId, m_programmeType, m_programmeId, m_requestId,
        m_visitCounter + 1,
        m_validityStart, m_validityStop,
        m_inputProcessingNumber, m_outputProcessingNumber,
        m_obsid,
        "test_input",
        m_creationDate,
        m_prevPassId);
   }

   ~TriggerFileFixture() {
     unlink(m_inputTriggerFileName1.c_str());
     unlink(m_inputTriggerFileName2.c_str());
   }

   std::string m_outputPath = "results";
   std::string m_inputProcessorName = "test_input";
   std::string m_processorName = "example_processor";
   std::string m_dataName = "test";
   std::string m_inputTriggerFileName1 = "";
   std::string m_inputTriggerFileName2 = "";

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

BOOST_FIXTURE_TEST_SUITE( TestReadWriteTriggerFile_ProgramParams_TwoTriggerFiles, TriggerFileFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( TestReadWriteTriggerFile )
{
  BOOST_CHECK_THROW( CheopsInit(framework::master_test_suite().argc,
                                framework::master_test_suite().argv),
                     std::runtime_error );
}

BOOST_AUTO_TEST_SUITE_END()
