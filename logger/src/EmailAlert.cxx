/** ****************************************************************************
 *  @file
 *  @ingroup common_sw
 *  @brief Implementation of class EmailAlert.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 13.0 2020-09-17 ABE #21949 Show target name in alert emails.
 *  @version 11.3 2019-04-09 ABE #18117 Improved contents of alert emails.
 *  @version  9.0 2018-01-11 ABE #15085 Add the -f option to sendmail to force
 *                                      the correct sender address.
 *  @version  3.3 2015-05-08 ABE        First version.
 */

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <time.h>

#include <EmailAlert.hxx>

const std::string EmailAlert::PARAM_TO_EMAIL = "toEmail";
const std::string EmailAlert::PARAM_FROM_EMAIL = "fromEmail";
const std::string EmailAlert::PARAM_PROGRAM_NAME = "programName";
const std::string EmailAlert::PARAM_PROGRAM_VERSION = "programVersion";
const std::string EmailAlert::PARAM_HOST_NAME = "hostName";
const std::string EmailAlert::PARAM_LOG_FILE_NAME = "logFile";
const std::string EmailAlert::PARAM_VISIT_ID = "visitId";
const std::string EmailAlert::PARAM_OBSID = "obsid";
const std::string EmailAlert::PARAM_PASS_ID = "passId";
const std::string EmailAlert::PARAM_PROCESSING_CHAIN = "processingChain";
const std::string EmailAlert::PARAM_TARGET_NAME = "targetName";
const std::string EmailAlert::PARAM_REVISION_NUMBER = "revisionNumber";
const std::string EmailAlert::PARAM_PROCESSING_NUMBER = "processingNumber";
const std::string EmailAlert::PARAM_ALERT_ID = "alertId";
const std::string EmailAlert::PARAM_ALERT_MESSAGE = "alertMessage";
const std::string EmailAlert::PARAM_ALERT_TIME = "alertTime";

EmailAlert::EmailAlert(
      const std::string & emailTemplateFile,
      const std::string & toEmail,
      const std::string & fromEmail,
      const std::string & programName,
      const std::string & programVersion,
      const std::string & hostName,
      const std::string & logFileName,
      const std::string & visitId,
      uint32_t obsid,
      const std::string & passId,
      const std::string & processingChain,
      const std::string & targetName,
      uint16_t revisionNumber,
      uint16_t processingNumber) :
              m_toEmail(toEmail),
              m_fromEmail(fromEmail),
              m_programName(programName),
              m_programVersion(programVersion),
              m_hostName(hostName),
              m_logFileName(logFileName),
              m_visitId(visitId),
              m_obsid(obsid),
              m_passId(passId),
              m_processingChain(processingChain),
              m_targetName(targetName),
              m_revisionNumber(revisionNumber),
              m_processingNumber(processingNumber) {

  if (m_logFileName.empty()) {
    m_logFileName = "N/A";
  }
  if (m_visitId.empty()) {
    m_visitId = "N/A";
  }
  if (m_passId.empty()) {
    m_passId = "N/A";
  }
  if (m_processingChain.empty()) {
    m_processingChain = "N/A";
  }
  if (m_targetName.empty()) {
    m_targetName = "N/A";
  }

  // Read the templates for the subject and email contents from file
  m_emailTemplate = preprocessTemplate(readTemplate(emailTemplateFile));
}

std::string EmailAlert::readTemplate(const std::string & path) {

  std::string tmpl;

  if (path.empty()) {
    throw std::runtime_error("Template file for alert emails is not defined");
  }
  else if (!boost::filesystem::exists(path)) {
    throw std::runtime_error("File [" + path + "] not found");
  }
  else {
    std::ifstream file(path);
    if (file.is_open()) {
      std::stringstream strStream;
      strStream << file.rdbuf();  //read the file
      tmpl = strStream.str();
      file.close();
    }
    else {
      throw std::runtime_error("Cannot read file [" + path   + "]");
    }
  }
  return tmpl;
}

std::string EmailAlert::preprocessTemplate(const std::string & tmpl) {

  // Remove all whitespace inside placeholders, to make it easier to search
  // and replace placeholders later.
  std::string newTmpl(tmpl.size(), ' ');
  bool insidePlaceholder = false;
  std::string::size_type ii = 0;
  for (std::string::size_type i = 0; i < tmpl.size(); ++i) {
    // The current index is the first character after the placeholder prefix
    if (i >= m_prefix.size() &&
        tmpl.substr(i-m_prefix.size(), m_prefix.size()) == m_prefix) {
      insidePlaceholder = true;
    }
    // The current index is the first character of the placeholder postfix
    else if (i <= tmpl.size()-m_postfix.size() &&
        tmpl.substr(i, m_postfix.size()) == m_postfix) {
      insidePlaceholder = false;
    }

    // Copy the character to the new string, except if we are inside a placeholder
    // and the character is whitespace.
    if (!(insidePlaceholder && isspace(tmpl[i]))) {
      newTmpl[ii++] = tmpl[i];
    }
  }

  // Remove "unused" characters from the end of the string
  newTmpl.resize(ii);
  return newTmpl;
}

std::string EmailAlert::render(
    const std::string & tmpl,
    const std::map<std::string, std::string>& context) {

  std::string result(tmpl);
  for (const auto & param : context) {
    boost::replace_all(result, m_prefix + param.first + m_postfix, param.second);
  }
  return result;
}

std::map<std::string, std::string> EmailAlert::getContext(
    const std::string & alertId,
    const std::string & alertMessage,
    const std::string & alertTime) {

  std::map<std::string, std::string> context;
  context[PARAM_TO_EMAIL] = m_toEmail;
  context[PARAM_FROM_EMAIL] = m_fromEmail;
  context[PARAM_PROGRAM_NAME] = m_programName;
  context[PARAM_PROGRAM_VERSION] = m_programVersion;
  context[PARAM_HOST_NAME] = m_hostName;
  context[PARAM_LOG_FILE_NAME] = m_logFileName;
  context[PARAM_VISIT_ID] = m_visitId;
  context[PARAM_OBSID] = std::to_string(m_obsid);
  context[PARAM_PASS_ID] = m_passId;
  context[PARAM_PROCESSING_CHAIN] = m_processingChain;
  context[PARAM_TARGET_NAME] = m_targetName;
  context[PARAM_REVISION_NUMBER] = std::to_string(m_revisionNumber);
  context[PARAM_PROCESSING_NUMBER] = std::to_string(m_processingNumber);
  context[PARAM_ALERT_ID] = alertId;
  context[PARAM_ALERT_MESSAGE] = alertMessage;
  context[PARAM_ALERT_TIME] = alertTime;

  return context;
}

bool EmailAlert::send(
    const std::string & alertId,
    const std::string & alertMessage) {

  bool retval = false;

  if (!m_toEmail.empty()) {
    // Get the current time
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *gmtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%X", &tstruct);
    std::string alertTime = std::string(buf);

    std::map<std::string, std::string> context = getContext(
        alertId,
        alertMessage,
        alertTime);
    std::string message = render(m_emailTemplate, context);
    std::string sendmail = "/usr/sbin/sendmail -f " + m_fromEmail + " -t";
    FILE *mailpipe = popen(sendmail.c_str(), "w");

    if (mailpipe != NULL) {
      fwrite(message.c_str(), 1, strlen(message.c_str()), mailpipe);
      fwrite("\n", 1, 1, mailpipe);
      pclose(mailpipe);
      retval = true;
    }
    else {
      perror("Failed to invoke sendmail");
    }
  }
  return retval;
}

bool EmailAlert::send(
    const std::string & to,
    const std::string & from,
    const std::string & subject,
    const std::string & message) {

  if (to == "") {
    return false;
  }

  bool retval = false;
  std::string sendmail = "/usr/sbin/sendmail -f " + from + " -t";

  FILE *mailpipe = popen(sendmail.c_str(), "w");

  if (mailpipe != NULL) {
    fprintf(mailpipe, "To: %s\n", to.c_str());
    if (from != "") {
      fprintf(mailpipe, "From: %s\n", from.c_str());
    }
    fprintf(mailpipe, "Subject: %s\n\n", subject.c_str());
    fwrite(message.c_str(), 1, strlen(message.c_str()), mailpipe);
    fwrite(".\n", 1, 2, mailpipe);
    pclose(mailpipe);
    retval = true;
  }
  else {
    perror("Failed to invoke sendmail");
  }

  return retval;
}
