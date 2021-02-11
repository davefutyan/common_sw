/** ****************************************************************************
 *  @file
 *  @brief Declaration of class BoostLog.
 *  @ingroup Logger
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 12.1.2 2020-03-23 ABE #20653 take log level into account when 
 *                                        writing to file.
 *  @version  9.0   2018-01-10 ABE #15286 add OBSID to log file name
 *  @version  7.3   2017-05-18 ABE #12941 add pass id, visit id, processing chain
 *                                        and program version to log file name
 *  @version  6.2   2016-09-12 ABE #11463 use archive revision number and
 *                                        processing number in log file name
 *  @version  5.2   2016-05-09 ABE #10690 add functionality for taking the full
 *                                        log file path as input*
 *  @version  3.2   2014-03-27 ABE        changed the names of the SeverityLevel
 *                                        enum values
 *  @version  1.0   2014-02-25 ABE        first version
 */

#ifndef BOOST_LOG_HXX_
#define BOOST_LOG_HXX_

#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;

/** ****************************************************************************
 *  @brief Defines the possible log levels.
 *  @ingroup Logger
 */
enum SeverityLevel {
    DEBUG_LEVEL,    ///< Basic software tracing messages.
    INFO_LEVEL,     ///< Information about the type/status of operations.
    ALERT_LEVEL,    ///< An error was found in the data, which requires user
                    ///< action. This level must be used for data issues only.
    PROGRESS_LEVEL, ///< Information on progress of operations.
    WARN_LEVEL,     ///< An error occurred, the processor was able to continue.
    ERROR_LEVEL     ///< An error occurred, the processor was not able to continue.
};


/** ****************************************************************************
 *  @brief This class handles the creation of log entries using the Boost
 *         Log library.
 *  @ingroup Logger
 *  @author Anja Bekkelien UGE
 */
class BoostLog {

 private:

  /** Standard error sink. */
  static boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > m_cerrSink;

  /** Standard output sink. */
  static boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > m_coutSink;

  /** File sink. */
  static boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > m_fileSink;

  /** **************************************************************************
   *  @brief The Boost logger object used for creating log entries.
   */
	static src::severity_logger<SeverityLevel> slg;

 public:

  /** **************************************************************************
   *  @brief Configures logging for cout and cerr.
   *
   *  The method configures console log messages according to monitor4EO
   *  specifications.
   *
   *  @param [in] procName    the process name.
   *  @param [in] procVersion the process version.
   */
  static void initConsoleLogging(const std::string& procName,
                                 const std::string& procVersion);

  /** **************************************************************************
   *  @brief Configures logging to file.
   *
   *  @param [in] procName         The process name.
   *  @param [in] procVersion      The process version.
   *  @param [in] outDir           (optional) The directory in which the output
   *                               log file is created. If this parameter is not
   *                               given, the file is created in the current
   *                               directory
   *  @param [in] fileName         (optional) the name of the log file, without
   *                               path. If this parameter is not provided, a
   *                               unique file name containing the current UTC
   *                               time is generated.
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
   *  @param [in] logLevel         (optional) the lowest log level that is
   *                               written to file. Defaults to DEBUG.
   *  @return the path to the log file
   */
  static std::string initFileLogging(const std::string& procName,
                                     const std::string& procVersion,
                                     const std::string & outDir="",
                                     const std::string & fileName="",
                                     uint16_t revisionNumber=0,
                                     uint16_t processingNumber=0,
                                     const std::string & visitId="",
                                     const std::string & passId="",
                                     uint32_t obsid=0,
                                     const std::string & processingChain="",
                                     SeverityLevel logLevel=DEBUG_LEVEL);


  /** **************************************************************************
   *  @brief Sets the lowest log level for cout and cerr.
   *
   *  @param stdoutLevel the lowest log level for cout
   *  @param stderrLevel the lowest log level for cerr
   */
  static void setLogLevel(SeverityLevel stdoutLevel,
                          SeverityLevel stderrLevel);


	/** **************************************************************************
	 *  @brief Creates a log entry.
	 *
	 *  @param [in] level the log entry's log level.
	 *  @param [in] message the log entry's text message.
	 */
	static void log(SeverityLevel level, const std::string& message);

};


#endif /* BOOST_LOG_HXX_ */
