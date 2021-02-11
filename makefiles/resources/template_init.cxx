/** ****************************************************************************
 *  @file
 *
 *  @brief Implementation of the CheopsInit function
 *
 *  @author Reiner Rohlfs UGE
 *
 *  Automatically created file. Do not modify it!
 *
 *  @version 13.0   2020-09-17 ABE #21949 Show target name in alert emails.
 *  @version 12.1.3 2020-04-01 ABE #20715 Read the pass id from job order parameters.
 *  @version 11.1   2018-12-11 ABE #17462 Read PRP_VST1 and PRP_VSTN from
 *                                        MPS_PRE_Visits
 *  @version 10.5.1 2018-12-11 ABE #17373 New optional argument to CheopsInit
 *                                        for turning off verbose logging
 *  @version 10.2   2018-10-12 ABE #16882 TriggerFile interface update for
 *                                        passing in the job order validity time.
 *  @version  9.2   2018-05-15 ABE #15807 Set PIPE_VER in data product headers
 *  @version  9.2   2018-05-09 ABE #15652 new argument to
 *                                        TriggerFile::WriteTriggerFile(): prevPassId
 *  @version  9.1.2 2018-03-23 ABE #15770 log program params and file names
 *                                        after the log file stream has be created
 *  @version  9.0   2018-01-10 ABE #15286 add OBSID to log file name
 *  @version  7.4   2017-05-31 ABE #12948 Read and write the creation date
 *                                        of the trigger file.
 *  @version  7.3   2017-05-18 ABE #12941 add pass id, visit id, processing chain
 *                                        and program version to log file name
 *  @version  7.3   2017-05-10 ABE #12806 Read and write the OBSID from the trigger
 *                                        file.
 *  @version  6.3   2016-10-26 ABE #11800 call BarycentricOffset::setEphemerisFileName()
 *                                        if EXT_APP_DE1 struct is present in job
 *                                        order
 *  @version  6.3   2016-10-25 ABE        remove usage of obsolete data structure
 *                                        REF_APP_LeapSeconds
 *  @version  6.2   2016-08-31 ABE #11514 copy output processing version of input
 *                                        TRIGGER file into output TRIGGER
 *  @version  5.2   2016-05-09 ABE #10690 read log file path from job order's
 *                                        dynamic params
 *  @version  5.1   2016-03-22 RRO #10458 implementation of the OBT reset counter
 *  @version  4.2   2015-09-09 RRO #9180  get PROC_CHN from jog-order file.
 *  @version  3.3                         configure logger with values from
 *                                        program_params
 *  @version  1.0                         first released version
 *
 */

#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fstream>
#include <limits>

#include "Logger.hxx"

#ifndef NO_PROG_PARAMS
#include "ProgramParams.hxx"
#endif

#ifndef NO_FITS_DAL
#include "FitsDalHeader.hxx"
#include "FitsDalTable.hxx"
#endif

#ifndef NO_UTILITIES
#include "BarycentricOffset.hxx"
#include "LeapSeconds.hxx"
#include "ObtUtcCorrelation.hxx"
#include "TriggerFile.hxx"
#endif

/** ****************************************************************************
 *  This version of the CheopsInit function is used for the target_name
 *  program.
 */
#ifndef NO_PROG_PARAMS
ParamsPtr CheopsInit(int argc, char * argv[], bool verboseLogging) {

   std::string processorName = "target_name";
   uint16_t processingNumber = 0;
   uint16_t revisionNumber = 0;
   std::string pipelineVersion = "N/A";
   std::string visitId;
   std::string passId;
   uint32_t obsid = 0;
   std::string processingChain = "not defined";
   std::string targetName = "N/A";
   uint32_t propFirstVisit = std::numeric_limits<uint32_t>::max();
   uint32_t propLastVisit = std::numeric_limits<uint32_t>::max();
   std::string outLogLevel = "INFO";
   std::string errLogLevel = "PROGRESS";
   std::string alertEmail;
   std::string outDir;
   std::string logFile;

   Logger::SetProgramVersion("target_name",
                             "target_version");

   ParamsPtr params = ParamsPtr(new ProgramParams(argc, argv, "target_name", "target_version"));

   // Read log configuration params in try/catch blocks in case they are not
   // defined, or no job order file was provided to the program
   try {
     processorName = params->GetAsString("Processor_Name");
   } catch (std::runtime_error & e) {}

   try {
      outLogLevel = params->GetAsString("Stdout_Log_Level");
   } catch (std::runtime_error & e) {}

   try {
     errLogLevel = params->GetAsString("Stderr_Log_Level");
   } catch (std::runtime_error & e) {}

   try {
      alertEmail = params->GetAsString("Alert_Email");
   } catch (std::runtime_error & e) {}

   try {
     logFile = params->GetAsString("Log_File");
   } catch (std::runtime_error & e) {}

   try {
     passId = params->GetAsString("pass-id");
     // If the pass id does not start with "PS", add it to the beginning of the string.
     if (passId.substr(0, 2).compare("PS") != 0) {
       passId = "PS" + passId;
     }
   } catch (std::runtime_error & e) {}

   outDir = params->GetOutDir();

   try {
      // The revision number is the first two digits of the job order's Version element
      revisionNumber = atoi(params->GetAsString("Version").substr(0, 2).c_str());
   } catch (std::runtime_error & e) {}

   try {
      processingChain = params->GetAsString("PROC_CHN");
   }
   catch(std::runtime_error & e) {}

#ifndef NO_UTILITIES
   if (params->GetInputStructNames().count("TRIGGER")) {
     TriggerFile::Initialize(
         outDir,
         processorName,
         params->GetInputFiles("TRIGGER"));
   }
   else {
     UTC jobOrderStartTime;
     UTC jobOrderStopTime;
     try {
       jobOrderStartTime = params->GetAsString("Start");
     } catch (std::runtime_error &e) {
       jobOrderStartTime = UTC();
     }

     try {
       jobOrderStopTime = params->GetAsString("Stop");
     } catch (std::runtime_error &e) {
       jobOrderStopTime = UTC();
     }
     TriggerFile::Initialize(outDir,
                             processorName,
                             jobOrderStartTime,
                             jobOrderStopTime);
   }

   processingNumber = TriggerFile::GetOutputProcessingNumber();
   obsid = TriggerFile::GetObsid();

   if (TriggerFile::GetVisitId().isValid()) {
     visitId = TriggerFile::GetVisitId().getFileNamePattern();
   }

   if (TriggerFile::GetPassId().isValid()) {
     passId = TriggerFile::GetPassId().getFileNamePattern();
   }
#endif

#ifndef NO_FITS_DAL

   // Read the file containing the pipeline version. This file exists only if
   // the pipeline was installed using rpms. The file shall contain one
   // line with the pipeline version.
   const char * cheops_sw = getenv("CHEOPS_SW");
   if (cheops_sw != nullptr) {
     std::string pipelineVersionFile = std::string(cheops_sw) + "/conf/pipeline_version";
     std::string line;
     std::ifstream file(pipelineVersionFile);
     if (file.is_open()) {
       while (std::getline(file, line)) {
         boost::trim(line);
         if (line.size() > 0) {     // Ignore empty lines
           pipelineVersion = line;
           break;
         }
       }
       file.close();
     }
   }

   FitsDalHeader::SetProgram("target_name", "target_version", "svnversion", processingChain,
                              revisionNumber, processingNumber, pipelineVersion);
#endif

#ifndef NO_UTILITIES
   if (params->GetInputStructNames().count("SOC_APP_LeapSeconds")) {
      LeapSeconds::setLeapSecondsFileNames(params->GetInputFiles("SOC_APP_LeapSeconds"));
   }
   if (params->GetInputStructNames().count("AUX_RES_ObtUtcCorrelation")) {
      OBTUTCCorrelation::SetCorrelationFileNames(params->GetInputFiles("AUX_RES_ObtUtcCorrelation"));
   }
   if (params->GetInputStructNames().count("REF_APP_ObtReset")) {
      OBT::SetResetCounterFileNames(params->GetInputFiles("REF_APP_ObtReset"));
   }
   if (params->GetInputStructNames().count("EXT_APP_DE1") &&
       params->GetInputFiles("EXT_APP_DE1").size() > 0) {
      BarycentricOffset::setEphemerisFileName(params->GetInputFiles("EXT_APP_DE1").front());
   }
   
   // Read PROP_FIRST_VISIT and PROP_LAST_VISIT from the MPS_PRE_Visits, if present
   if (obsid != 0 && 
       params->GetInputStructNames().count("MPS_PRE_Visits") &&
       params->GetInputFiles("MPS_PRE_Visits").size() > 0) {
      bool visitFound = false;
      // Iterate over the files, starting with the newest file
      std::list<std::string> fileNames = params->GetInputFiles("MPS_PRE_Visits");
      for (auto it = fileNames.rbegin(); it != fileNames.rend(); ++it) {
         FitsDalTable * table = new FitsDalTable(*it);
         uint32_t colObsid;
         uint32_t colPropFirstVisit;
         uint32_t colPropLastVisit;
         std::string colTargetName;
         table->Assign("OBSID", &colObsid);
         table->Assign("PROP_FIRST_VISIT", &colPropFirstVisit);
         table->Assign("PROP_LAST_VISIT", &colPropLastVisit);
         table->Assign("TARGET_NAME", &colTargetName, 24);
         while (table->ReadRow()) {
            // Break out of the loop once the visit has been found
            if (colObsid == obsid) {
               propFirstVisit = colPropFirstVisit;
               propLastVisit = colPropLastVisit;
               targetName = colTargetName;
               visitFound = true;
               break;
            }
         };
         delete table;
         if (visitFound) {
            break;
         }
      }
   }

#endif

   Logger::Configure(outLogLevel,
                     errLogLevel,
                     alertEmail,
                     outDir,
                     logFile,
                     revisionNumber,
                     processingNumber,
                     visitId,
                     passId,
                     obsid,
                     processingChain,
                     targetName);

   if (verboseLogging) {
     params->LogProgramParams(
         {{"PRP_VST1", std::to_string(propFirstVisit)},
          {"PRP_VSTN", std::to_string(propLastVisit)}});
   }
   return params;
}

/** ****************************************************************************
 *  This version of the CheopsExit function is used for the target_name
 *  program.
 */
void CheopsExit(bool createTriggerFile) {
#ifndef NO_UTILITIES
  if (createTriggerFile) {

    TriggerFile::WriteTriggerFile(
        TriggerFile::GetOutputDirectory(),
        (TriggerFile::GetProcessorName().empty() ? "target_name" : TriggerFile::GetProcessorName()),
        TriggerFile::GetPassId(),
        TriggerFile::GetVisitId(),
        TriggerFile::GetValidityStart(),
        TriggerFile::GetValidityStop(),
        TriggerFile::GetOutputProcessingNumber(),
        TriggerFile::GetOutputProcessingNumber(),
        TriggerFile::GetObsid(),
        "",
        TriggerFile::GetCreationDate(),
        TriggerFile::GetPrevPassId());
  }
#endif
}
#else
/** ****************************************************************************
 *  This version of the CheopsInit function is used for the target_name
 *  program.
 */
void CheopsInit(int argc, char * argv[], bool verboseLogging) {

   Logger::SetProgramVersion("target_name", "target_version");

#ifndef NO_FITS_DAL
   FitsDalHeader::SetProgram("target_name", "target_version", "svnversion");
#endif

}

/** ****************************************************************************
 *  This version of the CheopsExit function is used for the target_name
 *  program.
 */
void CheopsExit(bool createTriggerFile) {

}
#endif
