/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDataModel
 *  @brief Method definitions for the PyImageWrapper class.
 *
 *  @author Anja Bekkelien UGE
 *
 *  Defines the methods used to generate Python bindings for a single FITS image
 *  C++ class.
 *
 *  @version 6.4.3 2017-01-21 ABE #12526 Export image type float using NPY_FLOAT
 *  @version 5.2   2016-06-07 ABE        Export method
 *                                       FitsDalHeader::WriteCurrentStatus()
 *  @version 4.3.1 2015-11-30 ABE #9844  Access FITS images as numpy arrays in
 *                                       Python fits_data_model.
 *  @version 4.1                         Split python bindings into separate
 *                                       files for each fits data model class to
 *                                       increase performance.
 *  @version 1.0                         First released version
 *
 */

#include "PyFitsDataModel.hxx"


PyImageWrapper::PyImageWrapper(std::auto_ptr<Fits_schema_type> & fsd, const ParamsPtr progParam,
                  const std::string & schemaFileName,
                  const std::string & outputCodeDir,
                  const std::string & outputDocDir)
    : PyFitsWrapper(fsd, progParam, schemaFileName, outputCodeDir, outputDocDir) {}

/** ****************************************************************************
 * @ingroup FitsDataModel
 * @author  Anja Bekkelien UGE
 * @brief Returns the name of the numpy enum that represents the data type given
 *        as argument.
 *
 * @param dataType one of the valid FITS image C++ data types.
 * @return the name of the numpy enum representing the C++ data type.
 */
std::string convertImageDataType2NumpyEnum(const std::string & dataType) {

  if (dataType.compare("uint8_t") == 0) {
    return "NPY_UINT8";
  }
  if (dataType.compare("int16_t") == 0) {
    return "NPY_INT16";
  }
  if (dataType.compare("uint16_t") == 0) {
    return "NPY_UINT16";
  }
  if (dataType.compare("int32_t") == 0) {
    return "NPY_INT32";
  }
  if (dataType.compare("uint32_t") == 0) {
    return "NPY_UINT32";
  }
  if (dataType.compare("int64_t") == 0) {
    return "NPY_INT64";
  }
  if (dataType.compare("float") == 0) {
    return "NPY_FLOAT";
  }
  if (dataType.compare("double") == 0) {
    return "NPY_DOUBLE";
  }
  else {
    throw std::runtime_error("Invalid image data type [" + dataType + "]");
  }
}


void PyImageWrapper::wrapperClassDeclaration() {

  image_data_type & dataType = m_hdu.image().get().data_type();
  const char * dataTypeStr = DATA_TYPE_STR[ (int)dataType ];
  fprintf(m_incFile, "#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION\n");
  fprintf(m_incFile, "#include <numpy/noprefix.h>\n");

  fprintf(m_incFile, "class %s : public %s {\n\n", m_wrapperClassName.c_str(), ClassName().c_str());
  fprintf(m_incFile, " public:\n\n");
  fprintf(m_incFile, "  PyObject* m_imageData = nullptr;\n\n");

  fprintf(m_incFile, "  %s(const std::string & filename, const char* mode = \"READONLY\", const boost::python::list &axisSize = {}) :\n", m_wrapperClassName.c_str());
  fprintf(m_incFile, "          %s(filename, mode, \n", ClassName().c_str());
  fprintf(m_incFile, "          {boost::python::len(axisSize) > 0 ? static_cast<int>(boost::python::extract<long>(axisSize[0])) : 0L,\n");
  fprintf(m_incFile, "           boost::python::len(axisSize) > 1 ? static_cast<int>(boost::python::extract<long>(axisSize[1])) : 0L,\n");
  fprintf(m_incFile, "           boost::python::len(axisSize) > 2 ? static_cast<int>(boost::python::extract<long>(axisSize[2])) : 0L}\n");
  fprintf(m_incFile, "          ) { }\n\n");

  fprintf(m_incFile, "  %s GetItemDirectIndex(long index) {\n", dataTypeStr);
  fprintf(m_incFile, "    if (index < 0 || index >= m_numData) {\n");
  fprintf(m_incFile, "      PyErr_SetString(PyExc_IndexError, \"index out of range\");\n");
  fprintf(m_incFile, "      boost::python::throw_error_already_set();\n");
  fprintf(m_incFile, "    }\n");
  fprintf(m_incFile, "    return *(m_data + index);\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  void SetItemDirectIndex(long index, %s value) {\n", dataTypeStr);
  fprintf(m_incFile, "    if (index < 0 || index >= m_numData) {\n");
  fprintf(m_incFile, "      PyErr_SetString(PyExc_IndexError, \"index out of range\");\n");
  fprintf(m_incFile, "      boost::python::throw_error_already_set();\n");
  fprintf(m_incFile, "    }\n");
  fprintf(m_incFile, "    *(m_data + index) = value;\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  boost::python::list GetDimensions() {\n");
  fprintf(m_incFile, "    long dimensionSize[GetNumDim()];\n");
  fprintf(m_incFile, "    GetSize(dimensionSize);\n");
  fprintf(m_incFile, "    boost::python::list size;\n");
  fprintf(m_incFile, "    for (int i = 0; i < GetNumDim(); i++) {\n");
  fprintf(m_incFile, "      size.append(*(dimensionSize + i));\n");
  fprintf(m_incFile, "    }\n");
  fprintf(m_incFile, "    return size;\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  long GetNumData() {\n");
  fprintf(m_incFile, "    return m_numData;\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  long GetDimensionSize(long index) {\n");
  fprintf(m_incFile, "    return *(m_size + index);\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  long getIndex(boost::python::tuple indices);\n");

  fprintf(m_incFile, "  %s getItem(long index) {\n", dataTypeStr);
  fprintf(m_incFile, "    boost::python::tuple indices = boost::python::make_tuple(index);\n");
  fprintf(m_incFile, "    return getItem(indices);\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  %s getItem(boost::python::tuple & indices) {\n", dataTypeStr);
  fprintf(m_incFile, "    long index = getIndex(indices);\n");
  fprintf(m_incFile, "    return *(m_data + index);\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  %s setItem(long index, %s value) {\n", dataTypeStr, dataTypeStr);
  fprintf(m_incFile, "    boost::python::tuple indices = boost::python::make_tuple(index);\n");
  fprintf(m_incFile, "    return setItem(indices, value);\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "  %s setItem(boost::python::tuple & indices, %s value) {\n", dataTypeStr, dataTypeStr);
  fprintf(m_incFile, "    long index = getIndex(indices);\n");
  fprintf(m_incFile, "    *(m_data + index) = value;\n");
  fprintf(m_incFile, "    return value;\n");
  fprintf(m_incFile, "  }\n\n");

  // See https://mail.python.org/pipermail/cplusplus-sig/2009-January/014194.html
  // and https://mail.python.org/pipermail/cplusplus-sig/2008-October/013825.html
  // for the general approach for creating the numpy array from a C++ pointer (to array).
  //
  // See http://docs.scipy.org/doc/numpy-dev/reference/c-api.array.html
  // (function PyArray_New(...)) concerning setting the base object of the image
  // data and incrementing the references to the base objet.
  fprintf(m_incFile, "  // This method returns the image data as a numpy array.\n");
  fprintf(m_incFile, "  // The numpy array is explicitly set to know about the (python) object that\n");
  fprintf(m_incFile, "  // provides the data memory (the fits extension object), by storing a\n");
  fprintf(m_incFile, "  // pointer to it. The fits extension object is kept alive while any\n");
  fprintf(m_incFile, "  // referring arrays exist. The concept of numpy array views use this\n");
  fprintf(m_incFile, "  // approach, too, so there could be multiple referring arrays if we use\n");
  fprintf(m_incFile, "  // views of the array returned by getData\n");
  fprintf(m_incFile, "  static PyObject* getData(boost::python::object & o){\n");
  fprintf(m_incFile, "    boost::python::extract<%s&> get_self (o);\n", m_wrapperClassName.c_str());
  fprintf(m_incFile, "    if (get_self.check()) {\n");
  fprintf(m_incFile, "      %s & self = get_self();\n", m_wrapperClassName.c_str());
  fprintf(m_incFile, "      long dimensionSize[self.GetNumDim()];\n");
  fprintf(m_incFile, "      self.GetSize(dimensionSize);\n");
  fprintf(m_incFile, "      long rev_dimSize[self.GetNumDim()];\n");
  fprintf(m_incFile, "      for(long i = self.GetNumDim()-1, j = 0; i>=0; --i, j++) {\n");
  fprintf(m_incFile, "        rev_dimSize[j] = dimensionSize[i];\n");
  fprintf(m_incFile, "      }\n");
  fprintf(m_incFile, "      PyArrayObject *retval = (PyArrayObject*)PyArray_SimpleNewFromData(self.GetNumDim(), rev_dimSize, %s, self.m_data);\n", convertImageDataType2NumpyEnum(dataTypeStr).c_str());
  fprintf(m_incFile, "      PyArray_SetBaseObject(retval,o.ptr());\n");
  fprintf(m_incFile, "      Py_INCREF(o.ptr());\n");
  fprintf(m_incFile, "      return (PyObject *)retval;\n");
  fprintf(m_incFile, "    }\n");
  fprintf(m_incFile, "    else {\n");
  fprintf(m_incFile, "      Py_RETURN_NONE;\n");
  fprintf(m_incFile, "    }\n");
  fprintf(m_incFile, "  }\n\n");

  fprintf(m_incFile, "};\n\n");

}

/** *************************************************************************
 *  The extended classes adds methods for
 *  - accessing the "raw" image data array directly using indexes. Needed in order
 *  to implement Python iterators for the image.
 *  - returning the size of each image dimension as a list. Needed because we
 *  cannot return the result in a pass-by-reference method argument in Python.
 */
void PyImageWrapper::wrapperClassDefinition() {

  fprintf(m_srcFile, "long %s::getIndex(boost::python::tuple indices) {\n", m_wrapperClassName.c_str());
  fprintf(m_srcFile, "  long size = len(indices);\n");
  fprintf(m_srcFile, "  long index = 0;\n");
  fprintf(m_srcFile, "  long dimensionSize[m_numDim];\n");
  fprintf(m_srcFile, "  GetSize(dimensionSize);\n");
  fprintf(m_srcFile, "  \n");
  fprintf(m_srcFile, "  if (size != m_numDim) {\n");
  fprintf(m_srcFile, "    std::string msg = \"incorrect number of indices, got \";\n");
  fprintf(m_srcFile, "    msg.append(std::to_string(size));\n");
  fprintf(m_srcFile, "    msg.append(\" expected \");\n");
  fprintf(m_srcFile, "    msg.append(std::to_string(m_numDim));\n");
  fprintf(m_srcFile, "    PyErr_SetString(PyExc_IndexError, msg.c_str());\n");
  fprintf(m_srcFile, "    boost::python::throw_error_already_set();\n");
  fprintf(m_srcFile, "  }\n");
  fprintf(m_srcFile, "  \n");
  fprintf(m_srcFile, "  long inputIndex;\n");
  fprintf(m_srcFile, "  long maxIndex;\n");
  fprintf(m_srcFile, "  for (int i = 0; i < size; i++) {\n");
  fprintf(m_srcFile, "    inputIndex = extract<long>(indices[i]);\n");
  fprintf(m_srcFile, "    maxIndex = *(dimensionSize + (size - i - 1));\n");
  fprintf(m_srcFile, "    if (inputIndex >= maxIndex) {\n");
  fprintf(m_srcFile, "      PyErr_SetString(PyExc_IndexError, \"index out of range\");\n");
  fprintf(m_srcFile, "      boost::python::throw_error_already_set();\n");
  fprintf(m_srcFile, "    }\n");
  fprintf(m_srcFile, "    if (inputIndex < 0) {\n");
  fprintf(m_srcFile, "      if (inputIndex >= -maxIndex) {\n");
  fprintf(m_srcFile, "        inputIndex += maxIndex;\n");
  fprintf(m_srcFile, "      } else {\n");
  fprintf(m_srcFile, "        PyErr_SetString(PyExc_IndexError, \"index out of range\");\n");
  fprintf(m_srcFile, "        boost::python::throw_error_already_set();\n");
  fprintf(m_srcFile, "      }\n");
  fprintf(m_srcFile, "    }\n");
  fprintf(m_srcFile, "    index += *(m_size + i) * inputIndex;\n");
  fprintf(m_srcFile, "  }\n");
  fprintf(m_srcFile, "  return index;\n");
  fprintf(m_srcFile, "}\n\n");

}


static void AxisSizeDocumentation(FILE * file, image_type & img) {

   int32_t userSizeIndex = 1;

   if (img.naxis() > 0  && img.axis1().present() &&
         (uint32_t)img  .axis1().get() == 0           )  {
      fprintf(file, " *                 %d. value in list defines size of AXIS1\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 1  && img.axis2().present() &&
         (uint32_t)img.axis2().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS2\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 2  && img.axis3().present() &&
         (uint32_t)img.axis3().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS3\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 3  && img.axis4().present() &&
         (uint32_t)img.axis4().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS4\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 4  && img.axis5().present() &&
         (uint32_t)img.axis5().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS5\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 5  && img.axis6().present() &&
         (uint32_t)img.axis6().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS6\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 6  && img.axis7().present() &&
         (uint32_t)img.axis7().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS7\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 7  && img.axis8().present() &&
         (uint32_t)img.axis8().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS8\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (img.naxis() > 8  && img.axis9().present() &&
         (uint32_t)img.axis9().get() == 0           )  {
      fprintf(file, "   *                 %d. value in list defines size of AXIS9\n", userSizeIndex);
      ++userSizeIndex;
   }

   if (userSizeIndex == 1) {
      fprintf(file, "   *                 As the size of all axes are defined in the fsd file\n");
      fprintf(file, "   *                 for this data structure, this parameter is not used\n");
      fprintf(file, "   *                 for this specific class and can be ignored.\n");
   }

}


void PyImageWrapper::constructorDoc() {

  image_type & img = m_hdu.image().get();

  fprintf(m_docFile, "  /** *************************************************************************\n");
  fprintf(m_docFile, "   * @fn def __init__(filename, mode, axisSize) \n");
  fprintf(m_docFile, "   * @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "   * @brief Constructor, initialize the variables \n");
  fprintf(m_docFile, "   * \n");
  fprintf(m_docFile, "   * @param filename [string] file of be created / opened \n");
  fprintf(m_docFile, "   * @param mode     [string] can be READONLY, CREATE or APPEND\n");
  fprintf(m_docFile, "   * @param axisSize [list]   list of values, defining the size of axes, for\n");
  fprintf(m_docFile, "   *                 which the size is not defined in the fsd file.\n");
  fprintf(m_docFile, "   *                 For example: [200, 200] or [sizeA, sizeB].\n");
  fprintf(m_docFile, "   *                 Parameter is used only if @b mode is either\n");
  fprintf(m_docFile, "   *                 CREATE or APPEND.\n");
  AxisSizeDocumentation(m_docFile, img);
  fprintf(m_docFile, "   */ \n\n");
}


void PyImageWrapper::exportClass() {

  image_data_type & dataType = m_hdu.image().get().data_type();
  const char * dataTypeStr = DATA_TYPE_STR[ (int)dataType ];

  fprintf(m_srcFile, "  %s (%s::*%s_getItem_index)   (long)                      = &%s::getItem;\n", dataTypeStr, m_wrapperClassName.c_str(), ClassName().c_str(), m_wrapperClassName.c_str());
  fprintf(m_srcFile, "  %s (%s::*%s_getItem_indices) (boost::python::tuple&)     = &%s::getItem;\n", dataTypeStr, m_wrapperClassName.c_str(), ClassName().c_str(), m_wrapperClassName.c_str());
  fprintf(m_srcFile, "  %s (%s::*%s_setItem_index)   (long, %s)                  = &%s::setItem;\n", dataTypeStr, m_wrapperClassName.c_str(), ClassName().c_str(), dataTypeStr, m_wrapperClassName.c_str());
  fprintf(m_srcFile, "  %s (%s::*%s_setItem_indices) (boost::python::tuple&, %s) = &%s::setItem;\n", dataTypeStr, m_wrapperClassName.c_str(), ClassName().c_str(), dataTypeStr, m_wrapperClassName.c_str());
  fprintf(m_srcFile, "\n\n");

//  fprintf(m_srcFile, "  class_<%s, bases<FitsDalHeader>>(\"%s\", init<const std::string&, optional<const char*, const list&>>())\n", m_wrapperClassName.c_str(), ClassName().c_str());
  fprintf(m_srcFile, "  class_<%s>(\"%s\", init<const std::string&, optional<const char*, const list&>>())\n", m_wrapperClassName.c_str(), ClassName().c_str());

  getFileName();
  writeCurrentStatus();
  headerGettersSetters();

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def __getitem__(index) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Return the value of a pixel.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  The method expects numpy-style indices. For a 2D image use `image[1,2]`, for\n");
  fprintf(m_docFile, "    *  a 3D image use `image[1,2,3]` etc.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @param index [tuple] numpy-style indices into the image\n");
  fprintf(m_docFile, "    *  @return [%s] the value of a pixel.\n", dataType.c_str());
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"__getitem__\", %s_getItem_index)\n", ClassName().c_str());
  fprintf(m_srcFile, "    .def(\"__getitem__\", %s_getItem_indices)\n", ClassName().c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def __setitem__(index, value) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Sets the value of a pixel.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  The method expects numpy-style indices. For a 2D image use `image[1,2] = 1`, for\n");
  fprintf(m_docFile, "    *  a 3D image use `image[1,2,3]` = 1 etc.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @param index [integer] an index into the image\n");
  fprintf(m_docFile, "    *  @param value [%s] the new value of the pixel.\n", dataType.c_str());
  fprintf(m_docFile, "    *  @return [%s] the new value of the pixel.\n", dataType.c_str());
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"__setitem__\", %s_setItem_index)\n", ClassName().c_str());
  fprintf(m_srcFile, "    .def(\"__setitem__\", %s_setItem_indices)\n", ClassName().c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getItem(index) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Return the value of a pixel.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @param index [integer] an index into the 1 dimensional array containing the image ");
  fprintf(m_docFile, "    *  data, pointing to the pixel to return.\n");
  fprintf(m_docFile, "    *  @return [%s] the value of a pixel.\n", dataType.c_str());
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"getItem\", &%s::GetItemDirectIndex)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def setItem(index, value) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Sets the value of a pixel.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @param index [integer] an index into the 1 dimensional array containing the image ");
  fprintf(m_docFile, "    *  data, pointing to the pixel to update.\n");
  fprintf(m_docFile, "    *  @param value [%s] the new value of the pixel.\n", dataType.c_str());
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"setItem\", &%s::SetItemDirectIndex)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getSize() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns a list containing the length of each axis in pixels.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  The size of axis X is stored in index 0 of the returned vector, the size\n");
  fprintf(m_docFile, "    *  of axis Y is stored in index 1 of the returned vector, ...\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @return a list containing the length of each axis in pixels.\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"getSize\", &%s::GetDimensions)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getNumDim() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns the number of dimensions of the image.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @return the number of dimensions of the image.\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"getNumDim\", &%s::GetNumDim)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getNumData() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Return the total number of pixels in the image.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @return the total number of pixels in the image.\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"getNumData\", &%s::GetNumData)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getDimensionSize(index) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Return the number of pixels in each element in the dimension given as argument\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  Note that index=0 will return the size of the elements on the outer-most axis,\n");
  fprintf(m_docFile, "    *  e.g. the Z axis of an image cube, index=1 will return the size of the second \n");
  fprintf(m_docFile, "    *  outer-most axis (the Y axis of the image cube) and so on.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @param index the index of an image dimension\n");
  fprintf(m_docFile, "    *  @return [int] the total number of pixels of an element in the dimension\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"getDimensionSize\", &%s::GetDimensionSize)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def isNull(value) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Test if the @b value is the NULL value of the image\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  Note: The NULL value of an image has to be defined in the Fits\n");
  fprintf(m_docFile, "    *  Schema Document (fsd) file. It cannot be set by the application\n");
  fprintf(m_docFile, "    *  program.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @param value a value to compare to the image's NULL value.\n");
  fprintf(m_docFile, "    *  @return true if the NULL value is defined and @b value is the NULL\n");
  fprintf(m_docFile, "    *          value of the image.\n\n");
  fprintf(m_docFile, "    *          false if NULL value is not defined or if @b value is not the\n");
  fprintf(m_docFile, "    *          NULL value of the image.\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"isNull\", &%s::IsNull)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getNull() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns the NULL value of the image.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  The return.first is only valid if return.second == true.\n");
  fprintf(m_docFile, "    *  Note: The NULL value of an image has to be defined in the Fits\n");
  fprintf(m_docFile, "    *  Schema Document (fsd) file. It cannot be set by the application\n");
  fprintf(m_docFile, "    *  program. return.second is always true for float and double images and\n");
  fprintf(m_docFile, "    *  is true for integer images if the NULL value is defined in the fsd file.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  @return a tuple containing (NULL_Value, NULL_Defined)\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"getNull\", &%s::GetNull)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getData() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns a numpy array containing the image data.\n");
  fprintf(m_docFile, "    *\n");
  fprintf(m_docFile, "    *  The image data has the slowest changing axis first and the fastest\n");
  fprintf(m_docFile, "    *  changing axis last. For example, for an image cube, the first axis \n");
  fprintf(m_docFile, "    *  corresponds to the index of a 2D image in the cube, and the second and \n");
  fprintf(m_docFile, "    *  third axis corresponds to the Y and the X axis respectively of the 2D image.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  The returned numpy array provides direct access to the memory holding the\n");
  fprintf(m_docFile, "    *  image data and is not a copy. As long as any numpy arrays referencing this\n");
  fprintf(m_docFile, "    *  memory exist, the fits image object to which the image data belongs is\n");
  fprintf(m_docFile, "    *  kept alive.\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"getData\", &%s::getData)\n", m_wrapperClassName.c_str());

  fprintf(m_srcFile, "    ;\n\n");
}

void PyImageWrapper::exportClassToPythonFunction() {

  fprintf(m_srcFile, "// Needed to use Numpy routines\n");
  fprintf(m_srcFile, "// Note -- import_array() is a macro that behaves differently in Python2.x\n");
  fprintf(m_srcFile, "// vs. Python 3. See the discussion at:\n");
  fprintf(m_srcFile, "// https://groups.google.com/d/topic/astropy-dev/6_AesAsCauM/discussion\n");

  fprintf(m_srcFile, "#if (PY_VERSION_HEX < 0x03000000)\n");
  fprintf(m_srcFile, "void %sinit_numpy(void) {\n", ClassName().c_str());
  fprintf(m_srcFile, "  import_array();\n");
  fprintf(m_srcFile, "}\n");
  fprintf(m_srcFile, "#else\n");
  fprintf(m_srcFile, "int* %sinit_numpy(void) {\n", ClassName().c_str());
  fprintf(m_srcFile, "  import_array();\n");
  fprintf(m_srcFile, "  return NULL;\n");
  fprintf(m_srcFile, "}\n");
  fprintf(m_srcFile, "#endif\n\n");

  classDoc();
  constructorDoc();

  fprintf(m_srcFile, "void export%s() {\n\n", ClassName().c_str());

  fprintf(m_srcFile, "%sinit_numpy();\n\n", ClassName().c_str());

  exportClass();

  fprintf(m_srcFile, "}\n");
}
