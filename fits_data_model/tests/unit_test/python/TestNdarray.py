import numpy as np
import os
import sys
import time
import unittest

import fits_data_model

RESOURCES = "resources"

class TestNdarray(unittest.TestCase):
 
    def setUp(self):

        self.fileName = 'resources/testNdarray.fits'
        self.removeFile(self.fileName)

    def tearDown(self):

        self.removeFile(self.fileName)
        
    def removeFile(self, path):

        if os.path.isfile(path):
            os.remove(path)
                 
    def createImage(self, fileName, className, imgSize):

        self.removeFile(self.fileName)
        hdu = className(fileName, "CREATE", imgSize)
        self.updateImage(hdu)
    
    def updateImage(self, fitsImage):

        img = fitsImage.getData()
        val = 0
        
        if len(img.shape) == 2:
            for y in range(img.shape[0]):
                for x in range(img.shape[1]):
                    img[y,x] = val
                    val += 1

        elif len(img.shape) == 3:            
            for z in range(img.shape[0]):
                for y in range(img.shape[1]):
                    for x in range(img.shape[2]):
                        img[z,y,x] = val
                        val += 1
        
    def testGetNdarray(self):

        # Test 2D, 3D and 4D images, and all types of image data types
        self.imageDataTypeTest(self.fileName, fits_data_model.SciCalSubarray, [5,10,2], np.float64, 8)#
        #self.imageDataTypeTest(self.fileName, fits_data_model.PipTmpDarkarray, [5,10,2], np.float32, 4)#
        self.imageDataTypeTest(self.fileName, fits_data_model.SciPrwBlanklarge, [8,10,2], np.float32, 4)
        self.imageDataTypeTest(self.fileName, fits_data_model.SciRawSubarray, [8,10,2], np.uint32, 4)
        self.imageDataTypeTest(self.fileName, fits_data_model.SciPrwImagette, [8,10,2], np.uint32, 4)
        self.imageDataTypeTest(self.fileName, fits_data_model.RefAppWhitepsf, [200,200,4], np.float64, 8) # Imgage size is constant (200x200x4)
        
    def imageDataTypeTest(self, fileName, className, imgSize, dataType, bytesInDataType):

        # Create the image, close it and save it to disk
        self.createImage(self.fileName, className, imgSize)
        
        # Open the image (data type double)
        hdu = className(self.fileName)

        img = hdu.getData()
        # Test that the pointer is not destroyed
        img = hdu.getData()

        print('\nclassName               ' + str(className))
        print('hdu.getSize()           ' + str(hdu.getSize()))
        print('hdu.getNumDim()         ' + str(hdu.getNumDim()))
        print('hdu.getNumData()        ' + str(hdu.getNumData()))
        for dimension in range(hdu.getNumDim()):
            print('hdu.getDimensionSize(' + str(dimension) + ') ' + str(hdu.getDimensionSize(dimension)))

        print('img.shape:              ' + str(img.shape))
        print('img.strides:            ' + str(img.strides))
        print('img.type:               ' + str(img.dtype))
        print('imgSize:                ' + str(imgSize))
        
        self.assertEqual(img.dtype, dataType)
        
        # The number of bytes to jump to get to the next element in the 3rd dimension
        self.assertEqual(img.strides[-1], bytesInDataType)
        # The number of bytes to jump to get to the next element in the 2nd dimension
        self.assertEqual(img.strides[-2], img.shape[-1] * bytesInDataType)
        
        if len(img.shape) == 3:
            # The number of bytes to jump to get to the next element in the 1st dimension
            self.assertEqual(img.strides[-3], img.shape[-1] * bytesInDataType * img.shape[-2])
        
        self.assertEqual(img.shape[-1], imgSize[0])
        self.assertEqual(img.shape[-2], imgSize[1])
        if len(img.shape) == 3:
            self.assertEqual(img.shape[-3], imgSize[2])
        
        # Check that the values inserted into the image were stored in the
        # correct order
        val = 0
        
        if len(img.shape) == 2:
            for y in range(img.shape[0]):
                for x in range(img.shape[1]):
                    self.assertEqual(img[y,x], val)
                    val += 1
                    
        elif len(img.shape) == 3:            
            for z in range(img.shape[0]):
                for y in range(img.shape[1]):
                    for x in range(img.shape[2]):
                        self.assertEqual(img[z,y,x], val)
                        val += 1
                        
    def testImageDataValidityOnFitsObjectDestruction(self):
        
        self.removeFile(self.fileName)
        hdu = fits_data_model.SciRawSubarray(self.fileName, "CREATE", [4, 4, 1])
        imageCube = hdu.getData()
        expectedImageData = np.ones(imageCube.shape,dtype=np.uint32)
        # A second reference to the image data
        view = imageCube[:]
        
        # Delete the fits extension object
        del hdu
        
        # The fits extension should be kept alive as long as there are 
        # references to its image data
        # Modifying the image data should not cause a crash
        imageCube += expectedImageData
        
        del imageCube

        # Accessing the image cube should now cause error
        with self.assertRaises(UnboundLocalError):
            print(imageCube)
        
        # Modify the second reference to the image data. 
        view[0,0,0] = 3
        expectedImageData[0,0,0] = 3
        # Delete the second and last reference to the image data. This should 
        # cause the fits object's destructor to be called
        del view
        
        # Re-open the fits extension
        hdu = fits_data_model.SciRawSubarray(self.fileName)
        imageCube2 = hdu.getData()
        # Test that the modifications made to the image data after the first 
        # fits object was deleted were saved
        np.testing.assert_array_equal(imageCube2, expectedImageData)

        # The fist object is opened in READONLY mode so this change should not 
        # be saved when the fits object is destroyed
        imageCube2 += np.ones(imageCube2.shape, dtype=np.uint32)
        
        del imageCube2
        del hdu
        
        # Re-open the fits extension
        hdu = fits_data_model.SciRawSubarray(self.fileName)
        imageCube3 = hdu.getData()
        
        # Check that the previous modification of the image was not saved
        np.testing.assert_array_equal(imageCube3, expectedImageData)


if __name__ == "__main__":
    unittest.main()
    
