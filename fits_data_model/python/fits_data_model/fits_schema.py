##
# @file 
# @author Anja Bekkelien UGE
# @ingroup fits_data_model
#
# @brief This module contains classes for reading xml files that follow the
#        FITS_schema defined in fits_data_model_schema.xsd.
#
# @version 12.1.1 2020-03-13 ABE #21154 Update parsing of fsd files in python 
#                                       for compatibility with python 3.6.7.
# @version  9.3   2018-06-07 ABE        Store the default values of keywords.
# @version  8.1   2017-09-18 ABE        First version.
#

import logging
import os
import shutil
from xml.dom import pulldom

from xml.dom.pulldom import parse
from xml import sax

## The logger object
logger = logging.getLogger()

## The directory containing installed xml files
SCHEMA_DIRECTORY = os.path.expandvars('${CHEOPS_SW}/resources')

##
# @brief This is a container class for a single header keyword.
# @ingroup fits_data_model
# @author Anja Bekkelien UGE
#
class Keyword:
    
    ##
    #  @brief Constructor that initializes the values of the keyword.
    #
    #  @param name         the name of the keyword
    #  @param db           the value of the db attribute of this keyword's   
    #                      definition in its Report Schema Definition 
    #  @param type         the keyword's data type
    #  @param value        the value of the keyword  
    #  @param defaultValue default value of the keyword defined in the schema 
    # 
    def __init__(self, name, db, type, value=None, defaultValue=None):
        
        ## the name of the keyword
        self.name = name
        ## the value of the attribute's db keyword as defined in its RSD.
        self.db = db
        ## the keyword's data type
        self.type = type
        ## the value of the keyword  
        self.value = value
        ## the default value of the keyword defined in the schema
        self.defaultValue = defaultValue
        
    ##
    #  @brief Returns true if this keyword's db attribute is set to 'true'.
    #
    #  @return true if this keyword's db attribute is set to 'true'
    #
    def isDb(self):
        return self.db == 'true'
    
    ##
    #  @brief Returns true if this keyword's db attribute is set to 'common'.
    #
    #  @return true if this keyword's db attribute is set to 'common'
    #
    def isCommon(self):
        return self.db == 'common'
        
    ##
    #  @brief Returns a string representation of this Keyword.
    #
    #  @return a string representation of this Keyword
    #
    def __str__(self):
        return '{name: ' + str(self.name) + ', db: ' + str(self.db) + ', type: '+ str(self.type) + ', value: ' + str(self.value) + ', default: ' + str(self.defaultValue) + '}'

##
#  @brief This class provides methods for accessing the contents of Fits Schema
#         Definitions files (.fsd) and Report Schema Definition files (.rsd).
#  @ingroup fits_data_model
#  @author Anja Bekkelien UGE
#
class FitsSchema:
    
    ##
    #  @brief Opens a FITS_scema xml file and reads its contents.
    #
    #  @param schemaType         the type of schema to read. This is the value 
    #                            of the extname attribute of the HDU element.
    #  @param fileExtension      the extension of the xml file containing the 
    #                            FITS_scema element, e.g. '.fsd' or '.rsd'
    #  @param schemaDirectory    (optional) the directory containing the xml  
    #                            file. If set to None, the file will be searched
    #                            for in  $CHEOPS_SW/resources
    #  @param readAssociatedHdus (optional) if true, the schemas of all 
    #                            associated hdus also be parsed and will be
    #                            available in the dict attribute associatedHdus.
    #                            If false, the dict keys will still contain the 
    #                            names of those hdus but the values will point
    #                            to None.
    # 
    def __init__(self, schemaType, fileExtension, schemaDirectory=None, readAssociatedHdus=False):
        
        ## The type of the schema file, as defined by its EXTNAME attribute
        self.schemaType = schemaType
        
        ## The contents of the rsd's brief element
        self.brief = None
        
        ## The contents of the rsd's description element
        self.description = None
        
        ## The keyword names defined in the rsd
        self.keywords = {}
        
        ## "image" or "table"
        self.fitsType = None
        
        ## The associated HDUs. The dictionary maps extension names to the 
        ## FitsSchema object of the associated HDUs. 
        self.associatedHdus = {}
        
        if not schemaDirectory:
            schemaDirectory = SCHEMA_DIRECTORY
        ## The path to the rsd file
        self.schemaFileName = self.schemaType + fileExtension

        # Need to cd to the directory containing the .ifsd files, otherwise
        # their imports will not be resolved
        workingDir = os.getcwd()
        os.chdir(schemaDirectory)
        
        if not os.path.isfile(self.schemaFileName):
            raise FileNotFoundError('File [' + schemaDirectory + '/' + self.schemaFileName + ' not found')
        
        parser = sax.make_parser()
        parser.setFeature(sax.handler.feature_external_ges, True)
        
        # Use pulldom to parse the file because it is able to expand external 
        # entities (the .ifsd files) defined in xml files
        doc = pulldom.parse(self.schemaFileName, parser=parser)
        for event, node in doc:
            if event == pulldom.START_ELEMENT and node.tagName == 'brief':
                doc.expandNode(node)
                self.brief = ' '.join(t.nodeValue for t in node.childNodes if t.nodeType == t.TEXT_NODE)
            elif event == pulldom.START_ELEMENT and node.tagName == 'description':
                doc.expandNode(node)
                self.description = ' '.join(t.nodeValue for t in node.childNodes if t.nodeType == t.TEXT_NODE)
            elif event == pulldom.START_ELEMENT and node.tagName == 'keyword':
                doc.expandNode(node)
                name = node.getAttribute('name')
                db = node.getAttribute('db')
                dataType = node.getAttribute('data_type')
                defaultValue = node.getAttribute('default')
                 
                if node.getAttribute('db') == 'common' or node.getAttribute('db') == 'true':
                    self.keywords[name] = Keyword(name, db, dataType, defaultValue=defaultValue)
                # The value of the db attribute is an empty string if it is not
                # defined in the rsd file
                else:
                    self.keywords[name] = Keyword(name, 'false', dataType, defaultValue=defaultValue)
            elif event == pulldom.START_ELEMENT and node.tagName == 'Associated_HDU':
                doc.expandNode(node)
                associatedHduName = ''.join(t.nodeValue for t in node.childNodes if t.nodeType == t.TEXT_NODE)
                self.associatedHdus[associatedHduName] = None
            elif event == pulldom.START_ELEMENT and node.tagName == 'table':
                self.fitsType = 'table'
            elif event == pulldom.START_ELEMENT and node.tagName == 'image':
                self.fitsType = 'image'
        
        os.chdir(workingDir)
        
        if readAssociatedHdus:
            for associatedHduName in self.associatedHdus.keys():
                associatedHdu = self.__class__(associatedHduName, schemaDirectory)
                self.associatedHdus[associatedHduName] = associatedHdu
                   

##
#  @brief This class reads Fits Structure Definitions files (.fsd).
#  @ingroup fits_data_model
#  @author Anja Bekkelien UGE
#
class FitsSchemaDefinition(FitsSchema):

    ##
    #  @brief Opens an .fsd file and reads its contents.
    #
    #  @param schemaType         the type of schema to read. This is the value 
    #                            of the extname attribute of the HDU element.
    #  @param schemaDirectory    (optional) the directory containing the .fsd  
    #                            file. If set to None, the file will be searched 
    #                            for in $CHEOPS_SW/resources
    #  @param readAssociatedHdus (optional) if true, the schemas of all 
    #                            associated hdus also be parsed and will be
    #                            available in the dict attribute associatedHdus.
    #                            If false, the dict keys will still contain the 
    #                            names of those hdus but the values will point
    #                            to None.
    # 
    def __init__(self, schemaType, schemaDirectory=None, readAssociatedHdus=False):
        super().__init__(schemaType, '.fsd', schemaDirectory, readAssociatedHdus)


##
#  @brief This class reads Report Schems Definitions files (.rsd).
#  @ingroup fits_data_model
#  @author Anja Bekkelien UGE
#
class ReportSchemaDefinition(FitsSchema):

    ##
    #  @brief Opens an .rsd file and reads its contents.
    #
    #  @param schemaType      the type of schema to read. This is the value of
    #                         the extname attribute of the HDU element.
    #  @param schemaDirectory (optional) the directory containing the .rsd file. 
    #                         If set to None, the file will be searched for in 
    #                         $CHEOPS_SW/resources
    #  @param readAssociatedHdus (optional) if true, the schemas of all 
    #                            associated hdus also be parsed and will be
    #                            available in the dict attribute associatedHdus.
    #                            If false, the dict keys will still contain the 
    #                            names of those hdus but the values will point
    #                            to None.
    # 
    def __init__(self, schemaType, schemaDirectory=None, readAssociatedHdus=False):
        super().__init__(schemaType, '.rsd', schemaDirectory, readAssociatedHdus)
    

