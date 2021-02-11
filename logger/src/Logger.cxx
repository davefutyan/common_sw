/** ****************************************************************************
 *  @file
 *  @ingroup Logger
 *  @brief Implementation of log entry creation classes.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 13.0   2020-09-17 ABE #21949 Show target name in alert emails.
 *  @version 12.1.2 2020-03-23 ABE #20653 take log level into account when
 *                                        writing to file.
 *  @version 11.3   2019-04-09 ABE #18117 Improved contents of alert emails.
 *  @version  9.0   2018-01-11 ABE #15085 Define the mandatory email sender for
 *                                        alert emails.
 *  @version  9.0   2018-01-10 ABE #15286 Add OBSID to log file name.
 *  @version  7.3   2017-05-18 ABE #12941 Add pass id, visit id, processing chain
 *                                        and program version to log file name.
 *  @version  6.2   2016-09-12 ABE #11463 Use archive revision number and
 *                                        processing number in log file name.
 *  @version  5.2   2016-05-09 ABE #10690 Add functionality for taking the full
 *                                        log file path as input.
 *  @version  3.3   2015-05-07 ABE        Redefined interface for creating log
 *                                        entries.
 *  @version  3.2   2014-03-27 ABE        Added alert codes, and moved config
 *                                        code to the Settings class.
 *  @version  1.0   2014-02-26 ABE        First version.
 */

#include <boost/filesystem.hpp>

#include "Logger.hxx"

std::string Logger::m_procName;

std::string Logger::m_procVersion;

EmailAlert Endline::m_emailAlert;

/** ****************************************************************************
 *  @brief Converts a string to a log level.
 *  @ingroup Logger
 *
 *  @param [in] level a log level as a string. Must be "DEBUG", "INFO", "PROGRESS",
 *               "WARNING", or "ERROR"
 *  @return the log level corresponding to the input argument
 */
SeverityLevel string2LogLevel(const std::string& level) {

  if (level.compare("DEBUG") == 0) {
    return SeverityLevel::DEBUG_LEVEL;
  }
  else if (level.compare("INFO") == 0) {
    return SeverityLevel::INFO_LEVEL;
  }
  else if (level.compare("PROGRESS") == 0) {
    return SeverityLevel::PROGRESS_LEVEL;
  }
  else if (level.compare("WARN") == 0 || level.compare("WARNING") == 0) {
    return SeverityLevel::WARN_LEVEL;
  }
  else if (level.compare("ERROR") == 0) {
    return SeverityLevel::ERROR_LEVEL;
  }
  else {
    return SeverityLevel::INFO_LEVEL;
  }
}

/** ****************************************************************************
 */
void Logger::SetProgramVersion(const std::string & procName,
                               const std::string & procVersion) {
   m_procName = procName;
   m_procVersion = procVersion;

   BoostLog::initConsoleLogging(procName, procVersion);

}

/** ****************************************************************************
 */
std::string Logger::Configure(const std::string & stdoutLevel,
                              const std::string & stderrLevel,
                              const std::string & alertEmail,
                              const std::string & logDirectory,
                              const std::string & logFile,
                              uint16_t revisionNumber,
                              uint16_t processingNumber,
                              const std::string & visitId,
                              const std::string & passId,
                              uint32_t obsid,
                              const std::string & processingChain,
                              const std::string & targetName) {

  BoostLog::setLogLevel(string2LogLevel(stdoutLevel),
                        string2LogLevel(stderrLevel));

  std::string logPath = "";
  if (!logFile.empty()) {
    boost::filesystem::path path(logFile);
    logPath = BoostLog::initFileLogging(Logger::GetProgramName(),
                                        Logger::GetProgramVersion(),
                                        path.parent_path().string(),
                                        path.filename().string(),
                                        revisionNumber,
                                        processingNumber,
                                        visitId,
                                        passId,
                                        obsid,
                                        processingChain,
                                        string2LogLevel(stdoutLevel));
  }
  else if(!logDirectory.empty()) {
    logPath = BoostLog::initFileLogging(Logger::GetProgramName(),
                                        Logger::GetProgramVersion(),
                                        logDirectory,
                                        "",
                                        revisionNumber,
                                        processingNumber,
                                        visitId,
                                        passId,
                                        obsid,
                                        processingChain,
                                        string2LogLevel(stdoutLevel));
  }

  // Get the host name
  char hostName[32];
  gethostname(hostName, sizeof hostName);

  // get the schema file name of the trigger file file.
  // it has to be : $CHOEPS_SW/resources/trigger_file_schema.xsd
  char * cheopsSw = getenv("CHEOPS_SW");
  if (cheopsSw == NULL) {
     throw std::runtime_error("Environment variable CHEOPS_SW is not defined.");
  }

  // Configure email alerts
  Endline::setEmailAlert(EmailAlert(
      std::string(cheopsSw) + "/resources/logger_alert_email_template",
      alertEmail,
      "noreply@unige.ch",
      m_procName,
      m_procVersion,
      std::string(hostName),
      logPath,
      visitId,
      obsid,
      passId,
      processingChain,
      targetName,
      revisionNumber,
      processingNumber));

  return logPath;
}

/** ****************************************************************************
*/
std::string Logger::GetProgramName() {
  return m_procName;
}

/** ****************************************************************************
 */
std::string Logger::GetProgramVersion() {
  return m_procVersion;
}

Logger logger;
