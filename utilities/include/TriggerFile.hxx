/** ****************************************************************************
 *  @file
 *  @ingroup utilities
 *  @brief Definition of the TriggerFile class.
 *
 *  @author Anja Bekkelien UGE
 *
 *  @version 10.2 ABE 2018-10-12 #16882 interface update for passing in the job
 *                                      order validity time.
 *  @version  9.2 ABE 2018-05-09 #15652 new mandatory attribute in trigger file
 *                                      schema, PREV_PASS_ID
 *  @version  7.4 ABE 2017-05-31 #12948 Read and write the creation date
 *                                      of the trigger file.
 *  @version  7.3 ABE 2017-05-10 #12806 Add new method GetObsid()
 *  @version  7.3 ABE 2017-05-08 #13138 GetProgramId() and GetProgramType() have
 *                                      been replaced by GetProgrammeId() and
 *                                      GetProgrammeType()
 *  @version  7.3 ABE 2017-05-03 #12927 Add new method Empty() for checking if a
 *                                      trigger file has been read.
 *  @version  6.3 ABE 2016-10-24        Remove deprecated methods getObservationId()
 *                                      and getProcessingVersion()
 *  @version  6.2 ABE 2016-08-31 #11514 Add input/output number to TRIGGER file,
 *                                      rename observation id to request id
 *  @version  6.0 ABE 2016-06-29 #10960 Use validity start/stop from fixed
 *                                      header
 *  @version  5.0 ABE 2016-01-21 #9990  first version
 *
 */

#ifndef UTILITIES_INCLUDE_TRIGGERFILE_HXX_
#define UTILITIES_INCLUDE_TRIGGERFILE_HXX_

#include <string>

#include "Logger.hxx"
#include "PassId.hxx"
#include "ProgramParams.hxx"
#include "Utc.hxx"
#include "VisitId.hxx"

// NOTE: Further high level documentation is found in doxygen/TriggerFile.txt.
//       This file is also used for the python interface.
/** ****************************************************************************
 *  @brief   Class that implements functionality for reading and writing trigger
 *           files.
 *  @ingroup utilities
 *  @author  Anja Bekkelien, UGE
 *
 *  The following code example shows how to retrieve values from the
 *  trigger file:
 *
 *  @code
 *  #include "TriggerFile.hxx"
 *  #include "ProgramParams.hxx"
 *
 *  // ...
 *
 *  // Read the trigger file, if it is present in the job order file
 *  ParamsPtr params = CheopsInit(argc, argv);
 *
 *  // Returns true if no trigger file was present in the job order, and false
 *  // if a trigger file was present and read by CheopsInit()
 *  bool empty = TriggerFile::Empty();
 *
 *  std::string passId = TriggerFile::GetPassId();
 *  uint8_t programType = TriggerFile::GetProgrammeType();
 *  uint16_t programId = TriggerFile::GetProgrammeId();
 *  uint16_t requestId = TriggerFile::GetRequestId();
 *  uint8_t visitCounter = TriggerFile::GetVisitCounter();
 *  std::string validityStart = TriggerFile::GetValidityStart();
 *  std::string validityStop = TriggerFile::GetValidityStop();
 *
 *  // Creates an output trigger file. False can be passed as a method argument
 *  // to prevent this creation: CheopsExit(false). In this case, the trigger
 *  // file should be created by calling TriggerFile::WriteTriggerFile() at the
 *  // end of the main method.
 *  CheopsExit();
 *  @endcode
 *
 *  If a trigger file is not present in the job order, the method
 *  `TriggerFile::Empty()` will return true, and the static methods to retrieve
 *  the pass id, visit id and obsid will return default values.
 *
 *  Trigger files can be written manually by calling the static method
 *  `TriggerFile::WriteTriggerFile()`. It is possible to initialize the values
 *  of the TriggerFile class by reading in a trigger file directly:
 *  `TriggerFile::ReadTriggerFile(const std::string & path);`
 *
 */
class TriggerFile {

 private:

  /**
   * The directory to write trigger files to.
   */
  static std::string m_outputDirectory;

  /**
   * The processor name as defined in the job order element Processor_Name.
   */
  static std::string m_processorName;

  /**
   * The trigger file's pass id.
   */
  static PassId m_passId;

  /**
   * The id of the previous pass.
   */
  static PassId m_prevPassId;

  /**
   * The trigger file's visit id.
   */
  static VisitId m_visitId;

  /**
   * The creation date of the trigger file.
   */
  static UTC m_creationDate;

  /**
   * The validity start time of the trigger file.
   */
  static UTC m_validityStart;

  /**
   * The validity stop time of the trigger file.
   */
  static UTC m_validityStop;

  /**
   * The processing number of the program's input files.
   */
  static uint16_t m_inputProcessingNumber;

  /**
   * The processing number of the program's output files.
   */
  static uint16_t m_outputProcessingNumber;

  /**
   * The obsid.
   */
  static uint32_t m_obsid;

  /**
   * This variable is initially set to true, and is set to false when a trigger
   * file has been read.
   */
  static bool m_isEmpty;

  /** ************************************************************************
   * @brief This static method serves as the interface to the program_params
   *        module, and is used to initialize this class with the values
   *        defined in the job order and in the input TRIGGER file, if present.
   */
  static void Initialize(const std::string & outDir,
                         const std::string & processorName,
                         const std::list<std::string> & triggerFiles,
                         const UTC & jobOrderStartTime,
                         const UTC & jobOrderStopTime);

 public:

  /** *************************************************************************
   *  @brief Empty constructor.
   */
  TriggerFile() {};

  /** ************************************************************************
   * @brief This static method serves as the interface to the program_params
   *        module, and is used to initialize this class with the values
   *        defined in the job order when an input TRIGGER file is present.
   */
  static void Initialize(const std::string & outDir,
                         const std::string & processorName,
                         const std::list<std::string> & triggerFiles);

  /** ************************************************************************
   * @brief This static method serves as an an interface to the program_params
   *        module, and is used to initialize this class with the values
   *        defined in the job order when no TRIGGER files are present.
   */
  static void Initialize(const std::string & outDir,
                         const std::string & processorName,
                         const UTC & jobOrderStartTime,
                         const UTC & jobOrderStopTime);

  /** ************************************************************************
   *  @brief Writes a trigger file.
   */
  static std::string WriteTriggerFile(const std::string & directory,
                                      const std::string & processorName,
                                      const PassId & passId,
                                      const VisitId & visitId,
                                      const UTC & validityStart,
                                      const UTC & validityStop,
                                      uint16_t inputProcessingNumber,
                                      uint16_t outputProcessingNumber,
                                      uint32_t obsid = 0,
                                      const std::string & dataName = std::string(""),
                                      const UTC & creationDate = UTC(),
                                      const PassId & prevPassId = PassId());

  /** ************************************************************************
   *  @brief Writes a trigger file.
   */
  static std::string WriteTriggerFile(const std::string & directory,
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
                                      uint32_t obsid = 0,
                                      const std::string & dataName = std::string(""),
                                      const std::string & creationDate = std::string(""),
                                      const PassId & prevPassId = PassId());

//
//  /** ************************************************************************
//   * @brief Reads a trigger file from a job order file that has been read by
//   *        the ProgramParams object, and stores its values in static member
//   *        variables.
//   */
//  static void ReadTriggerFile(ParamsPtr & params);

  /** ************************************************************************
   * @brief Reads a trigger path and stores its values in member variables.
   */
  static void ReadTriggerFile(const std::string & path);


  /** ************************************************************************
   * @brief Returns the output directory for trigger files.
   *
   * @return the output directory for trigger files.
   */
  static std::string GetOutputDirectory() { return m_outputDirectory; }

  /** ************************************************************************
   * @brief Sets the output directory for trigger files.
   *
   * The output directory is only available from the job order. The default
   * value is an empty string.
   *
   * @param outputDirectory the output directory for trigger files.
   */
  static void SetOutputDirectory(const std::string & outputDirectory) {
    m_outputDirectory = outputDirectory;
  }
  
  /** ************************************************************************
   * @brief Returns the processor name.
   *
   * @return the processor name.
   */
  static std::string GetProcessorName() { return m_processorName; }

  /** ************************************************************************
   * @brief Sets the processor name.
   *
   * @param processorName the processor name.
   */
  static void SetProcessorName(const std::string & processorName) {
    m_processorName = processorName;
  }
  
  /** ************************************************************************
   * @brief Returns the pass id formatted as 'yymmddhh'.
   *
   * @return the pass id formatted as 'yymmddhh'.
   */
  static PassId GetPassId() { return m_passId; }

  /** ************************************************************************
   * @brief Returns the id of the previous pass, formatted as 'yymmddhh'.
   *
   * @return the id of the previous pass, formatted as 'yymmddhh'.
   */
  static PassId GetPrevPassId() { return m_prevPassId; }

  /** ************************************************************************
   * @brief Returns the visit id.
   *
   * @return the visit id.
   */
  static VisitId GetVisitId() { return m_visitId; }

  /** ************************************************************************
   * @deprecated This method is replaced by GetProgrammeType().
   *
   * @brief Returns the program type.
   *
   * @return the program type.
   */
  static uint8_t GetProgramType() { return m_visitId.getProgramType(); }

  /** ************************************************************************
   * @brief Returns the program type.
   *
   * @return the program type.
   */
  static uint8_t GetProgrammeType() { return m_visitId.getProgramType(); }

  /** ************************************************************************
   * @deprecated This method is replaced by GetProgrammeId().
   *
   * @brief Returns the program id.
   *
   * @return the program id.
   */
  static uint16_t GetProgramId() { return m_visitId.getProgramId(); }

  /** ************************************************************************
   * @brief Returns the program id.
   *
   * @return the program id.
   */
  static uint16_t GetProgrammeId() { return m_visitId.getProgramId(); }


  /** ************************************************************************
   * @brief Returns the observation request id.
   *
   * @return the observation request id.
   */
  static uint16_t GetRequestId() { return m_visitId.getRequestId(); }

  /** ************************************************************************
   * @brief Returns the visit counter.
   *
   * @return the visit counter.
   */
  static uint8_t GetVisitCounter() { return m_visitId.getVisitCounter(); }

  /** ************************************************************************
   * @brief Returns the creation date of the trigger file.
   *
   * @return the creation date formatted as yyyy-mm-ddThh:mm:ss.
   */
  static UTC GetCreationDate() { return m_creationDate; }

  /** ************************************************************************
   * @brief Returns the validity start time of the trigger file.
   *
   * @return the validity start formatted as yyyy-mm-ddThh:mm:ss.
   */
  static UTC GetValidityStart() { return m_validityStart; }

  /** ************************************************************************
   * @brief Returns the validity stop time of the trigger file.
   *
   * @return the validity stop formatted as yyyy-mm-ddThh:mm:ss.
   */
  static UTC GetValidityStop() { return m_validityStop; }

  /** ************************************************************************
   * @brief Returns the processing number of the program's input files.
   *
   * @return the processing number of the program's input files.
   */
  static uint16_t GetInputProcessingNumber() { return m_inputProcessingNumber; }

  /** ************************************************************************
   * @brief Returns the processing number of the program's output files.
   *
   * @return the processing number of the program's output files.
   */
  static uint16_t GetOutputProcessingNumber() { return m_outputProcessingNumber; }

  /** ************************************************************************
   * @brief Returns the OBSID.
   *
   * @return the OBSID.
   */
  static uint32_t GetObsid() { return m_obsid; }

  /** ************************************************************************
   * @brief Returns true if no trigger file has been read, otherwise false.
   *
   * @return true if no trigger file has been read, otherwise false.
   */
  static bool Empty() { return m_isEmpty; }

};

#endif /* UTILITIES_INCLUDE_TRIGGERFILE_HXX_ */

