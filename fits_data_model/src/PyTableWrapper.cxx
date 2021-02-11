/** ****************************************************************************
 *  @file PyTableWrapper.cxx
 *
 *  @ingroup FitsDataModel
 *  @brief Method definitions for the PyTableWrapper class.
 *
 *  @author Anja Bekkelien UGE
 *
 *  Defines the methods used to generate Python bindings for a single FITS table
 *  C++ class.
 *
 *  @version 9.3.1 2018-01-26 ABE #16505 Add new C++ method SetReadRow() to
 *                                       Python API
 *  @version 9.0.1 2018-01-26 ABE #15340 Change method names for getting units
 *                                       and comments to getUnitOf* and getComOf*
 *  @version 9.0   2018-01-10 ABE #15295 Add support for the new C++ methods
 *                                       getKey*Unit(), getKey*Com() and
 *                                       getCell*Unit()
 *  @version 6.4.4 2016-02-03 ABE #12552 Add additional Doxygen documentation
 *                                       for fits header keywords and table columns
 *  @version 6.0   2016-06-07 ABE #11163 New getSize* method for vector columns
 *  @version 5.2   2016-06-07 ABE        Export method
 *                                       FitsDalHeader::WriteCurrentStatus()*
 *  @version 5.2   2016-05-06 ABE #10580 implement index based access to table
 *                                       rows
 *  @version 4.3.1 2015-11-30 ABE #9844  Access FITS images as numpy arrays in
 *                                       Python fits_data_model.
 *  @version 4.1                         Split python bindings into separate
 *                                       files for each fits data model class to
 *                                       increase performance.
 *  @version 1.0                         First released version.
 *
 */

#include "PyFitsDataModel.hxx"


PyTableWrapper::PyTableWrapper(std::auto_ptr<Fits_schema_type> & fsd, const ParamsPtr progParam,
                  const std::string & schemaFileName,
                  const std::string & outputCodeDir,
                  const std::string & outputDocDir)
    : PyFitsWrapper(fsd, progParam, schemaFileName, outputCodeDir, outputDocDir) {}


void PyTableWrapper::wrapperClassDeclaration() {

  fprintf(m_incFile, "class %s : public %s {\n\n", m_wrapperClassName.c_str(), ClassName().c_str());
  fprintf(m_incFile, " public:\n\n");
  fprintf(m_incFile, "  %s(const std::string & filename, const char* mode = \"READONLY\") :\n", m_wrapperClassName.c_str());
  fprintf(m_incFile, "          %s(filename, mode) {}\n\n", ClassName().c_str());

  table_type::column_iterator i_col = m_hdu.table().get().column().begin();
  table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();

  while (i_col != i_colEnd) {

    // Data type string or array
    if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
        i_col->data_type() != column_data_type::string) {

      std::string varName = VariableName(i_col->name(), CELL);
      std::string funcName = FunctionName(i_col->name(), CELL);

      fprintf(m_incFile, "  boost::python::list get%s();\n", funcName.c_str());
      fprintf(m_incFile, "  void set%s(boost::python::list & %s);\n", funcName.c_str(), varName.c_str() + 2);
    }

    ++i_col;
  }

  fprintf(m_incFile, "};\n\n");
}


/** *************************************************************************
 *  The extended classes adds methods for accessing member arrays, because they
 *  cannot be exposed directly.
 */
void PyTableWrapper::wrapperClassDefinition() {

  table_type::column_iterator i_col = m_hdu.table().get().column().begin();
  table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();

  while (i_col != i_colEnd) {
    if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
        i_col->data_type() != column_data_type::string) {

      std::string varName = VariableName(i_col->name(), CELL);
      std::string funcName = FunctionName(i_col->name(), CELL);

      fprintf(m_srcFile, "boost::python::list %s::get%s() {\n", m_wrapperClassName.c_str(), funcName.c_str());
      fprintf(m_srcFile, "  boost::python::list values;\n");
      fprintf(m_srcFile, "  for (int i = 0; i < %u; i++) {\n", (uint32_t)i_col->bin_size());
      fprintf(m_srcFile, "    values.append(%s::get%s()[i]);\n", ClassName().c_str(), funcName.c_str());
      fprintf(m_srcFile, "  }\n");
      fprintf(m_srcFile, "  return values;\n");
      fprintf(m_srcFile, "}\n\n");

      fprintf(m_srcFile, "void %s::set%s(boost::python::list & %s) {\n", m_wrapperClassName.c_str(), funcName.c_str(), varName.c_str() + 2);
      fprintf(m_srcFile, "  if (len(%s) < %u) {\n", varName.c_str() + 2, (uint32_t)i_col->bin_size());
      fprintf(m_srcFile, "    PyErr_SetString(PyExc_IndexError, \"parameter list is too short (expected %u elements).\");\n", (uint32_t)i_col->bin_size());
      fprintf(m_srcFile, "    boost::python::throw_error_already_set();\n");
      fprintf(m_srcFile, "  }\n");
      fprintf(m_srcFile, "  for (int i = 0; i < %u; i++) {\n", (uint32_t)i_col->bin_size());
      fprintf(m_srcFile, "    %s::%s[i] = boost::python::extract<%s>(%s[i]);\n", ClassName().c_str(), varName.c_str(), COLUMN_DATA_TYPE[ i_col->data_type() ], varName.c_str() + 2);
      fprintf(m_srcFile, "  }\n");
      fprintf(m_srcFile, "}\n\n");
    }

    ++i_col;
  }
}


void PyTableWrapper::constructorDoc() {

  fprintf(m_docFile, "  /** *************************************************************************\n");
  fprintf(m_docFile, "   * @fn def __init__(filename, mode) \n");
  fprintf(m_docFile, "   * @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "   * @brief Constructor, initialize the variables \n");
  fprintf(m_docFile, "   * \n");
  fprintf(m_docFile, "   * @param filename [string] file to be created / opened \n");
  fprintf(m_docFile, "   * @param mode     [string] can be READONLY, CREATE or APPEND\n");
  fprintf(m_docFile, "   */ \n\n");
}


void PyTableWrapper::exportClass() {

  fprintf(m_srcFile, "  class_<%s>(\"%s\", init<const std::string&, optional<const char*>>())\n", m_wrapperClassName.c_str(), ClassName().c_str());
//  fprintf(m_srcFile, "  class_<%s, bases<FitsDalHeader>>(\"%s\", init<const std::string&, optional<const char*>>())\n", m_wrapperClassName.c_str(), ClassName().c_str());

  getFileName();
  writeCurrentStatus();

  headerGettersSetters();
  tableGettersSetters();


  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def prepareWriteRow(row) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  @brief Prepares a row that exist already to be updated at the next call of\n");
  fprintf(m_docFile, "    *         writeRow()\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  This method has to be called in case some columns should be updated in\n");
  fprintf(m_docFile, "    *  a row that was already written. Important: This method has to be called\n");
  fprintf(m_docFile, "    *  before any value of the columns is updated, for example with the secCellX\n");
  fprintf(m_docFile, "    *  methods of a fits_data_model class. Once this method is called then the next\n");
  fprintf(m_docFile, "    *  calls of writeRow() will not add a new row to the end of the table but will\n");
  fprintf(m_docFile, "    *  update the row, defined by the parameter @b row.\n");
  fprintf(m_docFile, "    *  Following calls of writeRow() will write to the next row, independent if\n");
  fprintf(m_docFile, "    *  it exist already or not. That means this function has to be called only\n");
  fprintf(m_docFile, "    *  once with prepareWriteRow(1) to update all rows with consecutive calls of\n");
  fprintf(m_docFile, "    *  writeRow()\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @param row [int] At the next call of writeRow() the @b row will be\n");
  fprintf(m_docFile, "    *             updated instead of creating a new row. If @b row\n");
  fprintf(m_docFile, "    *             is larger than the last row a new row will be added\n");
  fprintf(m_docFile, "    *             at the next call of writeRow(). Note: first row = 1!\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile,"    .def(\"prepareWriteRow\", &%s::PrepareWriteRow)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def writeRow() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Creates a new row at the end of the table and fills it with the\n");
  fprintf(m_docFile, "    *         values that have been assigned through the setCell...() methods.\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile,"    .def(\"writeRow\", &%s::WriteRow)\n", m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def readRow(row=0) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Reads the next row from the FITS table. The cells in this row are\n");
  fprintf(m_docFile, "    *         then accessible through the getCell...() methods.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @param row [int] Reads data of row number @b row. First row in the table\n");
  fprintf(m_docFile, "    *  is @b row == 1. If @b row is set to 0 (the\n");
  fprintf(m_docFile, "    *  default value) data of the next row are read.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return false if the \"read pointer\" points behind the end of the table and\n");
  fprintf(m_docFile, "    *  no further row can be read. (Or in case of an error)\n");
  fprintf(m_docFile, "    *  true  if a row could be read\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"readRow\", (bool (%s::*)())&%s::ReadRow)\n", m_wrapperClassName.c_str(), m_wrapperClassName.c_str());
  fprintf(m_srcFile, "    .def(\"readRow\", (bool (%s::*)(uint64_t))&%s::ReadRow)\n", m_wrapperClassName.c_str(), m_wrapperClassName.c_str());

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def setReadRow(row) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Sets the row of the FITS table that will be read with the next\n");
  fprintf(m_docFile, "    *         call of readRow(0), or the next time the table is iterated over \n");
  fprintf(m_docFile, "    *         (readRow() is called internally by the iterator).\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @param row [int] Row number of next row to be read by readRow(0).\n");
  fprintf(m_docFile, "    *                   First row in the table is @b row == 1.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return false if the @b row is == 0 or if the number of rows in the\n");
  fprintf(m_docFile, "    *                table is less than @b row.\n");
  fprintf(m_docFile, "    *          true if the row number @b row can be read.\n");
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile, "    .def(\"setReadRow\", (bool (%s::*)(uint64_t))&%s::SetReadRow)\n", m_wrapperClassName.c_str(), m_wrapperClassName.c_str());

  fprintf(m_srcFile, "    ;\n\n");
}


void PyTableWrapper::tableGettersSetters() {

  // Getter methods
  table_type::column_iterator i_col = m_hdu.table().get().column().begin();
  table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();
  std::string functionName;
  while (i_col != i_colEnd) {
    getterMethod(i_col);
    ++i_col;
  }

  fprintf(m_srcFile,"\n");

  // Setter methods
  i_col = m_hdu.table().get().column().begin();
  while (i_col != i_colEnd) {
    setterMethod(i_col);
    ++i_col;
  }
  fprintf(m_srcFile,"\n");
}

void PyTableWrapper::columnDescription(table_type::column_iterator i_col) {

  if (i_col->comment().present()) {
    fprintf(m_docFile, " Column description: %s", i_col->comment().get().c_str());
  }
  if (i_col->unit().present()) {
    fprintf(m_docFile, " (unit %s.)", i_col->unit().get().c_str());
  }
  fprintf(m_docFile, "\n");
  fprintf(m_docFile, "    *  \n");

  fprintf(m_docFile, "    *  This column has bin size %u", (uint32_t)i_col->bin_size());
  if (i_col->null().present()) {
    fprintf(m_docFile, ", null value %s", i_col->null().get().c_str());
  }
  if (i_col->tm_offset().present()) {
    fprintf(m_docFile, ", tm offset %lld", i_col->tm_offset().get());
  }
  if (i_col->cal_curve().present()) {
    fprintf(m_docFile, ", cal curve %s", i_col->cal_curve().get().c_str());
  }
  if (i_col->min_value().present()) {
    fprintf(m_docFile, ", min value %f", i_col->min_value().get());
  }
  if (i_col->max_value().present()) {
    fprintf(m_docFile, ", max value %f", i_col->max_value().get());
  }
  fprintf(m_docFile, ".\n");
}

void PyTableWrapper::getterMethod(table_type::column_iterator i_col) {

  std::string functionName = FunctionName(i_col->name(), CELL);

  std::string dataType = "";
  if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                               i_col->data_type() != column_data_type::string) {
    dataType = "list of " + i_col->data_type();
  } else {
    dataType = i_col->data_type();
  }

  // Getter method
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def get%s() \n", functionName.c_str());
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");

  fprintf(m_docFile, "    *  @brief Returns the value of the current row of column %s (type %s).", i_col->name().c_str(), dataType.c_str());
  columnDescription(i_col);
  fprintf(m_docFile, "    *  \n");

  fprintf(m_docFile, "    *  @return [%s] the value of the current row of column %s.\n", dataType.c_str(), i_col->name().c_str());
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"get%s\", &%s::get%s", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());
  if (i_col->data_type() == column_data_type::A ||
      i_col->data_type() == column_data_type::string) {
    fprintf(m_srcFile,", return_value_policy<copy_const_reference>()");
  }
  fprintf(m_srcFile,")\n");

  // IsNull method
  if (i_col->data_type() <= 1 || i_col->data_type() >= 20 ||
      (i_col->null().present() &&
       i_col->data_type() >= 4 && i_col->data_type() <= 19)) {
    functionName = FunctionName(i_col->name(), NULL_V);

    fprintf(m_docFile, "   /** *************************************************************************\n");
    // If this column has bin size > 1 and is not a string
    if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                                 i_col->data_type() != column_data_type::string) {
      fprintf(m_docFile, "    *  @fn def is%s(index) \n", functionName.c_str());
      fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @brief Test the value of a specific element in the current row of vector column %s against its NULL value.\n", i_col->name().c_str());
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @param index [int] an index into the vector at the current row of this column\n");
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @return True if the element at the given index of the current row of vector column %s equals the NULL value.\n", i_col->name().c_str());
    } else {
      fprintf(m_docFile, "    *  @fn def is%s() \n", functionName.c_str());
      fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @brief Test the value of the current row of column %s against its NULL value.\n", i_col->name().c_str());
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @return True if the value of the current row of column %s equals the NULL value.\n", i_col->name().c_str());
    }
    fprintf(m_docFile, "    */\n\n");

    fprintf(m_srcFile,"    .def(\"is%s\", &%s::is%s)\n", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());
  }

  // vector columns that are not strings
  if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                               i_col->data_type() != column_data_type::string) {

    // getSize method that returns the bin size of the column
    functionName = FunctionName(i_col->name(), SIZE);

    fprintf(m_docFile, "   /** *************************************************************************\n");
    fprintf(m_docFile, "    *  @fn def get%s() \n", functionName.c_str());
    fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
    fprintf(m_docFile, "    *  \n");
    fprintf(m_docFile, "    *  @brief Returns the bin size of vector column %s\n", i_col->name().c_str());
    fprintf(m_docFile, "    *  \n");
    fprintf(m_docFile, "    *  @return the bin size of vector column %s\n", i_col->name().c_str());
    fprintf(m_docFile, "    */\n\n");

    fprintf(m_srcFile,"    .def(\"get%s\", &%s::get%s)\n", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());

  }

  // GetUnitOfCell* method for table columns
  functionName = FunctionName(i_col->name(), CELL);
  std::string unitValue;
  if (i_col->unit().present()) {
    unitValue = i_col->unit().get();
  }
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getUnitOf%s() \n", functionName.c_str());
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns the unit of column %s: \"%s\".\n", i_col->name().c_str(), unitValue.c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return [string] the unit of column %s: \"%s\".\n", i_col->name().c_str(), unitValue.c_str());
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"getUnitOf%s\", &%s::getUnitOf%s, boost::python::return_value_policy<boost::python::copy_const_reference>())\n", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());
}


void PyTableWrapper::setterMethod(table_type::column_iterator i_col) {

  std::string functionName = FunctionName(i_col->name(), CELL);
  std::string varName = VariableName(i_col->name(), CELL);

  std::string dataType = "";
  if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                               i_col->data_type() != column_data_type::string) {
    dataType = "list of " + i_col->data_type();
  } else {
    dataType = i_col->data_type();
  }

  // Setter method
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def set%s(%s) \n", functionName.c_str(), varName.c_str() + 2);
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Sets the value of the current row of table column %s (type %s).", i_col->name().c_str(), dataType.c_str());
  columnDescription(i_col);
  fprintf(m_docFile, "    *  \n");

  fprintf(m_docFile, "    *  @param %s [%s] the value of the current row of column %s.\n", varName.c_str() + 2, dataType.c_str(), i_col->name().c_str());
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"set%s\", &%s::set%s)\n", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());

  // setNull method
  if (i_col->data_type() <= 1 || i_col->data_type() >= 20 ||
      (i_col->null().present() &&
       i_col->data_type() >= 4 && i_col->data_type() <= 19)) {
    functionName = FunctionName(i_col->name(), NULL_V);

    fprintf(m_docFile, "   /** *************************************************************************\n");

    // the setNull methods of vector columns take an index as argument
    if (i_col->data_type() != column_data_type::A       &&
        i_col->data_type() != column_data_type::string  &&
        i_col->bin_size() > 1 ) {
      fprintf(m_docFile, "    *  @fn def set%s(index) \n", functionName.c_str());
      fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @brief Set an element in the the current row of vector column %s to the NULL value.\n", i_col->name().c_str());
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @param index [int] the index of the an element in the vector cell. This\n");
      fprintf(m_docFile, "    *                     element will be set to its NULL value\n");
    }
    else {
      fprintf(m_docFile, "    *  @fn def set%s() \n", functionName.c_str());
      fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
      fprintf(m_docFile, "    *  \n");
      fprintf(m_docFile, "    *  @brief Set the NULL value in the current row of column %s.\n", i_col->name().c_str());
    }
    fprintf(m_docFile, "    */\n\n");

    fprintf(m_srcFile,"    .def(\"set%s\", &%s::set%s)\n", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());
  }

}
