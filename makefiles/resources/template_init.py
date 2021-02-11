##
#  @file
#
#  @brief Implementation of the cheopsInit function
#
#  @author Anja Bekkelien UGE
#
#  @version 13.0   2020-09-17 ABE #21949 Show target name in alert emails.
#  @version 12.1.3 2020-04-01 ABE #20715 Read the pass id from job order parameters.
#  @version 11.1   2018-12-11 ABE #17462 Read PRP_VST1 and PRP_VSTN from
#                                        MPS_PRE_Visits
#  @version 10.5.1 2018-12-11 ABE #17373 New optional argument to CheopsInit
#                                        for turning off verbose logging
#  @version 10.2   2018-10-12 ABE #16882 TriggerFile interface update for
#                                        passing in the job order validity time.
#  @version  9.2   2018-05-15 ABE #15807 Set PIPE_VER in data product headers
#  @version  9.2   2018-05-09 ABE #15652 new argument to 
#                                        TriggerFile.writeTriggerFile(): prevPassId
#  @version  9.1.2 2018-03-23 ABE #15770 log program params and file names
#                                        after the log file stream has be created
#  @version  9.0   2018-01-10 ABE #15286 add OBSID to log file name
#  @version  7.4   2017-05-31 ABE #12948 Read and write the creation date
#                                        of the trigger file.
#  @version  7.3   2017-05-18 ABE #12941 add pass id, visit id, processing chain
#                                        and program version to log file name
#  @version  7.3   2017-05-10 ABE #12806 Read and write the OBSID from the trigger
#                                        file.
#  @version  6.4   2016-12-15 ABE #12231 Bugfix: correctly check number of 
#                                        EXT_APP_DE1 structs in job order
#  @version  6.3   2016-10-26 ABE #11800 call BarycentricOffset::setEphemerisFileName()
#                                        if EXT_APP_DE1 struct is present in job
#                                        order
#  @version  6.3   2016-10-25 ABE        remove usage of obsolete data structure
#                                        REF_APP_LeapSeconds
#  @version  6.2   2016-09-12 ABE #11463 set archive revision number and processing
#                                        number in fits headers
#  @version  6.2   2016-08-31 ABE #11514 copy output processing version of input
#                                        TRIGGER file into output TRIGGER
#  @version  5.2   2016-05-09 ABE #10690 read log file path from job order's
#                                        dynamic params
#  @version  5.1   2016-03-22 RRO #10458 implementation of the OBT reset counter
#  @version  4.2                         call fits_data_model.setProgram(...)
#  @version  3.3                         configure logger with values from 
#                                        program_params
#  @version  1.0                         first released version
#/

import log.config
import os.path

## The name of the executable
processorName = "target_name"
## The processor's version
processorVersion = "target_version"
## The SVN revision number
svnRevision = "svnversion"


# Check which modules are available
try:
    import program_params
    ## True if the program_params module could be imported
    _progParamsAvailable = True
except ImportError:
    _progParamsAvailable = False
try:
    import fits_data_model
    ## True if the fits_data_model module could be imported
    _fitsDataModelAvailable = True
except ImportError:
    _fitsDataModelAvailable = False
        
##
#  @brief Implementation of the cheopsInit function
#  @author Anja Bekkelien UGE
#  @ingroup ProgParam
#
def cheopsInit(verboseLogging=True):
    
    log.config.configure(processorName, processorVersion)

    ## The std out log level
    outLogLevel = 'INFO'
    ## The std err log level
    errLogLevel = 'PROGRESS'
    ## The address that receives alert emails
    alertEmail = ''
    ## The directory to which log files are saved
    outDir = None 
    ## The path to the log file
    logFile = None
    ## The archive revision number
    revisionNumber = 0
    ## The processing number
    processingNumber = 0
    ## The pipeline version
    pipelineVersion = 'N/A'
    ## The visit id
    visitId = None
    ## The pass id
    passId = None
    ## The obsid
    obsid = 0
    ## The object containing the program's input parameters
    progParams = None
    ## The processing chain
    processingChain = None
    ## The name of the target
    targetName = 'N/A'
    ## Proprietary period of the first visit. Default value is the max value of
    ## an unsigned 32 bit integer
    propFirstVisit = 4294967295
    ## Proprietary period of the last visit. Default value is the max value of
    ## an unsigned 32 bit integer
    propLastVisit = 4294967295
    
    if _progParamsAvailable:
        
        ## The programs input parameters
        progParams = program_params.ProgramParams(processorName, processorVersion)
       
        # Read log configuration params in try/except blocks in case they are not
        # defined, or no job order file was provided to the program
        try:
            outLogLevel = progParams.getParam('Stdout_Log_Level')
        except KeyError:
            pass
        
        try:
            errLogLevel = progParams.getParam('Stderr_Log_Level')
        except KeyError:
            pass
    
        try:
            alertEmail = progParams.getParam('Alert_Email')
        except KeyError:
            pass
    
        try:
            logFile = progParams.getParam('Log_File')
        except KeyError:
            pass
    
        try:
            passId = progParams.getParam('pass-id')
            # If the pass id does not start with "PS", add it to the beginning of the string.
            if not passId.startswith('PS'):
                passId = 'PS' + passId
        except KeyError:
            pass

        try:
            processingChain = progParams.getParam("PROC_CHN")
        except KeyError:
            processingChain = "not defined"
            
        try:
            # The revision number is the first two digits of the job order's Version
            # element
            revisionNumber = int(progParams.getParam("Version")[0:2])
        except KeyError:
            pass
           
        outDir = progParams.getOutDir()
                
        if _fitsDataModelAvailable:
            try:
                jobOrderProcessorName = progParams.getParam('Processor_Name')
            except KeyError:
                jobOrderProcessorName = processorName
            
            if 'TRIGGER' in progParams.getInputStructNames():
                fits_data_model.TriggerFile.initialize(
                        outDir if outDir else "",
                        jobOrderProcessorName if jobOrderProcessorName else "",
                        progParams.getInputFiles('TRIGGER'))
            else:
                try:
                    jobOrderStartTime = fits_data_model.UTC(progParams.getParam('Start'))
                except KeyError:
                    jobOrderStartTime = fits_data_model.UTC()
    
                try:
                    jobOrderStopTime = fits_data_model.UTC(progParams.getParam('Stop'))
                except KeyError:
                    jobOrderStopTime = fits_data_model.UTC()
                    
                # Read input trigger trigger file, if present in job order
                fits_data_model.TriggerFile.initialize(
                        outDir if outDir else "",
                        jobOrderProcessorName if jobOrderProcessorName else "",
                        jobOrderStartTime,
                        jobOrderStopTime)
            
            processingNumber = fits_data_model.TriggerFile.getOutputProcessingNumber()

            if fits_data_model.TriggerFile.getVisitId().isValid():
                visitId = fits_data_model.TriggerFile.getVisitId().getFileNamePattern()

            if fits_data_model.TriggerFile.getPassId().isValid():
                passId = fits_data_model.TriggerFile.getPassId().getFileNamePattern()

            obsid = fits_data_model.TriggerFile.getObsid()
            
            # Read the file containing the pipeline version. This file exists 
            # only if the pipeline was installed using rpms. The file shall 
            # contain one line with the pipeline version.
            pipelineVersionFile = os.path.expandvars('${CHEOPS_SW}/conf/pipeline_version')
            try:
                with open(pipelineVersionFile, 'r') as file:
                    for line in file:
                        line = line.strip()
                        if len(line) > 0:    # Ignore empty lines
                            pipelineVersion = line
                            break
            except FileNotFoundError:
                pass
            
            fits_data_model.FitsDalHeader.setProgram(
                    processorName, 
                    processorVersion, 
                    svnRevision, 
                    processingChain, 
                    revisionNumber, 
                    processingNumber, 
                    pipelineVersion)
        
            if 'SOC_APP_LeapSeconds' in progParams.getInputStructNames():
                fits_data_model.LeapSeconds.setLeapSecondsFileNames(progParams.getInputFiles('SOC_APP_LeapSeconds'))

            if 'AUX_RES_ObtUtcCorrelation' in progParams.getInputStructNames():
                fits_data_model.OBTUTCCorrelation.setCorrelationFileNames(progParams.getInputFiles('AUX_RES_ObtUtcCorrelation'))

            if 'REF_APP_ObtReset' in progParams.getInputStructNames():
                fits_data_model.OBT.setResetCounterFileNames(progParams.getInputFiles('REF_APP_ObtReset'))
            
            if ('EXT_APP_DE1' in progParams.getInputStructNames() and 
                len(progParams.getInputFiles('EXT_APP_DE1')) > 0):
                fits_data_model.BarycentricOffset.setEphemerisFileName(progParams.getInputFiles('EXT_APP_DE1')[0])
            
            # Read PROP_FIRST_VISIT and PROP_LAST_VISIT from the MPS_PRE_Visits, if present
            if (obsid != 0 and 
                'MPS_PRE_Visits' in progParams.getInputStructNames() and 
                len(progParams.getInputFiles('MPS_PRE_Visits')) > 0):
                visitFound = False
                # Iterate over the files, starting with the newest file
                for fileName in reversed(progParams.getInputFiles('MPS_PRE_Visits')):
                    mpsPreVisits = fits_data_model.MpsPreVisits(fileName)
                    for row in mpsPreVisits:
                        # Break out of the loop once the visit has been found
                        if row.getCellObsid() == obsid:
                            propFirstVisit = row.getCellPropFirstVisit()
                            propLastVisit = row.getCellPropLastVisit()
                            targetName = row.getCellTargetName()
                            visitFound = True
                    del mpsPreVisits
                    if visitFound:
                        break
                        
    else:
        if _fitsDataModelAvailable:
            fits_data_model.FitsDalHeader.setProgram(processorName, processorVersion, svnRevision)
        
    log.config.configure(
            processorName, 
            processorVersion, 
            outLogLevel,
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
            targetName)
    
    if progParams and verboseLogging:
        progParams.logProgramParams(externalParams={"PRP_VST1": propFirstVisit,
                                                    "PRP_VSTN": propLastVisit})

    return progParams

## 
# @brief Performs exit routines before program termination.
# @author Anja Bekkelien UGE
# @ingroup ProgParam
#
# By default, the function creates an output trigger file containing values
# read from an input trigger file defined in the job order. If no input
# trigger file found in the job order, the output trigger file will contain
# no values.
# 
# @param createTriggerFile if True, an output trigger file is created, if
#                          False, no output trigger file is created. The
#                          default value is True.
#
def cheopsExit(createTriggerFile=True):
    
    if _fitsDataModelAvailable:
        
        if createTriggerFile:
            fits_data_model.TriggerFile.writeTriggerFile(
                    fits_data_model.TriggerFile.getOutputDirectory(),
                    fits_data_model.TriggerFile.getProcessorName() if fits_data_model.TriggerFile.getProcessorName() else processorName,
                    fits_data_model.TriggerFile.getPassId(), 
                    fits_data_model.TriggerFile.getVisitId(), 
                    fits_data_model.TriggerFile.getValidityStart(),
                    fits_data_model.TriggerFile.getValidityStop(),
                    fits_data_model.TriggerFile.getOutputProcessingNumber(),
                    fits_data_model.TriggerFile.getOutputProcessingNumber(),
                    fits_data_model.TriggerFile.getObsid(),
                    '',
                    fits_data_model.TriggerFile.getCreationDate(),
                    fits_data_model.TriggerFile.getPrevPassId())
        
