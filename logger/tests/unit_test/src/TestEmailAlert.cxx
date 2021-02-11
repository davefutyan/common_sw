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

     m_emailAlert = EmailAlert(
         m_emailTemplateFile,
         m_toEmail,
         m_fromEmail,
         m_programName,
         m_programVersion,
         m_hostName,
         m_logFile,
         m_visitId,
         m_obsid,
         m_passId,
         m_processingChain,
         m_targetName,
         m_revisionNumber,
         m_processingNumber);
   }

   ~LoggerFixture() {
     unlink(m_logFile.c_str());
   }

   std::string m_outDir = "results";

   std::string m_emailTemplateFile = "../../resources/logger_alert_email_template";
   std::string m_toEmail = "";
   std::string m_fromEmail = "test.from.email";
   std::string m_programName = "test_program";
   std::string m_programVersion = "1.0";
   std::string m_hostName = "chpstest";
   std::string m_logFile = "results/test_program.log";
   std::string m_visitId = "PR100001_TG045401";
   uint32_t m_obsid = 1234;
   std::string m_passId = "20010203";
   std::string m_processingChain = "test chain";
   std::string m_targetName = "test target";
   uint16_t m_revisionNumber = 1;
   uint16_t m_processingNumber = 2;
   std::string m_alertId = "30";
   std::string m_alertMessage = "This is an alert message!";
   std::string m_alertTime = "2020-01-01T01:01:01";

   EmailAlert m_emailAlert;
};


BOOST_FIXTURE_TEST_SUITE( test_alert, LoggerFixture )

BOOST_AUTO_TEST_CASE( testPreprocessTemplate ) {

  std::string tmpl = "Hello {{ test }}";
  EmailAlert emailAlert;
  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{test}}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{ test}}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{  test}}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{test }}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{test  }}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{ test }}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{  test   }}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate(" {{test}}"),
      " {{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{test}} "),
      "{{test}} ");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate(" {{test}} "),
      " {{test}} ");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("{{test\n}}"),
      "{{test}}");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate(" { test } "),
      " { test } ");

  BOOST_CHECK_EQUAL(
      emailAlert.preprocessTemplate("aaa {{test}} bbb {{ test2  }} ccc"),
      "aaa {{test}} bbb {{test2}} ccc");
}

BOOST_AUTO_TEST_CASE( testGetContext ) {

  std::map<std::string, std::string> context = m_emailAlert.getContext(
      m_alertId,
      m_alertMessage,
      m_alertTime);

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_TO_EMAIL), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_TO_EMAIL], m_toEmail );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_FROM_EMAIL), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_FROM_EMAIL], m_fromEmail );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_PROGRAM_NAME), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_PROGRAM_NAME], m_programName );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_PROGRAM_VERSION), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_PROGRAM_VERSION], m_programVersion );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_HOST_NAME), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_HOST_NAME], m_hostName );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_LOG_FILE_NAME), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_LOG_FILE_NAME], m_logFile );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_VISIT_ID), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_VISIT_ID], m_visitId );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_OBSID), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_OBSID], std::to_string(m_obsid) );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_PASS_ID), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_PASS_ID], m_passId );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_PROCESSING_CHAIN), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_PROCESSING_CHAIN], m_processingChain );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_TARGET_NAME), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_TARGET_NAME], m_targetName );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_REVISION_NUMBER), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_REVISION_NUMBER], std::to_string(m_revisionNumber) );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_PROCESSING_NUMBER), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_PROCESSING_NUMBER], std::to_string(m_processingNumber) );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_ALERT_ID), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_ALERT_ID], m_alertId );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_ALERT_MESSAGE), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_ALERT_MESSAGE], m_alertMessage );

  BOOST_CHECK_EQUAL( (int)context.count(EmailAlert::PARAM_ALERT_TIME), 1 );
  BOOST_CHECK_EQUAL( context[EmailAlert::PARAM_ALERT_TIME], m_alertTime );
}

BOOST_AUTO_TEST_CASE( testRenderTemplate ) {

  std::string param1 = "bbb";
  std::string param2 = "ddd eee";
  std::string tmpl = "aaa {{ param1 }} ccc {{param2}}";
  std::string renderedTmpl = "aaa " + param1 + " ccc " + param2;

  tmpl = m_emailAlert.preprocessTemplate(tmpl);

  std::map<std::string, std::string> context;
  context["param1"] = param1;
  context["param2"] = param2;

  BOOST_CHECK_EQUAL( m_emailAlert.render(tmpl, context), renderedTmpl );
}

BOOST_AUTO_TEST_CASE( testSend ) {

  // This will not send an email because the "to" email is empty
  m_emailAlert.send(m_alertId, m_alertMessage);
}

BOOST_AUTO_TEST_SUITE_END()
