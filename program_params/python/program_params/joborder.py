##
# @file
# @brief Module for parsing job order files.
# @ingroup ProgParam
# @author: Anja Bekkelien UGE
# 
# @version 6.1 2016-07-26 ABE #10658 format sensing start/stop time to a valid 
#                                    UTC string
# @version 5.2 2016-05-03 ABE #10677 bugfix: input files were read twice when 
#                                    \<Time_Interval\> is present in \<Input\>
# @version 5.2 2016-05-03 ABE #10691 log program parameters
# @version 5.0 2016-01-20 ABE #9990  remove intermediate output dir
# @version 3.3 2015-05-27 ABE        parse \<Ipf_Conf\> element of job order 
#                                    file
# @version 1.0 2014-11-12 ABE        first released version
#

import logging
import os
import re
import xml.etree.ElementTree as ET


## The namespace of the xml elements.
NAMESPACE = ''
#NAMESPACE = '{http://www.deimos-space.com/csw4eo/csw}'


##
# @brief Class for parsing accessing data in a job order file.
# @ingroup ProgParam
#
class JobOrderParser:


    ##
    # @brief Initialize class attributes.
    #
    def __init__(self):
        ## The class' logger object
        self.logger = logging.getLogger()
    
    ##
    # @brief Parse the job order file given as method argument and return a 
    # JobOrder object
    # 
    # @param path the path to the job order file.
    # @param progName the name of the program as defined in the job order element
    # `<Task_Name>`. If no `<Task_Name>` element with this name is found, a warning 
    # is displayed and no data is read from the job order file.
    # @return JobOrder object containing the values read from the job order file.
    #
    def parseXml(self, path, progName):
       
        if not os.path.isfile(path):
            self.logger.warning('Job order file ' + path + ' not found.')
            return JobOrder()
            
        self.logger.info('Reading the job order file ' + path)
        jobOrder = JobOrder()
        tree = ET.parse(path)
        root = tree.getroot()
        
        # Parse <Ipf_Conf>
        ipfConf = root.find(NAMESPACE + 'Ipf_Conf')
        self.parseIpfConf(ipfConf, jobOrder)
        
        procCount = int(root.find(NAMESPACE + 'List_of_Ipf_Procs').attrib['count'])
        numProcesses = 0
        foundTask = False

        # Find this program's task and parse it
        for proc in root.iter(NAMESPACE + 'Ipf_Proc'):
            taskName = proc.find(NAMESPACE + 'Task_Name').text

            if taskName == progName:

                foundTask = True

                self.parseInputFiles(proc, jobOrder, progName, path)
                self.parseOutputFiles(proc, jobOrder, progName, path)
                
            numProcesses += 1
            
        # task not found
        if not foundTask:
            self.logger.warn('There is no entry for program ' + progName 
                             + ' in the job order file ' + path)
            
        if numProcesses != procCount:
            self.logger.warn('Fond data for ' + str(numProcesses) + ' processes in job-order file '
                             + path + ', but expected data for ' + str(procCount) + ' processes.')
        
        return jobOrder 

    
    ##
    # @brief Parses the Ipf_Conf element and returns its contents as a
    # mapping of parameter names to parameter values.
    # 
    # @param ipfConf the \<Ipf_Conf\> element of the document.
    # @param jobOrder the job order object to be filled
    #
    def parseIpfConf(self, ipfConf, jobOrder):
        
        self.addParam('Processor_Name', ipfConf.find(NAMESPACE + 'Processor_Name').text, jobOrder)
        self.addParam('Version', ipfConf.find(NAMESPACE + 'Version').text, jobOrder)
        self.addParam('Stdout_Log_Level', ipfConf.find(NAMESPACE + 'Stdout_Log_Level').text, jobOrder)
        self.addParam('Stderr_Log_Level', ipfConf.find(NAMESPACE + 'Stderr_Log_Level').text, jobOrder)
        self.addParam('Test', ipfConf.find(NAMESPACE + 'Test').text, jobOrder)
        self.addParam('Breakpoint_Enable', ipfConf.find(NAMESPACE + 'Breakpoint_Enable').text, jobOrder)
        self.addParam('Processing_Station', ipfConf.find(NAMESPACE + 'Processing_Station').text, jobOrder)
        
        sensingTime = ipfConf.find(NAMESPACE + 'Sensing_Time')
        if sensingTime:
            startTime = sensingTime.find(NAMESPACE + 'Start').text
            stopTime = sensingTime.find(NAMESPACE + 'Stop').text
            
            # Check that the format is yyyymmdd_hhmmssffffff
            if re.fullmatch(r'[0-9]{8}_[0-9]{12}', startTime) is None:
                self.logger.error('Invalid sensing start time: expected '
                                  'yyyymmdd_hhmmssffffff, found '
                                  + str(startTime))
            
            if re.fullmatch(r'[0-9]{8}_[0-9]{12}', stopTime) is None:
                self.logger.error('Invalid sensing stop time: expected '
                                  'yyyymmdd_hhmmssffffff, found '
                                  + str(stopTime))
            
            # Convert to UTC format yyyy-mm-ddThh:mm:ss.ffffff
            startTime = '{}-{}-{}T{}:{}:{}.{}'.format(startTime[0:4], 
                                                      startTime[4:6], 
                                                      startTime[6:8], 
                                                      startTime[9:11], 
                                                      startTime[11:13], 
                                                      startTime[13:15], 
                                                      startTime[15:])
            stopTime = '{}-{}-{}T{}:{}:{}.{}'.format(stopTime[0:4], 
                                                     stopTime[4:6], 
                                                     stopTime[6:8], 
                                                     stopTime[9:11], 
                                                     stopTime[11:13], 
                                                     stopTime[13:15], 
                                                     stopTime[15:])
            
            self.addParam('Start', startTime, jobOrder)
            self.addParam('Stop', stopTime, jobOrder)
        
        for param in ipfConf.iter(NAMESPACE + 'Processing_Parameter'):

            name = param.find(NAMESPACE + 'Name').text
            value = param.find(NAMESPACE + 'Value').text
            self.addParam(name, value, jobOrder)

    ##
    # @brief Adds a parameter to the job order.
    #
    # Ensures that all parameter values are strings.
    #
    # @param paramName the name of the parameter
    # @param paramValue the value of the parameter
    # @param jobOrder the job order object to add the parameter to
    #
    def addParam(self, paramName, paramValue, jobOrder):
        
        jobOrder.params[paramName] = paramValue if paramValue is not None else ''
        
    
    ##
    # @brief Parses an Input element and returns its contents as a mapping of file 
    # type to a list of input files of that type. Environment variables in the 
    # paths are expanded.
    # 
    # @param proc         the Proc element containing the Input element.
    # @param jobOrder     the job order object to be filled
    # @param progName     the name of the program, used for logging purposes
    # @param jobOrderPath the path to the job order file, used for logging 
    #                     purposes
    #
    def parseInputFiles(self, proc, jobOrder, progName, jobOrderPath):

        inputCount = int(proc.find(NAMESPACE + 'List_of_Inputs').attrib['count'])
        inputCounter = 0

        # For each struct type
        for input in proc.iter(NAMESPACE + 'Input'):
            
            fileNameType = input.find(NAMESPACE + 'File_Name_Type').text
            listOfFileNames = input.find(NAMESPACE + 'List_of_File_Names')
            fileCount = int(listOfFileNames.attrib['count'])
            inputCounter += 1

            if fileNameType != 'Physical':
                continue
            
            fileType = input.find(NAMESPACE + 'File_Type').text
            jobOrder.inputFiles[fileType] = []
            fileCounter = 0

            # For each file of this struct type
            for file in listOfFileNames.findall(NAMESPACE + 'File_Name'):
                fileName = os.path.expandvars(file.text)

                if fileName.endswith('.fits') or fileName.endswith('.fits.gz'):
                    fileName += '[' + fileType + ']'
                
                jobOrder.inputFiles[fileType].append(fileName)
                fileCounter += 1
                
            if fileCounter != fileCount:
                self.logger.warn('Fond ' + str(fileCounter) + ' file names of File_Type '
                                 + fileType + ' of process ' + progName + 
                                 ' in the  job-order file ' + jobOrderPath + ' but expected ' + str(fileCount))
        
        if inputCounter != inputCount:
            self.logger.warn('Found ' + str(inputCounter) + ' types of Inputs of process '
                     + progName + ' in the job-order file ' + jobOrderPath + ' but expected ' + str(inputCount)
                     + ' types of Inputs.')

    
    ##
    # @brief Parses an Output element and returns its contents as a mapping of file 
    # type to an output file of that type. Environment variables in the 
    # paths are expanded.
    # 
    # @param proc         the Proc element containing the Input element.
    # @param jobOrder     the job order object to be filled
    # @param progName     the name of the program, used for logging purposes
    # @param jobOrderPath the path to the job order file, used for logging 
    #                     purposes
    #
    def parseOutputFiles(self, proc, jobOrder, progName, jobOrderPath):
        
        foundOutDir = False
        foundTmpDir = False
        outputCount = int(proc.find(NAMESPACE + 'List_of_Outputs').attrib['count'])
        outputCounter = 0
        
        for output in proc.iter(NAMESPACE + 'Output'):

            fileType = output.find(NAMESPACE + 'File_Type').text
            
            if fileType == 'OUT':
                dir = os.path.expandvars(output.find(NAMESPACE + 'File_Name').text)
                jobOrder.outDir = dir
                foundOutDir = True
        
            if fileType == 'TMP':
                dir = os.path.expandvars(output.find(NAMESPACE + 'File_Name').text)
                jobOrder.tmpDir = dir
                foundTmpDir = True
            
            outputCounter += 1
        
        if outputCounter != outputCount:
            self.logger.warn('Found ' + str(outputCounter) + ' types of Outputs of process ' 
                             + progName + ' in job-order file ' + jobOrderPath 
                             + ' but expected ' + str(outputCount) + ' types of Outputs.')
        if not foundOutDir:
            raise RuntimeError('OUT directory for process ' + progName +
                               ' is not defined in job-order file.')
        if not foundTmpDir:
            raise RuntimeError('TMP directory for process ' + progName +
                               ' is not defined in job-order file.')

##
# @brief This class is a simple container for the data read from a job order file.
# @ingroup ProgParam
# @author Anja Bekkelien UGE
#
class JobOrder():
    
    ##
    # Initialize class attributes.
    #
    def __init__(self):

        ## 
        # A mapping of parameter names to parameter values. The parameters are
        # read from the \<Ipf_Conf\> element.
        self.params = {}

        ## 
        # A mapping of file types to lists of paths. Environment variables
        # in the paths are expanded.
        self.inputFiles = {}

        ## 
        # The output directory. Files stored here will be ingested into the 
        # archive 
        self.outDir = None

        ## 
        # Temporary directory. Files stored here will be deleted after program 
        # execution
        self.tmpDir = None

##
# @ingroup ProgParam
# @brief Parses a job order file and returns the data for the processor given as
#        argument
# 
# @param path the path to the job order file.
# @param progName the name of the processor.
# @return the job order data extracted for the processor
#
def getJobOrder(path, progName):
    parser = JobOrderParser()
    jobOrder = parser.parseXml(path, progName)
    return jobOrder

