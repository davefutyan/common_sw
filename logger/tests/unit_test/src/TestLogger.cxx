/** ****************************************************************************
 *  @file
 *
 *  @ingroup Logger
 *  @brief unit_test of classes LogMessageReader, AlertCodeReader, Logger and
 *         Settings.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 1.0   2014-03-11 ABE first version.
 *  @version 3.2   2015-03-27 Updated unit tests for alert codes and different
 *                            types of ints.
 */

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "Logger.hxx"

using namespace boost::unit_test;


struct LoggerFixture {

   LoggerFixture() {

     // Prevent duplication of log output
     boost::log::core::get()->remove_all_sinks();

     Logger::SetProgramVersion("TestCxxLogger", "0.0");
     m_logFile = Logger::Configure("DEBUG", "DEBUG", m_alertEmail,  m_outDir);
   }

   ~LoggerFixture() {
     unlink(m_logFile.c_str());
   }

   const char*  m_charPointer = "a";
   std::string  m_string = "b";
   int16_t      m_int16_t = 1;
   int32_t      m_int32_t = 2;
   int64_t      m_int64_t = 3;
   uint16_t     m_uint16_t = 4;
   uint32_t     m_uint32_t = 5;
   uint64_t     m_uint64_t = 6;
   double       m_double = 1.0;

   std::string m_alertEmail = "";
   std::string m_outDir = "results";
   std::string m_logFile = "";

};


BOOST_FIXTURE_TEST_SUITE( test_logger, LoggerFixture )


BOOST_AUTO_TEST_CASE( configure_logger ) {

	std::string programName = "Test";
	std::string programVersion = "0.1";

	// Initialize the Logger as it is done in the generated CheopsInit(...) method.
	Logger::SetProgramVersion(programName, programVersion);
	std::string logFile = Logger::Configure("DEBUG", "DEBUG", m_alertEmail, m_outDir);

  logger << debug << "Log message" << " " << 1 << std::endl;
  logger << info << "Log message " << 2 << std::endl;
  logger << progress << 65 << std::endl;
  logger << warn << "Log message " << 4 << std::endl;
  logger << alert << 15 << "An alert happend!" << std::endl;
  logger << error << "Log message " << 5 << std::endl;

	BOOST_CHECK_EQUAL( Logger::GetProgramName(), programName );
	BOOST_CHECK_EQUAL( Logger::GetProgramVersion(), programVersion );

	unlink(logFile.c_str());
}


BOOST_AUTO_TEST_CASE( test_debug_level ) {

  TextMessage textMessage = logger << debug << m_charPointer;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::string(m_charPointer) );
  textMessage << std::endl;

  textMessage = logger << debug << m_string;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), m_string );
  textMessage << std::endl;

  textMessage = logger << debug << m_int16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int16_t) );
  textMessage << std::endl;

  textMessage = logger << debug << m_int32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int32_t) );
  textMessage << std::endl;

  textMessage = logger << debug << m_int64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int64_t) );
  textMessage << std::endl;

  textMessage = logger << debug << m_uint16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint16_t) );
  textMessage << std::endl;

  textMessage = logger << debug << m_uint32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint32_t) );
  textMessage << std::endl;

  textMessage = logger << debug << m_uint64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint64_t) );
  textMessage << std::endl;

  textMessage = logger << debug << m_double;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::DEBUG_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_double) );
  textMessage << std::endl;

}


BOOST_AUTO_TEST_CASE( test_info_level ) {

  TextMessage textMessage = logger << info << m_charPointer;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::string(m_charPointer) );
  textMessage << std::endl;

  textMessage = logger << info << m_string;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), m_string );
  textMessage << std::endl;

  textMessage = logger << info << m_int16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int16_t) );
  textMessage << std::endl;

  textMessage = logger << info << m_int32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int32_t) );
  textMessage << std::endl;

  textMessage = logger << info << m_int64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int64_t) );
  textMessage << std::endl;

  textMessage = logger << info << m_uint16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint16_t) );
  textMessage << std::endl;

  textMessage = logger << info << m_uint32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint32_t) );
  textMessage << std::endl;

  textMessage = logger << info << m_uint64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint64_t) );
  textMessage << std::endl;

  textMessage = logger << info << m_double;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::INFO_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_double) );
  textMessage << std::endl;

}


BOOST_AUTO_TEST_CASE( test_progress_level ) {

  Endline endline = logger << progress << m_int16_t;
  BOOST_CHECK_EQUAL( endline.GetLogLevel(), SeverityLevel::PROGRESS_LEVEL );
  BOOST_CHECK_EQUAL( endline.GetMessage(), std::to_string(m_int16_t) );
  endline << std::endl;

  endline = logger << progress << m_int32_t;
  BOOST_CHECK_EQUAL( endline.GetLogLevel(), SeverityLevel::PROGRESS_LEVEL );
  BOOST_CHECK_EQUAL( endline.GetMessage(), std::to_string(m_int32_t) );
  endline << std::endl;

  endline = logger << progress << m_int64_t;
  BOOST_CHECK_EQUAL( endline.GetLogLevel(), SeverityLevel::PROGRESS_LEVEL );
  BOOST_CHECK_EQUAL( endline.GetMessage(), std::to_string(m_int64_t) );
  endline << std::endl;

  endline = logger << progress << m_uint16_t;
  BOOST_CHECK_EQUAL( endline.GetLogLevel(), SeverityLevel::PROGRESS_LEVEL );
  BOOST_CHECK_EQUAL( endline.GetMessage(), std::to_string(m_uint16_t) );
  endline << std::endl;

  endline = logger << progress << m_uint32_t;
  BOOST_CHECK_EQUAL( endline.GetLogLevel(), SeverityLevel::PROGRESS_LEVEL );
  BOOST_CHECK_EQUAL( endline.GetMessage(), std::to_string(m_uint32_t) );
  endline << std::endl;

  endline = logger << progress << m_uint64_t;
  BOOST_CHECK_EQUAL( endline.GetLogLevel(), SeverityLevel::PROGRESS_LEVEL );
  BOOST_CHECK_EQUAL( endline.GetMessage(), std::to_string(m_uint64_t) );
  endline << std::endl;

}


BOOST_AUTO_TEST_CASE( test_warn_level ) {

  TextMessage textMessage = logger << warn << m_charPointer;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::string(m_charPointer) );
  textMessage << std::endl;

  textMessage = logger << warn << m_string;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), m_string );
  textMessage << std::endl;

  textMessage = logger << warn << m_int16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int16_t) );
  textMessage << std::endl;

  textMessage = logger << warn << m_int32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int32_t) );
  textMessage << std::endl;

  textMessage = logger << warn << m_int64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int64_t) );
  textMessage << std::endl;

  textMessage = logger << warn << m_uint16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint16_t) );
  textMessage << std::endl;

  textMessage = logger << warn << m_uint32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint32_t) );
  textMessage << std::endl;

  textMessage = logger << warn << m_uint64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint64_t) );
  textMessage << std::endl;

  textMessage = logger << warn << m_double;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::WARN_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_double) );
  textMessage << std::endl;

}


BOOST_AUTO_TEST_CASE( test_alert) {

  BOOST_CHECK_NO_THROW( logger << alert << m_int16_t );
  BOOST_CHECK_NO_THROW( logger << alert << m_int32_t );
  BOOST_CHECK_NO_THROW( logger << alert << m_int64_t );
  BOOST_CHECK_NO_THROW( logger << alert << m_uint16_t );
  BOOST_CHECK_NO_THROW( logger << alert << m_uint32_t );
  BOOST_CHECK_NO_THROW( logger << alert << m_uint64_t );

  std::string alertId = std::to_string(m_int16_t) + " ";
  TextMessage textMessage = logger << alert << m_int16_t << m_charPointer;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::string(m_charPointer) );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_string;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + m_string );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_int16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::to_string(m_int16_t) );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_int32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::to_string(m_int32_t) );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_int64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::to_string(m_int64_t) );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_uint16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::to_string(m_uint16_t) );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_uint32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::to_string(m_uint32_t) );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_uint64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::to_string(m_uint64_t) );
  textMessage << std::endl;

  textMessage = logger << alert << m_int16_t << m_double;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ALERT_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), alertId + std::to_string(m_double) );
  textMessage << std::endl;
}


BOOST_AUTO_TEST_CASE( test_error_level ) {

  TextMessage textMessage = logger << error << m_charPointer;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::string(m_charPointer) );
  textMessage << std::endl;

  textMessage = logger << error << m_string;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), m_string );
  textMessage << std::endl;

  textMessage = logger << error << m_int16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int16_t) );
  textMessage << std::endl;

  textMessage = logger << error << m_int32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int32_t) );
  textMessage << std::endl;

  textMessage = logger << error << m_int64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_int64_t) );
  textMessage << std::endl;

  textMessage = logger << error << m_uint16_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint16_t) );
  textMessage << std::endl;

  textMessage = logger << error << m_uint32_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint32_t) );
  textMessage << std::endl;

  textMessage = logger << error << m_uint64_t;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_uint64_t) );
  textMessage << std::endl;

  textMessage = logger << error << m_double;
  BOOST_CHECK_EQUAL( textMessage.GetLogLevel(), SeverityLevel::ERROR_LEVEL );
  BOOST_CHECK_EQUAL( textMessage.GetMessage(), std::to_string(m_double) );
  textMessage << std::endl;

}


BOOST_AUTO_TEST_SUITE_END()
