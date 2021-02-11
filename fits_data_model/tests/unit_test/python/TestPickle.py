from astropy.table import Table
from collections import OrderedDict
import copy
import os
import pickle
import sys
import unittest

from fits_data_model import SciRawHkifsw, UTC, PassId

RESOURCES = "resources"

class TestPickle(unittest.TestCase):
 
    def setUp(self):

        self.createdFiles = []
        self.fileName = 'result/TestPickls_SciRawHkifsw.fits'
        self.pickleName = 'result/utc.pickle'

        self.removeFile(self.fileName)
        self.removeFile(self.pickleName)

    def tearDown(self):
        
        self.removeFile(self.fileName)
        self.removeFile(self.pickleName)
        
    def removeFile(self, path):

        if os.path.isfile(path):
            os.remove(path)
                 
    def testDeepCopy(self):
        
        sciRawHkifsw = SciRawHkifsw(self.fileName, "CREATE")

        procNum = sciRawHkifsw.getKeyProcNum()
        procNumCopy = copy.deepcopy(procNum)
        
        utc = sciRawHkifsw.getKeyVStrtU()
        utcCopy = copy.deepcopy(utc)
        self.assertNotEqual(id(utc), id(utcCopy))
        
        mjd = sciRawHkifsw.getKeyVStrtM()
        mjdCopy = copy.deepcopy(mjd)
        
        passId = sciRawHkifsw.getKeyPassId()
        passIdCopy = copy.deepcopy(passId)
        
        sciRawHkifsw.setKeyProgtype(10)
        sciRawHkifsw.setKeyProgId(1)
        sciRawHkifsw.setKeyReqId(1)
        sciRawHkifsw.setKeyVisitctr(0)
        visitId = sciRawHkifsw.getKeyVisitId()
        visitIdCopy = copy.deepcopy(visitId)
    
    def testPassIdDeepCopy(self):

        # Test empyt/invalid pass id
        passId = PassId()
        self.assertFalse(passId.isValid())
        self.assertEqual(passId.__str__(), '')
        self.assertEqual(passId.getYear(), 10)
        self.assertEqual(passId.getMonth(), 1)
        self.assertEqual(passId.getDay(), 1)
        self.assertEqual(passId.getHour(), 0)
        
        passIdCopy = copy.deepcopy(passId)
        self.assertFalse(passIdCopy.isValid())
        self.assertEqual(passIdCopy.__str__(), '')
        self.assertEqual(passIdCopy.getYear(), 10)
        self.assertEqual(passIdCopy.getMonth(), 1)
        self.assertEqual(passIdCopy.getDay(), 1)
        self.assertEqual(passIdCopy.getHour(), 0)
        
        # Test valid pass id
        year = 20
        month = 2
        day = 2
        hour = 2
        passIdString = "{:0>2}{:0>2}{:0>2}{:0>2}".format(year, month, day, hour)
        
        passId = PassId(year, month, day, hour)
        self.assertTrue(passId.isValid())
        self.assertEqual(passId.__str__(), passIdString)
        self.assertEqual(passId.getYear(), year)
        self.assertEqual(passId.getMonth(), month)
        self.assertEqual(passId.getDay(), day)
        self.assertEqual(passId.getHour(), hour)
        
        passIdCopy = copy.deepcopy(passId)
        self.assertTrue(passIdCopy.isValid())
        self.assertEqual(passIdCopy.__str__(), passIdString)
        self.assertEqual(passIdCopy.getYear(), year)
        self.assertEqual(passIdCopy.getMonth(), month)
        self.assertEqual(passIdCopy.getDay(), day)
        self.assertEqual(passIdCopy.getHour(), hour)

    def testPickle(self):
        
        sciRawHkifsw = SciRawHkifsw(self.fileName, "CREATE")

        utc = sciRawHkifsw.getKeyVStrtU()

        with open(self.pickleName, 'wb') as f:
            pickle.dump(utc, f, pickle.HIGHEST_PROTOCOL)
        
        with open(self.pickleName, 'rb') as f:
            data = pickle.load(f)
        
if __name__ == "__main__":
    unittest.main()