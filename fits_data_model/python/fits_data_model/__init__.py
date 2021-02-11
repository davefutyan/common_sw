##
# @author: Anja Bekkelien UGE
# @ingroup FitsDataModel
#

import os
import sys
import fits_data_model
from collections import namedtuple
import logging

# Search for the C++ extension module in ./lib, ../lib and $CHEOPS_SW/lib, in 
# that order
sys.path.insert(0, os.path.abspath('./lib'))
sys.path.insert(0, os.path.abspath('../lib'))
sys.path.insert(0, os.path.expandvars('${CHEOPS_SW}/lib'))

logger = logging.getLogger()

from libpy_fits_data_model import *


##
# @ingroup FitsDataModel
# @author Anja Bekkelien, UGE
#
# @brief This class defines an iterator for FitsDalImages. It recursively iterates over
# the one dimensional array that contains the image data in FitsDalImage, such 
# that we can iterate over the image, axis by axis.
#
# As an example, imagine a data cube containing 10 200 by 200 pixel images. In the 
# underlying one dimensional array, each image is stored sequentially.
# Each element returned when iterating over the cube will be a new iterator that
# internally contains the start and end indices of a single two dimensional image.
# When using this new iterator, it will return iterators that contains the start
# and end indices of each row in that image. In turn, these iterators will return
# each pixel in its respective row.
#
class FitsDalImageIterator:
    ##
    # @brief Initialize class attributes
    #
    # @param fitsDalImage   the FitsDalImage to iterate over
    # @param dimensionIndex the index of the current image dimension, or axis. 0
    #                       is the outermost axis, e.g the Z axis of an image cube
    # @param startIndex     the index of the first pixel in this dimension
    # @param endIndex       the index of the last pixel in this dimension
    #
    def __init__(self, fitsDalImage, dimensionIndex, startIndex, endIndex):
        ## The FitsDalImage that we are iterating over
        self.fitsDalImage = fitsDalImage
        ## 
        # The index of the current image dimension, or axis. Given an image 
        # cube, the outer-most dimension (the z axis) is 0 and the inner-most
        # (the x axis) is 2.
        self.dimensionIndex = dimensionIndex
        ## The index of the first pixel in this dimension
        self.startIndex = startIndex
        ## The size of each sub image in this dimension
        self.subImageSize = self.fitsDalImage.getDimensionSize(dimensionIndex)
        ## The number of sub images in this dimension
        self.maxIterations = int((endIndex - startIndex) / self.subImageSize)
        ## The number of elements that have been iterated over so far
        self.currentIteration = 0
 
    ##
    # Return a reference to self, as is customary for iterators.
    # @return self
    def __iter__(self):
        return self
    ##
    # Return the next item in the iteration. Unless we are iterating over the 
    # inner-most axis of the image, in which case a single pixel value is returned,
    # this method returns a new iterator that iterates over a sub section of the
    # area that the current iterator iterates over. For example, if we are iterating
    # over a cube, the object returned by __next__ is an iterator for the next 
    # 2D image in the cube. That iterator will in turn return objects that are 
    # iterators for the rows in the 2D image. Lastly, these iterators returns 
    # the values of each pixel in the rows.
    #
    # @return iterator of a subsection of the image, or a pixel value
    # 
    def __next__(self):
        currentIteration = self.currentIteration
        if currentIteration == self.maxIterations: 
            raise StopIteration
        
        subImageSize = self.subImageSize
        index = int(self.startIndex + (subImageSize * currentIteration))
        self.currentIteration += 1
        
        if subImageSize > 1:
            return FitsDalImageIterator(self.fitsDalImage, self.dimensionIndex + 1, index, index + subImageSize)
         
        else:
            return self.fitsDalImage.getItem(index)

##
# @ingroup FitsDataModel
# @author Anja Bekkelien, UGE
#
# @brief Iterator for iterating over the rows of a fits dal table.
#
class FitsDalTableIterator:
    
    ##
    # Initialize the iterator with a fits table object
    #
    # @param fitsTable a fits table HDU object. The object must be an instance
    #                  of one of the fits_data_model fits table classes
    #
    def __init__(self, fitsTable):
        ## The row index.
        self.i = 0
        ## The fits table to iterate over
        self.fitsTable = fitsTable

    ##
    # @brief Returns itself
    #
    # @return itself
    #
    def __iter__(self):
        return self

    ##
    # @brief Returns the next item in the iteration.
    #
    # This item is the fits table HDU itself, having had the next row in the 
    # table read.
    #
    # @throws StopIteration when the last row of the table has been read
    #
    def __next__(self):
        if self.fitsTable.readRow():
            self.i += 1
            return self.fitsTable
        else:
            raise StopIteration()
        
##
#  @brief This class is used to get the reference file that is applicable for a
#         given UTC time.
#  @ingroup FitsDataModel
#  @author  Anja Bekkelien UGE
#
#  The class takes list of reference file names in the constructor. The
#  className argument specifies the data structure type of the files. The method
#  @b getFile will return, among those files, the one that is applicable for a
#  specific UTC time.
#
class ValidRefFile:
    
    ##
    #  @brief Constructor that takes a class and a list of reference file names
    #         as input.
    #  
    #  @param className the class of a reference data structure (RefApp*)
    #  @param fileNames A list of names of reference files. All files must be of
    #                   the type specified by className. All files
    #                   must have the same type defined in their TYPE header
    #                   keyword.
    #
    def __init__(self, className, fileNames):
        fullClassName = "Valid" + className.__name__
        class_ = getattr(fits_data_model, fullClassName)
        
        ## An object of tHe ValidRefFile<HDU> C++ class template
        self.validRefFile = class_(fileNames)
    
    ##
    #  @brief Returns the HDU of the reference file that is applicable for the UTC
    #         time given as method argument.
    #  
    #  @param dataTime The UTC time for which the returned reference file must 
    #                  be valid.
    #  @return the HDU of the reference file that is valid at the specified UTC 
    #          time.
    #  
    #  @throws runtime_error if no reference file is valid for the given UTC 
    #          @b dataTime.
    #
    def getFile(self, dataTime):
        return self.validRefFile.getFile(dataTime)
##
# @brief This method patches the code generated from C++ wherever necessary.
#
# The method modifies the generated code the following way:
#
# <h4>Adding an iterator to fits images.</h4> 
# The Python interface generated from the C++ fits_data_model API exposes fits
# images as one dimensional arrays, regardless of the actual number of dimensions
# in the images. This method turns the image into an iterable
# that works the same way as the C++ fits_data_model API: the image pixels can be
# iterated over, axis by axis, as if the image was a multidimensional array. 
# 
# When iterating over an axis, each element returned by the iterator is a new 
# iterator that iterates over a subsection of the image. When that subsection
# contains only one pixel, the actual pixel value is returned instead of an iterator.
#
# <h4>Modifying the VisitId class</h4>
# The VisitId class in C++ uses template methods that are difficult to wrap in
# Python. Therefore, these methods are re-implemented here and added to the class
# dynamically.
#
def initFitsDataModel():
   
    # Add a Python iterator to the classes defined for image structures (those
    # derived from FitsDalImage).
    import inspect
    clssmembers = inspect.getmembers(sys.modules['libpy_fits_data_model'], inspect.isclass)
    for name, clss in clssmembers:
        # If a class has the method __getitem__(), it means it's an image.
        if hasattr(clss, '__getitem__'):
            
            # Dynamically add the __iter__() method to the class. This method
            # returns a new iterator that iterates over a subsection of the image,
            # defined by the arguments startIndex and endIndex given to the 
            # constructor. 
            def __iter__(self):
                return FitsDalImageIterator(self, dimensionIndex=0, startIndex=0, endIndex=self.getNumData())
            
            clss.__iter__ = __iter__
            
        if hasattr(clss, 'readRow'):

            def __iter__(self):
                return FitsDalTableIterator(self)
            
            clss.__iter__ = __iter__
    
initFitsDataModel()



################################################################################
#
# Methods for creating FITS files.
#
################################################################################

## #############################################################################
#
#  @ingroup FitsDataModel
#  @author Reiner Rohlfs UGE
#
#  @brief  This function is used to create FITS files with either a table or an 
#          image as first extension. The optional parameter imgSize can be used 
#          to provide a size for an image extension.
#
#  The filename is build up, following the definition in the Data Product
#  Definition Document:\n
#  path/CH_PRppnnnn_TGttttnn_PSyymmhhdd_TUyyyy-mm-ddThh-mm-ss_DataStructureDefinition-DataName_Vnnn.fits
#  - PRppnnnn_TGttttnn (optional) is derived from the @b visitId.
#  - PSyymmhhdd (optional) is derived from the @b passId
#  - TUyyyy-mm-ddThh-mm-ss is the UTC of the first data in the file and is
#    derived from the parameter @b utc.
#  - DataStructureDefinition is the name of the data structure, created
#    by this function. If is derived by the static member function
#    HDU::getExtName()
#  - DataName (optional) is the name of the data to distinguish data for the same time
#    period in the same DataStructureDefinition.
#  - Vnnn is the version of the file. It is the next higher version of the
#    same file that is already in the CHEOPS archive. It will be set to V000
#    if there is not yet a file of the filename in the archive. The version
#    will be set to V000 if the program creating this file is
#    running outside the CHEOPS system, i.e. there is no CHEOSP archive
#    available, for example as unit_tests.
#
#    The header keywords in the created HDU, defining the visit id
#    are set by this function.
#
#  @param [in] hduClass The class of the data structure to be created. It must
#                       be one of the classes of the FITS Data Model. 
#  @param [in] path     Directory path where the FITS file will be
#                       created. It should be the path returned by the
#                       ProgramParams::getOutputDir() method.
#  @param [in] utc      The UTC of the first data in the new FITS
#                       file.
#  @param [in] visitId  The visitID as it was assigned to the data by the
#                       Mission Planning System.
#                       The program type (pp), the program number
#                       (nnnn), the target number (tttt) and the
#                       visit counter (nn) of the new filename are
#                       derived from this visitId. The visitId will not be
#                       part of the filename if @b visitId is pass as
#                       default constructor ( VisitId() ) to this
#                       function.
#  @param [in] imgSize  (optional) Defines the size of the image. imgSize[0] == size of
#                       x-axis, imgSize[1] == size of y-axis, ... The size of a specific
#                       dimension will be ignored, if the size of that dimension
#                       is already defined in the fsd-file of the data structure.
#  @param [in] passId   (optional) The passId the data belong to. Shall not be set for
#                       data to be processed by Data Reduction. The default
#                       ( PassId() ) can be used in this case.
#  @param [in] dataName (optional) An additional identifier of the data. Shall be used
#                       if several data for the same time period, i.e. the same
#                       VisitId and / or the same PassId shall be created in identical
#                       data structures. The parameter is ignored if the string
#                       is empty (the default).
#
#  @return  An object of the data type specified by @b hduClass.
#
def createFitsFile(hduClass, path, utc, visitId, imgSize=None, passId=PassId(), dataName=""):
    
    fileName = fits_data_model.buildFileName(path, utc, visitId, passId, dataName, hduClass.getExtName())

    if imgSize is not None:
        hdu = hduClass(fileName, "CREATE", imgSize)
    else:
        hdu = hduClass(fileName, "CREATE")

    hdu.setKeyVisitId(visitId)
    return hdu
