/** ****************************************************************************
 *  @file
 *  @ingroup utilities
 *  @brief Implementation of the TriggerFile class.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 10.2   ABE 2018-10-12 #16882 interface update for passing in the job
 *                                        order validity time.
 *  @version  9.2   ABE 2018-05-09 #15652 new mandatory attribute in trigger file
 *                                        schema, PREV_PASS_ID
 *  @version  9.1.2 ABE 2018-03-23 #15774 Trigger files from executables with a
 *                                        file extension contained a dot in the
 *                                        file name.
 *  @version  8.2   ABE 2017-10-26 #14876 Add additional checks for invalid VisitIds
 *  @version  8.1   ABE 2017-10-04 #14501 The default visit id is now an invalid
 *                                        visit id instead of programme type 50
 *  @version  7.4   ABE 2017-05-31 #12948 Read and write the creation date
 *                                        of the trigger file.
 *  @version  7.3   ABE 2017-05-10 #12806 Read and write the new element OBSID.
 *                                        Updated according to schema modifications.
 *  @version  7.3   ABE 2017-05-03 #12927 Add new method Empty() for checking if a
 *                                        trigger file has been read.
 *  @version  7.3   ABE 2017-05-03 #12959 Sleep 1 sec before creating trigger file
 *  @version  6.3   ABE 2016-10-27        The trigger file schema now defines the
 *                                        default namespace "cheops:gs" for xml
 *                                        instances
 *  @version  6.2   ABE 2016-08-31 #11514 Add input/output processing number to
 *                                        TRIGGER file, rename observation id to
 *                                        request id
 *  @version  6.0   ABE 2016-06-29 #10960 Use validity start/stop from fixed
 *                                        header
 *  @version  5.0   ABE 2016-01-21 #9990  first version
 */

#include <fstream>
#include <sstream>
#include <time.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>

#include "FitsDalHeader.hxx"
#include "TriggerFile.hxx"
#include "trigger_file_schema.hxx"

using namespace std;

std::string TriggerFile::m_outputDirectory;
std::string TriggerFile::m_processorName;
PassId TriggerFile::m_passId;
PassId TriggerFile::m_prevPassId;
VisitId TriggerFile::m_visitId;
UTC TriggerFile::m_creationDate;
UTC TriggerFile::m_validityStart;
UTC TriggerFile::m_validityStop;
uint16_t    TriggerFile::m_inputProcessingNumber;
uint16_t    TriggerFile::m_outputProcessingNumber;
uint32_t    TriggerFile::m_obsid = 0;
bool        TriggerFile::m_isEmpty = true;


/** ****************************************************************************
 * @brief Returns a valid trigger file name.
 *
 * @ingroup utilities
 * @author Anja Bekkelien UGE
 *
 * If a processor name is provided, the format of the file name is:
 *
 * path/CH_visitId_passId_utc_TRIGGER-processorName_V000.xml
 *
 * Otherwise the processor name is left out of the file name:
 *
 * path/CH_visitId_passId_utc_TRIGGER_V000.xml
 *
 * @param [in] path          the directory in which the output file will be
 *                           saved.
 * @param [in] processorName the processor name as defined in the job order
 *                           element "Processor_Name".
 * @param [in] validityStart the validity start.
 * @param [in] visitId       the visit id.
 * @param [in] passId        the pass id.
 * @param [in] dataName      an additional identifier of the data
 *
 * @return a file name created from the input arguments.
 */
std::string BuildTriggerFileName(const std::string & path,
                                 const std::string processorName,
                                 const UTC & validityStart,
                                 const VisitId & visitId,
                                 const PassId & passId,
                                 const std::string & dataName) {

   std::string fileName = path.empty() ? "." : path;

   fileName += "/CH";

   // the program and the visit as PRppnnnn_TGttttnn_
   if (visitId.isValid())
      fileName += "_" + visitId.getFileNamePattern();

   // the pass ID if it is defined
   if (passId.isValid())
      fileName += "_" + passId.getFileNamePattern();

   // the time
   fileName += "_" + validityStart.getFileNamePattern();

   // the name of the data structure
   fileName += "_TRIGGER";

   // the processor name
   if (!processorName.empty())
     // remove any file extension, as dots inside the log file name are
     // problematic for monitor4EO
     fileName += "-" + boost::filesystem::path(processorName).stem().string();

   // the data name
   if (!dataName.empty())
     fileName += "-" + dataName;

   // the file version
   char hstr[10];
   sprintf(hstr, "_V%04hu", FitsDalHeader::getDataVersion());
   fileName += hstr;

   fileName += ".xml";

   return fileName;

}

/** ****************************************************************************
 * @param [in] directory              the directory in which the file will be
 *                                    written
 * @param [in] processorName          the processor name as defined in the job
 *                                    order element Processor_Name
 * @param [in] passId                 a pass id formatted as 'yymmddhh'
 * @param [in] programmeType          the program type
 * @param [in] programmeId            the program id
 * @param [in] requestId              the observation request id
 * @param [in] visitCounter           the visit counter
 * @param [in] validityStart          the validity start time of the telemetry
 *                                    data for which the TRIGGER file is
 *                                    created, formatted as a UTC time:
 *                                    yyyy-mm-ddThh:mm:ss
 * @param [in] validityStop           the validity stop time of the telemetry
 *                                    data for which the TRIGGER file is
 *                                    created, formatted as a UTC time:
 *                                    yyyy-mm-ddThh:mm:ss
 * @param [in] inputProcessingNumber  the processing number of the program's
 *                                    input files
 * @param [in] outputProcessingNumber the processing number of the program's
 *                                    output files
 * @param [in] obsid                  the observation id
 * @param [in] dataName               (optional) An additional identifier of
 *                                    the data
 * @param [in] creationDate           (optional) The creation date of the
 *                                    trigger file. Default value is the current
 *                                    time.
 * @param [in] prevPassId             (optional) The id of the previous pass
 *
 * @return the path to the created trigger file
 */
string TriggerFile::WriteTriggerFile(const std::string & directory,
                                     const std::string & processorName,
                                     const PassId & passId,
                                     uint8_t programmeType,
                                     uint16_t programmeId,
                                     uint16_t requestId,
                                     uint8_t visitCounter,
                                     const std::string & validityStart,
                                     const std::string & validityStop,
                                     uint16_t inputProcessingNumber,
                                     uint16_t outputProcessingNumber,
                                     uint32_t obsid,
                                     const std::string & dataName,
                                     const std::string & creationDate,
                                     const PassId & prevPassId) {

  // Validate input values
  VisitId vi;
  UTC vstart;
  UTC vstop;
  UTC cdate;

  try {
    vi = VisitId(programmeType, programmeId, requestId, visitCounter);
  }
  catch(exception & e) {
	vi = VisitId();
  }

  try {
    vstart = validityStart.empty() ? UTC() : UTC(validityStart);
  }
  catch(exception & e) {
    throw runtime_error("Error writing TRIGGER file: invalid validity start time [" + validityStart
                        + "]. Expected UTC time formatted as yyyy-mm-ddThh:mm:ss or yyyy-mm-ddThh:mm:ss.ffffff or an empty string");
  }

  try {
    vstop = validityStop.empty() ? UTC() : UTC(validityStop);
  }
  catch(exception & e) {
    throw runtime_error("Error writing TRIGGER file: invalid validity stop time [" + validityStop
                        + "]. Expected UTC time formatted as yyyy-mm-ddThh:mm:ss or yyyy-mm-ddThh:mm:ss.ffffff or an empty string");
  }

  try {
    cdate = creationDate.empty() ? UTC() : UTC(creationDate);
  }
  catch(exception & e) {
    throw runtime_error("Error writing TRIGGER file: invalid creation time [" + creationDate
                        + "]. Expected UTC time formatted as yyyy-mm-ddThh:mm:ss or yyyy-mm-ddThh:mm:ss.ffffff or an empty string");
  }

  return TriggerFile::WriteTriggerFile(directory, processorName,
                                       passId, vi, vstart, vstop,
                                       inputProcessingNumber, outputProcessingNumber,
                                       obsid, dataName, cdate, prevPassId);
}

/** ****************************************************************************
 * @param [in] directory              the directory in which the file will be
 *                                    written
 * @param [in] processorName          the processor name as defined in the job
 *                                    order element Processor_Name
 * @param [in] passId                 a pass id
 * @param [in] visitId                a visit id
 * @param [in] validityStart          the validity start time
 * @param [in] validityStop           the validity stop time
 * @param [in] inputProcessingNumber  the processing number of the program's
 *                                    input files
 * @param [in] outputProcessingNumber the processing number of the program's
 *                                    output files
 * @param [in] obsid                  an observation id
 * @param [in] dataName               (optional) An additional identifier of
 *                                    the data
 * @param [in] creationDate           (optional) The creation date of the
 *                                    trigger file. Default value is the current
 *                                    time.
 * @param [in] prevPassId             (optional) The id of the previous pass
 *
 * @return the path to the created trigger file
 */
std::string TriggerFile::WriteTriggerFile(const std::string & directory,
                                          const std::string & processorName,
                                          const PassId & passId,
                                          const VisitId & visitId,
                                          const UTC & validityStart,
                                          const UTC & validityStop,
                                          uint16_t inputProcessingNumber,
                                          uint16_t outputProcessingNumber,
                                          uint32_t obsid,
                                          const std::string & dataName,
                                          const UTC & creationDate,
                                          const PassId & prevPassId) {

  // An empty string makes boost::filesystem::exists throw an exception
  std::string outDirectory = directory.empty() ? "." : std::string(directory);

  // Create output directory if it does not exist
  if (!boost::filesystem::exists(outDirectory)) {
    logger << info << "Creating output directory [" << outDirectory << "]" << std::endl;
    boost::filesystem::create_directories(outDirectory);
  }

  string triggerFileName = BuildTriggerFileName(outDirectory, processorName, validityStart, visitId, passId, dataName);

  // Earth Explorer Fixed Header values
  string fileName = boost::filesystem::path(triggerFileName).stem().string();
  string fileDescription;
  string notes = "";
  string mission = "CHEOPS";
  string fileClass = "";
  string fileType = "TRIGGER";
  trigger_file_schema::Validity_Period::Validity_Start_type _vstart("UTC=" + validityStart.getUtc(true));
  trigger_file_schema::Validity_Period::Validity_Stop_type _vstop("UTC=" + validityStop.getUtc(true));
  trigger_file_schema::Fixed_HeaderType::Validity_Period_type validityPeriod(_vstart, _vstop);
  char hstr[10];
  sprintf(hstr, "%04hu", FitsDalHeader::getDataVersion());
  string fileVersion = hstr;
  string system = "SOC";
  string creatorVersion = "1.0";

  UTC _creationDate;
  // Set the creation time to the current gmt time
  if (creationDate.empty()) {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *gmtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%X", &tstruct);
    _creationDate = UTC(string(buf));
  }
  else {
    _creationDate = creationDate;
  }

  trigger_file_schema::Fixed_HeaderType::Source_type source(system,
                                                            processorName,
                                                            creatorVersion,
                                                            trigger_file_schema::eeDateTime("UTC=" + _creationDate.getUtc(true)));
  trigger_file_schema::Fixed_HeaderType fixedHeader(fileName,
                                                    fileDescription,
                                                    notes,
                                                    mission,
                                                    fileClass,
                                                    fileType,
                                                    validityPeriod,
                                                    fileVersion,
                                                    source);

  // Earth Explorer Variable header values
  trigger_file_schema::Variable_HeaderType variableHeader((std::string) passId,
                                                          (std::string) prevPassId,
                                                          visitId.getProgramType(),
                                                          visitId.getProgramId(),
                                                          visitId.getRequestId(),
                                                          visitId.getVisitCounter(),
                                                          obsid,
                                                          inputProcessingNumber,
                                                          outputProcessingNumber);

  trigger_file_schema::Earth_Explorer_HeaderType eeHeader(fixedHeader, variableHeader);

  // The root element of the output xml file
  trigger_file_schema::Earth_Explorer_FileType eeFile(eeHeader);

  // Pass namespace information to the object model
  xml_schema::namespace_infomap map;
  map[""].name = "cheops:gs";
  map[""].schema = "trigger_file_schema.xsd";

  // Sleep before creating the trigger file, to ensure that the trigger file is
  // the last output file to be written to disk
  boost::this_thread::sleep( boost::posix_time::seconds(1) );

  // Serialize the object model to XML.
  logger << info << "Writing trigger file [" + triggerFileName + "]" << std::endl;
  std::ofstream ofs (triggerFileName);
  trigger_file_schema::Earth_Explorer_File(ofs, eeFile, map);

  return triggerFileName;
}

/** ************************************************************************
 *  The values of the input arguments should all be taken from the job order
 *  file via the ProgramParams class.
 *
 *  If the trigger file list is empty, then the validity time of the
 *  TriggerClass is set to the job order start and stop times arguments.
 *
 *  If the trigger file list contains exactly one file, then that file is read
 *  and the job order start and stop time is ignored.
 *
 *  If the trigger file list contains more than one file, a runtime_error is
 *  thrown.
 *
 *  @param [in] outDir            The job order's output directory
 *  @param [in] processorName     The processor name
 *  @param [in] triggerFiles      The job order's list of input TRIGGER files.
 *                                This list can be empty, in which case the
 *                                validity start and stop of this class is set
 *                                to that of the jobOrderStartTime and
 *                                jobOrderStopTime arguments.
 *  @param [in] jobOrderStartTime The value of the Start parameter of the job
 *                                order, formatted as a UTC string.
 *  @param [in] jobOrderStopTime The value of the Stop parameter of the job
 *                                order, formatted as a UTC string.
 */
void TriggerFile::Initialize(const std::string & outDir,
                             const std::string & processorName,
                             const std::list<std::string>& triggerFiles,
                             const UTC & jobOrderStartTime,
                             const UTC & jobOrderStopTime) {

  m_outputDirectory = outDir;
  m_processorName = processorName;

  // There shall not be more that one trigger files
  if (triggerFiles.size() > 1) {
    throw runtime_error("Found [" + std::to_string(triggerFiles.size())
    + "] trigger files, expected 0 or 1.");
  }
  else if (triggerFiles.size() == 1) {
    ReadTriggerFile(triggerFiles.front());
  }
  // No input trigger file, make values from the job order available for the
  // output trigger file
  else {
    m_validityStart = jobOrderStartTime;
    m_validityStop = jobOrderStopTime;
    m_passId = PassId();
    m_prevPassId = PassId();
    m_visitId = VisitId();
    m_obsid = 0;
    m_creationDate = UTC();
    m_inputProcessingNumber = 0;
    m_outputProcessingNumber = 0;
    m_isEmpty = true;
  }
}


/** ************************************************************************
 *  The values of the input arguments should all be taken from the job order
 *  file via the ProgramParams class.
 *
 *  The triggerFiles list shall contain exactly one trigger file. Otherwise,
 *  a runtime_error is thrown.
 *
 *  @param [in] outDir            The job order's output directory
 *  @param [in] processorName     The processor name
 *  @param [in] triggerFiles      The job order's list of input TRIGGER files.
 */
void TriggerFile::Initialize(const std::string & outDir,
                       const std::string & processorName,
                       const std::list<std::string> & triggerFiles) {

  if (triggerFiles.size() != 1) {
    throw runtime_error("Found [" + std::to_string(triggerFiles.size())
    + "] trigger files, expected 1.");
  }
  TriggerFile::Initialize(outDir,
                          processorName,
                          triggerFiles,
                          UTC(), UTC());
}


/** ************************************************************************
 *  The values of the input arguments should all be taken from the job order
 *  file via the ProgramParams class.
 *
 *  @param [in] outDir            The job order's output directory
 *  @param [in] processorName     The processor name
 *  @param [in] jobOrderStartTime The value of the Start parameter of the job
 *                                order converted to a UTC object.
 *  @param [in] jobOrderStopTime The value of the Stop parameter of the job
 *                                order converted to a UTC object.
 */
void TriggerFile::Initialize(const std::string & outDir,
                       const std::string & processorName,
                       const UTC & jobOrderStartTime,
                       const UTC & jobOrderStopTime) {

  TriggerFile::Initialize(outDir,
                          processorName,
                          std::list<std::string>(),
                          jobOrderStartTime,
                          jobOrderStopTime);
}

//
///** ************************************************************************
// *  If a job order was given as input to the program but it does not contain a
// *  TRIGGER input data structure, then only the out directory, the processor
// *  name and the validity time of the job order is stored. The other variables
// *  are left with their default values.
// *
// *  If no job order was given to the program, then the output directory will be
// *  the current directory and the processor name will be an empty string. All
// *  other variables will take their default values.
// *
// *  @param [in] params the program's input parameters.
// */
//void TriggerFile::ReadTriggerFile(ParamsPtr & params) {
//
//  // Try to read the processor name from the job order
//  try {
//    m_processorName = params->GetAsString("Processor_Name");
//  } catch (exception) {
//    m_processorName = "";
//  }
//
//  m_outputDirectory = params->GetOutDir();
//
//  // Find all trigger data structures defined in the job order
//  std::list<std::string> triggerStructs;
//  for (auto & structName : params->GetInputStructNames()) {
//    if (structName.compare("TRIGGER") == 0) {
//      triggerStructs.push_back(structName);
//    }
//  }
//
//  // Throw exception if there are more than one trigger file
//  if (triggerStructs.size() > 1) {
//    throw runtime_error("Found [" + std::to_string(triggerStructs.size())
//    + "] trigger structures in job order file, expected 1.");
//  }
//
//  // One input trigger file is available, read it
//  else if (triggerStructs.size() == 1) {
//    const std::list<std::string> & triggerFiles = params->GetInputFiles(triggerStructs.front());
//
//    // There shall be exactly one trigger file
//    if (triggerFiles.size() != 1) {
//      throw runtime_error("Found [" + std::to_string(triggerFiles.size())
//      + "] trigger files of type [" + triggerStructs.front() + "], expected 1.");
//    }
//
//    ReadTriggerFile(triggerFiles.front());
//  }
//
//  // No input trigger file, make values from the job order available for the
//  // output trigger file
//  else if (triggerStructs.size() == 0) {
//    m_validityStart = UTC(params->GetAsString("Start"));
//    m_validityStop = UTC(params->GetAsString("Stop"));
//    m_passId = PassId();
//    m_prevPassId = PassId();
//    m_visitId = VisitId();
//    m_obsid = 0;
//    m_creationDate = UTC();
//    m_inputProcessingNumber = 0;
//    m_outputProcessingNumber = 0;
//    m_isEmpty = true;
//  }
//}

/** ****************************************************************************
 *  Note: this method is used by the Python interface for fits_data_model which
 *  cannot access to the C++ ProgramParams class.
 *
 *  @param [in] path            the path to the trigger file to read
 */
void TriggerFile::ReadTriggerFile(const std::string & path) {

  if (!boost::filesystem::exists(path))
     throw runtime_error("Could not find trigger file " + path);

  // get the schema file name of the trigger file file.
  // it has to be : $CHOEPS_SW/resources/trigger_file_schema.xsd
  char * cheopsSw = getenv("CHEOPS_SW");
  if (cheopsSw == NULL)
     throw runtime_error("Environment variable CHEOPS_SW is not defined. "
                              "It is used to access the file $CHEOPS_SW/resources/trigger_file_schema.xsd.");

  string schemaFileName(cheopsSw);
  schemaFileName += "/resources/trigger_file_schema.xsd";

  if (!boost::filesystem::exists(schemaFileName))
       throw runtime_error("Failed to find schema of trigger file: " + schemaFileName);

  logger << info << "Reading trigger file [" << path << "]" << std::endl;

  // prepare to use schema file $CHOEPS_SW/resources/trigger_file_schema.xsd
  xml_schema::properties props;
  props.schema_location ("cheops:gs", schemaFileName.c_str());

  try {
    auto_ptr<trigger_file_schema::Earth_Explorer_FileType> eef(trigger_file_schema::Earth_Explorer_File(path, 0, props));
    trigger_file_schema::Earth_Explorer_HeaderType::Fixed_Header_type fixedHeader = eef->Earth_Explorer_Header().Fixed_Header();
    trigger_file_schema::Earth_Explorer_HeaderType::Variable_Header_type variableHeader = eef->Earth_Explorer_Header().Variable_Header();

    m_passId = PassId(variableHeader.PASS_ID());
    m_prevPassId = PassId(variableHeader.PREV_PASS_ID());

    // The visit ID can be invalid
    try {
      m_visitId = VisitId(variableHeader.PROGTYPE(),
                          variableHeader.PROG_ID(),
                          variableHeader.REQ_ID(),
                          variableHeader.VISITCTR());
    }
    catch(exception & e) {
      // It's not possible to instantiate an invalid visit ID by passing
      // arguments to the constructor
      m_visitId = VisitId();
    }

    m_obsid = variableHeader.OBSID();

    std::string cdate = fixedHeader.Source().Creation_Date();
    std::string vstart = fixedHeader.Validity_Period().Validity_Start();
    std::string vstop = fixedHeader.Validity_Period().Validity_Stop();
    if (boost::starts_with(cdate, "UTC=")) {
      cdate = cdate.substr(4);
    }
    if (boost::starts_with(vstart, "UTC=")) {
      vstart = vstart.substr(4);
    }
    if (boost::starts_with(vstop, "UTC=")) {
      vstop = vstop.substr(4);
    }
    m_creationDate = UTC(cdate);
    m_validityStart = UTC(vstart);
    m_validityStop = UTC(vstop);

    m_inputProcessingNumber = variableHeader.PROC_NUM();
    m_outputProcessingNumber = variableHeader.PROC_NUM_OUT();

    m_isEmpty = false;
  }
  // Error parsing trigger file. Method e.what() does not contain the full error
  // message
  catch (const xml_schema::exception& e) {
    std::stringstream message;
    message << e;
    logger << error << message.str() << std::endl;
    throw;
  }

}

