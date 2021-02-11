/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief  Implementatino of the FitsDalImage class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 9.3   2018-06-07 RRO #15574 new method: ResizeThirdDimension()
 *  @version 5.2   2016-06-05 RRO        new Method: Flush()
 *  @version 3.2   2015-04-08 RRO #7721: define the size of the axis by a
 *                                       std::vector<long>
 *  @version 3.0   2014-12-22 RRO #7054: support NULL value of image
 *                 2015-01-05 RRO #7056: better error message if a new HDU
 *                                       cannot be created.
 *  @version 1.3   2014-07-24 RRO #6076: Call FitsDalHeader::VerifyChecksum()
 *  @version 1.2   2014-05-13 RRO #5263 new FitsDalImage - constructor
 *  @version 1.1   2014-05-14 RRO use required types for integer, for
 *                                example int64_t
 *  @version 1.0   2014-03-14 RRO first version
 *
 */
#include <string>
#include <cstring>
#include <limits>
#include <typeinfo>
#include <string.h>
#include <sys/stat.h>

#include "FitsDalImage.hxx"

using namespace std;

/** ****************************************************************************
 *  @brief Returns the cfitsio type of the pixel data type
 */
template <typename T> int GetFitsImgType()
{  throw runtime_error(string("Requesting a not supported data type in GetFitsDataType :") +
			            typeid(T).name() ); }
template<> int GetFitsImgType<uint8_t>()   {return BYTE_IMG;}      ///< Returns the cfitsio type of a uint8_t pixel
template<> int GetFitsImgType<int16_t>()   {return SHORT_IMG;}     ///< Returns the cfitsio type of a int16_t pixel
template<> int GetFitsImgType<uint16_t>()  {return USHORT_IMG;}    ///< Returns the cfitsio type of a uint16_t pixel
template<> int GetFitsImgType<int32_t>()   {return LONG_IMG;}      ///< Returns the cfitsio type of a int32_t pixel
template<> int GetFitsImgType<uint32_t>()  {return ULONG_IMG;}     ///< Returns the cfitsio type of a uint32_t pixel
template<> int GetFitsImgType<int64_t>()   {return LONGLONG_IMG;}  ///< Returns the cfitsio type of a int64_t pixel
template<> int GetFitsImgType<float>()     {return FLOAT_IMG;}     ///< Returns the cfitsio type of a float pixel
template<> int GetFitsImgType<double>()    {return DOUBLE_IMG;}    ///< Returns the cfitsio type of a double pixel

/** ****************************************************************************
 *  @brief Returns the cfitsio data type for the implemented image data types
 */
template <typename T> int GetFitsDataType()
{  throw runtime_error(string("Requesting a not supported data type in GetFitsDataType :") +
			            typeid(T).name() ); }
template<> int GetFitsDataType<uint8_t>()  {return TBYTE;}    ///< Returns the cfitsio data type for the uint8_t image data types
template<> int GetFitsDataType<int16_t>()  {return TSHORT;}   ///< Returns the cfitsio data type for the int16_t image data types
template<> int GetFitsDataType<uint16_t>() {return TUSHORT;}  ///< Returns the cfitsio data type for the uint16_t image data types
template<> int GetFitsDataType<int32_t>()  {return TINT;}     ///< Returns the cfitsio data type for the int32_t image data types
template<> int GetFitsDataType<uint32_t>() {return TUINT;}    ///< Returns the cfitsio data type for the uint32_t image data types
template<> int GetFitsDataType<int64_t>()  {return TLONG;}    ///< Returns the cfitsio data type for the int64_t image data types
template<> int GetFitsDataType<float>()    {return TFLOAT;}   ///< Returns the cfitsio data type for the float image data types
template<> int GetFitsDataType<double>()   {return TDOUBLE;}  ///< Returns the cfitsio data type for the double image data types



/** ****************************************************************************
 *  Depending on the mode it creates a
 *  new image or opens an existing FITS image.
 *  @param [in] filename  filename of the FITS file with the new or existing
 *                        image extension. If @b mode is READONLY it can
 *                        include the specification of the FITS extension to
 *                        be opened.
 *  @param [in] mode      READONLY: opens an existing FITS image extension\n
 *                        CREATE:   creates a new FITS file with a new
 *                                  image extension. Throws an exception if the
 *                                  file exist already.\n
 *                        APPEND    it is identical as CREATE if the file does
 *                                  not yet exist. A new extension is appended
 *                                  in the file if it exist already.
 *  @param [in] size      The size of each dimension. For example {2, 3, 4}
 *                        will create a 3 dimensional image with a
 *                        X-Axis - size of 2 pixels, a Y-Axis size of 3 pixels
 *                        and a Z-Axis of 4 pixels.
 */
template <typename T>
FitsDalImage<T>::FitsDalImage(const std::string & filename, const char * mode,
      const std::vector<long> & size)
{
   m_data = nullptr;
   m_size = nullptr;
   m_nullDefined = false;

   if (strcmp(mode, "READONLY") == 0) {
      OpenImage(filename);
   }
   else if (strcmp(mode, "CREATE") == 0 || strcmp(mode, "APPEND") == 0) {

      CreateImage(filename, mode, size);
   }
   else
      throw runtime_error("To open / create the file " + filename + " a not supported mode was defined: " +
                        string(mode) + ". Supported modes are READONLY, CREATE and APPEND" );

}

/** ****************************************************************************
 *  Creates a new image extension either in a new FITS file or in an already
 *  existing FITS file.
 *  @param [in] filename  filename of the FITS file with the new or existing
 *                        image extension.
 *  @param [in] mode      CREATE:   creates a new FITS file with a new
 *                                  image extension. Throws an exception if the
 *                                  file exist already.\n
 *                        APPEND    it is identical as CREATE if the file does
 *                                  not yet exist. A new extension is appended
 *                                  in the file if it exist already.
 *  @param [in] size      The size of each dimension. The size of the vector
 *                        defines the number of dimensions of the image.
 */
template <typename T>
void FitsDalImage<T>::CreateImage(const std::string & filename, const char * mode,
                                  const std::vector<long> & size )
{
	m_numDim = size.size();

	m_update = true;

	// create the image
	int status = 0;
	fits_create_file(&m_fitsFile, filename.c_str(), &status);
	if (status == 105)
	{
	   // check if the file exist already
	   struct stat buf;
	   if (stat(filename.c_str(), &buf) == 0) {
	      // the files exist already
	      if (strcmp(mode, "CREATE") == 0)
	         // the file must not exist already
	         throw runtime_error("File " + filename + " exist already.");

	      // in APPEND mode we try to open it to be able to create a new extension
	      status = 0;
	      fits_open_file(&m_fitsFile, filename.c_str(), READWRITE, &status);
	      if (status != 0)
	         // file exist, but we cannot open it
	         throw runtime_error("File " + filename +
	                             " cannot be opened to append a new HDU,"
	                             " cfitsio error: " + to_string(status));
	   }
	   else
	      throw runtime_error("Cannot create file " + filename +
	                          ". Does its directory exist already?"
	                          " cfitsio error: 105");
	}
	else
	{
		// it is a new file, create the primary array, which we will not use
		long   axis[1] = {0};
		fits_create_img(m_fitsFile, SHORT_IMG, 0, axis, &status);
	}

	// copy the values for the vector size to the sizeArray
   long sizeArray[size.size()];
   int16_t index = 0;
   for (long sz : size) {
      sizeArray[index++] = sz;
   }

	fits_create_img( m_fitsFile, GetFitsImgType<T>(), size.size(), sizeArray ,
	                 &status);
	if (status != 0)
	   	  throw runtime_error("Failed to create " + filename +
				              ", cfitsio error: " + to_string(status));

	 if (m_numDim > 0)
	   {
	   m_size = new long[m_numDim];
	   m_size[m_numDim-1] = 1;
	   for (int dim = m_numDim - 2; dim >= 0; dim--)
		   m_size[dim] = m_size[dim+1] * size[m_numDim - 2 - dim];
	   m_numData = m_size[0] * size[m_numDim-1];

	   m_data = new T[m_numData]();
	   }
}

/** ****************************************************************************
 *  Opens a FITS image in a FITS file.
 *  @param [in] filename  filename of the FITS file with existing
 *                        image extension. It can include the specification of
 *                        the FITS extension to be opened. The first image with
 *                        NAXIS greater than 0 will be opened if the extension
 *                        is not specified in the filename.
 */
template <typename T>
void FitsDalImage<T>::OpenImage(const string & filename)
{

   m_update = false;

   // open the image
   int status = 0;
   fits_open_image(&m_fitsFile, filename.c_str(), READONLY, &status);
   if (status != 0)
   	  throw runtime_error("Failed to open image " + filename +
			              ", cfitsio error: " + to_string(status));

   VerifyChecksum();

   // get image size
   m_numDim = 0;
   fits_get_img_dim(m_fitsFile, &m_numDim, &status);
   if (m_numDim > 0)
   {
	   long axisLength[m_numDim]; // length of each axis
	   m_size = new long[m_numDim];
	   fits_get_img_size(m_fitsFile, m_numDim, axisLength, &status);
	   if (status != 0)
			  throw runtime_error("Failed read the size of the image in " + filename +
							  ", cfitsio error: " + to_string(status));

	   // get the data
	   m_size[m_numDim-1] = 1;
	   for (int dim = m_numDim - 2; dim >= 0; dim--)
		   m_size[dim] = m_size[dim+1] * axisLength[m_numDim - 2 - dim];
	   m_numData = m_size[0] * axisLength[m_numDim-1];

	   m_data = new T[m_numData];

		int anyNull;
		long firstPixel[m_numDim];
		for (int dim = 0; dim < m_numDim; dim++)
			firstPixel[dim] = 1;

		fits_read_pix(m_fitsFile, GetFitsDataType<T>(), firstPixel, m_numData, NULL,
					  m_data, &anyNull, &status);
		if (status != 0)
			throw runtime_error("Failed read the image in " + filename +
								  ", cfitsio error: " + to_string(status));

   }

   ReadAllKeywords();

   // set the NULL value
   if ( GetFitsDataType<T>() == TFLOAT || GetFitsDataType<T>() ==  TDOUBLE) {
      m_nullDefined = true;
      m_null = std::numeric_limits<T>::quiet_NaN();
   }
   else {
      try {
         if (GetFitsDataType<T>() == TUSHORT) {
             // unsigned short
             int16_t buffer = GetAttr<int16_t>("BLANK");
             buffer ^= 0x8000;
             memcpy(&m_null, &buffer, 2);
          }
          else if (GetFitsDataType<T>() == TUINT) {
             // unsigned int
             int32_t buffer = GetAttr<int32_t>("BLANK");
             buffer ^= 0x80000000;
             memcpy(&m_null, &buffer, 4);
         }
         else {
            m_null = GetAttr<T>("BLANK");
         }
         m_nullDefined = true;
      }
      // we accept if the BLANK keyword is not defined.
      // m_nullDefined will stay as false.
      catch(runtime_error) {}
   }

}

/** ****************************************************************************
 *
 */
template <typename T>
FitsDalImage<T>::~FitsDalImage()
{

   Flush();

	delete [] m_data;
	delete [] m_size;
}

/** *************************************************************************
 */
template <typename T>
void FitsDalImage<T>::Flush() {

   if (m_update && m_fitsFile)
   {
      int status = 0;

      // write the NULL value into the BLANK keyword
      if (m_nullDefined &&
         GetFitsDataType<T>() != TFLOAT &&  GetFitsDataType<T>() !=  TDOUBLE) {
         if (GetFitsDataType<T>() == TUSHORT) {
            // unsigned short
            int16_t buffer;
            memcpy(&buffer, &m_null, 2);
            buffer ^= 0x8000;
            SetAttr("BLANK", buffer, "NULL value of image" );
         }
         else if (GetFitsDataType<T>() == TUINT) {
            // unsigned int
            int32_t buffer;
            memcpy(&buffer, &m_null, 4);
            buffer ^= 0x80000000;
            SetAttr("BLANK", buffer, "NULL value of image" );
            }
         else
            SetAttr("BLANK", m_null, "NULL value of image");
      }

      long firstPixel[m_numDim];
         for (int dim = 0; dim < m_numDim; dim++)
            firstPixel[dim] = 1;

      fits_write_pix (m_fitsFile, GetFitsDataType<T>(), firstPixel, m_numData,
            m_data, &status);

      if (status != 0)
         throw runtime_error("Failed update the image in " + GetFileName() +
                          ", cfitsio error: " + to_string(status));
   }
}

/** ****************************************************************************
 *  @param [out] size  the size of the array has to be at least as
 *                     long as the number of dimensions of the image.
 */
template <typename T>
void FitsDalImage<T>::GetSize(long * size) const
{
   if (m_numDim == 0)
      return;

   for (int dim = 0; dim < m_numDim -1; dim++)
      size[dim] = m_size[m_numDim - dim - 2] / m_size[m_numDim - dim -1];
   size[m_numDim - 1] = m_numData / m_size[0];
}

/** ****************************************************************************
 */
template <typename T>
std::vector<long> FitsDalImage<T>::GetSize() const
{
   std::vector<long> size;
   if (m_numDim == 0)
      return size;

   for (int dim = 0; dim < m_numDim -1; dim++)
      size.push_back(m_size[m_numDim - dim - 2] / m_size[m_numDim - dim -1]);
   size.push_back( m_numData / m_size[0]);

   return size;
}

/** ****************************************************************************
 */
template <typename T>
void FitsDalImage<T>:: ResizeThirdDimension(long size) {

   if (m_numDim != 3)
      throw runtime_error("Image in " + GetFileName() +
                          "must have exactly 3 dimensions to resize the "
                          "3rd dimension.");

   if (m_update  == false)
      throw runtime_error("Image in " + GetFileName() +
                          "is open in READONLY mode. The size cannot be updated.");

   if (size < 0)
      throw runtime_error("3rd dimension of image in " + GetFileName() +
                          "cannot be set to " + to_string(size));

   // get current size of all 3 dimensions
   long imageSize[m_numDim];
   GetSize(imageSize);
   imageSize[m_numDim - 1] = size;

   // calculate the new m_size and the new total number of pixels
   long newNumData;

   m_size[m_numDim-1] = 1;
   for (int dim = m_numDim - 2; dim >= 0; dim--)
      m_size[dim] = m_size[dim+1] * imageSize[m_numDim - 2 - dim];
   newNumData = m_size[0] * imageSize[m_numDim-1];

   T * newData = new T[newNumData]();

   memcpy(newData, m_data,
           sizeof(T) * (newNumData < m_numData ? newNumData : m_numData));

   delete [] m_data;
   m_data = newData;
   m_numData = newNumData;

   // finally resize the image in the FITS file
   int status = 0;
   fits_resize_img(m_fitsFile, GetFitsImgType<T>(), m_numDim, imageSize, &status);
   if (status != 0)
      throw runtime_error("Failed to resize image in " + GetFileName()  +
                          " cfitsio error: " + to_string(status));

}


// explicit instantiation
template class FitsDalImage<uint8_t>;
template class FitsDalImage<int16_t>;
template class FitsDalImage<uint16_t>;
template class FitsDalImage<int32_t>;
template class FitsDalImage<uint32_t>;
template class FitsDalImage<int64_t>;
template class FitsDalImage<float>;
template class FitsDalImage<double>;


