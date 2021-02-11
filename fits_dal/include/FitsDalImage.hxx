/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief Declaration of the FitsDalImage class and its iterator class
 *         FitsDalImageIterator
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 9.3   2018-06-07 RRO #15574 new method: ResizeThirdDimension()
 *  @version 9.1.3 2018-04-10 ABE #15579 FitsDalImage::GetNull() and IsNull()
 *                                       changed to const member functions
 *  @version 7.3   2017-03-12 RRO #13092 define FitsDalImageIterator::iterator
 *                                       set T & operator *() to a const method
 *  @version 5.2   2016-06-05 RRO        new Method: Flush()
 *  @version 5.0   2016-02-04 GKO #9408: more methods in FitsDalImageIterator
 *  @version 4.3   2015-10-21 RRO better doxygen documentation for the operator[].
 *  @version 3.2   2015-04-08 RRO #7721: define the size of the axis by a
 *                                       std::vector<long>
 *  @version 3.0   2014-12-22 RRO #7054: support NULL value of image
 *  @version 1.1   2014-05-13 RRO #5263: new FitsDalImage - constructor
 *  @version 1.0 first released version
 *
 */

#ifndef _FITS_DAL_IMAGE_HXX_
#define _FITS_DAL_IMAGE_HXX_

#include <vector>
#include <iterator>

#include "FitsDalHeader.hxx"

/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief The iterator class of the FitsDalImage class. It can iterate over
 *         all pixels on one axis.
 *
 *  @note This class has pointer-like const behavior,
 *  i.e. a const instance may point to non-const values.
 *
 */
template<typename T>
class FitsDalImageIterator: public std::iterator<
        std::random_access_iterator_tag, T, long, T*, T&>
    {
    T * m_data;       ///< pointer to data of reduced image
    const long * m_size;    ///< size of image cube of the image with 1 less dimension

public:
    // Names are not automatically looked up in the present _template_ code below in a base _template_ class
    typedef long difference_type;
    typedef FitsDalImageIterator<T> iterator;

    /** *************************************************************************
     *  @brief constructor
     *
     *  @param [in] data  pointing to the first pixel of the axis this
     *                    iterator belongs to.
     *  @param [in] size  defines the size of the reduced image
     */
    FitsDalImageIterator(T * data, const long * size) :
            m_data(data), m_size(size)
        {
        }

    /** *************************************************************************
     *  @brief indirection operator, returns the pixel value as reference
     */
    T & operator *() const
        {
        return *m_data;
        }

    /** *************************************************************************
     *  @brief  "pointer to member" operator, returns the value of the pixel
     */
    T* operator ->() const
        {
        return m_data; // operator-> unintuitively should return a pointer
        }

    /** *************************************************************************
     *  @brief increments the iterator, i.e. it points now to the next pixel in
     *  its axis
     */
    FitsDalImageIterator & operator ++()
        {
        m_data += *m_size;
        return *this;
        }

    /** *************************************************************************
     *  @brief increments the iterator, i.e. it points now to the next pixel in
     *  its axis
     */
    FitsDalImageIterator operator ++(int)
        {
        FitsDalImageIterator result = *this;
        m_data += *m_size;
        return result;
        }

    /** *************************************************************************
     *  @brief decrements the iterator, i.e. it points now to the next pixel in
     *  its axis
     */
    FitsDalImageIterator & operator --()
        {
        m_data -= *m_size;
        return *this;
        }

    /** *************************************************************************
     *  @brief decrements the iterator, i.e. it points now to the next pixel in
     *  its axis
     */
    FitsDalImageIterator operator --(int)
        {
        FitsDalImageIterator result = *this;
        m_data -= *m_size;
        return result;
        }

    /** *************************************************************************
     *   @brief gives new iterator that points now by N pixel ahead in
     *  the same axis
     */
    FitsDalImageIterator operator +(difference_type const N) const
        {
        return FitsDalImageIterator(m_data + (*m_size) * N, m_size);
        }

    /** *************************************************************************
     *  @brief increments the iterator by N, i.e. it points now by N pixel ahead in
     *  its axis
     */
    FitsDalImageIterator& operator +=(difference_type const N)
        {
        m_data += (*m_size) * N;
        return *this;
        }

    /** *************************************************************************
     *  @brief gives new iterator that points now by N pixel back in
     *  the same axis
     */
    FitsDalImageIterator operator -(difference_type const N) const
        {
        return FitsDalImageIterator(m_data - (*m_size) * N, m_size);
        }

    /** *************************************************************************
     *  @brief decrements the iterator by N, i.e. it points now by N pixel ahead in
     *  its axis
     */
    FitsDalImageIterator& operator -=(difference_type const N)
        {
        m_data -= (*m_size) * N;
        return *this;
        }

    /** *************************************************************************
     *   @brief gives the number of steps between two states of the iterator.
     *   m_size must be equal for the two operands. Otherwise behavior is meaningless.
     */
    difference_type operator -(const FitsDalImageIterator & rhs) const
        {
        return (m_data - rhs.m_data) / (*m_size);
        }
    /** *************************************************************************
     *  @brief equal operator
     *  @details Iterators are equal if they point to the same pixel irrespective of
     *  the iterating axis.
     *
     */
    bool operator ==(const FitsDalImageIterator & iter) const
        {
        return iter.m_data == m_data;
        }

    /** *************************************************************************
     *  @brief not equal operator
     */
    bool operator !=(const FitsDalImageIterator & iter) const
        {
        return iter.m_data != m_data;
        }

    /** *************************************************************************
     *  @brief subscript operator, returns the value of the pixel as reference
     */
    T & operator [](const difference_type N) const
        {
        return *(m_data + (*m_size) * N);
        }

    /** *************************************************************************
     *  @brief iterator, pointing to the first pixel of the next axis
     */
    FitsDalImageIterator<T> begin() const
        {
        return FitsDalImageIterator<T>(m_data, m_size + 1);
        }

    /** *************************************************************************
     *  @brief iterator, pointing behind the last pixel of the next axis
     */
    FitsDalImageIterator<T> end() const
        {
        return FitsDalImageIterator<T>(m_data + *m_size, m_size + 1);
        }

    /** *************************************************************************
     *  @brief iterator, pointing to the same pixel but iterating along the previous axis
     */
    FitsDalImageIterator<T> previousAxis() const
        {
        return FitsDalImageIterator<T>(m_data, m_size - 1);
        }

    };


/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 * @brief Represents an image with on less dimension. It is the return
 *        value of the index operator of the FitsDalImage class.
 *        Class cannot be used for the outermost dimension (m_size - 1 must be valid).
 */

template<class T>
class ReducedImage
    {
    T * m_data;  	 ///< pointer to data of reduced image
    const long * m_size;   ///< size of image cube of the image with 1 less dimension

public:
    /** *************************************************************************
     *  @brief Defines the axis iterator for the defined data type
     */
    typedef FitsDalImageIterator<T> iterator;

    /** *************************************************************************
     *  @brief For templating
     */
    typedef T value_type;

    /** *************************************************************************
     *  @brief constructor
     *
     *  @param [in] data  pointing to the first pixel of reduced image
     *  @param [in] size  defines the size of the reduced image
     */
    ReducedImage(T * data, const long * size) :
            m_data(data), m_size(size)
        {
        }

    /** *************************************************************************
     *  @brief Index operator to give access to the data of the reduced image.
     */
    ReducedImage<T> operator [](long index)
        {
        return ReducedImage<T>(m_data + index * *m_size, m_size + 1);
        }

    /** *************************************************************************
     *  @brief Index operator to give access to the data of the reduced image.
     */
    const ReducedImage<T> operator [](long index) const
        {
        return ReducedImage<T>(m_data + index * *m_size, m_size + 1);
        }

    /** *************************************************************************
     *  @brief cast operator to provide finally the data after the last indexing.
     */
    operator T & () const
        {
        return *m_data;
        }


    /** *************************************************************************
     *  @brief assign operator to set the value to one pixel
     */
    T operator =(T val)
        {
        *m_data = val;
        return val;
        }

    /** *************************************************************************
     *  @brief Returns an axis iterator, pointing to the first pixel of the axis.
     */
    iterator begin() const
        {
        return iterator(m_data, m_size);
        }

    /** *************************************************************************
     *  @brief Returns an axis iterator, pointing to the pixel after the last
     *         pixel of the axis. Warning: Assumes valid *(m_size-1).
     */
    iterator end() const
        {
        return iterator(m_data + *(m_size - 1), m_size);
        }
    };


/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Representing a FITS image with access methods to the data of the
 *         image.
 *
 *  Following data types of the image (T) are supported:
 *  - uint8_t
 *  - int16_t
 *  - uint16_t
 *  - int32_t
 *  - uint32_t
 *  - int64_t
 *  - float
 *  - double
 */
template<typename T>
class FitsDalImage: public FitsDalHeader
    {

protected:
    T * m_data;  	  ///< the full image
    long m_numData;  ///< number of pixels in image
    int m_numDim;   ///< number of dimensions [0..9]
    long * m_size;   ///< defining size of image

    T m_null; ///< the NULL value in the image. Only usable if m_nullDefined is true.
    bool m_nullDefined; ///< set to true if a NULL value is defined for this image.

private:
    /** *************************************************************************
     *  @brief Opens a FITS image on disk
     */
    void OpenImage(const std::string & filename);

    /** *************************************************************************
     *  @brief Creates a FITS image on disk
     */
    void CreateImage(const std::string & filename, const char * mode,
            const std::vector<long> & size);

protected:
    /** *************************************************************************
     *  @brief Writes data to disk
     */
    virtual void Flush();


public:

    /** *************************************************************************
     *  @brief Defines the axis iterator for the defined data type
     */
    typedef FitsDalImageIterator<T> iterator;

    /** *************************************************************************
     *  @brief For templating
     */
    typedef T value_type;

    /** *************************************************************************
     *  @brief Opens / creates a FITS image on disk
     */
    FitsDalImage(const std::string & fileName, const char *mode = "READONLY",
            const std::vector<long> & size = {});

    /** *************************************************************************
     *  @brief Writes the image data to disk and closes the image
     */
    ~FitsDalImage();

    /** **************************************************************************
     * @brief Returns the class name as a string.
     *
     * @return class name
     */
    static std::string getClassName()
        {
        return "FitsDalImage";
        }

    /** *************************************************************************
     *  @brief Returns the number of dimensions of the image.
     */
    int GetNumDim() const
        {
        return m_numDim;
        }

    /** *************************************************************************
     *  Provides the length of each axis in pixels. The size of axis X is
     *  stored in size[0] of @b size, the size of axis Y is stored
     *  in size[1] of @b size, ...
     */
    void GetSize(long * size) const;

    /** *************************************************************************
     *  Provides the length of each axis in pixels. The size of axis X is
     *  stored in index 0 of the returned vector, the size of axis Y is stored
     *  in the index 1 of the returned vector, ...
     */
    std::vector<long> GetSize() const;

    /** *************************************************************************
     *  @brief re-sizes the third dimension of the image to @b size
     *
     *  Any iterator on the image will be violated by this method!
     *  Only images opened in CREATE or in APPEND mode can be updated.
     *  This method should be used carefully. It is very processing time
     *  intensive as the FITS file is re-structured each time. All extension
     *  following this image will be shifted inside the file.
     *
     *  @param [in] size  new size of the third dimension.
     *
     *  @throw  runtime_error
     *  - if he image has not exactly 3 dimensions
     *  - if the image was opened in READONLY mode.
     *  - if the @b size is < 0
     *  - if there is not enough memory to copy the data from the previous
     *    data buffer to the re-sized data buffer
     *  - if the FITS file cannot be re-sized, i.e. if cfitsio returns an error
     */
    void ResizeThirdDimension(long size);

    /** *************************************************************************
     *  @brief Returns the NULL value of the image.
     *
     *  The return.first is only valid if return.second == true.
     *  Note: The NULL value of an image has to be defined in the Fits
     *  Schema Document (fsd) file. It cannot be set by the application
     *  program. return.second is always true for float and double images and
     *  is true for integer images if the NULL value is defined in the fsd file.
     *
     *  @return a pair of <NULL_Value, NULL_Defined>
     */
    std::pair<T, bool> GetNull() const
        {
        return std::pair<T, bool>(m_null, m_nullDefined);
        }

    /** *************************************************************************
     *  @brief Test if the @b value is the NULL value of the image
     *
     *  Note: The NULL value of an image has to be defined in the Fits
     *  Schema Document (fsd) file. It cannot be set by the application
     *  program.
     *
     *  @return true if the NULL value is defined and @b value is the NULL
     *          value of the image.\n
     *          false if NULL value is not defined or if @b value is not the
     *          NULL value of the image.
     *
     */
    bool IsNull(T value) const
        {
        return m_nullDefined && (value == m_null);
        }

    /** *************************************************************************
     *  @brief Index operator to give access to the data of the image.
     */
    ReducedImage<T> operator [](long index)
        {
        return ReducedImage<T>(m_data + index * *m_size, m_size + 1);
        }

    /** *************************************************************************
     *  @brief Index operator to give access to the data of the image.
     */
    const ReducedImage<T> operator [](long index) const
        {
        return ReducedImage<T>(m_data + index * *m_size, m_size + 1);
        }

    /** *************************************************************************
     *  @brief Returns an axis iterator, pointing to the first pixel of the axis.
     */
    iterator begin() const
        {
        return iterator(m_data, m_size);
        }

    /** *************************************************************************
     *  @brief Returns an axis iterator, pointing to the pixel after the last
     *         pixel of the axis.
     */
    iterator end() const
        {
        return iterator(m_data + m_numData, m_size);
        }

    };

/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *   @brief Determines if two images have the same size.
 *
 *   Two images have the same size if they have the same number of dimensions
 *   and if the size of each axis is identical.
 *
 *   @param [in] img1  first image to be compared.
 *   @param [in] img2  second image to be compared.
 *
 *   @return true if both input images have same size\n
 *           false if the images have either different number of dimensions
 *                 or if at least the size of one axis is different.
 */
template<typename T1, typename T2>
inline bool SameImageSize(const FitsDalImage<T1> & img1,
        const FitsDalImage<T2> & img2)
    {
    if (img1.GetNumDim() != img2.GetNumDim())
        return false;

    long size1[9], size2[9];
    img1.GetSize(size1);
    img2.GetSize(size2);

    for (int dim = 0; dim < img1.GetNumDim(); dim++)
        if (size1[dim] != size2[dim])
            return false;

    return true;
    }

#endif /* _FITS_DAL_IMAGE_HXX_ */
