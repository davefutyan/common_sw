import os
import sys
import unittest


from program_params import joborder, programparams

class TestDefaultValueParams(unittest.TestCase):
    
    
    def setUp(self):
        
        self.processorName = 'TestDefaultValueParams.py'
        self.processorVersion = '0.1'

        

    # Check that when parameters with default values are not given on the 
    # command line, the returned value is as defined in the xml configuration 
    # file    
    def testDefaultValues(self):
                
        paramValues = {'MinDefParam' : 'MinDefParamValue',
                       'CommandLine' : 'CommandLineValue',
                       'StringParam' : 'test string',
                       'StringVectorParam' : ['test string'],
                       'IntParam' : 465,
                       'IntVectorParam' : [465],
                       'DoubleParam' : -1.65e-4,
                       'DoubleVectorParam' : [-1.65e-4],
                       'BoolParam' : True,
                       'BoolVectorParam' : [False]}
        
        # Set the command line arguments passed to ProgramParams
        sys.argv = [self.processorName, 
                    '--CommandLine', 'CommandLineValue', 
                    '--MinDefParam', 'MinDefParamValue']
        
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)

        # Test parameters read from parameter file
        for name, value in paramValues.items():
            print(name + ': got \'' + str(progParams.getParam(name)) + '\', expected \'' + str(value) + '\'')
            self.assertEqual(progParams.getParam(name), value)


    # Check that when parameters with default values are given on the 
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
                       'BoolParam' : False,
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
        print()
        print(sys.argv)
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)

        # Test parameters read from parameter file
        for name, value in paramValues.items():
            print(name + ': got \'' + str(progParams.getParam(name)) + '\', expected \'' + str(value) + '\'')
            self.assertEqual(progParams.getParam(name), value)


    # Check that the types of parameters that have default values and were not 
    # given on the command line are correct
    def testValueTypesOfDefaultValues(self):
         
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
                    '--CommandLine', 'CommandLineValue', 
                    '--MinDefParam', 'MinDefParamValue']
         
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)
  
        # Test parameters read from parameter file
        for name, value in paramTypes.items():
            print(name + ': got \'' + str(type(progParams.getParam(name))) + '\', expected \'' + str(value) + '\'')
            self.assertTrue(isinstance(progParams.getParam(name), value))
                                    
    # Check that the types of parameters that have default values and were 
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
            
    # Check that the returned values of repeatable parameters with default
    # values are correct when the default values a parameters with default
    def testRepeatableParameters(self):
          
        paramValues = {'MinDefParam' : 'MinDefParamValue',
                       'CommandLine' : 'CommandLineValue',
                       'StringParam' : 'test string',
                       'StringVectorParam' : ['StringVectorParamVal1', 'StringVectorParamVal2'],
                       'IntParam' : 465,
                       'IntVectorParam' : [500], 
                       'DoubleParam' : -1.65e-4,
                       'DoubleVectorParam' : [1.1, 2.2],
                       'BoolParam' : True,
                       'BoolVectorParam' : [True, False]}
          
        paramTypes = {'MinDefParam' : str,
                      'CommandLine' : str,
                      'StringParam' : str,
                      'StringVectorParam' : str,
                      'IntParam' : int,
                      'IntVectorParam' : int,
                      'DoubleParam' : float,
                      'DoubleVectorParam' : float,
                      'BoolParam' : bool,
                      'BoolVectorParam' : bool}
          
        args = [self.processorName,
                '--MinDefParam', 'MinDefParamValue',
                '--CommandLine', 'CommandLineValue',
                  
                '--StringVectorParam', 'StringVectorParamVal1', 
                '--StringVectorParam', 'StringVectorParamVal2',
                 
                '--IntVectorParam', '500',
                     
                '--DoubleVectorParam', '1.1', 
                '--DoubleVectorParam', '2.2',
                  
                '--BoolVectorParam', 'True', 
                '--BoolVectorParam', 'False']
          
        # Set the command line arguments passed to ProgramParams
        sys.argv = args
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)
  
        # Check the values of repeatable parameters
        for name, value in paramValues.items():
            print(name + ': got \'' + str(progParams.getParam(name)) + '\', expected \'' + str(value) + '\'')
            self.assertEqual(progParams.getParam(name), value)
              
        # Check the types of repeatable parameters
        for name, value in paramValues.items():
            if isinstance(value, list):
                self.assertTrue(isinstance(progParams.getParam(name), list))
                self.assertEqual(len(progParams.getParam(name)), len(value))
                for v in progParams.getParam(name):
                    self.assertTrue(isinstance(v, paramTypes[name]))
    
            
if __name__ == '__main__':

    unittest.main()
    