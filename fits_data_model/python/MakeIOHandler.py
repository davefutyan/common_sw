#!/usr/bin/env python3

##
# @file 
# @author Anja Bekkelien UGE
# @ingroup FitsDataModel
#
# @brief This script creates the fits_data_model.IOHandler python module which
#        is used for accessing fits data products as astropy.Table objects.
#
# @version 6.0 2016-07-01 ABE #11081 first released version
#

import os
import shutil
import sys

sys.path.insert(0, os.path.abspath('python'))

from MakePythonTableReaders import processMultiFiles

## The path to the auto-generated file python_readAll.py
readAllPath = 'python_readAll.py'

## The path to the auto-generated file python_readers.py
readersPath = 'python_readers.py'

#writersPath = 'python_writers.py'

## The path to the auto-generated file python_reader_unittest.py
readerUnitTestPath = 'python_reader_unittest.py'

## The path to the auto-generatod IOHandler module
outputIOHandlerPath = 'python/fits_data_model/IOHandler.py'

## The path to the auto-generated IOHandler unit test module
outputIOHandlerUnitTestPath = 'tests/unit_test/python/TestIOHandler.py'

## The start of the IOHandler class
ioHandlerBegin = '''
##
#  @file
#  @brief This module provides classes and methods for reading fits tables as
#  astropy.Table objects.
#
#  This file is automatically generated. DO NOT MODIFY.
#
#  @ingroup FitsDataModel
#  @author Gabor Kovacs
#

import os
import sys
import logging
from collections import OrderedDict, deque

## The logger object
logger = logging.getLogger()
from astropy.table import Table
import numpy as np

# Search for the C++ extension module in ./lib, ../lib and $CHEOPS_SW/lib, in
# that order
sys.path.insert(0, os.path.abspath('./lib'))
sys.path.insert(0, os.path.abspath('../lib'))
sys.path.insert(0, os.path.expandvars('${CHEOPS_SW}/lib'))

import libpy_fits_data_model as fits_data_model

class IOHandler:

    """!
    This class contains one method per fits table defined in fits_data_model
    that returns the fits table as an astropy.table.Table object. The methods 
    are named readClassName, where ClassName is the class name of the a Python
    fits_data_model class, e.g readSciRawHkIfsw().
    """
    
'''

## The start of the IOHander unit test module
unitTestBegin = '''################################################################################
#
#  This file is automatically generated. DO NOT MODIFY.
#
##############################################################################

import os
import sys

import unittest
import astropy.table

import program_params
import fits_data_model
import fits_data_model.IOHandler

RESOURCES_PATH = 'resources'
PROGRAM_NAME = "TestIOHandler.py"
PROGRAM_VERSION = '0.1'

class TestIOHandler(unittest.TestCase):

    def setUp(self):
        sys.argv = [PROGRAM_NAME, ]
        self.progParams = program_params.ProgramParams(PROGRAM_NAME, PROGRAM_VERSION)
        self.IOfname = 'result/testprod.fits'
    
    def tearDown(self):
        self.removeFile(self.IOfname)
    
    def removeFile(self, path):        
        if os.path.isfile(path):
            os.remove(path)

'''

## The end of the IOHandler unit test module
unitTestEnd = '''
if __name__ == "__main__":
    unittest.main()
'''   

##
#  @brief Returns a list of src/PyExt_*.cxx files that are fits tables
# 
#  @ingroup FitsDataModel
#
#  To determine if a PyExt file contains a fits table, this method looks for the
#  presence of the string '.def("writeRow"', which is a method that only exist
#  for fits table classes.
#
#  @param directory the directory containing the auto-generated PyExt* files
#  @return list of paths to PyExt* files for fits tables
#
def getFitsTableFiles(directory):
    
    files = []
    for file in os.listdir(directory):
        if file.startswith('PyExt_') and file.endswith(".cxx"):
            with open(directory + '/' + file) as fh:
                for line in fh.readlines():
                    if '.def("writeRow"' in line:
                        files.append(directory + '/' + file)
    return files

##
#  @brief Creates the Python module fits_data_model.IOHandler and its unit test
#         module
# 
#  @ingroup FitsDataModel
#
def makeIOHandler():
    
    print('Generate python module fits_data_model.IOHandler')

    fitsTableCxxFiles = getFitsTableFiles('src')
    processMultiFiles(fitsTableCxxFiles)
    
    with open(outputIOHandlerPath, 'w') as outputIoHandlerFp:
        
        outputIoHandlerFp.write(ioHandlerBegin)
        
#         with open(readAllPath) as fp:
#             contents = fp.read()
#             outputIoHandlerFp.write(contents)        
      
        with open(readersPath) as fp:
            contents = fp.read()
            outputIoHandlerFp.write(contents)
        
#         with open(writersPath) as fp:
#             contents = fp.read()
#             outputIoHandlerFp.write(contents)
            
    with open(outputIOHandlerUnitTestPath, 'w') as unitTestFp:            
        unitTestFp.write(unitTestBegin)
        
        with open(readerUnitTestPath) as fp:
            contents = fp.read()
            unitTestFp.write(contents)
        
        unitTestFp.write(unitTestEnd)
        
    os.remove(readAllPath)
    os.remove(readersPath)
#    os.remove(writersPath)
    os.remove(readerUnitTestPath)
    
if __name__ == '__main__':
    
    makeIOHandler()
    
