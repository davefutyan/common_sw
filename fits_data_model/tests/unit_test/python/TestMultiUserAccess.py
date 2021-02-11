import os
import subprocess
import sys
import unittest
 
import fits_data_model

################################################################################
#
#  This unit test file implements test specification 
#  CHEOPS-SOC-TC-WP05-FUNC-005 Multi-User Access to Data Products
#
class TestMultiUserAccess(unittest.TestCase):
     
    def setUp(self):
         
        self.externalScript = 'python/OpenFitsFile.py'
        self.productType = 'SCI_COR_Lightcurve'
        self.productTypeClass = 'SciCorLightcurve'
        self.dataName = 'testMultiUserAccess'
        self.outputDir = 'result'
        
        self.programType = 10
        self.programId = 2
        self.requestId = 3
        self.visitCounter = 4
        self.visitId = fits_data_model.VisitId(self.programType, self.programId, self.requestId, self.visitCounter)
        self.passId = fits_data_model.PassId(23, 1, 1, 1)
        self.utc = fits_data_model.UTC(2023, 1, 1, 1, 1, 1, 0.5)
        
        # Delete old test data
        self.fileName = fits_data_model.buildFileName(self.outputDir, self.utc, self.visitId, self.passId, self.dataName, self.productType)
        self.removeFile(self.fileName)
 
        hdu = fits_data_model.createFitsFile(fits_data_model.SciCorLightcurve, 
                                             self.outputDir, 
                                             self.utc, 
                                             self.visitId,
                                             passId=self.passId,
                                             dataName=self.dataName)

    def tearDown(self):
         
        self.removeFile(self.fileName)
    
    def removeFile(self, path):
        
        if os.path.isfile(path):
            os.remove(path)
          
    def testMultiUserAccess(self):
            
        args1 = [self.externalScript, 'p1', self.productTypeClass, self.fileName]
        args2 = [self.externalScript, 'p2', self.productTypeClass, self.fileName]
        
        p1 = subprocess.Popen(args1)
        p2 = subprocess.Popen(args2)
        
        p1.wait()
        p2.wait()
 

if __name__ == "__main__":
    unittest.main()