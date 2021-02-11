import os
import sys
import unittest

from program_params import ProgramParams
from fits_data_model import TriggerFile
from fits_data_model import LeapSeconds
from fits_data_model import FitsDalHeader
from fits_data_model import PassId
from fits_data_model import VisitId
from fits_data_model import UTC


class TestTriggerFile(unittest.TestCase):
 
    def setUp(self):

        self.programName = "TestTriggerFile.py"
        self.programVersion = "0.1"
        self.svnRevision = "1"
        self.processingChain = "not defined"
        self.revisionNumber = 1
        
        self.inputPath = "resources"
        self.outputPath = "result"
        self.jobOrderPath = self.inputPath + "/test_job_order_py.xml"
        
        self.processorName = "example_processor"
        self.passId = PassId("18010112");
        self.programmeType = 99
        self.programmeId = 1
        self.requestId = 1
        self.visitCounter = 1
        self.visitId= VisitId(self.programmeType, self.programmeId, self.requestId, self.visitCounter)
        self.creationDate = UTC("2018-01-01T00:00:00")
        self.validityStart = UTC("2020-01-01T01:01:01")
        self.validityStop = UTC("2022-01-01T01:01:01")
        self.inputProcessingNumber = 0
        self.outputProcessingNumber = 1
        self.obsid = 1
        self.dataName = "test"

        self.jobOrderStartTime = "2014-08-01T00:00:00"
        self.jobOrderStopTime = "2014-08-02T00:00:00"
        
     
        self.outputTriggerFile = self.outputPath + "/CH_PR990001_TG000101_PS18010112_TU2020-01-01T01-01-01_TRIGGER-" + self.processorName + "-" + self.dataName + "_V0101.xml"
        
        self.inputFile = TriggerFile.writeTriggerFile(self.inputPath, 
                                     self.processorName,
                                     self.passId,
                                     self.visitId,
                                     self.validityStart,
                                     self.validityStop,
                                     self.inputProcessingNumber,
                                     self.outputProcessingNumber,
                                     self.obsid,
                                     '',
                                     self.creationDate)

        print('ctor: ' + str(self.inputFile))
        
    def tearDown(self):
        
        print(self.inputFile)
        self.removeFile(self.inputFile)
        self.removeFile(self.outputTriggerFile)
    
    def removeFile(self, path):

        if os.path.isfile(path):
            os.remove(path)
              
    def validateTriggerFile(self,
                            outputPath=None,
                            processorName=None,
                            passId=None,
                            visitId=None,
                            creationDate=None,
                            validityStart=None,
                            validityStop=None,
                            inputProcessingNumber=None,
                            outputProcessingNumber=None,
                            obsid=None,
                            ignoreCreationDate=False):
        
        self.assertEqual(TriggerFile.getOutputDirectory(), outputPath if outputPath is not None else self.outputPath)
            
        self.assertEqual(TriggerFile.getOutputDirectory(), 
                         outputPath if outputPath is not None else self.outputPath)
        self.assertEqual(TriggerFile.getProcessorName(), 
                         processorName if processorName is not None else self.processorName)
        self.assertEqual(str(TriggerFile.getPassId()), 
                         str(passId) if passId is not None else str(self.passId))
        self.assertEqual(TriggerFile.getVisitId(), 
                         visitId if visitId is not None else self.visitId)
        if not ignoreCreationDate:
            self.assertEqual(TriggerFile.getCreationDate().getUtc(), 
                             creationDate.getUtc() if creationDate is not None else self.creationDate.getUtc())
        self.assertEqual(TriggerFile.getValidityStart().getUtc(), 
                         validityStart.getUtc() if validityStart is not None else self.validityStart.getUtc())
        self.assertEqual(TriggerFile.getValidityStop().getUtc(), 
                         validityStop.getUtc() if validityStop is not None else self.validityStop.getUtc())
        self.assertEqual(TriggerFile.getInputProcessingNumber(), 
                         inputProcessingNumber if inputProcessingNumber is not None else self.inputProcessingNumber)
        self.assertEqual(TriggerFile.getOutputProcessingNumber(), 
                         outputProcessingNumber if outputProcessingNumber is not None else self.outputProcessingNumber)   
        self.assertEqual(TriggerFile.getObsid(), 
                         obsid if obsid is not None else self.obsid)
        
    def testInitialize(self):
        
        # One input TRIGGER file
        sys.argv = [self.programName, self.jobOrderPath]
        progParams = ProgramParams(self.programName, self.programVersion)

        self.assertEqual(TriggerFile.empty(), True)
        self.assertEqual(TriggerFile.getVisitId().isValid(), False)
        
        TriggerFile.initialize(
            progParams.getOutDir(),
            progParams.getParam('Processor_Name'),
            progParams.getInputFiles('TRIGGER'),
            )

        self.assertEqual(TriggerFile.empty(), False)
        self.validateTriggerFile()

        # No input TRIGGER file
        programName = "TestTriggerFileNoInputFile.py"
        sys.argv = [programName, self.jobOrderPath]
        progParams = ProgramParams(programName, self.programVersion)
        
        self.assertTrue('TRIGGER' not in progParams.getInputStructNames())

        TriggerFile.initialize(
            progParams.getOutDir(),
            progParams.getParam('Processor_Name'),
            UTC(progParams.getParam('Start')),
            UTC(progParams.getParam('Stop'))
            )

        self.assertEqual(TriggerFile.empty(), True)
        self.validateTriggerFile(
            outputPath=progParams.getOutDir(),
            processorName=progParams.getParam('Processor_Name'),
            passId=PassId(),
            visitId=VisitId(),
            validityStart=UTC(self.jobOrderStartTime),
            validityStop=UTC(self.jobOrderStopTime),
            inputProcessingNumber=0,
            outputProcessingNumber=0,
            obsid=0,
            ignoreCreationDate=True)

    def testWriteTriggerFile(self):
         
        FitsDalHeader.setProgram(self.programName, 
                                 self.programVersion,
                                 self.svnRevision, 
                                 self.processingChain,
                                 self.revisionNumber, 
                                 self.outputProcessingNumber)
                                  
#         sys.argv = [self.programName, self.jobOrderPath]
#         progParams = ProgramParams(self.programName, self.programVersion)
         
        print(self.outputTriggerFile)
        self.removeFile(self.outputTriggerFile)
         
        TriggerFile.writeTriggerFile(self.outputPath, 
                                     self.processorName, 
                                     self.passId, 
                                     self.visitId, 
                                     self.validityStart,
                                     self.validityStop,
                                     self.inputProcessingNumber,
                                     self.outputProcessingNumber,
                                     self.obsid,
                                     self.dataName,
                                     self.creationDate)
         
        self.assertTrue(os.path.isfile(self.outputTriggerFile))
        
        TriggerFile.setOutputDirectory(self.outputPath)
        TriggerFile.setProcessorName(self.processorName)
        TriggerFile.readTriggerFile(self.outputTriggerFile)
         
        self.validateTriggerFile()
         
        # Test WriteTriggerFile using utility classes as method arguments
        self.removeFile(self.outputTriggerFile)

        TriggerFile.writeTriggerFile(self.outputPath, 
                                     self.processorName, 
                                     self.passId, 
                                     self.visitId, 
                                     self.validityStart,
                                     self.validityStop,
                                     self.inputProcessingNumber,
                                     self.outputProcessingNumber,
                                     self.obsid,
                                     self.dataName,
                                     self.creationDate)
         
        self.assertTrue(os.path.isfile(self.outputTriggerFile))
        
        TriggerFile.setOutputDirectory(self.outputPath)
        TriggerFile.setProcessorName(self.processorName)
        TriggerFile.readTriggerFile(self.outputTriggerFile)
        
        self.validateTriggerFile()

    # Test writeTriggerFile with various combinations of optional arguments
    def testWriteTriggerFileOptionalArguments(self):
         
        FitsDalHeader.setProgram(self.programName, self.programVersion,
                                 self.svnRevision, self.processingChain,
                                 self.revisionNumber, 
                                 self.outputProcessingNumber)
        
        # With obsid, dataName and creationDate
        fileName = TriggerFile.writeTriggerFile(self.outputPath, 
                                                self.processorName, 
                                                self.passId, 
                                                self.visitId, 
                                                self.validityStart,
                                                self.validityStop,
                                                self.inputProcessingNumber,
                                                self.outputProcessingNumber,
                                                self.obsid,
                                                self.dataName,
                                                self.creationDate)
         
        self.assertTrue(os.path.isfile(fileName))
        TriggerFile.readTriggerFile(fileName)
        self.validateTriggerFile()
        self.removeFile(fileName)
        
        # With obsid and dataName 
        fileName = TriggerFile.writeTriggerFile(self.outputPath, 
                                                self.processorName, 
                                                self.passId, 
                                                self.visitId, 
                                                self.validityStart,
                                                self.validityStop,
                                                self.inputProcessingNumber,
                                                self.outputProcessingNumber,
                                                self.obsid,
                                                self.dataName)
         
        self.assertTrue(os.path.isfile(fileName))
        TriggerFile.readTriggerFile(fileName)
        self.validateTriggerFile(ignoreCreationDate=True)
        self.removeFile(fileName)
        
        # With obsid
        fileName = TriggerFile.writeTriggerFile(self.outputPath, 
                                                self.processorName, 
                                                self.passId, 
                                                self.visitId, 
                                                self.validityStart,
                                                self.validityStop,
                                                self.inputProcessingNumber,
                                                self.outputProcessingNumber,
                                                self.obsid)
         
        self.assertTrue(os.path.isfile(fileName))
        TriggerFile.readTriggerFile(fileName)
        self.validateTriggerFile(ignoreCreationDate=True)
        self.removeFile(fileName)
        
        
if __name__ == "__main__":
    unittest.main()
