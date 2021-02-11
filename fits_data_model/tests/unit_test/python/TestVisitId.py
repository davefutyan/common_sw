import os
import sys
import unittest

import fits_data_model

RESOURCES = "resources"

class TestVisitId(unittest.TestCase):
 
    def setUp(self):

        self.createdFiles = []

    def tearDown(self):
        
        for file in self.createdFiles:
            self.removeFile(file)
        self.createdFiles = []
        
    def removeFile(self, path):

        if os.path.isfile(path):
            os.remove(path)
                 
    def createFile(self, className, fileName, imgSize=None):

        self.removeFile(fileName)
        
        if imgSize:
            className(fileName, "CREATE", imgSize)
        else:
            className(fileName, "CREATE")

        self.createdFiles.append(fileName)
        
    def testVisitIdConstructors(self):
        
        # Test invalid visits
        
        with self.assertRaises(RuntimeError):
            print(fits_data_model.VisitId(1, 0, 0, 1))
 
        with self.assertRaises(RuntimeError):
            print(fits_data_model.VisitId(51, 0, 0, 1))
        
        programType = 10
        programId = 2
        requestId = 3
        visitCounter = 4
        fileNamePattern = "PR100002_TG000304"

        visitId = fits_data_model.VisitId()
        self.assertIsNotNone(visitId)
        self.assertFalse(visitId.isValid())
        
        visitId2 = fits_data_model.VisitId(programType+10, programId, requestId, visitCounter)
        
        visitId = fits_data_model.VisitId(programType, programId, requestId, visitCounter)
        self.assertIsNotNone(visitId)
        self.assertTrue(visitId.isValid())
        self.assertEqual(visitId.getProgramType(), programType)
        self.assertEqual(visitId.getProgramId(), programId)
        self.assertEqual(visitId.getRequestId(), requestId)
        self.assertEqual(visitId.getVisitCounter(), visitCounter)
           
        visitId = fits_data_model.VisitId(fileNamePattern)
        self.assertIsNotNone(visitId)
        self.assertTrue(visitId.isValid())
        self.assertEqual(visitId.getProgramType(), programType)
        self.assertEqual(visitId.getProgramId(), programId)
        self.assertEqual(visitId.getRequestId(), requestId)
        self.assertEqual(visitId.getVisitCounter(), visitCounter)
        self.assertEqual(visitId.getFileNamePattern(), fileNamePattern)
        
        # Test string conversion
        self.assertTrue(hasattr(visitId, '__str__'))
        self.assertEqual(visitId.__str__(), fileNamePattern)
      
        # Test operators
        self.assertTrue(visitId < visitId2)
        self.assertTrue(visitId != visitId2)
        self.assertTrue(visitId == visitId)
        
        # Test hash
        visitId3 = fits_data_model.VisitId(programType, programId, requestId, visitCounter)
        self.assertEqual(hash(visitId), hash(visitId3))
        self.assertNotEqual(hash(visitId), hash(visitId2))
        
        s = set([visitId, visitId2, visitId3])
        self.assertEqual(len(s), 2)
        
    def testVisitId_HDU_Constructor(self):
        
        sciRawSubarrayFile = RESOURCES + "/TestUtilityClasses_VisitIdCtor_SciRawSubarray.fits"
        programType = 10
        programId = 2
        requestId = 3
        visitCounter = 4
        
        # Create test data file
        self.createFile(fits_data_model.SciRawSubarray, sciRawSubarrayFile, [200, 200, 1])
        # Open the file
        sciRawSubarray = fits_data_model.SciRawSubarray(sciRawSubarrayFile)
        # Udate header keywords
        sciRawSubarray.setKeyProgtype(programType)
        sciRawSubarray.setKeyProgId(programId)
        sciRawSubarray.setKeyReqId(requestId)
        sciRawSubarray.setKeyVisitctr(visitCounter)
        
        visitId = fits_data_model.VisitId(sciRawSubarray)
        
        # Test visit id   
        self.assertTrue(visitId.isValid())
        self.assertEqual(visitId.getProgramType(), programType)
        self.assertEqual(visitId.getProgramId(), programId)
        self.assertEqual(visitId.getRequestId(), requestId)
        self.assertEqual(visitId.getVisitCounter(), visitCounter)
        
    def testSetGetVisitIdOnHDU(self):
        
        fileName = "resources/TestVisitId_SciRawSubarray.fits"
        self.createFile(fits_data_model.SciRawSubarray, fileName, [200, 200, 1])
        
        visitId = fits_data_model.VisitId(10, 1, 1, 1)
        sciRawSubArray = fits_data_model.SciRawSubarray(fileName)
        sciRawSubArray.setKeyVisitId(visitId)
        
        visitId2 = sciRawSubArray.getKeyVisitId()
        
        self.assertEqual(visitId.getProgramType(), visitId2.getProgramType())
        self.assertEqual(visitId.getProgramId(), visitId2.getProgramId())
        self.assertEqual(visitId.getRequestId(), visitId2.getRequestId())
        self.assertEqual(visitId.getVisitCounter(), visitId2.getVisitCounter())
        
if __name__ == "__main__":
    unittest.main()