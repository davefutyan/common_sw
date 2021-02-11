import os
import sys
import unittest

import fits_data_model

RESOURCES = "resources"

class TestPassId(unittest.TestCase):

    def setUp(self):
        
        self.fileName = RESOURCES + "/TestPassId_SciRawSubarray.fits"

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
        
    def testPassId(self):
        
        year = 20
        month = 2
        day = 3
        hour = 12
        fileNamePattern = 'PS{}{:0>2}{:0>2}{:0>2}'.format(year, month, day, hour)
        
        passId = fits_data_model.PassId()
        passId = fits_data_model.PassId(year, month, day, hour)
        
        passId2 = fits_data_model.PassId(year+1, month, day, hour)
        
        self.assertEqual(passId.getYear(), year)
        self.assertEqual(passId.getMonth(), month)
        self.assertEqual(passId.getDay(), day)
        self.assertEqual(passId.getHour(), hour)
        
        self.assertEqual(passId.getFileNamePattern(), fileNamePattern)
                
        # Test string conversion
        self.assertTrue(hasattr(passId, '__str__'))
        self.assertEqual(passId.__str__(), '{}{:0>2}{:0>2}{:0>2}'.format(year, month, day, hour))
        
        # Test operators
        self.assertTrue(passId < passId2)
        self.assertTrue(passId != passId2)
        self.assertTrue(passId == passId)
        
        # Test hash
        passId3 = fits_data_model.PassId(year, month, day, hour)
        self.assertEqual(hash(passId), hash(passId3))
        self.assertNotEqual(hash(passId), hash(passId2))
        
        s = set([passId, passId2, passId3])
        self.assertEqual(len(s), 2)
        
    def testGetSetPassIdOnHdu(self):
        
        self.createFile(fits_data_model.SciRawSubarray, self.fileName, [2,2,1])
        
        passId = fits_data_model.PassId(20, 1, 1, 1)
        
        sciRawSubArray = fits_data_model.SciRawSubarray(self.fileName)
        sciRawSubArray.setKeyPassId(passId)
        
        passId2 = sciRawSubArray.getKeyPassId()
        
        self.assertEqual(passId.getYear(), passId2.getYear())
        self.assertEqual(passId.getMonth(), passId2.getMonth())
        self.assertEqual(passId.getDay(), passId2.getDay())
        self.assertEqual(passId.getHour(), passId2.getHour())
        
    def testEqualsOperator(self):

        passId1 = fits_data_model.PassId()
        passId2 = fits_data_model.PassId()
        self.assertTrue(passId1 == passId1)
        self.assertTrue(passId1 == passId2)

        passId1 = fits_data_model.PassId('20010100')
        self.assertTrue(passId1 != passId2)

        passId2 = fits_data_model.PassId('20010100')
        self.assertTrue(passId1 == passId2)

        passId2 = fits_data_model.PassId('20010101')
        self.assertTrue(passId1 != passId2)

if __name__ == "__main__":
    unittest.main()
