import unittest
import math
import os
import re
import sys
import logging

import log.config

# This variable shall be set to a valid email recipient address to perform test 
# specification CHEOPS-SOC-TC-WP05-FUNC-007 Alert API.
emailAddress = None

programName = 'TestPythonLogger'
programVersion = "0.1"
logDir = 'results'
revisionNumber = 1
processingNumber = 2
visitId = 'PR990001_TG000101'
passId = 'PS18010112'
obsid = 3
processingChain = 'test'
targetName = 'test target'

logger = logging.getLogger()

if not os.path.isdir(logDir):
    os.makedirs(logDir, exist_ok=True)
    

class TestLogInit(unittest.TestCase):
    
    def setUp(self):
        self.logFile = None
    
    def tearDown(self):
        if self.logFile and os.path.isfile(self.logFile):
            os.remove(self.logFile)
        
    def testLogConfiguration_debug(self):
        self.logFile = log.config.configure(programName, programVersion, outDir=logDir, stdoutLevel="DEBUG", stderrLevel="DEBUG", revisionNumber=revisionNumber, processingNumber=processingNumber, visitId=visitId, passId=passId)
        self.assertTrue(os.path.isfile(self.logFile))
        
    def testLogConfiguration_info(self):
        self.logFile = log.config.configure(programName, programVersion, outDir=logDir, stdoutLevel="INFO", stderrLevel="INFO", revisionNumber=revisionNumber, processingNumber=processingNumber, visitId=visitId, passId=passId, processingChain=processingChain)
        self.assertTrue(os.path.isfile(self.logFile))

    def testLogConfiguration_alert(self):
        self.logFile = log.config.configure(programName, programVersion, outDir=logDir, stdoutLevel="ALERT", stderrLevel="ALERT", revisionNumber=revisionNumber, processingNumber=processingNumber, visitId=visitId, passId=passId, processingChain=processingChain)
        self.assertTrue(os.path.isfile(self.logFile))
        
    def testLogConfiguration_progress(self):
        self.logFile = log.config.configure(programName, programVersion, outDir=logDir, stdoutLevel="PROGRESS", stderrLevel="PROGRESS", revisionNumber=revisionNumber, processingNumber=processingNumber, visitId=visitId, passId=passId, processingChain=processingChain)
        self.assertTrue(os.path.isfile(self.logFile))

    def testLogConfiguration_warn(self):
        self.logFile = log.config.configure(programName, programVersion, outDir=logDir, stdoutLevel="WARN", stderrLevel="WARN", revisionNumber=revisionNumber, processingNumber=processingNumber, visitId=visitId, passId=passId, processingChain=processingChain)
        self.assertTrue(os.path.isfile(self.logFile))

    def testLogConfiguration_error(self):
        self.logFile = log.config.configure(programName, programVersion, outDir=logDir, stdoutLevel="ERROR", stderrLevel="ERROR", revisionNumber=revisionNumber, processingNumber=processingNumber, visitId=visitId, passId=passId, processingChain=processingChain)
        self.assertTrue(os.path.isfile(self.logFile))

    def testLogConfiguration_invalid_empty(self):
        with self.assertRaises(Exception):
            self.logFile = log.config.configure(programName, programVersion, outDir=logDir, stdoutLevel="", stderrLevel="", revisionNumber=revisionNumber, processingNumber=processingNumber, visitId=visitId, passId=passId, processingChain=processingChain)
            self.assertTrue(os.path.isfile(self.logFile))


class TestLog(unittest.TestCase):
    
    def setUp(self):
        
        self.logFile = log.config.configure(programName, 
                                            programVersion, 
                                            outDir=logDir, 
                                            stdoutLevel="DEBUG", 
                                            stderrLevel="PROGRESS", 
                                            revisionNumber=revisionNumber, 
                                            processingNumber=processingNumber, 
                                            visitId=visitId, 
                                            passId=passId,
                                            obsid=obsid,
                                            processingChain=processingChain,
                                            targetName=targetName)
 
    def tearDown(self):
        if self.logFile and os.path.isfile(self.logFile):
            os.remove(self.logFile)

    def testLogFile(self):
        
        self.assertTrue(os.path.isfile(self.logFile))
        print(self.logFile)
        result = re.search(r'CH(_PR(?P<PROGTYPE>[0-9]{2})(?P<PROG_ID>[0-9]{4})_TG(?P<REQ_ID>[0-9]{4})(?P<VISITCTR>[0-9]{2}))?(_PS(?P<PASS_ID>[0-9]{8}))?_TU(?P<V_STRT_U>[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}-[0-9]{2}-[0-9]{2})_(?P<program>.+?(?=_OBSID|_V))(?:_OBSID(?P<OBSID>[0-9].*?))?_V(?P<ARCH_REV>[0-9]{2})(?P<PROC_NUM>[0-9]{2})',
                           self.logFile)
        self.assertTrue(result is not None)
        self.assertEqual(result.groupdict()['program'], programName)
        self.assertEqual(int(result.groupdict()['OBSID']), obsid)
        
    def testLogConfiguration(self):
        
        self.assertTrue(hasattr(logging.Logger, "alert"))
        self.assertTrue(hasattr(logging.Logger, "progress"))
        self.assertFalse(hasattr(logging.Logger, "critical"))
        
        # Check that the level names have been changed to 'D', 'I', 'W' etc.
        for level, levelName in log.config.logLevelNames.items():
            self.assertEqual(levelName, logging.getLevelName(level))

    def testLogRecords(self):

        logMsg = "first message"
        loggerName = "root"
        
        output = ["D:" + loggerName + ":" + logMsg,
                  "I:" + loggerName + ":" + logMsg,
                  "A:" + loggerName + ":" + logMsg,
                  "P:" + loggerName + ":" + logMsg,
                  "W:" + loggerName + ":" + logMsg,
                  "E:" + loggerName + ":" + logMsg]
        
        with self.assertLogs(level="DEBUG") as cm:
            logging.getLogger().debug(logMsg)
            logging.getLogger().info(logMsg)
            logging.getLogger().alert(logMsg)
            logging.getLogger().progress(logMsg)
            logging.getLogger().warn(logMsg)
            logging.getLogger().error(logMsg)
        
        for record in cm.records:
            # https://docs.python.org/3/library/logging.html#logging.LogRecord
            self.assertTrue(hasattr(record, 'node'))
            self.assertTrue(hasattr(record, 'programName'))
            self.assertTrue(hasattr(record, 'programVersion'))
            
            self.assertEqual(record.programName, programName)
            self.assertEqual(record.programVersion, programVersion)
        
        # Test that the output contains the same log levels that were called
        # inside the context manager
        self.assertEqual(cm.output, output)

    def testAlertEmailHandler(self):
        emailTemplateFile = "../../resources/logger_alert_email_template"
        toEmail = "toEmail"
        fromEmail = "fromEmail"
        hostName = "testHost"
        logFile = "CH_test.log"
        alertId = "11"
        alertMessage = "This is a test alert"
        alertTime = "2019-04-10T11:46:11.782000"
        
        context = log.config.AlertEmailContext(
                toEmail,
                fromEmail=fromEmail,
                programName=programName,
                programVersion=programVersion,
                hostName=hostName,
                logFile=logFile,
                visitId=visitId,
                obsid=obsid,
                passId=passId,
                processingChain=processingChain,
                revisionNumber=revisionNumber,
                processingNumber=processingNumber,
                targetName=targetName)
        
        # Check the values of the context used to fill the email template
        self.assertEqual(context.alertId, None)
        self.assertEqual(context.alertMessage, None)
        self.assertEqual(context.alertTime, None)
        self.assertEqual(context.toEmail, toEmail)
        self.assertEqual(context.fromEmail, fromEmail)
        self.assertEqual(context.programName, programName)
        self.assertEqual(context.programVersion, programVersion)
        self.assertEqual(context.hostName, hostName)
        self.assertEqual(context.logFile, logFile)
        self.assertEqual(context.visitId, visitId)
        self.assertEqual(context.obsid, obsid)
        self.assertEqual(context.passId, passId)
        self.assertEqual(context.processingChain, processingChain)
        self.assertEqual(context.targetName, targetName)
        self.assertEqual(context.revisionNumber, revisionNumber)
        self.assertEqual(context.processingNumber, processingNumber)
        
        alertHandler = log.config.AlertEmailHandler(
            emailTemplateFile,
            context)
        
        email = alertHandler.render(alertId, alertMessage, alertTime)
        self.assertTrue(len(email) > 0)

    # This test will attempt to send an email ONLY if the variable emailAddress
    # has been changed to something other than None
    def testSendAlertEmail(self):
        
        self.logFile = log.config.configure("TestPythonEmailAlert", 
                                            programVersion, 
                                            outDir=logDir, 
                                            stdoutLevel="DEBUG", 
                                            stderrLevel="PROGRESS",
                                            alertEmail=emailAddress,
                                            revisionNumber=revisionNumber, 
                                            processingNumber=processingNumber, 
                                            visitId=visitId, 
                                            passId=passId,
                                            obsid=obsid,
                                            processingChain=processingChain,
                                            targetName=targetName)
        logger.alert("1000 This is a test alert message.")
        

if __name__ == "__main__":
    unittest.main()
