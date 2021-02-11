import os
import sys
import unittest

import fits_data_model

RESOURCES = "resources"

class TestFitsDal(unittest.TestCase):
 
    def setUp(self):

        self.fileName = 'resources/testFitsDal.fits'
        self.removeFile(self.fileName)

    def tearDown(self):

        self.removeFile(self.fileName)
        
    def removeFile(self, path):

        if os.path.isfile(path):
            os.remove(path)
                 
    def createFile(self, className, fileName, imgSize=None):

        self.removeFile(fileName)
        
        if imgSize:
            className(fileName, "CREATE", imgSize)
        else:
            className(fileName, "CREATE")

        
#     def testClassHierarchy(self):
#         
#         self.assertTrue(issubclass(fits_data_model.SciRawSubarray, fits_data_model.FitsDalHeader))

    def testSetProgram(self):

        programName = 'testProgram'
        programVersion = '0.1'
        svnRevision = '1000'
        procChain = 'testChain'
        revisionNumber = 1
        processingNumber = 2
         
        fits_data_model.FitsDalHeader.setProgram(programName, programVersion, svnRevision)
        self.assertEquals(fits_data_model.FitsDalHeader.getDataVersion(), 0)
        fits_data_model.FitsDalHeader.setProgram(programName, programVersion, svnRevision, procChain)
        self.assertEquals(fits_data_model.FitsDalHeader.getDataVersion(), 0)
        fits_data_model.FitsDalHeader.setProgram(programName, programVersion, svnRevision, procChain, revisionNumber)
        self.assertEquals(fits_data_model.FitsDalHeader.getDataVersion(), 100)
        fits_data_model.FitsDalHeader.setProgram(programName, programVersion, svnRevision, procChain, revisionNumber, processingNumber)
        self.assertEquals(fits_data_model.FitsDalHeader.getDataVersion(), 102)

        self.createFile(fits_data_model.SciRawSubarray, self.fileName, [2,2,1])     

        sciRawSubarray = fits_data_model.SciRawSubarray(self.fileName)
        self.assertEqual(sciRawSubarray.getKeyProcChn(), procChain)
        self.assertEqual(sciRawSubarray.getKeyArchRev(), revisionNumber)
        self.assertEqual(sciRawSubarray.getKeyProcNum(), processingNumber)
        
    def testGetAttr(self):
        
        self.createFile(fits_data_model.SciRawSubarray, self.fileName, [2,2,1])     

        # Open the fits file with FitsDalHeader
        header = fits_data_model.FitsDalHeader(self.fileName)
        self.assertEqual(header.getAttr('EXTNAME'), 'SCI_RAW_SubArray')
        self.assertEqual(header.getAttr('PROC_NUM'), '0')

        
        
        
if __name__ == "__main__":
    unittest.main()
