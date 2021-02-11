##
# @file
# @brief This module contains classes and methods for processing and accessing 
#        input to the program.
# @ingroup ProgParam
# @author: Anja Bekkelien UGE
#
# @version 10.0 2018-07-30 ABE #16830 Corrected documentation of parseArguments()
# @version  9.0 2018-01-10 ABE #15293 log input files
# @version  5.2 2016-05-03 ABE #10691 log program parameters
# @version  5.0 2016-02-01 ABE #10136 add option for relative paths
# @version  5.0 2016-01-20 ABE #9990  move CheopsExit() implementation to
#                                     makefiles module, remove intermediate
#                                     output dir
# @version  5.0 2016-01-11 ABE #9942  allow for repeatable command line args
# @version  3.3            ABE        replace getDynamicProcParams() with getParam()
#                          ABE #8408  add cheopsExit method
# @version  1.0            ABE        first released version
#

import argparse
import builtins
import logging
import os
import shutil
import sys
import xml.etree.ElementTree as ET
from program_params import joborder

## The logger object
logger = logging.getLogger()

##
# @brief parses the program arguments.
# @ingroup ProgParam
# @author Anja Bekkelien UGE
#
# The expected arguments are either a -h/--help or a -v/--version, or the path 
# to a job order file. If -v or -h is found, the program displays the expected 
# information and exits. If a path is found, it is returned.
#
# Additional program arguments can be defined in the program's configuration 
# file.
#
# Invalid arguments will causes the program to display an error message and exit.
#
# @param progName the name of the program, aka the task name
# @param version the program version
# @return If the path to a job order was given on the command line, the method 
#         returns a map containing one key, 'job_order', whose value is the path
#         to the job order file. Otherwise, it returns None.
#
def parseArguments(progName, version):
    
    parser = argparse.ArgumentParser(prog=progName)
    
    # Add default arguments
    parser.add_argument('job_order', nargs='?', help='the path to the job order file')
    parser.add_argument('-v', '--version', action='version', version='%(prog)s v' + version)
    
    paramConfig = parseParameterFile(progName, parser)
    
    if paramConfig:
    
        addParserArguments(parser, paramConfig)
    
    args = vars(parser.parse_args())
    
    if paramConfig:
        for name, conf in paramConfig.items():
            
            # This is a repeatable parameter. It's value is a list containing 
            # the value of each repetition of the argument. Convert its values 
            # to the type defined in the parameter config file.
            if paramConfig[name]['repeatable']:
                
                # No values were given on the command line
                if args[name] is None:
                    # If the parameter has a default value in the config, assign it
                    if 'value' in paramConfig[name]:
                        if paramConfig[name]['value'] is not None:
                            args[name] = [paramConfig[name]['value']]
                        # If the default value element is present but contains no value
                        else:
                            args[name] = []
                else:
                    convertedValues = []
                    for value in args[name]:
                        if paramConfig[name]['type'] == bool:
                            convertedValues.append(convertBoolString(value, name))
                        else:
                            convertedValues.append(paramConfig[name]['type'](value))
                    args[name] = convertedValues
    
    return args

##
# @brief Parses program parameters defined in the program parameter 
#        configuration file.
# @ingroup ProgParam
# @author Anja Bekkelien UGE
#
# The method returns a dictionary containing all parameters read from the 
# configuration file.
#
# The dictionary key is the 'name' of the parameter, and the value is another
# dictionary containing the values of all elements contained inside the param
# element, including the 'name' element. The key names of this inner dictionary
# are identical to the element names found in the xml file. 
#
# @param progName the name of the program
# @param parser   an ArgumentParser object to which the parameters read from 
#                 file will be added
# @return dictionary containing program parameters
#
def parseParameterFile(progName, parser):
    
    xmlFile = progName + '.xml'

    if not os.path.isfile(xmlFile):
        xmlFile = './conf/' + progName + '.xml'
        if not os.path.isfile(xmlFile):
            if not 'CHEOPS_SW' in os.environ:
                raise RuntimeError('Environmenet variable CHEOPS_SW is not defined.')
            xmlFile = os.environ['CHEOPS_SW'] + '/conf/' + progName + '.xml'
            if not os.path.isfile(xmlFile):
                return
    
    tree = ET.parse(xmlFile)
    root = tree.getroot()
   
    params = {}
     
    for param in root.iter('param'):
        
        name = param.find('name')
        if name is not None and name.text is not None:
            name = name.text.strip()
            params[name] = {'name': name}
        else:
            raise RuntimeError('Config file [' + xmlFile + '] contains parameter without name')

        shortName = param.find('short_name')
        if shortName is not None and shortName.text is not None:
            shortName = shortName.text.strip()
            params[name]['short_name'] = shortName

        # Set the data type of the argument
        paramType = param.find('type')
        if paramType is not None and paramType.text is not None:
            paramType = paramType.text.strip()
            if paramType == 'string':
                params[name]['type'] = str 
            elif paramType == 'int':
                params[name]['type'] = int
            elif paramType == 'double':
                params[name]['type'] = float
            elif paramType == 'bool':
                params[name]['type'] = bool
            else:
                raise RuntimeError('Invalid parameter type [' + paramType + '] found in [' + xmlFile + ']')
        else:
            params[name]['type'] = str  

        defaultValue = param.find('value')
        if defaultValue is not None and defaultValue.text is not None:
            defaultValue = defaultValue.text.strip()
            if paramType is not None:
                if paramType == 'bool':
                    params[name]['value'] = convertBoolString(defaultValue, name)
                else:
                    try:
                        params[name]['value'] = params[name]['type'](defaultValue)
                    except ValueError:
                        logger.error('Error parsing program parameter [' + str(posArgs) + ']: could not convert default value [' + defaultValue + '] to [' + paramType + ']')
            else:
                params[name]['value'] = defaultValue
        # The <value> element is present but empty
        elif defaultValue is not None and defaultValue.text is None:
            params[name]['value'] = None
        
        help = param.find('help')
        if help is not None and help.text is not None:
            help = help.text.strip()
            params[name]['help'] = help

        repeatable = param.find('repeatable')
        if repeatable is not None and repeatable.text is not None:
            params[name]['repeatable'] = convertBoolString(repeatable.text.strip(), name)
        else:
            params[name]['repeatable'] = False
    
    return params 
               
##
# @brief Addes command line arguments a an argparser.
# @ingroup ProgParam
# @author Anja Bekkelien UGE
#
# The arguments to add are defined in the 'params' method argument. This is a
# dictionary containing the information read from a parameter configuration xml 
# file. The dictionary key is the 'name' of the parameter, and the value is another
# dictionary containing the values of all elements contained inside the param
# element, including the 'name' element. The key names of this inner dictionary
# are identical to the element names found in the xml file.
#
# @param parser an argparser
# @param params a dictionary containing information read from a parameter 
#               configuration file
def addParserArguments(parser, params):
   
    for name, values in params.items():
         
        # Positional arguments passed to parser.add_argument()
        posArgs = []
        # Keyword arguments passed to parser.add_argument()
        keyArgs = {} 
        
        posArgs.append('--' + name)

        if 'short_name' in values:
            posArgs.append('-' + values['short_name'])

        if 'help' in values:
            keyArgs['help'] = values['help']

        if not values['repeatable']:
            if 'type' in values:
                if values['type'] == bool:
                    keyArgs['action'] = 'store_true'
                    if 'value' in values and values['value'] == False:
                        keyArgs['action'] = 'store_false'
                else:
                    keyArgs['type'] = values['type']
                    if 'value' in values:
                        keyArgs['default'] = values['value']
                    else:
                        keyArgs['required'] = True
            
        else:
            keyArgs['action'] = 'append'
            if 'value' not in values:
                keyArgs['required'] = True
        
        parser.add_argument(*posArgs, **keyArgs)
        
##
# @brief Converts a string value to boolean False of True.
# @ingroup ProgParam
# @author Anja Bekkelien UGE
#
# The following values are converted to True:
# 'on',  'yes', '1', 'true', 'True'
#
# The following values are converted to False:
# 'off', 'no',  '0', 'false', 'False', '', None
#
# @param stringValue the string to be converted to bool.
# @param paramName   the name of the parameter to which the value belongs
# @return boolean value
#
def convertBoolString(stringValue, paramName):
    
    trueValues = ['on',  'yes', '1', 'true', 'True']
    falseValues = ['off', 'no',  '0', 'false', 'False', '', None]
    
    if stringValue in trueValues:
        return True
    if stringValue in falseValues:
        return False
    
    raise RuntimeError('Cannot parse value of parameter ' + paramName 
                       + ': found invalid boolean value [' + stringValue + ']')
        
##
# @brief Top level interface class to access all parameters of a program.
# @ingroup ProgParam
# @author Anja Bekkelien UGE
#
# An application program should not create an instance of this class by
# itself, instead is shall call the CheopsInit() function, which returns an 
# instance of this class.
#
# The class will try to read parameters from the following locations:
# - the command line arguments provided in sys.argv
# - a job order file, if the path is given as a command line argument
# - a program parameter file if such a file with the same name as the program
#   but with the extension xml exists in ./, ./conf/ or $CHEOPS_SW/conf/
#
class ProgramParams:
    
    ##
    # @brief Parses a job order file, and creates the temporary output directory
    #        in which programs shall store output data. If the job order path is
    #        not valid, a warning is logged.
    # 
    # If program arguments are defined in a configuration file, these will be
    # read.
    #       
    # @param progName the name of the program.
    # @param version the version of the program.
    #
    def __init__(self, progName, version):
        
        ## The name of the program
        self.progName = progName
        
        args = parseArguments(self.progName, version)
        
        ## Store the program parameters read from the command line
        self.params = {}
        for argName, argValue in args.items():
            self.params[argName] = argValue 
        
        # Read the job  order file
        if 'job_order' in args and args['job_order'] is not None:
            ## 
            # Object containing data read from the job order given as input to the
            # program
            self.jobOrder = joborder.getJobOrder(args['job_order'], self.progName)
        else:
            logger.warn('No job order file defined.')
            self.jobOrder = joborder.JobOrder()

    ##
    # @ brief Log the program parameters, which can come from both the job order 
    #         and from the command line (including default values taken from 
    #         the program parameter config file)
    #
    # @param externalParams optional argument for passing in additional
    #                       key/value pairs to be written to the "Program
    #                       Parameters" section of the log file in addition
    #                       to those defined in the job order, on the
    #                       command line or in the config file.
    #
    def logProgramParams(self, externalParams={}):
        parameters = []
        
        # First read the command line params/config file params.
        # If this parameter also exists in the job order, ignore it and print the
        # value found in the job order instead. This is in line with the
        # implementation of GetAsString, in which the value found in the job order
        # takes precedence if the parameter exists both in the job order and on
        # the command line/in the config file.
        #
        # If the parameter is defined in the externalParameters, the value in
        # externalParameters takes precedence
        for name, value in self.params.items():
            if (name not in self.jobOrder.params and 
                name not in externalParams):
                if isinstance(value, list):
                    for v in value:
                        parameters.append(name + '=' + str(v) )
                else:    
                    parameters.append(name + '=' + str(value))
        
        # Then, read the job order params.
        # If a parameter is defined in the externalParameters, the value in the
        # externalParameters takes precedence.
        for name, value in self.jobOrder.params.items():
            if name not in externalParams:
                parameters.append(name + '=' + str(value))
                
        # Then, read the externalParams
        for name, value in externalParams.items():
            parameters.append(name + '=' + str(value))
        
        if len(parameters) > 0:
            logger.info('Program parameters:')
            # Sort params alphabetically while ignoring case
            for param in sorted(parameters, key=str.lower):
                logger.info('  ' + param)
            
        # Log the input files
        for structName in sorted(self.getInputStructNames(), key=str.lower):
            logger.info('Input files of type ' + structName + ':')
            for file in sorted(self.getInputFiles(structName), key=str.lower):
                baseName = os.path.basename(file)
                
                # If this is a fits file, remove the name of the first extension
                # from the end of the file name
                extStartIndex = baseName.find('[')
                if extStartIndex != -1:
                    baseName = baseName[:extStartIndex]
                    
                logger.info('  ' + baseName)

    ##
    # @brief Return the list of input struct names found in the job order file
    #        for this process.
    # 
    # @return list of input struct names
    #
    def getInputStructNames(self):

        return list(self.jobOrder.inputFiles.keys())
        
    ##
    # @brief Return the list of input files for the given struct name.
    #
    # Input files that are FITS files will have the extension included in the
    # file name.
    # 
    # @param dataStructName the name of the struct for which to return files
    # @return list of input files. If a data structure of that name does not 
    #         exist in the job order file, an empty list is returned.
    # @throw KeyError if the data struct is not defined in the job order
    #
    def getInputFiles(self, dataStructName):

        if dataStructName in self.jobOrder.inputFiles:
            return self.jobOrder.inputFiles[dataStructName]
        else:
            raise KeyError('Input data structure [' + dataStructName 
                           + '] is not defined in the job order file')

    ##
    # @brief Returns the temporary output directory relative to the OUT 
    #        directory defined in the job order.
    # 
    # If the program runs without error, files in this directory will be  
    # ingested into the archive after the program exits. 
    # 
    # @return the output directory for files created by the program
    #
    def getOutDir(self):

        return self.jobOrder.outDir

    ##
    # @brief Return the directory defined for the output file type 'TMP'.
    # 
    # Files placed in this directory will be automatically deleted when the 
    # process exits.
    # 
    # @return the output directory for temporary files
    #
    def getTmpDir(self):
        
        return self.jobOrder.tmpDir
        
    ##
    # @brief Return the value of the job order parameter whose name is
    #        given as argument to the method.
    # 
    # @return the value of a job order parameter. 
    # @throw KeyError if the parameter does not exist in the job order file.
    #
    def getParam(self, name):
        
        if name in self.jobOrder.params:
            return self.jobOrder.params[name]
        if name in self.params:
            return self.params[name]
        raise KeyError('Parameter [' + name + '] not found')
    
    