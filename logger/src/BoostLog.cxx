/** ****************************************************************************
 *  @file
 *  @ingroup Logger
 *  @brief Implementation of class BoostLog.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 12.1.2 2020-03-23 ABE #20653 take log level into account when
 *                                        writing to file.
 *  @version  9.1   2018-01-15 ABE #15279 use boost::null_deleter to create streams
 *                                        instead of boost::empty_deleter, which is
 *                                        deprecated in boost 1.57 and removed in
 *                                        boost 1.59.
 *  @version  9.0   2018-01-10 ABE #15286 add OBSID to log file name
 *  @version  7.3   2017-05-18 ABE #12941 add pass id, visit id, processing chain
 *                                        and program version to log file name
 *  @version  6.2   2016-09-12 ABE #11463 use archive revision number and
 *                                        processing number in log file name
 *  @version  5.2   2016-05-09 ABE #10690 add functionality for taking the full
 *                                        log file path as input*
 *  @version  3.3   2015-05-27 ABE add log file
 *  @version  1.0   2014-02-25 ABE first version
 */

#include <ostream>
#include <fstream>
#include <stdio.h>      /* puts, printf */
#include <time.h>       /* time_t, struct tm, time, gmtime */

#include <boost/filesystem.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
// Deprecated since boost 1.55.0 and removed in boost 1.57.0
//#include <boost/utility/empty_deleter.hpp>	
#include "BoostLog.hxx"

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

using boost::shared_ptr;

// Define names and types of log attributes
BOOST_LOG_ATTRIBUTE_KEYWORD(node_name, "NodeName", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(proc_name, "ProcName", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(proc_version, "ProcVersion", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(process_id, "ProcessID", attrs::current_process_id::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", SeverityLevel)

boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > BoostLog::m_cerrSink;
boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > BoostLog::m_coutSink;
boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > BoostLog::m_fileSink;

src::severity_logger<SeverityLevel> BoostLog::slg;

/** ****************************************************************************
 *  @brief The formatting logic for log levels.
 */
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
  std::basic_ostream< CharT, TraitsT >& strm, SeverityLevel lvl) {
  static const char* const str[] = {
      "D",
      "I",
      "A",
      "P",
      "W",
      "E"
  };
  if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str))) {
    strm << str[lvl];
  } else {
    strm << static_cast< int >(lvl);
  }
  return strm;
}

/** ****************************************************************************
 * @brief Returns the full path to a log file.
 * @brief Constructs the name of a log file from a directory and the program
 *        name.
 *
 * If no file name is provided, a unique file name will be created, having the
 * following format:
 *
 * path/CH_PRppnnnn_TGttttnn_PSnnnnnnnn_TUYYYY-mm-ddTHH-MM-SS_procName_OBSIDnnnnnnnnnn_V0000.log
 *
 * where the date time is the current UTC date time. The OBSID can have up to 10 digits.
 *
 * The visit id, pass id and OBSID will only be included if provided as method
 * arguments.
 *
 * If no directory is provided, the log file will be created in the current
 * directory.
 *
 * @param [in] procName         the name of the program writing to the log file
 * @param [in] procVersion      the version of the program writing to the log file
 * @param [in] directory        (optional) the directory part of the log file's
 *                              path
 * @param [in] fileName         (optional) the file name part of the log file's
 *                              path
 * @param [in] revisionNumber   (optional) the archive revision number
 * @param [in] processingNumber (optional) the processing number
 * @param [in] visitId          (optional) the visit id as a string formatted
 *                              as PRppnnnn-TGoooonn. If this argument is
 *                              provided, the visit id is included in the
 *                              file name.
 *  @param [in] passId          (optional) the pass id  as a string formatted
 *                              as PSyymmddhh. If this argument is provided,
 *                              the pass id is included in the file name.
 *  @param [in] obsid           (optional) the obsid
 *  @param [in] processingChain (optional) the processing chain. If provided,
 *                              it is included in the log file name.
 *
 * @return file name
 */
std::string createFileName(const std::string & procName,
                           const std::string & procVersion,
                           const std::string & directory="",
                           const std::string & fileName="",
                           uint16_t revisionNumber=0,
                           uint16_t processingNumber=0,
                           const std::string & visitId="",
                           const std::string & passId="",
                           uint32_t obsid=0,
                           const std::string & processingChain="") {


  std::string logFileName(directory);

  if (logFileName.empty()) {
    logFileName.append(".");
  }

  if (logFileName.length() > 0 && !boost::algorithm::ends_with(logFileName, "/")) {
    logFileName.append("/");
  }

  if (fileName.empty()) {
    logFileName.append("CH_");

    // the program and the visit as PRppnnnn_TGttttnn_
    if (!visitId.empty()) {
      logFileName.append(visitId + "_");
    }

    // the pass ID if it is defined
    if (!passId.empty()) {
      logFileName.append(passId + "_");
    }

    // the current UTC time
    time_t rawtime;
    struct tm * utc;
    char buffer [80];

    time ( &rawtime );
    utc = gmtime ( &rawtime );
    strftime (buffer,80,"%Y-%m-%dT%H-%M-%S", utc);

    logFileName.append("TU");
    logFileName.append(buffer);
    logFileName.append("_");

    // the program name, without file extension
    boost::filesystem::path path(procName);
    logFileName.append(path.stem().string());

    // the program version
//    logFileName.append("_" + procVersion);

//    // the processing chain
//    if (!processingChain.empty() && processingChain != "not defined") {
//      logFileName.append("_" + processingChain);
//    }

    // the obsid
    if (obsid != 0) {
      logFileName.append("_OBSID" + std::to_string(obsid));
    }

    // the file version
    char hstr[10];
    sprintf(hstr, "_V%04hu", revisionNumber * 100 + processingNumber);
    logFileName.append(hstr);

    logFileName.append(".log");
  }
  else {
    logFileName.append(fileName);
  }

  return logFileName;
}

void BoostLog::initConsoleLogging(const std::string& procName,
                                  const std::string& procVersion) {

  // Get the host name
  char nodeName[32];
  gethostname(nodeName, sizeof nodeName);

  logging::add_common_attributes();

  // Add our own custom attributes.
  logging::core::get()->add_global_attribute("NodeName", attrs::constant< std::string >(nodeName));
  logging::core::get()->add_global_attribute("ProcName", attrs::constant< std::string >(procName));
  logging::core::get()->add_global_attribute("ProcVersion", attrs::constant< std::string >(procVersion));

  // Define the format of the log entries
  logging::formatter fmt = expr::stream
      << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%dT%H:%M:%S.%f")
      << " " << node_name
      << " " << proc_name
      << " " << proc_version
      << " [" << process_id << "]:"
      << " [" << severity << "] "
      << expr::message;

  typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;

  // Construct sinks for cout and cerr
  m_cerrSink = boost::make_shared< text_sink >();
  m_coutSink = boost::make_shared< text_sink >();

  // We have to provide an empty deleter to avoid destroying the global stream object
  // nb: empty_deleter is deprecated in boost 1.57 and removed in boost 1.59,
  // null_deleter should be used instead.
  // m_cerrSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cerr, boost::empty_deleter()));
  // m_coutSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cout, boost::empty_deleter()));
  m_cerrSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cerr, boost::null_deleter()));
  m_coutSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));

  BoostLog::setLogLevel(INFO_LEVEL, PROGRESS_LEVEL);

  // Set the log format for cout and cerr
  m_cerrSink->set_formatter(fmt);
  m_coutSink->set_formatter(fmt);

  logging::core::get()->add_sink(m_cerrSink);
  logging::core::get()->add_sink(m_coutSink);
}

std::string BoostLog::initFileLogging(const std::string & procName,
                                      const std::string & procVersion,
                                      const std::string & outDir,
                                      const std::string & fileName,
                                      uint16_t revisionNumber,
                                      uint16_t processingNumber,
                                      const std::string & visitId,
                                      const std::string & passId,
                                      uint32_t obsid,
                                      const std::string & processingChain,
                                      SeverityLevel logLevel) {

  if (!boost::filesystem::exists(outDir)) {
    boost::filesystem::create_directories(outDir);
  }

  std::string logFileName = createFileName(procName,
                                           procVersion,
                                           outDir,
                                           fileName,
                                           revisionNumber,
                                           processingNumber,
                                           visitId,
                                           passId,
                                           obsid,
                                           processingChain);
  typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;

  boost::shared_ptr< text_sink > m_fileSink = boost::make_shared< text_sink >();

  m_fileSink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(
      logFileName,
      std::ios_base::app));

  m_fileSink->set_filter( (severity == DEBUG_LEVEL ||
                           severity == INFO_LEVEL ||
                           severity == ALERT_LEVEL ||
                           severity == PROGRESS_LEVEL ||
                           severity == WARN_LEVEL ||
                           severity == ERROR_LEVEL) &&
                          (severity >= logLevel) );

  logging::formatter fmt = expr::stream
        << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%dT%H:%M:%S.%f")
        << " " << node_name
        << " " << proc_name
        << " " << proc_version
        << " [" << process_id << "]:"
        << " [" << severity << "] "
        << expr::message;
  m_fileSink->set_formatter(fmt);

  logging::core::get()->add_sink(m_fileSink);

  return logFileName;
}

void BoostLog::setLogLevel(SeverityLevel stdoutLevel,
                           SeverityLevel stderrLevel) {

  // Define the log levels that are output to cout
  m_coutSink->set_filter( (severity == DEBUG_LEVEL ||
                           severity == INFO_LEVEL ||
                           severity == ALERT_LEVEL) &&
                          (severity >= stdoutLevel) );

  // Define the log levels that are output to cerr
  m_cerrSink->set_filter( (severity == PROGRESS_LEVEL ||
                           severity == WARN_LEVEL ||
                           severity == ERROR_LEVEL) &&
                          (severity >= stderrLevel) );
}

void BoostLog::log(SeverityLevel level, const std::string& message) {

	BOOST_LOG_SEV(slg, level) << message;
}
