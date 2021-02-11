/** ****************************************************************************
 *  @file
 *
 *  @ingroup Logger
 *  @brief Unit test for logger alerts.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 6.2 2016-09-02 ABE first version.
 */

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "Logger.hxx"

using namespace boost::unit_test;


struct LoggerFixture {

   LoggerFixture() {

     // Prevent duplication of log output
     boost::log::core::get()->remove_all_sinks();

	   // Initialize the Logger as it is done in the auto-generated
     // CheopsInit(...) method.
     Logger::SetProgramVersion(m_programName, m_programVersion);
     m_logFile = Logger::Configure(
         "DEBUG",
         "DEBUG",
         m_emailAddress,
         m_outDir,
         "",
         m_revisionNumber,
         m_processingNumber,
         m_visitId,
         m_passId,
         m_obsid,
         m_processingChain,
         m_targetName);
   }

   ~LoggerFixture() {
     unlink(m_logFile.c_str());
   }

   // This variable shall be set to a real email address to perform test
   // specification CHEOPS-SOC-TC-WP05-FUNC-007 Alert API
   std::string m_emailAddress = "";

   std::string m_outDir = "results";
   std::string m_logFile;

   std::string m_programName = "TestCxxEmailAlert";
   std::string m_programVersion = "1.0";

   std::string m_visitId = "PR100001_TG045401";
   uint32_t m_obsid = 1234;
   std::string m_passId = "PS20010203";
   std::string m_processingChain = "test chain";
   std::string m_targetName = "test target";
   uint16_t m_revisionNumber = 1;
   uint16_t m_processingNumber = 2;

   int32_t m_alertId = 1000;
   std::string m_alertMessage = "This is a test alert message.";
};


BOOST_FIXTURE_TEST_SUITE( test_alert, LoggerFixture )

////////////////////////////////////////////////////////////////////////////////
//  This test implements Infrastructure test specification
//  CHEOPS-SOC-TC-WP05-FUNC-007 Alert API
//
BOOST_AUTO_TEST_CASE( test_create_alert ) {

  std::cout << "Sending email to [" << m_emailAddress << "]" << std::endl;
  logger << alert << m_alertId << m_alertMessage << std::endl;

}

BOOST_AUTO_TEST_SUITE_END()
