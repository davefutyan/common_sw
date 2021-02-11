/** ****************************************************************************
 *  @file
 *  @ingroup Logger
 *  @brief Declaration of log entry creation classes.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 13.0 2020-09-17 ABE #21949 Show target name in alert emails.
 *  @version 11.3 2019-04-09 ABE #18117 Improved contents of alert emails.
 *  @version  9.0 2018-01-10 ABE #15286 Add OBSID to log file name.
 *  @version  7.3 2017-05-18 ABE #12941 Add pass id, visit id, processing chain
 *                                      and program version to log file name.
 *  @version  6.2 2016-09-12 ABE #11463 Use archive revision number and
 *                                      processing number in log file name.
 *  @version  5.2 2016-05-09 ABE #10690 Add functionality for taking the full
 *                                      log file path as input.
 *  @version  3.3 2015-05-07 ABE        Redefined interface for creating log
 *                                      entries.
 *  @version  3.2 2014-03-27 ABE        Added alert codes, and moved config
 *                                      code to the Settings class.
 *  @version  1.0 2014-02-26 ABE        First version.
 */

#ifndef LOG_HXX_
#define LOG_HXX_

#include <string>
#include <iostream>
#include <stdint.h>

#include "BoostLog.hxx"
#include "EmailAlert.hxx"

/** ****************************************************************************
 *  @brief Typedef used for overloading << std::endl.
 */
typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

/** ****************************************************************************
 *  @brief Typedef used for overloading << std::endl.
 */
typedef CoutType& (*StandardEndLine)(CoutType&);

/** ****************************************************************************
 *  @brief Enum that defines the debug log level.
 *  @ingroup Logger
 */
enum Debug { debug };

/** ****************************************************************************
 *  @brief Enum that defines the info log level.
 *  @ingroup Logger
 */
enum Info  { info };

/** ****************************************************************************
 *  @brief Enum that defines the progress log level.
 *  @ingroup Logger
 */
enum Progress  { progress };

/** ****************************************************************************
 *  @brief Enum that defines the warn log level.
 *  @ingroup Logger
 */
enum Warn  { warn };

/** ****************************************************************************
 *  @brief Enum that defines the alert log level.
 *  @ingroup Logger
 */
enum Alert { alert };

/** ****************************************************************************
 *  @brief Enum that defines the error log level.
 *  @ingroup Logger
 */
enum Error { error };


/** ****************************************************************************
 *  @brief The base class of all classes that are part of the log entry creation
 *         interface.
 *  @ingroup Logger
 *  @author Anja Bekkelien UGE
 */
class LogMessage {

 protected:

  SeverityLevel m_level;  ///< The log entry's severity level.
  std::string m_message;  ///< The text message of the entry.

  /** **************************************************************************
   *  @brief Constructor for creating an instance with a log level but no
   *         message.
   *
   *  @param [in] level the log level
   */
  LogMessage(SeverityLevel level) {
    m_level = level;
  }

  /**
   *  @brief Constructor for creating an instance with a log level and a
   *         message.
   *
   *  @param [in] level   the log level
   *  @param [in] message the log message
   */
  LogMessage(SeverityLevel level, const std::string & message) {
    m_level = level;
    m_message = message;
  }

 public:

  /** **************************************************************************
   *  @brief Returns the log level.
   *
   *  @return the log level
   */
  SeverityLevel GetLogLevel() {
    return m_level;
  }

  /** **************************************************************************
   *  @brief Returns the log message.
   *
   *  @return the log message
   */
  std::string GetMessage() {
    return m_message;
  }
};

/** ****************************************************************************
 *  @brief Class used for readin in std::endl using the << operator.
 *  @ingroup Logger
 *  @author Anja Bekkelien UGE
 *
 *  This class is part of the interface for creating log messages. Its
 *  resposibility is to read in the final std::endl of the message and create
 *  the actual log entry.
 */
class Endline : public LogMessage {

 private:

  /**
   * Object used for sending email alerts.
   */
  static EmailAlert m_emailAlert;

 public:

  /**
   *  @brief Constructor for creating an instance with a log level but no
   *         message.
   *  @param [in] level the log level.
   */
  Endline(SeverityLevel level) : LogMessage(level) {}

  /**
   *  @brief Constructor for creating an instance with a log level and a
   *         message.
   *  @param [in] level   the log level
   *  @param [in] message the log message
   */
  Endline(SeverityLevel level, const std::string & message) : LogMessage(level, message) {}

  /** **************************************************************************
   *  @brief Calling this method triggers the creation of the log entry.
   */
  void operator << (StandardEndLine) {
    processLogMessage();
  }

  /** **************************************************************************
   * @brief Processes and creates a log entry.
   */
  void processLogMessage() {

    if (m_level == ALERT_LEVEL) {
      // Find the index of the space character that separates the alert id and
      // the alert message
      std::string::size_type separatorIndex = m_message.find(" ");
      if (separatorIndex != std::string::npos) {
        std::string alertId = m_message.substr(0, separatorIndex);
        std::string alertMessage = m_message.substr(separatorIndex+1);
        m_emailAlert.send(alertId, alertMessage);
      }
    }

    BoostLog::log(m_level, m_message);
  }

  /** **************************************************************************
   * @brief Sets the object used to send email alerts.
   *
   * @param [in] emailAlert object used to send email alerts.
   */
  static void setEmailAlert(const EmailAlert & emailAlert) {
    m_emailAlert = emailAlert;
  }
};


/** ****************************************************************************
 *  @brief Class used for creating a text message for a log entry.
 *  @ingroup Logger
 *  @author Anja Bekkelien UGE
 *
 *  The class overloads the << operator to allow the passing in of various data
 *  types to create a generic text message.
 *
 *  The class concatenates each input to << to a string until an end of line is
 *  encountered, which triggers the creation of a log record.
 */
class TextMessage : public Endline {

 public:

  /**
   *  @brief Constructor for creating an instance with a log level but no
   *         message.
   *  @param [in] level the log level.
   */
  TextMessage(SeverityLevel level) : Endline(level) {}

  /**
   *  @brief Constructor for creating an instance with a log level and a
   *         message.
   *  @param [in] level   the log level
   *  @param [in] message the log message
   */
  TextMessage(SeverityLevel level, const std::string & message) : Endline(level, message) {}

  /** **************************************************************************
   *  @brief Calling this method triggers the creation of the log entry.
   */
  void operator << (StandardEndLine) {
    processLogMessage();
  }

  /** **************************************************************************
   *  @brief Appends a char array to the text message.
   *
   *  @param [in] msg a char array to be appended to the text message
   *  @return this instance
   */
  TextMessage & operator << (const char * msg) {
    m_message.append(msg);
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends a string to the text message.
   *
   *  @param [in] msg a string to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const std::string & msg) {
    m_message.append(msg);
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends an int to the text message.
   *
   *  @param [in] number a signed int (16 bit) to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const int16_t number) {
    m_message.append(std::to_string(number));
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends an int to the text message.
   *
   *  @param [in] number a signed int (32 bit) to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const int32_t number) {
    m_message.append(std::to_string(number));
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends an int to the text message.
   *
   *  @param [in] number a signed int (64 bit) to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const int64_t number) {
    m_message.append(std::to_string(number));
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends an unsigned int to the text message.
   *
   *  @param [in] number an unsigned int (16 bit) to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const uint16_t number) {
    m_message.append(std::to_string(number));
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends an unsigned int to the text message.
   *
   *  @param [in] number an unsigned int (32 bit) to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const uint32_t number) {
    m_message.append(std::to_string(number));
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends an unsigned int to the text message.
   *
   *  @param [in] number an unsigned int (64 bit) to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const uint64_t number) {
    m_message.append(std::to_string(number));
    return *this;
  }

  /** **************************************************************************
   *  @brief Appends a double to the text message.
   *
   *  @param [in] number a double to be appended to the text message
   *
   *  @return this instance
   */
  TextMessage & operator << (const double number) {
    m_message.append(std::to_string(number));
    return *this;
  }
};

/** ****************************************************************************
 *  @brief Class used for reading in a percentage for a progress log entry.
 *  @ingroup Logger
 *  @author Anja Bekkelien UGE
 *
 *  The class overloads the << operator to read in an integer.
 */
class ProgressStatus : public LogMessage {

 public:

  /**
   *  @brief Constructor for creating an instance with a log level but no
   *         message.
   *  @param [in] level the log level.
   */
  ProgressStatus(SeverityLevel level) : LogMessage(level) {}

  /**
   *  @brief Constructor for creating an instance with a log level and a
   *         message.
   *  @param [in] level   the log level
   *  @param [in] message the log message
   */
  ProgressStatus(SeverityLevel level, const std::string & message) : LogMessage(level, message) {}

  /** **************************************************************************
   *  @brief Reads in a progress percentage as an integer.
   *
   *  @param [in] number a percentage between 0 and 100 as a 16 bit signed int
   *
   *  @return an Endline instance for reading in std::endl and create the
   *          process log entry.
   */
  Endline operator << (const int16_t number) {
    return Endline(m_level, std::to_string(number));
  }

  /** **************************************************************************
   *  @brief Reads in a progress percentage as an integer.
   *
   *  @param [in] number a percentage between 0 and 100 as a 32 bit signded int
   *
   *  @return an Endline instance for reading in std::endl and create the
   *          process log entry.
   */
  Endline operator << (const int32_t number) {
    return Endline(m_level, std::to_string(number));
  }

  /** **************************************************************************
   *  @brief Reads in a progress percentage as an integer.
   *
   *  @param [in] number a percentage between 0 and 100 as a 64 bit signed int
   *
   *  @return an Endline instance for reading in std::endl and create the
   *          process log entry.
   */
  Endline operator << (const int64_t number) {
    return Endline(m_level, std::to_string(number));
  }

  /** **************************************************************************
   *  @brief Reads in a progress percentage as an integer.
   *
   *  @param [in] number a percentage between 0 and 100 as a 16 bit unsigned int
   *
   *  @return an Endline instance for reading in std::endl and create the
   *          process log entry.
   */
  Endline operator << (const uint16_t number) {
    return Endline(m_level, std::to_string(number));
  }

  /** **************************************************************************
   *  @brief Reads in a progress percentage as an integer.
   *
   *  @param [in] number a percentage between 0 and 100 as a 32 bit unsigned int
   *
   *  @return an Endline instance for reading in std::endl and create the
   *          process log entry.
   */
  Endline operator << (const uint32_t number) {
    return Endline(m_level, std::to_string(number));
  }

  /** **************************************************************************
   *  @brief Reads in a progress percentage as an integer.
   *
   *  @param [in] number a percentage between 0 and 100 as a 64 bit unsigned int
   *
   *  @return an Endline instance for reading in std::endl and create the
   *          process log entry.
   */
  Endline operator << (const uint64_t number) {
    return Endline(m_level, std::to_string(number));
  }
};

/** ****************************************************************************
 *  @brief Class used for reading in an alert ID for an alert log entry.
 *  @ingroup Logger
 *  @author Anja Bekkelien UGE
 *
 *  The class overloads the << operator to read in an alert ID as an integer.
 */
class AlertId : public LogMessage {

 public:

  /**
   *  @brief Constructor for creating an instance with a log level but no
   *         message.
   *  @param [in] level the log level.
   */
  AlertId(SeverityLevel level) : LogMessage(level) {}

  /**
   *  @brief Constructor for creating an instance with a log level and a
   *         message.
   *  @param [in] level   the log level
   *  @param [in] message the log message
   */
  AlertId(SeverityLevel level, const std::string & message) : LogMessage(level, message) {}

  /** **************************************************************************
   *  @brief Reads in an alert ID as an integer.
   *
   *  @param [in] number an alert ID as a 16 bin signed int
   *
   *  @return a TextMessage instance for reading in the text message of the
   *          alert log record
   */
  TextMessage operator << (const int16_t number) {
    return TextMessage(m_level, std::to_string(number) + " ");
  }

  /** **************************************************************************
   *  @brief Reads in an alert ID as an integer.
   *
   *  @param [in] number an alert ID as a 32 bit signded int
   *
   *  @return a TextMessage instance for reading in the text message of the
   *          alert log record
   */
  TextMessage operator << (const int32_t number) {
    return TextMessage(m_level, std::to_string(number) + " ");
  }

  /** **************************************************************************
   *  @brief Reads in an alert ID as an integer.
   *
   *  @param [in] number an alert ID as a 64 bit signed int
   *
   *  @return a TextMessage instance for reading in the text message of the
   *          alert log record
   */
  TextMessage operator << (const int64_t number) {
    return TextMessage(m_level, std::to_string(number) + " ");
  }

  /** **************************************************************************
   *  @brief Reads in an alert ID as an integer.
   *
   *  @param [in] number an alert ID as a 16 bin unsigned int
   *
   *  @return a TextMessage instance for reading in the text message of the
   *          alert log record
   */
  TextMessage operator << (const uint16_t number) {
    return TextMessage(m_level, std::to_string(number) + " ");
  }

  /** **************************************************************************
   *  @brief Reads in an alert ID as an integer.
   *
   *  @param [in] number an alert ID as a 32 bit unsigned int
   *
   *  @return a TextMessage instance for reading in the text message of the
   *          alert log record
   */
  TextMessage operator << (const uint32_t number) {
    return TextMessage(m_level, std::to_string(number) + " ");
  }

  /** **************************************************************************
   *  @brief Reads in an alert ID as an integer.
   *
   *  @param [in] number an alert ID as a 64 bit unsigned int
   *
   *  @return a TextMessage instance for reading in the text message of the
   *          alert log record
   */
  TextMessage operator << (const uint64_t number) {
    return TextMessage(m_level, std::to_string(number) + " ");
  }
};


/** ****************************************************************************
 *  @ingroup Logger
 *  @author Reiner Rohlfs UGE
 *  @author Anja Bekkelien UGE
 *
 *  @brief Class used for reading in the log level of a log entry.
 *
 *  It overloads the << operator to take a log level as input. The operator
 *  returns an object used to input the rest of the log message. What type of
 *  object that is, depends on the log level.
 */
class Logger {

 private:

  static std::string m_procName;    ///< The name of the processor
  static std::string m_procVersion; ///< The version of the processor

 public:

  /** **************************************************************************
   *  @brief Default constructor.
   */
  Logger()   { }

  /** ****************************************************************************
   *  @brief Sets the logger's program name and version.
   *
   *  @param [in] procName the program name
   *  @param [in] procVersion  the program version
   */
  static void SetProgramVersion(const std::string & procName,
                                const std::string & procVersion);

  /** ****************************************************************************
   *  @brief Configures the logger.
   *
   *  The path to the log file can be provided in two ways. 1) By defining a
   *  directory. A new log file is created every time the program is run, with a
   *  unique file name containing the current time in UTC. 2) By defining the
   *  full path to a log file. If this file already exists, logs are appended to
   *  it.
   *
   *  @param [in] stdoutLevel      (optional) the log level for stdout. Must be
   *                               "DEBUG", "INFO", "PROGRESS", "WARNING", "ERROR".
   *                               Set to INFO by default.
   *  @param [in] stderrLevel      (optional) the log level for stderr. Must be
   *                               "DEBUG", "INFO", "PROGRESS", "WARNING", "ERROR".
   *                               Set to PROGRESS by default.
   *  @param [in] alertEmail       (optional) the email to which alerts are
   *                               sent. If this parameter is not provided, no
   *                               emails are sent
   *  @param [in] logDirectory     (optional) the directory in which a log file
   *                               is created. If parameter @b logFile is
   *                               provided, @b logDirectory is ignored
   *  @param [in] logFile          (optional) the path to the log file,
   *                               including directories and file name. If this
   *                               parameter is provided, parameter
   *                               @b logDirectory is ignored
   *  @param [in] revisionNumber   (optional) The archive revision number
   *  @param [in] processingNumber (optional) The processing number
   *  @param [in] visitId          (optional) the visit id as a string formatted
   *                               as PRppnnnn-TGoooonn. If this argument is
   *                               provided, the visit id is included in the
   *                               file name.
   *  @param [in] passId           (optional) the pass id  as a string formatted
   *                               as PSyymmddhh. If this argument is provided,
   *                               the pass id is included in the file name.
   *  @param [in] obsid            (optional) the obsid
   *  @param [in] processingChain  (optional) the processing chain. If provided,
   *                               it is included in the log file name.
   *  @return the path to the log file, or an empty string if no log file was
   *          created
   */
  static std::string Configure(const std::string & stdoutLevel="INFO",
                               const std::string & stderrLevel="PROGRESS",
                               const std::string & alertEmail="",
                               const std::string & logDirectory="",
                               const std::string & logFile="",
                               uint16_t revisionNumber=0,
                               uint16_t processingNumber=0,
                               const std::string & visitId="",
                               const std::string & passId="",
                               uint32_t obsid=0,
                               const std::string & processingChain="",
                               const std::string & targetName="");

  /** **************************************************************************
   *  @brief Overloaded << operator that takes log level debug as input.
   *
   *  @param [in] debug the debug log level
   *  @return a TextMessage object used to read in the text message of the log
   *          entry
   */
  TextMessage operator << (Debug debug) {
    return TextMessage(SeverityLevel::DEBUG_LEVEL);
  }

  /** **************************************************************************
   *  @brief Overloaded << operator that takes log level info as input.
   *
   *  @param [in] info the info log level
   *  @return a TextMessage object used to read in the text message of the log
   *          entry
   */
  TextMessage operator << (Info info) {
    return TextMessage(SeverityLevel::INFO_LEVEL);
  }

  /** **************************************************************************
   *  @brief Overloaded << operator that takes progress level info as input.
   *
   *  @param [in] progress the progress log level
   *  @return a ProgressStatus object used to read in the progress percentage
   *          of the log entry.
   */
  ProgressStatus operator << (Progress progress) {
    return ProgressStatus(SeverityLevel::PROGRESS_LEVEL);
  }

  /** **************************************************************************
   *  @brief Overloaded << operator that takes log level warn as input.
   *
   *  @param [in] warn the warn log level
   *  @return a TextMessage object used to read in the text message of the log
   *          entry.
   */
  TextMessage operator << (Warn warn) {
    return TextMessage(SeverityLevel::WARN_LEVEL);
  }

  /** **************************************************************************
   *  @brief Overloaded << operator that takes log level alert as input.
   *
   *  @param [in] alert the alert log level
   *  @return an AlertId object used to read in the alert ID.
   */
  AlertId operator << (Alert alert) {
    return AlertId(SeverityLevel::ALERT_LEVEL);
  }

  /** **************************************************************************
   *  @brief Overloaded << operator that takes log level error as input.
   *
   *  @param [in] error the error log level
   *  @return a TextMessage object used to read in the text message of the log
   *          entry.
   */
  TextMessage operator << (Error error) {
    return TextMessage(SeverityLevel::ERROR_LEVEL);
  }

  /** **************************************************************************
    *  @brief Returns this logger's program name
    *
    *  @return this logger's program name
    */
  static std::string GetProgramName();

  /** **************************************************************************
    *  @brief Returns this logger's program name
    *
    *  @return this logger's program name
    */
  static std::string GetProgramVersion();

};


/** ****************************************************************************
  *  @ingroup Logger
  *  @brief The global Logger instance for creating log entries.
  */
extern Logger logger;


#endif /* LOG_HXX_ */

