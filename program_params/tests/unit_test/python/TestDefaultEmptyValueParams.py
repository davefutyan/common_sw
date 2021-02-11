import os
import sys
import unittest

from program_params import joborder, programparams

class TestDefaultEmptyValueParams(unittest.TestCase):
    
    
    def setUp(self):
        
        self.processorName = 'TestDefaultEmptyValueParams.py'
        self.processorVersion = '0.1'

        

    # Check that when parameters with default empty values are not given on the 
    # command line, the returned value is None, or an empty list for repeatable
    # parameters  
    def testDefaultValues(self):
                
        paramValues = {'MinDefParam' : 'MinDefParamValue',
                       'CommandLine' : 'CommandLineValue',
                       'StringParam' : None,
                       'StringVectorParam' : [],
                       'IntParam' : None,
                       'IntVectorParam' : [],
                       'DoubleParam' : None,
                       'DoubleVectorParam' : [],
                       'BoolParam' : False,
                       'BoolVectorParam' : []}
        
        # Set the command line arguments passed to ProgramParams
        sys.argv = [self.processorName, 
                    '--CommandLine', 'CommandLineValue', 
                    '--MinDefParam', 'MinDefParamValue']
        
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)

        # Test parameters read from parameter file
        for name, value in paramValues.items():
            print(name + ': got \'' + str(progParams.getParam(name)) + '\', expected \'' + str(value) + '\'')
            self.assertEqual(progParams.getParam(name), value)


    # Check that when parameters with default empty values are given on the 
    # command line, the returned value is the value from the command line
    def testOverrideDefaultValues(self):
                
        paramValues = {'MinDefParam' : 'MinDefParamValue',
                       'CommandLine' : 'CommandLineValue',
                       'StringParam' : 'new value',
                       'StringVectorParam' : ['new value'],
                       'IntParam' : 500,
                       'IntVectorParam' : [500],
                       'DoubleParam' : -2.65e-4,
                       'DoubleVectorParam' : [-2.65e-4],
                       'BoolParam' : True,
                       'BoolVectorParam' : [True]}
        
        # Set the command line arguments passed to ProgramParams
        sys.argv = [self.processorName, 
                    '--CommandLine=CommandLineValue', 
                    '--MinDefParam=MinDefParamValue',
                    '--StringParam=new value',
                    '--StringVectorParam=new value',
                    '--IntParam=500',
                    '--IntVectorParam=500',
                    '--DoubleParam=-2.65e-4',
                    '--DoubleVectorParam=-2.65e-4',
                    '--BoolParam',
                    '--BoolVectorParam=true']
        
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)

        # Test parameters read from parameter file
        for name, value in paramValues.items():
            print(name + ': got \'' + str(progParams.getParam(name)) + '\', expected \'' + str(value) + '\'')
            self.assertEqual(progParams.getParam(name), value)


    # Check that the types of parameters that have default empty values and were not 
    # given on the command line are None, or (empty) list for repeatable parameters
    def testValueTypesOfDefaultValues(self):
         
        paramTypes = {'MinDefParam' : str,
                      'CommandLine' : str,
                      'StringParam' : type(None),
                      'StringVectorParam' : list,
                      'IntParam' : type(None),
                      'IntVectorParam' : list,
                      'DoubleParam' : type(None),
                      'DoubleVectorParam' : list,
                      'BoolParam' : bool,
                      'BoolVectorParam' : list}
         
        # Set the command line arguments passed to ProgramParams
        sys.argv = [self.processorName, 
                    '--CommandLine', 'CommandLineValue', 
                    '--MinDefParam', 'MinDefParamValue']
         
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)
  
        # Test parameters read from parameter file
        for name, value in paramTypes.items():
            print(name + ': got \'' + str(type(progParams.getParam(name))) + '\', expected \'' + str(value) + '\'')
            self.assertTrue(isinstance(progParams.getParam(name), value))
                                    
    # Check that the types of parameters that have default empty values and were 
    # given on the command line are correct
    def testValueTypesOfOverriddenDefaultValues(self):
         
        paramTypes = {'MinDefParam' : str,
                      'CommandLine' : str,
                      'StringParam' : str,
                      'StringVectorParam' : list,
                      'IntParam' : int,
                      'IntVectorParam' : list,
                      'DoubleParam' : float,
                      'DoubleVectorParam' : list,
                      'BoolParam' : bool,
                      'BoolVectorParam' : list}
         
        # Set the command line arguments passed to ProgramParams
        sys.argv = [self.processorName, 
                    '--CommandLine=CommandLineValue', 
                    '--MinDefParam=MinDefParamValue',
                    '--StringParam=new value',
                    '--StringVectorParam=new value',
                    '--IntParam=500',
                    '--IntVectorParam=500',
                    '--DoubleParam=-2.65e-4',
                    '--DoubleVectorParam=-2.65e-4',
                    '--BoolParam',
                    '--BoolVectorParam=true']
         
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)
  
        # Test parameters read from parameter file
        for name, value in paramTypes.items():
            print(name + ': got \'' + str(type(progParams.getParam(name))) + '\', expected \'' + str(value) + '\'')
            self.assertTrue(isinstance(progParams.getParam(name), value))
            
        
if __name__ == '__main__':

    unittest.main()
    