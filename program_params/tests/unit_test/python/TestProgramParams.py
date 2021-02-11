import os
import sys
import unittest

from program_params import joborder, programparams

class TestProgramParams(unittest.TestCase):
    
    
    def setUp(self):
        
        self.jobOrderPath = 'resources/test_job_order_py.xml'
        
        self.processorName = 'TestProgramParams'
        self.processorVersion = '0.1'
        
        self.params = {'Processor_Name' : 'example_ipf',
                       'Stdout_Log_Level' : 'INFO',
                       'Stderr_Log_Level' : 'ERROR',
                       'Test' : 'true',
                       'Breakpoint_Enable' : 'false',
                       'Processing_Station' : 'TEST',
                       'Start' : '2014-08-01T00:00:00.000000',
                       'Stop' : '2014-08-02T00:00:00.000000',
                       'Configuration_Baseline' : '000000',
                       'Version' : '01.01',
                       'Configuration_Dir' : 'resources/config',
                       'Alert_Email' : ""}
        
        self.XML = ['resources/CH_PR990001_TG000001_TU2018-01-01T12-00-30_Metadata_V000.xml']
        
        self.SCI_PRW_FullArrayFiles = ['resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000000_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000001_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000002_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000003_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000004_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000005_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000006_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000007_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000008_V000.fits[SCI_PRW_FullArray]',
                                       'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_FullArray_20180101T000009_V000.fits[SCI_PRW_FullArray]']
        
        self.SCI_PRW_ImageMetadataFiles = ['resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000000_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000001_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000002_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000003_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000004_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000005_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000006_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000007_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000008_V000.fits[SCI_PRW_ImageMetadata]',
                                           'resources/input/cheops_P9900001_O00001_S001_C00001_SCI_PRW_ImageMetadata_20180101T000009_V000.fits[SCI_PRW_ImageMetadata]']
        self.SCI_PRW_SubArray = []
        
        self.inputStructNames = {'XML': self.XML,
                                 'SCI_PRW_FullArray': self.SCI_PRW_FullArrayFiles,
                                 'SCI_PRW_ImageMetadata': self.SCI_PRW_ImageMetadataFiles,
                                 'SCI_PRW_SubArray': self.SCI_PRW_SubArray}
        
        self.outputStructNames = {'OUT': 'results', 
                                  'TMP': 'results'}

        sys.argv = [self.processorName, self.jobOrderPath]
        self.progParams = programparams.ProgramParams(self.processorName, self.processorVersion)
        

    def testJobOrderPathNone(self):
        
        sys.argv = [self.processorName]
        progParams = programparams.ProgramParams(self.processorName, self.processorVersion)

        self.assertListEqual(progParams.getInputStructNames(), [])
        self.assertEqual(progParams.getOutDir(), None)
        self.assertEqual(progParams.getTmpDir(), None)
        with self.assertRaises(KeyError):
            progParams.getParam('Stdout_Log_Level')
        
    def testInputFiles(self):
        
        # Test input struct names
        inputStructNames = self.progParams.getInputStructNames()
        self.assertNotEqual(inputStructNames, None)
        self.assertListEqual(sorted(inputStructNames), sorted(list(self.inputStructNames.keys())))
        
        for structName in inputStructNames:
            files = self.progParams.getInputFiles(structName)
            self.assertListEqual(files, self.inputStructNames[structName])
    
    def testOutput(self):
        
        # Test out directory
        self.assertEqual(self.progParams.getOutDir(), self.outputStructNames['OUT'])
        
        # Test tmp directory
        self.assertEqual(self.progParams.getTmpDir(), self.outputStructNames['TMP'])
        
    def testParams(self):
        
        # Test dynamic processing params
        for name, value in self.params.items():
            self.assertEqual(self.progParams.getParam(name), value)
        
        with self.assertRaises(KeyError):
            self.progParams.getParam('Does_not_exist')
            
            
if __name__ == '__main__':

    unittest.main()
    