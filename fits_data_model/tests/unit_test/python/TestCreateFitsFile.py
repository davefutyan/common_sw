import os
import sys
import unittest
 
import fits_data_model
 
RESOURCES = "resources"
 
class TestCreateFitsFile(unittest.TestCase):
     
    def setUp(self):
    
        fits_data_model.FitsDalHeader.setProgram('TestCreateFitsFile.py', '0.1', '1', 'undefined', 0, 0)
         
        self.programType = 10
        self.programId = 2
        self.observationId = 3
        self.visitCounter = 4
        self.fileNamePattern = "PR010002_TG000304"
         
        self.path = RESOURCES
        self.visitId = fits_data_model.VisitId(10, 1, 1, 1)
        self.passId = fits_data_model.PassId(10, 1, 1, 1)
        self.obt = fits_data_model.OBT(1, 1)
        self.utc = fits_data_model.UTC(2023, 1, 1, 1, 1, 1, 0.5)
        self.imgSize = [200, 200, 1]

    def tearDown(self):
        
        self.removeFile(fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_SubArray"))
        self.removeFile(fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_FullArray"))
        self.removeFile(fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_ImageMetadata"))
    
    def removeFile(self, path):
        
        if os.path.isfile(path):
            os.remove(path)
          
    def testBuildFileName(self):
        
        structName = "SCI_PRW_SubArray"
        fileName = fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", structName)
        self.assertEqual(fileName, self.path + "/CH_PR100001_TG000101_PS10010101_TU2023-01-01T01-01-01_SCI_PRW_SubArray_V0000.fits")
 
 
    def testCreateFitsImage(self):
         
        # Delete old test data
        fileName = fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_SubArray")
        self.removeFile(fileName)
 
        hdu = fits_data_model.createFitsFile(fits_data_model.SciPrwSubarray, 
                                             self.path, 
                                             self.utc, 
                                             self.visitId, 
                                             passId=self.passId,
                                             imgSize=self.imgSize)
          
        self.assertEqual(hdu.getExtName(), "SCI_PRW_SubArray")
        self.assertIsInstance(hdu, fits_data_model.SciPrwSubarray)
 
    def testCreateFitsTable(self):
         
        # Delete old test data
        fileName = fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_ImageMetadata")
        self.removeFile(fileName)
 
        hdu = fits_data_model.createFitsFile(fits_data_model.SciPrwImagemetadata, 
                                             self.path, 
                                             self.utc, 
                                             self.visitId,
                                             passId=self.passId)
          
        self.assertEqual(hdu.getExtName(), "SCI_PRW_ImageMetadata")
        self.assertIsInstance(hdu, fits_data_model.SciPrwImagemetadata)

    def testAppendHdu(self):
          
        # Create file with three extensions. Done in separate method because the
        # file is saved only when the object is destroyed.
        filenameSubArray, filenameFullArray = self.appendHdu()
  
        # Try to open the newly created extensions
        sciPrwSubarray = fits_data_model.SciPrwSubarray(filenameSubArray, "READONLY")
        sciPrwSubArray_imagemetadata = fits_data_model.SciPrwImagemetadata(filenameSubArray + "[SCI_PRW_ImageMetadata]", "READONLY")
        sciPrwSubArray_darklarge = fits_data_model.SciPrwDarklarge(filenameSubArray + "[SCI_PRW_DarkLarge]", "READONLY")
        
        self.assertEqual(sciPrwSubArray_imagemetadata.getExtName(), "SCI_PRW_ImageMetadata")
        self.assertEqual(sciPrwSubArray_darklarge.getExtName(), "SCI_PRW_DarkLarge")
        
        sciPrwFullarray = fits_data_model.SciPrwFullarray(filenameFullArray, "READONLY")
        sciPrwFullArray_imagemetadata = fits_data_model.SciPrwImagemetadata(filenameFullArray + "[SCI_PRW_ImageMetadata]", "READONLY")
        self.assertEqual(sciPrwFullArray_imagemetadata.getExtName(), "SCI_PRW_ImageMetadata")
  
    def appendHdu(self):
  
        # Create a fits file, and append an extension to it
        # Delete old test data
        fileNameSubArray = fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_SubArray")
        fileNameFullArray = fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_FullArray")
        self.removeFile(fileNameSubArray)
        self.removeFile(fileNameFullArray)

        sciPrwSubarray = fits_data_model.createFitsFile(fits_data_model.SciPrwSubarray, 
                                                        self.path, 
                                                        self.utc, 
                                                        self.visitId, 
                                                        passId=self.passId,
                                                        imgSize=self.imgSize)
        
        sciPrwFullarray = fits_data_model.createFitsFile(fits_data_model.SciPrwFullarray, 
                                                        self.path, 
                                                        self.utc, 
                                                        self.visitId, 
                                                        passId=self.passId,
                                                        imgSize=self.imgSize)
        
        # Append table
        #fileNameSubArray = sciPrwSubarray.getFileName()
        sciPrwSubArray_Imagemetadata = fits_data_model.appendSciPrwImagemetadata(sciPrwSubarray)
          
        self.assertIsInstance(sciPrwSubArray_Imagemetadata, fits_data_model.SciPrwImagemetadata)
        self.assertEqual(sciPrwSubArray_Imagemetadata.getExtName(), "SCI_PRW_ImageMetadata")
        
        # Append image
        sciPrwSubArray_Darklarge = fits_data_model.appendSciPrwDarklarge(sciPrwSubarray, [16, 1, 1])
        
        self.assertIsInstance(sciPrwSubArray_Darklarge, fits_data_model.SciPrwDarklarge)
        self.assertEqual(sciPrwSubArray_Darklarge.getExtName(), "SCI_PRW_DarkLarge")
          
        # Try to append an extension that is undefined for SciPrwSubarray
        self.assertRaises(Exception, fits_data_model.appendSciRawImagemetadata, sciPrwSubarray)
        
        # Use the same append method for a different data type
        sciPrwFullArray_imageMetadata = fits_data_model.appendSciPrwImagemetadata(sciPrwFullarray)
        self.assertIsInstance(sciPrwFullArray_imageMetadata, fits_data_model.SciPrwImagemetadata)
        self.assertEqual(sciPrwFullArray_imageMetadata.getExtName(), "SCI_PRW_ImageMetadata")
          
        return fileNameSubArray, fileNameFullArray
        
            
    def createTestFileforOpenHdu(self):
        
                # Delete old test data
        fileName = fits_data_model.buildFileName(self.path, self.utc, self.visitId, self.passId, "", "SCI_PRW_SubArray")
        self.removeFile(fileName)
 
        hdu = fits_data_model.createFitsFile(fits_data_model.SciPrwSubarray, 
                                             self.path, 
                                             self.utc, 
                                             self.visitId, 
                                             passId=self.passId,
                                             imgSize=self.imgSize)
          
        fits_data_model.appendSciPrwImagemetadata(hdu)
        
        return fileName
    
    def testOpenHdu(self):
        
        fileName = self.createTestFileforOpenHdu()
        firstHdu = fits_data_model.SciPrwSubarray(fileName)
        secondHdu = fits_data_model.openSciPrwImagemetadata(firstHdu)
        # Open an HDU that is defined as an associated HDU but that has not been
        # appended to the fits file
        nonExistantHdu = fits_data_model.openSciPrwBlankreduced(firstHdu)

        self.assertEqual(secondHdu.getExtName(), "SCI_PRW_ImageMetadata")
        self.assertEqual(nonExistantHdu, None)
        # Try to open an extension that is not defined as an associated HDU for 
        # the first extension
        self.assertRaises(Exception, fits_data_model.openSciRawImagemetadata, firstHdu)
        
if __name__ == "__main__":
    unittest.main()
