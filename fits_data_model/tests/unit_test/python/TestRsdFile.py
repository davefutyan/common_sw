import os
import sys
import unittest

from fits_data_model.fits_schema import ReportSchemaDefinition

def removeFile(path):
    if os.path.isfile(path):
        os.remove(path)
        
class TestRsdFile(unittest.TestCase):

    def setUp(self):
        
        self.resources = 'resources'
        self.rsdType = 'RPT_RAW_CHEOPSim'
        
    def testExtractPdfMetadata(self):
        
        rsd = ReportSchemaDefinition(self.rsdType)
        self.assertEqual(rsd.schemaType, self.rsdType)

if __name__ == "__main__":
    unittest.main()
