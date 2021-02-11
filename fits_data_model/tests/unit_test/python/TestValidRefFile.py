import os
import sys
import unittest

import fits_data_model

RESOURCES = "resources"

class TestValidRefFile(unittest.TestCase):
 
    def setUp(self):
        
        self.fileNameRefAppLimits = "resources/TestValidRefFile_RefAppLimits.fits"
        self.fileNameSocAppLeapseconds = "resources/TestValidRefFile_SocAppLeapseconds.fits"
        
        self.validityStart = fits_data_model.UTC(2020, 1, 1, 1, 1, 1)
        self.validityStop = fits_data_model.UTC(2021, 1, 1, 1, 1, 1)
        self.validDataTime = fits_data_model.UTC(2020, 2, 1, 1, 1, 1)
        self.invalidDataTime = fits_data_model.UTC(2022, 2, 1, 1, 1, 1)
        
        self.createRefFile(fits_data_model.RefAppLimits, self.fileNameRefAppLimits, self.validityStart, self.validityStop)
        self.createRefFile(fits_data_model.SocAppLeapseconds, self.fileNameSocAppLeapseconds, self.validityStart, self.validityStop)
        
    def tearDown(self):
        
        self.removeFile(self.fileNameRefAppLimits)
        self.removeFile(self.fileNameSocAppLeapseconds)

    def removeFile(self, path):
        
        if os.path.isfile(path):
            os.remove(path)
                 
    def createRefFile(self, className, fileName, validityStart, validityStop):
        
        self.removeFile(fileName)
        refFile = className(fileName, "CREATE")
        refFile.setKeyVStrtU(validityStart)
        refFile.setKeyVStopU(validityStop)
        
    def testValidRefFile(self):
        
        validRefAppLimits = fits_data_model.ValidRefFile(fits_data_model.RefAppLimits, [self.fileNameRefAppLimits])
        refFile = validRefAppLimits.getFile(self.validDataTime)
        self.assertIsInstance(refFile, fits_data_model.RefAppLimits)        
        with self.assertRaises(RuntimeError):
            validRefAppLimits.getFile(self.invalidDataTime)
        
        validSocAppLeapseconds = fits_data_model.ValidRefFile(fits_data_model.SocAppLeapseconds, [self.fileNameSocAppLeapseconds])
        refFile = validSocAppLeapseconds.getFile(self.validDataTime)
        self.assertIsInstance(refFile, fits_data_model.SocAppLeapseconds)
        with self.assertRaises(RuntimeError):
            validSocAppLeapseconds.getFile(self.invalidDataTime) 

        # Instantiate a ValidRefFile object that handles RefAppLimits files
        # The arguments are the class name and a list of file names
        validRefAppLimits = fits_data_model.ValidRefAppLimits([self.fileNameRefAppLimits])
        refFile = validRefAppLimits.getFile(self.validDataTime)
        with self.assertRaises(RuntimeError):
            validRefAppLimits.getFile(self.invalidDataTime)
            
        
if __name__ == "__main__":
    unittest.main()
