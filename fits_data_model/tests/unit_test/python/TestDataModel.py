import math
import numpy
import os
import sys
import unittest

import fits_data_model

RESOURCES = "resources"

TMP_IMAGE = "tmpImage.fits"
TMP_IMAGE_CURRENT_STATUS = "tmpImage_current_status.fits"
TMP_TABLE = "tmpTable.fits"

class TestDataModel(unittest.TestCase):
    
    def tearDown(self):
        # If a test fails, it might not clean up temporary files itself.
        if os.path.isfile(RESOURCES + "/" + TMP_IMAGE):
            os.remove(RESOURCES + "/" + TMP_IMAGE)
            
        if os.path.isfile(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS):
            os.remove(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS)
         
        if os.path.isfile(RESOURCES + "/" + TMP_TABLE):
            os.remove(RESOURCES + "/" + TMP_TABLE)
 
    def setUp(self):
        if os.path.isfile(RESOURCES + "/" + TMP_IMAGE):
            os.remove(RESOURCES + "/" + TMP_IMAGE)
            
        if os.path.isfile(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS):
            os.remove(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS)
         
        if os.path.isfile(RESOURCES + "/" + TMP_TABLE):
            os.remove(RESOURCES + "/" + TMP_TABLE)
    
    
    ###############################################
    #
    #  Testing FITS images
    #
    ############################################### 

    def testCreateImage(self):
        dimensions = [200, 200, 2]
        filename = RESOURCES + "/" + TMP_IMAGE
        self.createImage(filename, dimensions)
        os.remove(filename)
        
    def testOpenImage(self):
        dimensions = [200, 200, 2]
        filename = RESOURCES + "/" + TMP_IMAGE
        self.createImage(filename, dimensions)
        self.openImage(filename, dimensions)
        os.remove(filename)
    
    def createImage(self, filename, dimensions):
        image = fits_data_model.SciRawSubarray(filename, "CREATE", dimensions)
        self.assertEqual(image.getNumDim(), len(dimensions))
        self.assertListEqual(image.getSize(), dimensions)
    
    def openImage(self, filename, dimensions):
        image = fits_data_model.SciRawSubarray(filename, "READONLY")
        self.assertEqual(image.getNumDim(), len(dimensions))
        self.assertListEqual(image.getSize(), dimensions)
    
    def testWriteCurrentStatus(self):
        image = fits_data_model.SciRawSubarray(RESOURCES + "/" + TMP_IMAGE, "CREATE")

        self.assertTrue(not os.path.isfile(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS))
        image.writeCurrentStatus(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS)
        self.assertTrue(os.path.isfile(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS))
        
        # Check that the written file can be opened
        currentStatus = fits_data_model.SciRawSubarray(RESOURCES + "/" + TMP_IMAGE_CURRENT_STATUS)
        del currentStatus
        
    def testIterateImage(self):
        dimensions = [4, 3, 2]
        filename = RESOURCES + "/" + TMP_IMAGE
        image = fits_data_model.SciRawSubarray(filename, "CREATE", dimensions)
                    
        # Test iterating over the image using the Python iterator
        for zIndex, z in enumerate(image):
            self.assertIsInstance(z, fits_data_model.FitsDalImageIterator)
               
            for yIndex, y in enumerate(z):
                self.assertIsInstance(y, fits_data_model.FitsDalImageIterator)
                   
                for xIndex, x in enumerate(y):
                    self.assertIsInstance(x, int)
                       
                self.assertEqual(xIndex, dimensions[-3] -1, "Expected " + str(dimensions[-3]) + " elements on x axis")
            self.assertEqual(yIndex, dimensions[-2] -1, "Expected " + str(dimensions[-2]) + " elements on y axis")
        self.assertEqual(zIndex, dimensions[-1] -1, "Expected " + str(dimensions[-1]) + " elements on z axis")
     
        # Test iterating using positive indices
        value = 0
        for z in range(dimensions[-1]):
            for y in range(dimensions[-2]):
                for x in range(dimensions[-3]):
                    self.assertIsInstance(image[z,y,x], int)
                    self.assertEqual(image[z,y,x], 0)
                    # The value of the cells indicates the order in which they are accessed
                    image[z,y,x] = value
                    value += 1
                    
#       

        value = 0
        # Test iterating using negative indices
        for z in range(-dimensions[-1], 0):
            for y in range(-dimensions[-2], 0):
                for x in range(-dimensions[-3], 0):
                    self.assertIsInstance(image[z,y,x], int)
                    # Check that the order is the same as for positive indices
                    self.assertEqual(image[z,y,x], value)
                    value += 1
        
        # Test updating individual image pixels
        value = 0
        for z in range(dimensions[-1]):
            for y in range(dimensions[-2]):
                for x in range(dimensions[-3]):
                    image[z,y,x] = value
                    value += 1
 
        value = 0
        for z in range(dimensions[-1]):
            for y in range(dimensions[-2]):
                for x in range(dimensions[-3]):
                    self.assertEqual(image[z,y,x], value)
                    value += 1

        # Test that an exception is raised if giving an incorrect number of indices
        with self.assertRaises(IndexError):
            image[0]
        with self.assertRaises(IndexError):
            image[0,0]
        with self.assertRaises(IndexError):
            image[0,0,0,0]
        
        # Test that an exception is raised when indices are out of bounds
        with self.assertRaises(IndexError):
            image[11,0,0]
        with self.assertRaises(IndexError):
            image[-11,0,0]
        
        os.remove(filename)
    
    def testImageHeaderKeywords(self):
        extVer = "test"
        tmpImg = RESOURCES + "/" + TMP_IMAGE
        self.writeImageHeaderKeywords(extVer)
        self.readImageHeaderKeywords(extVer)
        
    def writeImageHeaderKeywords(self, extVer):
        image = fits_data_model.SciRawSubarray(RESOURCES + "/" + TMP_IMAGE, "CREATE", [2,2,1])
        image.setKeyExtVer(extVer)
        
    def readImageHeaderKeywords(self, extVer):
        image = fits_data_model.SciRawSubarray(RESOURCES + "/" + TMP_IMAGE, "READONLY")
        self.assertEqual(image.getKeyExtVer(), extVer)
        
    def testGetUnitOfKey(self):
        sciPrwSubarray = fits_data_model.SciPrwSubarray(RESOURCES + "/" + TMP_IMAGE, "CREATE", [2,2,1])
        retVal = sciPrwSubarray.getUnitOfKeyVStrtU()
        self.assertEqual(retVal, 'TIMESYS=UTC')
        
    def testGetComOfKey(self):
        sciPrwSubarray = fits_data_model.SciPrwSubarray(RESOURCES + "/" + TMP_IMAGE, "CREATE", [2,2,1])
        retVal = sciPrwSubarray.getComOfKeyVStrtU()
        self.assertEqual(retVal, 'Start of validity time in UTC')
        
    def testIsNullImage(self):
        sciPrwSubarray = fits_data_model.SciPrwSubarray(RESOURCES + "/" + TMP_IMAGE, "CREATE", [2,2,1])
        self.assertTrue(sciPrwSubarray.isNull(0))
        sciRawSubarray = fits_data_model.SciRawSubarray(RESOURCES + "/" + TMP_IMAGE, "APPEND", [2,2,1])
        self.assertFalse(sciRawSubarray.isNull(1))
        
    def testGetNullImage(self):
        sciPrwSubarray = fits_data_model.SciPrwSubarray(RESOURCES + "/" + TMP_IMAGE, "CREATE", [2,2,1])
        retVal = sciPrwSubarray.getNull()
        self.assertIsInstance(retVal, tuple)
        self.assertEqual(retVal, (0, True))
        
        sciRawSubarray = fits_data_model.SciRawSubarray(RESOURCES + "/" + TMP_IMAGE, "APPEND", [2,2,1])
        retVal = sciRawSubarray.getNull()
        self.assertIsInstance(retVal, tuple)
        self.assertEqual(retVal[1],True)
        
        os.remove(RESOURCES + "/" + TMP_IMAGE)
        
        sciCalSubarray = fits_data_model.SciCalSubarray(RESOURCES + "/" + TMP_IMAGE, "CREATE", [2,2,1])
        retVal = sciCalSubarray.getNull()
        self.assertIsInstance(retVal, tuple)
        self.assertEqual(type(retVal[0]), float)
        self.assertTrue(numpy.isnan(retVal[0]))
        self.assertEqual(retVal[1], True)
    
    ###############################################
    #
    #  Testing FITS tables
    #
    ###############################################
    
    def testCreateTable(self):
        filename = RESOURCES + "/" + TMP_TABLE
        self.createTable(filename)
        os.remove(filename)
        
    def testOpenTable(self):
        filename = RESOURCES + "/" + TMP_TABLE
        self.createTable(filename)
        self.openTable(filename)
        os.remove(filename)
    
    def createTable(self, filename):
        table = fits_data_model.SciRawImagemetadata(filename, "CREATE")
        
    def openTable(self, filename):
        table = fits_data_model.SciRawImagemetadata(filename, "READONLY")
        
    def testReadWriteRow(self):
        tmpTable = RESOURCES + "/" + TMP_TABLE
        rowValues = [ [fits_data_model.OBT(1), fits_data_model.UTC(2020, 1, 1, 1, 1, 1), fits_data_model.MJD(1.0)],
                      [fits_data_model.OBT(2), fits_data_model.UTC(2020, 1, 1, 1, 1, 2), fits_data_model.MJD(2.0)] ]
        
        # We must read and write in different methods to make sure that the FITS
        # table object destructor is called, because the changes are saved in the
        # destructor.
        self.writeRow(tmpTable, rowValues)
        self.readRow(tmpTable, rowValues)

    def writeRow(self, tmpTable, rows):
        table = fits_data_model.SciRawImagemetadata(tmpTable, "CREATE")
        
        for row in rows:
            table.setCellObtTime(row[0])
            table.setCellUtcTime(row[1])
            table.setCellMjdTime(row[2])

            table.writeRow()
    
    def readRow(self, tmpTable, rows):
        table = fits_data_model.SciRawImagemetadata(tmpTable, "READONLY")
        
        for row in rows:
            res = table.readRow()
            self.assertEqual(table.getCellObtTime(), row[0])
            self.assertEqual(table.getCellUtcTime().getUtc(), row[1].getUtc())
            self.assertEqual(table.getCellMjdTime(), row[2].getMjd())
            
        self.assertFalse(table.readRow())
        
        # Test reading a specific row
        for rowIndex in range(len(rows)):
            res = table.readRow(rowIndex+1)
            print(rows[rowIndex])
            self.assertEqual(table.getCellObtTime(), rows[rowIndex][0])
            self.assertEqual(table.getCellUtcTime().getUtc(), rows[rowIndex][1].getUtc())
            self.assertEqual(table.getCellMjdTime(), rows[rowIndex][2].getMjd())
        
        self.assertFalse(table.readRow(3))
                
    def testSetReadRow(self):
        tmpTable = RESOURCES + "/" + TMP_TABLE
        rows = [ [fits_data_model.OBT(1), fits_data_model.UTC(2020, 1, 1, 1, 1, 1), fits_data_model.MJD(1.0)],
                 [fits_data_model.OBT(2), fits_data_model.UTC(2020, 1, 1, 1, 1, 2), fits_data_model.MJD(2.0)],
                 [fits_data_model.OBT(3), fits_data_model.UTC(2020, 1, 1, 1, 1, 3), fits_data_model.MJD(3.0)] ]
        
        # We must read and write in different methods to make sure that the FITS
        # table object destructor is called, because the changes are saved in the
        # destructor.
        self.writeRow(tmpTable, rows)

        table = fits_data_model.SciRawImagemetadata(tmpTable, "READONLY")
        table.readRow()
        table.readRow()
        self.assertEqual(table.getCellObtTime(), rows[1][0])
        self.assertEqual(table.getCellUtcTime().getUtc(), rows[1][1].getUtc())
        self.assertEqual(table.getCellMjdTime(), rows[1][2].getMjd())
        
        table.setReadRow(1)
        table.readRow()
        self.assertEqual(table.getCellObtTime(), rows[0][0])
        self.assertEqual(table.getCellUtcTime().getUtc(), rows[0][1].getUtc())
        self.assertEqual(table.getCellMjdTime(), rows[0][2].getMjd())
        
    def testPrepareWriteRow(self):
        tmpTable = RESOURCES + "/" + TMP_TABLE
        obt = fits_data_model.OBT(1)
        utc = fits_data_model.UTC(2020, 1, 1, 1, 1, 1)
        mjd = fits_data_model.MJD(1.1)
        rows = [ [obt, utc, mjd, 2, 1.5],
                 [obt, utc, mjd, 3, 2.5] ]
        
        # write some date to the table
        tableObject = fits_data_model.SciRawImagemetadata(tmpTable, "CREATE")
        for row in rows:
            tableObject.setCellObtTime(row[0])
            tableObject.setCellUtcTime(row[1])
            tableObject.setCellMjdTime(row[2])
            tableObject.writeRow()
            
        # Overwrite the previously inserted rows with some new data
        tableObject.prepareWriteRow(1)
        rows[0][3] = rows[0][3] + 1
        rows[1][3] = rows[1][3] + 1
        for row in rows:
            tableObject.setCellObtTime(row[0])
            tableObject.setCellUtcTime(row[1])
            tableObject.setCellMjdTime(row[2])
            tableObject.writeRow()

        del tableObject
        self.readRow(tmpTable, rows)
        
    def testTableHeaderKeywords(self):
        extVer = "test"
        tmpImg = RESOURCES + "/" + TMP_IMAGE
        self.writeTableHeaderKeywords(extVer)
        self.readTableHeaderKeywords(extVer)
        
    def writeTableHeaderKeywords(self, extVer):
        table = fits_data_model.SciRawImagemetadata(RESOURCES + "/" + TMP_TABLE, "CREATE")
        table.setKeyExtVer(extVer)
        
    def readTableHeaderKeywords(self, extVer):
        table = fits_data_model.SciRawImagemetadata(RESOURCES + "/" + TMP_TABLE, "READONLY")
        self.assertEqual(table.getKeyExtVer(), extVer)
    
    def testArrayCell(self):
        self.writeArrayCell()
        self.readArrayCell()
    
    def writeArrayCell(self):
        table = fits_data_model.SimTruSubarray(RESOURCES + "/" + TMP_TABLE, "CREATE")

        arrayLen = table.getSizeDeadQe()
        self.assertEqual(arrayLen, 200)        

        # Test array column with the correct number of elements
        table.setCellDeadQe([float(i) for i in range(arrayLen)])
        table.writeRow()
        
        # Test array column with too many elements
        table.setCellDeadQe([float(i) for i in range(arrayLen + 1)])
        table.writeRow()
        
        # Test array column with too few elements
        self.assertRaises(IndexError, table.setCellDeadQe, [float(i) for i in range(arrayLen - 1)])
        
    def readArrayCell(self):
        table = fits_data_model.SimTruSubarray(RESOURCES + "/" + TMP_TABLE, "READONLY")

        arrayLen = table.getSizeDeadQe()
        self.assertEqual(arrayLen, 200)        
        
        table.readRow()
        deadQe = table.getCellDeadQe()
        self.assertEqual(len(deadQe), arrayLen)
        self.assertListEqual(deadQe, [float(i) for i in range(arrayLen)])
        
        # If we insert an array that is too long, the extra elements will not be
        # stored in the table.
        table.readRow()
        deadQe = table.getCellDeadQe()
        self.assertEqual(len(deadQe), arrayLen)
        self.assertListEqual(deadQe, [float(i) for i in range(arrayLen)])

    def testCellNullValue(self):
        table = fits_data_model.SciRawImagemetadata(RESOURCES + "/" + TMP_TABLE, "CREATE")
        
        # Check that the initial value of the ObtTime column is its null value
        utcTime = table.getCellUtcTime()
        self.assertIsInstance(utcTime, fits_data_model.UTC)
        self.assertTrue(table.isNullUtcTime())
        
        # Modify obTime and check that it no longer equals its column's null value
        utc = fits_data_model.UTC(2020, 1, 1, 1, 1, 1)
        table.setCellUtcTime(utc)
        self.assertEqual(table.getCellUtcTime(), utc)
        self.assertFalse(table.isNullUtcTime())
        
        # Reset obTime to the null value
        table.setNullUtcTime()
        self.assertTrue(table.isNullUtcTime())
        self.assertEqual(table.getCellUtcTime(), fits_data_model.UTC())
        
    def testGetUnitOfCell(self):
        table = fits_data_model.SciRawImagemetadata(RESOURCES + "/" + TMP_TABLE, "CREATE")
        table.readRow()
        retVal = table.getUnitOfCellUtcTime()
        self.assertEqual(retVal, 'TIMESYS=UTC')
        
        
if __name__ == "__main__":
    unittest.main()
