import os
import sys
import unittest


from program_params import joborder, programparams

class TestJobOrder(unittest.TestCase):
    
    
    def setUp(self):
        
        self.jobOrderPath = 'resources/test_job_order_py.xml'
        
        self.processorName = 'TestJobOrder'
        
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
                       'Log_File': 'log/test.log',
                       'Alert_Email' : ''}
        
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
        
        parser = joborder.JobOrderParser()
        self.jobOrder = parser.parseXml(self.jobOrderPath, self.processorName)
        
        
    def testParams(self):
        """ 
        Check that the job order parameters were read correctly.
        """
        
        self.assertDictEqual(self.jobOrder.params, self.params)
        

    def testInputStructs(self):
        """
        Check that the input struct names and input files were read correctly.
        """
        
        self.assertEqual(len(self.jobOrder.inputFiles), len(self.inputStructNames))
        for structName in self.inputStructNames:
            self.assertIn(structName, self.jobOrder.inputFiles)
            
            # Check each  file belonging to the struct name
            self.assertListEqual(self.inputStructNames[structName], self.jobOrder.inputFiles[structName])


    def testOutputDirectories(self):
        """
        Check that the output struct names were read correctly.
        """

        self.assertEqual(self.jobOrder.outDir, self.outputStructNames['OUT'])
        self.assertEqual(self.jobOrder.tmpDir, self.outputStructNames['TMP'])
        
            
if __name__ == '__main__':
    unittest.main()
    