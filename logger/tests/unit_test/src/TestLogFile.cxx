/** ****************************************************************************
 *  @file
 *
 *  @ingroup Logger
 *  @brief unit_test of class Logger
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 7.4 2017-06-09 ABE checking the file name using regex has been
 *                              commented out to avoid the extra dependency on
 *                              boost regex.
 *  @version 5.2 2016-05-09 ABE first version.
 */

#define BOOST_TEST_MAIN

#include <boost/filesystem.hpp>
//#include <boost/regex.hpp>
#include <boost/test/unit_test.hpp>

#include "Logger.hxx"

using namespace boost::unit_test;



struct LogFileFixture {

  LogFileFixture() {

    // Prevent duplication of log output
    boost::log::core::get()->remove_all_sinks();
  }
   ~LogFileFixture() {}


   std::string m_directory = "results";
   std::string m_logFile = m_directory + "/TestCxxLogFile.log";
};

BOOST_FIXTURE_TEST_SUITE( test_log_file, LogFileFixture )

BOOST_AUTO_TEST_CASE( test_full_file_name_logger ) {

  unlink(m_logFile.c_str());

  Logger::SetProgramVersion("TestCxxLogFile", "7.0");
  Logger::Configure("DEBUG", "DEBUG", "", m_directory, m_logFile);

  BOOST_CHECK( boost::filesystem::exists(m_logFile) );

  unlink(m_logFile.c_str());
}

BOOST_AUTO_TEST_CASE( test_directory_name_logger ) {

  std::string procName = "TestCxxLogDirectory";
  std::string procVersion = "7.0";
  uint16_t revisionNumber = 1;
  uint16_t processingNumber = 2;
  std::string visitId = "PR990001_TG000101";
  std::string passId = "PS18010112";
  uint32_t obsid = 2;

  Logger::SetProgramVersion(procName, procVersion);

  std::string logFile = BoostLog::initFileLogging(procName,
                                                  procVersion,
                                                  m_directory,
                                                  "",
                                                  revisionNumber,
                                                  processingNumber);
  logger << info << "test" << std::endl;

  BOOST_CHECK( boost::filesystem::exists(logFile) );

  // We cannot compare the file name and the expected file name because the UTC date
  // is set to the current time
//  //boost::regex regex(m_directory + "/CH_TU[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}-[0-9]{2}-[0-9]{2}_(.+?)_[0-9.]+_V[0-9]{2}[0-9]{2}.log");
//  boost::regex regex(m_directory + "/CH_TU[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}-[0-9]{2}-[0-9]{2}_(.+?)_V[0-9]{2}[0-9]{2}.log");
//  BOOST_CHECK( regex_match(logFile, regex) );
  unlink(logFile.c_str());

  logFile = BoostLog::initFileLogging(procName,
                                      procVersion,
                                      m_directory,
                                      "",
                                      revisionNumber,
                                      processingNumber,
                                      visitId,
                                      passId,
                                      obsid);

  logger << info << "test" << std::endl;

  BOOST_CHECK( boost::filesystem::exists(logFile) );

  // We cannot compare the file name and the expected file name because the UTC date
  // is set to the current time
//  //regex = boost::regex(m_directory + "/CH_PR([0-9]{2}[0-9]{4}_TG[0-9]{4}[0-9]{2})?(_PS[0-9]{8})?_TU[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}-[0-9]{2}-[0-9]{2}_(.+?)_[0-9.]+_V[0-9]{2}[0-9]{2}.log");
//  regex = boost::regex(m_directory + "/CH_PR([0-9]{2}[0-9]{4}_TG[0-9]{4}[0-9]{2})?(_PS[0-9]{8})?_TU[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}-[0-9]{2}-[0-9]{2}_(.+?)_V[0-9]{2}[0-9]{2}.log");
//  BOOST_CHECK( regex_match(logFile, regex) );
  //unlink(logFile.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
