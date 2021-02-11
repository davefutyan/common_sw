##
# @file
# @brief This Python module configures the standard python logging module 
#        according to the requirements of monitor4EO.
# @ingroup Logger
#
# @author Anja Bekkelien UGE
#
#
# @version 13.0   2020-09-17 ABE #21949 Show target name in alert emails.
# @version 12.1.2 2020-03-23 ABE #20653 take log level into account when
#                                       writing to file.
# @version 12.0.2 2020-02-24 ABE #20331 Use the mail server 
#                                       mailhost.astro.unige.ch for alerts
# @version 11.4.4 2019-09-13 ABE #19545 Corrected import of email package.
# @version 11.3   2019-04-09 ABE #18117 Improved contents of alert emails. 
# @version  9.0   2018-01-10 ABE #15085 Send alert emails using smtplib instead 
#                                       of subprocess, and define mandatory  
#                                       sender address.
# @version  9.0   2018-01-10 ABE #15286 Add OBSID to log file name.
# @version  7.3   2017-05-18 ABE #12941 Add pass id, visit id, processing chain.
#                                       And program version to log file name.
# @version  6.2   2016-09-12 ABE #11463 Use archive revision number and
#                                       processing number in log file name.
# @version  5.2   2016-05-09 ABE #10690 Add functionality for taking the full
#                                       log file path as input.
# @version  3.3   2015-05-22 ABE        Add alert and progress log levels.
# @version  1.0   2014-02-25 ABE        First version.
#

from email.parser import Parser
from jinja2 import Environment, FileSystemLoader
import logging
import logging.config
import os
import platform
import re
import smtplib
import sys
from time import gmtime, strftime


##
# @ingroup Logger 
# Maps the names of custom log levels to their log level numbers.
logLevels = {'DEBUG'   : logging.DEBUG,
             'INFO'    : logging.INFO,
             'ALERT'   : logging.INFO + 2, # New log level ALERT 
             'PROGRESS': logging.INFO + 4, # New log level PROGRESS
             'WARN'    : logging.WARN,
             'WARNING' : logging.WARN,
             'ERROR'   : logging.ERROR}

##
# @ingroup Logger 
# Defines how each log level is displayed.
logLevelNames = {logLevels['DEBUG']    : 'D',
                 logLevels['INFO']     : 'I',
                 logLevels['ALERT']    : 'A', 
                 logLevels['PROGRESS'] : 'P',
                 logLevels['WARNING']  : 'W',
                 logLevels['ERROR']    : 'E'}

##
# @ingroup Logger 
# The name of the node on which the program is running
NODE_NAME = platform.node()

##
# @ingroup Logger 
## The program name to be displayed in the log messages
PROGRAM_NAME = None

##
# @ingroup Logger 
## The program version to be displayed in the log messages
PROGRAM_VERSION = None

##
# @ingroup Logger
# The name of the log file.
LOG_FILE = None

##
# @brief Filters log levels for stdout.
# @ingroup Logger
#
class StdoutFilter(logging.Filter):
    
    ##
    # @brief Return true if the level of the log record is DEBUG, INFO or ALERT.
    # @param rec the log record
    # @return true if the level of the log record is DEBUG, INFO or ALERT
    #
    def filter(self, rec):
        return (rec.levelno == logLevels['DEBUG'] or
                rec.levelno == logLevels['INFO'] or
                rec.levelno == logLevels['ALERT'])

##
# @brief Filters log levels for stderr.
# @ingroup Logger
#
class StderrFilter(logging.Filter):
    
    ##
    # @brief Return true if the level of the log record is PROGRESS, WARN or 
    #        ERROR.
    # @param rec the log record
    # @return true if the level of the log record is PROGRESS, WARN or ERROR
    #
    def filter(self, rec):
        return (rec.levelno == logLevels['PROGRESS'] or
                rec.levelno == logLevels['WARNING'] or
                rec.levelno == logLevels['ERROR'])

##
# @brief Filters out all log levels except ALERT.
# @ingroup Logger
#
class AlertEmailFilter(logging.Filter):
    
    ##
    # @brief Initialize the object.
    #
    # @param emailEnabled if set to false, this argument disables all emails. 
    #                     Set to True by default
    #
    def __init__(self, emailEnabled=True):
        
        ## If false then email alerts are disabled
        self.emailEnabled = emailEnabled
    
    ##
    # @brief Return true if the level of the log record is ALERT.
    # @param rec the log record
    # @return true if the level of the log record is ALERT
    #
    def filter(self, rec):
        return self.emailEnabled and rec.levelno == logLevels['ALERT']
    
##
# @brief Class that stores the values that are included in alert emails.
# @ingroup Logger
#
# Instances of this class are passed to the alert email template as its "context".
# For this reason, the names of the class' member attributes must be the same as
# the placeholder names that are used in the template.
#
class AlertEmailContext:
    
    ##
    # @brief Initialize the member variables.
    #
    # @param toEmail          the "to" email address
    # @param fromEmail        the "from" email address (optional)
    # @param programName      a program name (optional)
    # @param programVersion   a program version (optional)
    # @param hostName         a host name (optional)
    # @param logFile          the path to a log file (optional)
    # @param visitId          a visit id (optional)
    # @param obsid            an obsid (optional)
    # @param passId           a pass id (optional)
    # @param processingChain  the name of a processing chain (optional)
    # @param targetName       (optional) the name of the target.
    # @param revisionNumber   an archive revision number (optional)
    # @param processingNumber a processing number (optional)
    # 
    def __init__(self,
                 toEmail, 
                 fromEmail='noreply@unige.ch', 
                 programName='N/A', 
                 programVersion='N/A',
                 hostName='N/A',
                 logFile='N/A',
                 visitId='N/A',
                 obsid=0,
                 passId='N/A',
                 processingChain='N/A',
                 targetName='N/A',
                 revisionNumber=0,
                 processingNumber=0):
        
        ## the "to" email address
        self.toEmail = toEmail
        ## the "from" email address
        self.fromEmail = fromEmail
        ## a program name 
        self.programName = programName
        ## a program version
        self.programVersion = programVersion
        ## a host name
        self.hostName = hostName
        ## the path to a log file 
        self.logFile = logFile
        ## a visit id
        self.visitId = visitId
        ## an obsid
        self.obsid = obsid
        ## a pass id
        self.passId = passId
        ## the name of a processing chain
        self.processingChain = processingChain
        ## the name of the target
        self.targetName = targetName
        ## an archive revision number
        self.revisionNumber = revisionNumber
        ## a processing number 
        self.processingNumber = processingNumber
        ## an alert id
        self.alertId = None
        ## an alert message
        self.alertMessage = None
        ## the time of the alert
        self.alertTime = None

##
# @brief A log handler that sends a log record to an email address.
# @ingroup Logger
#
class AlertEmailHandler(logging.Handler):
    
    ##
    # @brief Initializes the handler.
    #
    # @param templateFile The path to the template used to generate alert emails.
    #                     The path may contain environment variables.
    # @param context      An AlertEmailContext object containing the values
    #                     to be inserted into the email template.
    # 
    def __init__(self, templateFile, context):

        logging.Handler.__init__(self)
        
        ## the path to the template used to generate alert emails
        self.templateFile = os.path.expandvars(templateFile)
        if not os.path.isfile(self.templateFile):
            raise RuntimeError('Template for alert emails \'' + self.templateFile + '\' not found')
        
        ## 
        # Object that stores the values that are included in alert emails.
        self.context = context
    
    ##
    # @brief Fills the email template with values and returns the result as a string.
    #
    # @param alertId      The alert id.
    # @param alertMessage The alert message.
    # @param alertTime    The time of the alert.
    # @return The rendered email template as a string.
    # 
    def render(self, alertId, alertMessage, alertTime):
        
        self.context.alertId = alertId
        self.context.alertMessage = alertMessage
        self.context.alertTime = alertTime

        # Split the directory and the file name from the full path
        directory, file = os.path.split(self.templateFile)
    
        # Fill the template 
        env = Environment(loader=FileSystemLoader(directory),
                          lstrip_blocks=True,
                          trim_blocks=True,
                          extensions=['jinja2.ext.loopcontrols'])
        env.globals['os'] = os
    
        try:
            template = env.get_template(file)
            result = template.render(vars(self.context))
        except Exception as e:
            raise e
        
        self.context.alertId = None
        self.context.alertMessage = None
        self.context.alertTime = None
        
        return result          
   
    ##
    # @brief Sends an email.
    #
    # @param emailStr The email to be sent, including all necessary email headers.
    #  
    def send(self, emailStr):

        msg = Parser().parsestr(emailStr)
        s = smtplib.SMTP(host='mailhost.astro.unige.ch', port=25)
        s.send_message(msg)
        s.quit() 
        
    ##
    # @brief Sends an email containing the log record using sendmail.
    #
    # @param record the log record to be sent by email
    #
    def emit(self, record):
        
        # Only send emails if a "to" email address is defined
        if self.context.toEmail: 
            formattedRecord = self.format(record)
    
            # Extract the alert id and alert message from the log entry
            res = re.search(r'(?P<alertId>[0-9]+)\s+(?P<alertMessage>.*)', record.getMessage())
            if res:
                alertId = res.group('alertId')
                alertMessage = res.group('alertMessage')
            else:
                alertId = 'N/A'
                alertMessage = record.getMessage()
            
            # Format the alert time to have a 6 digit second fraction, same as
            # is used in the log files.
            alertTime = '{}.{:03d}000'.format(record.asctime, int(record.msecs))
            
            # Fill the email template with values
            emailStr = self.render(alertId, alertMessage, alertTime)
    
            self.send(emailStr)
    
##
# @ingroup Logger
# @brief Configures python's standard logger according to the requirements of
#        monitor4EO
#
# This method will:
# - format the fields of the log entries:
# \<time\> \<nodeName\> \<programName\> \<programVersion\> [\<PID\>]: [\<logLevel\>] \<logMessage\>
# - add two additional log levels that do not exist in the standard logger: alert and progress
# - remove the critical log level
# - direct logs to stdout, stderr and to file
#
# The path to the log file can be provided in two ways. 1) By defining a
# directory. A new log file is created every time the program is run, with a
# unique file name containing the current time in UTC. 2) By defining the
# full path to a log file. If this file already exists, logs are appended to
# it.
#
# @param programName    the name of the program
# @param programVersion the program's version
# @param stdoutLevel    the log level for stdout. Must be "DEBUG",
#                       "INFO", "PROGRESS", "WARNING" or "ERROR". Set to "INFO"
#                       by default.
# @param stderrLevel    the log level for stderr. Must be "DEBUG", "INFO", 
#                       "PROGRESS", "WARNING" or "ERROR". Set to "PROGRESS" by
#                       default.
# @param alertEmail     the email to which alerts are sent, empty string by
#                       default
# @param outDir         (optional) Note: if parameter @b logFile is provided, 
#                       @b outDir is ignored. This is the directory in which the
#                       log file is stored. If the directory does not exist, it 
#                       is created. .
# @param logFile        (optional) the path the log file. Any non-existent 
#                       directory in the path are created. If the file already 
#                       exist, logs are appended to it. If this parameter is
#                       provided, parameter @b outDir is ignored
# @param revisionNumber (optional) the archive revision number.
# @param processingNumber (optional) the processing number.
# @param visitId        (optional) the visit id as a string formatted
#                       as PRppnnnn-TGoooonn. If this argument is
#                       provided, the visit id is included in the
#                       file name.
# @param passId         (optional) the pass id  as a string formatted
#                       as PSyymmddhh. If this argument is provided,
#                       the pass id is included in the file name.
# @param obsid          (optional) the obsid. If this argument is provided, the
#                       obsid is included in the file name.
# @param processingChain (optional) the processing chain. If provided,
#                       it is included in the log file name.
# @param targetName     (optional) the name of the target.
#   
def configure(programName, 
              programVersion,
              stdoutLevel='INFO', 
              stderrLevel='PROGRESS',
              alertEmail=None, 
              outDir=None,
              logFile=None,
              revisionNumber=0,
              processingNumber=0,
              visitId=None,
              passId=None,
              obsid=0,
              processingChain=None,
              targetName=''): 

    global PROGRAM_NAME
    global PROGRAM_VERSION
    global LOG_FILE
    PROGRAM_NAME = programName
    PROGRAM_VERSION = programVersion
    
    customizeLogRecords(PROGRAM_NAME, PROGRAM_VERSION)
    customizeLogLevels()
    
    stdoutLevelCode = logLevels[stdoutLevel]
    stderrLevelCode = logLevels[stderrLevel]
   
    alertEmailContext = AlertEmailContext(
        alertEmail,
        programName=PROGRAM_NAME,
        programVersion=PROGRAM_VERSION,
        hostName=NODE_NAME,
        logFile=LOG_FILE,
        visitId=visitId,
        obsid=obsid,
        passId=passId,
        processingChain=processingChain,
        targetName=targetName,
        revisionNumber=revisionNumber,
        processingNumber=processingNumber)

    config = {
        'version': 1,
        'disable_existing_loggers': False,
        'filters': {
            'alertEmailFilter': {
                '()': AlertEmailFilter,
                'emailEnabled' : alertEmail != None
            },
            'stdoutFilter': {
                '()': StdoutFilter,
            },
            'stderrFilter': {
                '()': StderrFilter,
            },
        },
        'formatters': {
            'standard': {
                'format': '%(asctime)s.%(msecs)03d000 %(node)s %(programName)s %(programVersion)s [%(process)010d]: [%(levelname)s] %(message)s',
                'datefmt': '%Y-%m-%dT%H:%M:%S',
            },
        },
        'handlers': {
            'stdout': {
                'class': 'logging.StreamHandler',
                'level': stdoutLevelCode,
                'formatter': 'standard',
                'stream' : 'ext://sys.stdout',
                'filters' : ['stdoutFilter'],
            },
            'stderr': {
                'class' : 'logging.StreamHandler',
                'level' : stderrLevelCode,
                'formatter' : 'standard',
                'stream' : 'ext://sys.stderr',
                'filters' : ['stderrFilter'],
            },
            'alertEmail' : {
                'class' : 'log.config.AlertEmailHandler',
                'formatter' : 'standard',
                'templateFile' : '${CHEOPS_SW}/resources/logger_alert_email_template',
                'context' : alertEmailContext,
                'filters' : ['alertEmailFilter']
            },
        },
        'root': {
            'level': 'DEBUG',
            'handlers': ['stdout', 'stderr', 'alertEmail'],
        },
    }

    if logFile is not None:
        LOG_FILE = logFile
        
    elif outDir is not None:
        # If the directory is an empty string, set it to the current directory to avoid
        # accidentally setting it to the root directory when appending a file name
        logFileDir = outDir
        if logFileDir == '':
            logFileDir = '.'
        
        LOG_FILE = logFileDir + '/CH_'

        if visitId:
            LOG_FILE += visitId + '_'
            
        if passId:
            LOG_FILE += passId + '_'
        
        LOG_FILE += 'TU' + strftime('%Y-%m-%dT%H-%M-%S', gmtime()) + '_'

        # Program name without extension
        LOG_FILE += os.path.splitext(programName)[0]

#         LOG_FILE += '_' + programVersion
#         if processingChain and processingChain != 'not defined':
#             LOG_FILE += '_' + processingChain
        
        if obsid != 0:
            LOG_FILE += '_OBSID' + str(obsid)

        LOG_FILE += '_V{:0>2}{:0>2}.log'.format(revisionNumber, processingNumber)
               
    if LOG_FILE:
        
        # Create the log directory if it doesn't exist
        if not os.path.isdir(os.path.dirname(LOG_FILE)):
            os.makedirs(os.path.dirname(LOG_FILE), exist_ok=True)
            
        config['handlers']['file'] = {'class' : 'logging.FileHandler',
                                      'level' : stdoutLevelCode,
                                      'formatter' : 'standard',
                                      'filename' : LOG_FILE,
                                      'encoding' : 'utf8'}
        config['root']['handlers'].append('file')
    
    logging.config.dictConfig(config)
    
    return LOG_FILE

     
##
# @brief Customizes the log levels
# @ingroup Logger
#
# This method
# - Changes the way log level names are displayed to "D", "I", "P", "W" and "E"
# - Adds the 'alert' log level.
# - Adds the 'progress' log level
# - Removes the 'critical' log level
#
def customizeLogLevels():
    
    # Rename/add log levels according to monitor4EO
    for level, name in logLevelNames.items():
        logging.addLevelName(level, name)
    
    # Add a method for the new 'alert' level, which lets us write 
    # logger.alert(...)
    def alert(self, message, *args, **kws):
        if self.isEnabledFor(logLevels ['ALERT']):
            self._log(logLevels ['ALERT'], message, args, **kws)
    logging.Logger.alert = alert
   
    # Add a method for the new 'progress' level, which lets us write 
    # logger.progress(...)
    def progress(self, message, *args, **kws):
        if self.isEnabledFor(logLevels ['PROGRESS']):
            self._log(logLevels ['PROGRESS'], message, args, **kws)
    logging.Logger.progress = progress
    
    # Remove the 'critical' log method
    if hasattr(logging.Logger, 'critical'): 
        delattr(logging.Logger, 'critical')


##
# @ingroup Logger
# @brief Add custom fields to the log: nodeName, programName and programVersion.
#
# @param programName the name of the processor, which will be included in the log
# @param programVersion the processor's version, which will be included in the log
#
def customizeLogRecords(programName, programVersion):
    
    oldFactory = logging.getLogRecordFactory()
    def recordFactory(name, level, fn, lno, msg, args, exc_info, func=None, sinfo=None, **kwargs):
        record = oldFactory(name, level, fn, lno, msg, args, exc_info, func=None, sinfo=None)
        record.node = NODE_NAME
        record.programName = programName
        record.programVersion = programVersion
        return record
    
    logging.setLogRecordFactory(recordFactory)
 
