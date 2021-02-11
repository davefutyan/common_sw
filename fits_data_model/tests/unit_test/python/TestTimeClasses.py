import os
import sys
import unittest

import fits_data_model

RESOURCES = "resources"

class TestTimeClasses(unittest.TestCase):
 
    def setUp(self):

        pass

    def tearDown(self):

        pass
     
    def removeFile(self, path):
        
         if os.path.isfile(path):
             os.remove(path)
             
    def createFile(self, className, fileName, imgSize=None):

        self.removeFile(fileName)
        
        if imgSize:
            className(fileName, "CREATE", imgSize)
        else:
            className(fileName, "CREATE")
        
       
    def testObt(self):
        
        obt = fits_data_model.OBT();
        obt = fits_data_model.OBT(1000)
        obt2 = fits_data_model.OBT(2000)
        obt3 = fits_data_model.OBT(2000)
        
        # Test equals
        self.assertFalse(obt == obt2)
        self.assertTrue(obt2 == obt3)
        
        # Test less than
        self.assertTrue(obt.__lt__(obt2))
        self.assertTrue(obt < obt2)
        self.assertFalse(obt2.__lt__(obt))
        self.assertFalse(obt2 < obt)
        
        # Get int value
        self.assertEqual(obt.getObt(), 1000)
           
        # Convert object to int
        self.assertEqual(int(obt), 1000)
        self.assertEqual(int(obt2), 2000)
        
        # Test using the synchronized bit
        self.assertEqual(int(fits_data_model.OBT(1000, True)), 1001)
        
        # Get UTC
#         utc = obt.getUtc()
#         self.assertIsInstance(utc, fits_data_model.UTC)

        fits_data_model.OBT.setResetCounterFileNames([])
                
        # Test string conversion
        self.assertTrue(hasattr(obt, '__str__'))
        self.assertEqual(obt.__str__(), '1000')
        
        # Test hashing
        self.assertEqual(hash(obt2), hash(obt3))
        self.assertNotEqual(hash(obt), hash(obt2))
        
        s = set([obt, obt2, obt3])
        self.assertEqual(len(s), 2)        
        
    def testBarycentricOffset(self):
        
        ra = 1.5
        dec = 2.5
        bo = fits_data_model.BarycentricOffset(ra, dec)
        
        self.assertIsInstance(bo, fits_data_model.BarycentricOffset)
        
    def testMjd(self):
        
#         utc = fits_data_model.UTC(2020, 1, 1, 1, 1, 1)
#         utc2 = fits_data_model.UTC(2020, 2, 1, 1, 1, 1)

        # Test constructors
        mjd = fits_data_model.MJD()
        mjd = fits_data_model.MJD(2458850.0)    # 01.01.2020
        mjd2 = fits_data_model.MJD(mjd)
        
#         mjd = utc.getMjd()
#         mjd2 - utc2.getMjd()
        
        # Test comparison
        mjd2 = fits_data_model.MJD(2458881.0)   # 02.01.2020
        self.assertTrue(mjd < mjd2)
        self.assertTrue(mjd <= mjd2)
        self.assertFalse(mjd2 < mjd)
        self.assertFalse(mjd2 <= mjd)
        self.assertTrue(mjd2 == 2458881.0)
        self.assertTrue(mjd != mjd2)
        self.assertFalse(mjd != mjd)
        self.assertTrue(mjd2 > mjd)
        self.assertTrue(mjd2 >= mjd)
        self.assertFalse(mjd > mjd2)
        self.assertFalse(mjd >= mjd2)
        
        # Test getMjd
        self.assertEqual(mjd.getMjd(), 2458850.0)
        
        # Test getUtc
#         utc = mjd.getUtc()
#         self.assertIsInstance(utc, fits_data_model.UTC)
        
        # Test Barycentric offset
#        bjd = mjd + fits_data_model.BarycentricOffset(1, 1)
        
        # Test float
        f = float(mjd)
        self.assertEqual(f, 2458850.0)
        
        # Test string conversion
        self.assertTrue(hasattr(mjd, '__str__'))
        self.assertEqual(mjd.__str__(), '2458850.000000')
        
        # Test hashing
        mjd3 = fits_data_model.MJD(mjd)
        self.assertEqual(hash(mjd), hash(mjd3))
        self.assertNotEqual(hash(mjd), hash(mjd2))
        
        s = set([mjd, mjd2, mjd3])
        self.assertEqual(len(s), 2)
        
    def testBjd(self):
        
        bjdValue = 1.0
        bjdValue2 = bjdValue + 1.0
        
        # Test constructors
        bjd = fits_data_model.BJD()
        bjd = fits_data_model.BJD(bjdValue)
        bjd2 = fits_data_model.BJD(bjd)
        bjd2 = fits_data_model.BJD(bjdValue2)
        
        # Test get value
        self.assertEqual(bjd.getBjd(), bjdValue)
        self.assertEqual(bjd2.getBjd(), bjdValue2)
        
        # Test comparison
        self.assertTrue(bjd < bjd2)
        self.assertFalse(bjd2 < bjd)
        self.assertTrue(bjd <= bjd2)
        self.assertFalse(bjd2 <= bjd)
#        self.assertTrue(bjd2 == bjdValue2)
        self.assertTrue(bjd2 == bjd2)
        self.assertFalse(bjd2 == bjd)
        self.assertTrue(bjd != bjd2)
        self.assertFalse(bjd2 != bjd2)
        
        # Test subtraction
        #mjd = bjd - fits_data_model.BarycentricOffset(1, 1)
        #self.assertIsInstance(mjd, fits_data_model.MJD)
        
        # Test string conversion
        self.assertTrue(hasattr(bjd, '__str__'))
        self.assertEqual(bjd.__str__(), '1.000000')
        
        # Test hashing
        bjd3 = fits_data_model.BJD(1.0)

        self.assertEqual(bjd, bjd3)
        self.assertEqual(hash(bjd), hash(bjd3))

        self.assertNotEqual(bjd, bjd2)
        self.assertNotEqual(hash(bjd), hash(bjd2))
        
        s = set([bjd, bjd2, bjd3])
        self.assertEqual(len(s), 2)
        
    def testUtc(self):
        
        year = 2020
        month = 2
        day = 3
        hour = 12
        minute = 5
        second = 6
        secFraction = 0.15
        
        utcString = "{}-{:0>2}-{:0>2}T{:0>2}:{:0>2}:{:0>2}.{:0>6}".format(year, month, day, hour, minute, second, int(secFraction * 1000000))
        
        # Test constructors
        utc = fits_data_model.UTC()
        utc = fits_data_model.UTC(year, month, day, hour, minute, second, secFraction)
        utc = fits_data_model.UTC(utcString)
        
        # Test values
        self.assertEqual(utc.getYear(), year)
        self.assertEqual(utc.getMonth(), month)
        self.assertEqual(utc.getDay(), day)
        self.assertEqual(utc.getHour(), hour)
        self.assertEqual(utc.getMinute(), minute)
        self.assertEqual(utc.getSecond(), second)
        self.assertEqual(utc.getSecFraction(), secFraction)
        self.assertEqual(utc.getUtc(), utcString)
        
        # Test MJD
#         mjd = utc.getMjd()
#         self.assertIsInstance(mjd, fits_data_model.MJD)
        
        # Test OBT
#         obt = utc.getObt()
#         self.assertIsInstance(obt, fits_data_model.OBT)
    
        # Test conversion to string
        self.assertEqual(str(utc), utcString)
        
        # Test comparison
        utcEqual = fits_data_model.UTC(year, month, day, hour, minute, second, secFraction)
        utc2 = fits_data_model.UTC(year + 1, month, day, hour, minute, second, secFraction)
        utc2Equal = fits_data_model.UTC(year + 1, month, day, hour, minute, second, secFraction)

        self.assertTrue(utc < utc2)
        self.assertTrue(utc <= utc2)
        self.assertFalse(utc2 < utc)
        self.assertFalse(utc2 <= utc)

        self.assertTrue(utc2 > utc)
        self.assertTrue(utc2 >= utc)
        self.assertFalse(utc > utc2)
        self.assertFalse(utc >= utc2)
        
        self.assertTrue(utc != utc2)
        self.assertFalse(utc != utcEqual)
        self.assertTrue(utc == utcEqual)
        self.assertFalse(utc == utc2)
#         self.assertTrue(utc == utcString)
#         self.assertFalse(utc2 == utcString)
        
        # Test hashing
        self.assertEqual(hash(utc2), hash(utc2Equal))
        self.assertNotEqual(hash(utc), hash(utc2))
        
        s = set([utc, utc2, utc2Equal])
        self.assertEqual(len(s), 2)
        
    def testLeapSeconsd(self):
        
        fits_data_model.LeapSeconds.setLeapSecondsFileNames([])

    def testOBTUTCCorrelation(self):
        
        fits_data_model.OBTUTCCorrelation.setCorrelationFileNames([])
    
    
if __name__ == "__main__":
    unittest.main()
