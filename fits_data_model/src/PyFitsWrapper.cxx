/** ****************************************************************************
 *  @file
 *  @ingroup FitsDataModel
 *  @author  Anja Bekkelien UGE
 *
 *  @brief   Implementation of calss PyFitsWrapper.
 *
 *  @version 9.0.1 2018-01-26 ABE #15340 Change method names for getting units
 *                                       and comments to getUnitOf* and getComOf*
 *  @version 9.0   2018-01-10 ABE #15295 Add support for the new C++ methods
 *                                       getKey*Unit(), getKey*Com() and
 *                                       getCell*Unit()
 *  @version 6.4.4 2016-02-03 ABE #12552 Add additional Doxygen documentation
 *                                       for fits header keywords and table columns
 *  @version 5.2   2016-06-07 ABE        Export method
 *                                       FitsDalHeader::WriteCurrentStatus()
 *  @version 4.4   2015-12-03 ABE #9857  Export Open_* functions to Python.
 *  @version 4.3.1 2015-11-30 ABE #9844  Access FITS images as numpy arrays in
 *                                       Python fits_data_model.
 *  @version 4.1                         Split python bindings into separate
 *                                       files for each fits data model class to
 *                                       increase performance.
 *  @version 1.0                         First released version.
 *
 */

#include <algorithm>    // std::find
#include <boost/algorithm/string/predicate.hpp> // boost::starts_with
#include <boost/filesystem.hpp>

#include "PyFitsDataModel.hxx"

const std::string PyFitsWrapper::FILE_NAME_PREFIX = "PyExt_";


PyFitsWrapper::PyFitsWrapper(std::auto_ptr<Fits_schema_type> & fsd, const ParamsPtr progParam,
                  const std::string & schemaFileName,
                  const std::string & outputCodeDir,
                  const std::string & outputDocDir)
    : OutputFile(fsd->HDU(), progParam, schemaFileName),
      m_outputCodeDir(outputCodeDir),
      m_outputDocDir(outputDocDir),
      m_fsd(fsd),
      m_wrapperClassName(ClassName() + "Wrapper") {}


PyFitsWrapper::~PyFitsWrapper() { }


std::unique_ptr<PyFitsWrapper> PyFitsWrapper::factory(std::auto_ptr<Fits_schema_type> & fsd,
                          const ParamsPtr progParam,
                          const std::string & schemaFileName,
                          const std::string & outputCodeDir,
                          const std::string & outputDocDir) {

  if (fsd->HDU().image().present()) {
    return std::unique_ptr<PyFitsWrapper>(new PyImageWrapper(fsd, progParam, schemaFileName, outputCodeDir, outputDocDir));
  }
  else if (fsd->HDU().table().present()) {
    return std::unique_ptr<PyFitsWrapper>(new PyTableWrapper(fsd, progParam, schemaFileName, outputCodeDir, outputDocDir));
  }
  throw std::runtime_error("Cannot instantiate PyImageWrapper: the HDU does not contain a table nor an image.");
}


void PyFitsWrapper::openOutputFiles() {

  std::string incFileName = m_outputCodeDir + "/include/" + FILE_NAME_PREFIX + BaseFileName() + ".hxx";
  std::string srcFileName = m_outputCodeDir + "/src/" + FILE_NAME_PREFIX + BaseFileName() + ".cxx";
  std::string docFileName = m_outputDocDir + "/" + FILE_NAME_PREFIX + BaseFileName() + ".txt";

//  logger << debug << "Opening output include file [" << incFileName << "]" << std::endl;
  m_incFile = fopen(incFileName.c_str(), "w");
  if (m_incFile == NULL) {
    throw std::runtime_error("Failed to create include file [" + incFileName + "]");
  }

//  logger << debug << "Opening output source file [" << srcFileName << "]" << std::endl;
  m_srcFile = fopen(srcFileName.c_str(), "w");
  if (m_srcFile == NULL) {
    throw std::runtime_error("Failed to create source file [" + srcFileName + "]");
  }

//  logger << debug << "Opening output doc file [" << docFileName << "]" << std::endl;
  m_docFile = fopen(docFileName.c_str(), "w");
  if (m_docFile == NULL) {
    throw std::runtime_error("Failed to create Doxygen file [" + docFileName + "]");
  }
}


void PyFitsWrapper::closeOutputFiles() {

  if (m_incFile) {
    fclose(m_incFile);
  }
  if (m_srcFile) {
    fclose(m_srcFile);
  }
  if (m_docFile) {
    fclose(m_docFile);
  }
}


void PyFitsWrapper::create() {

  openOutputFiles();
  fileDoc();
  includeFile();
  sourceFile();
  closeOutputFiles();

  logger << debug << "Generated Python interface for extension " << BaseFileName() << std::endl;
}


void PyFitsWrapper::fileDoc() {

//  fprintf(m_docFile, "/** ****************************************************************************\n");
//  fprintf(m_docFile, " *  @file\n");
//  fprintf(m_docFile, " *  @ingroup FitsDataModel\n");
//  fprintf(m_docFile, " *\n");
//  fprintf(m_docFile, " */\n\n");
}


void PyFitsWrapper::includeFile() {

  fprintf(m_incFile, "/** ****************************************************************************\n");
  fprintf(m_incFile, " *  \n");
  fprintf(m_incFile, " *  This is an auto generated file. Do not modify.\n");
  fprintf(m_incFile, " */\n\n");

  fprintf(m_incFile, "#ifndef _%s%s_HXX_\n", FILE_NAME_PREFIX.c_str(), BaseFileName().c_str());
  fprintf(m_incFile, "#define _%s%s_HXX_\n", FILE_NAME_PREFIX.c_str(), BaseFileName().c_str());
  fprintf(m_incFile, "\n");
  fprintf(m_incFile, "#include <boost/python.hpp>\n");
  fprintf(m_incFile, "\n");
  // Include the wrapped FITS class from fits_data_model
  fprintf(m_incFile, "#include \"%s.hxx\"\n\n", BaseFileName().c_str());

  // Include the classes of any assocated hdus
  if (m_fsd->List_of_Associated_HDUs().present()) {

    List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_hdu =
        m_fsd->List_of_Associated_HDUs().get().Associated_HDU().begin();
    List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_end =
        m_fsd->List_of_Associated_HDUs().get().Associated_HDU().end();

    while (i_hdu != i_end) {
      fprintf(m_incFile, "#include \"%s.hxx\"\n", std::string(FILE_NAME_PREFIX + *i_hdu).c_str());
      ++i_hdu;
    }
  }
  fprintf(m_incFile, "\n");

  wrapperClassDeclaration();
  associatedHdusWrapperFunctions();

  fprintf(m_incFile, "void export%s();\n", ClassName().c_str());

  fprintf(m_incFile, "\n");
  fprintf(m_incFile, "#endif /* _%s%s_HXX_ */\n", FILE_NAME_PREFIX.c_str(), BaseFileName().c_str());

}


void PyFitsWrapper::sourceFile() {

  fprintf(m_srcFile, "/** ****************************************************************************\n");
  fprintf(m_srcFile, " *  \n");
  fprintf(m_srcFile, " *  This is an auto generated file. Do not modify.\n");
  fprintf(m_srcFile, " */\n\n");

  fprintf(m_srcFile, "#include \"%s.hxx\"\n", std::string(FILE_NAME_PREFIX + BaseFileName()).c_str());
  fprintf(m_srcFile, "using namespace boost::python;\n\n");

  wrapperClassDefinition();
  exportClassToPythonFunction();
}


std::string PyFitsWrapper::BaseFileName() {

   // Fits schema file name without path and without extension
   return boost::filesystem::path(m_schemaFileName).stem().string();
}


void PyFitsWrapper::classDoc() {

  if (boost::starts_with(ClassName(), "RefApp") ||
      boost::starts_with(ClassName(), "SocApp")) {

    // Note: the _class definition is created in method validRefFileForHDU()
    fprintf(m_docFile, "/** ****************************************************************************\n");
    fprintf(m_docFile, " *  @class Valid%s\n", ClassName().c_str());
    fprintf(m_docFile, " *  @ingroup FitsDataModel\n");
    fprintf(m_docFile, " *  @author Anja Bekkelien, UGE\n");
    fprintf(m_docFile, " *  @brief This class is used to get the reference file of data structure type %s that is applicable for a given UTC time.\n", ClassName().c_str());
    fprintf(m_docFile, " *  \n");
    fprintf(m_docFile, " *  The class takes a list of reference file names in the constructor. The method\n");
    fprintf(m_docFile, " *  @b getFile will return, among those files, the one that is applicable for a\n");
    fprintf(m_docFile, " *  specific UTC time.\n");
    fprintf(m_docFile, " */\n\n");

    fprintf(m_docFile, "  /** *************************************************************************\n");
    fprintf(m_docFile, "   *  @fn def __init__(fileNames)\n");
    fprintf(m_docFile, "   *  @memberof Valid%s\n", ClassName().c_str());
    fprintf(m_docFile, "   *  @brief Constructor that takes a list of reference file names as input.\n");
    fprintf(m_docFile, "   *  \n");
    fprintf(m_docFile, "   *  @param fileNames A list containing names of reference files. All files must be of\n");
    fprintf(m_docFile, "   *                   the data structure type %s.\n", ClassName().c_str());
    fprintf(m_docFile, "   */\n\n");

    fprintf(m_docFile, "  /** *************************************************************************\n");
    fprintf(m_docFile, "   *  @fn def getFile(dataTime)\n");
    fprintf(m_docFile, "   *  @memberof Valid%s\n", ClassName().c_str());
    fprintf(m_docFile, "   *  @brief Returns the HDU of the reference file that is applicable for the UTC\n");
    fprintf(m_docFile, "   *         time given as method argument.\n");
    fprintf(m_docFile, "   *  \n");
    fprintf(m_docFile, "   *  @param dataTime A UTC object containing the time for which the returned reference file must be\n");
    fprintf(m_docFile, "   *                  valid.\n");
    fprintf(m_docFile, "   *  @return an object of type the %s, that is valid at the specified UTC time.\n", ClassName().c_str());
    fprintf(m_docFile, "   *  \n");
    fprintf(m_docFile, "   *  @throws runtime_error if no reference file is valid for the given UTC @b dataTime.\n");
    fprintf(m_docFile, "   */\n\n");
  }

  fprintf(m_docFile, "/** ****************************************************************************\n");
  fprintf(m_docFile, " *  @class %s\n", ClassName().c_str());
  fprintf(m_docFile, " *  @ingroup FitsDataModel\n");
  fprintf(m_docFile, " *  @author Reiner Rohlfs UGE\n");
  fprintf(m_docFile, " *\n");
  if (m_hdu.brief().present()) {
     fprintf(m_docFile, " *  @brief %s\n", m_hdu.brief().get().c_str());
  }
  if (m_hdu.description().present()) {
     fprintf(m_docFile, " *\n");
     fprintf(m_docFile, " *  %s\n", m_hdu.description().get().c_str());
  }

  fprintf(m_docFile, " */\n\n");
}


void PyFitsWrapper::headerGettersSetters() {

  // Getter method for ext name.
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getExtName() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Static method that returns the name of the FITS extension.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return [string] the name of the FITS extension.\n");
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"getExtName\", &%s::getExtName)\n", m_wrapperClassName.c_str());
  fprintf(m_srcFile,"    .staticmethod(\"getExtName\")\n");

  // Getter method for class name.
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getClassName() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns the class name of the FITS extension.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return [string] the class name of the FITS extension.\n");
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"getClassName\", &%s::getClassName)\n", m_wrapperClassName.c_str());
  fprintf(m_srcFile,"    .staticmethod(\"getClassName\")\n");

  // Getter method for all keywords
  header_type::keyword_iterator i_key = m_hdu.header().keyword().begin();
  header_type::keyword_iterator i_keyEnd = m_hdu.header().keyword().end();
  std::string functionName;
  while (i_key != i_keyEnd) {
    getterMethod(i_key);
    ++i_key;
  }

  //Getter methods for all keyword groups
  header_type::group_iterator i_group = m_hdu.header().group().begin();
  header_type::group_iterator i_groupEnd = m_hdu.header().group().end();
  while (i_group != i_groupEnd) {

    // loop over all header keywords in this group
    i_key = i_group->keyword().begin();
    i_keyEnd = i_group->keyword().end();
    while (i_key != i_keyEnd) {
      getterMethod(i_key);
      ++i_key;
    }
    if (i_group->description() == "Visit" ||
        i_group->description() == "Pass and Visit") {
      getVisitId();
    }
    ++i_group;
  }

  fprintf(m_srcFile,"\n");

  // Setter method for all keywords
  i_key = m_hdu.header().keyword().begin();
  i_keyEnd = m_hdu.header().keyword().end();
  while (i_key != i_keyEnd) {
    setterMethod(i_key);
    ++i_key;
  }

  //Setter methods for all keyword groups
  i_group = m_hdu.header().group().begin();
  while (i_group != i_groupEnd) {
    // loop over all header keywords in this group
    i_key = i_group->keyword().begin();
    i_keyEnd = i_group->keyword().end();
    while (i_key != i_keyEnd) {
      setterMethod(i_key);
      ++i_key;
    }
    if (i_group->description() == "Visit" ||
        i_group->description() == "Pass and Visit") {
      setVisitId();
    }
    ++i_group;
  }
  fprintf(m_srcFile,"\n");
}

void PyFitsWrapper::keywordDescription(header_type::keyword_iterator i_key) {

  if (i_key->comment().present()) {
    fprintf(m_docFile, " Keyword description: %s", i_key->comment().get().c_str());
  }
  if (i_key->unit().present()) {
    fprintf(m_docFile, " (unit %s)", i_key->unit().get().c_str());
  }
  fprintf(m_docFile, "\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  ");
  if (i_key->default_().present()) {
    fprintf(m_docFile, "This keyword has the default value %s. ", i_key->default_().get().c_str());
  }
  if (i_key->db() == "true" || i_key->db() == "common") {
    fprintf(m_docFile, "This keyword can be queried for in the CHEOPS Archive and must therefore be given a non-empty value.");
  }
  fprintf(m_docFile, "\n");
}

void PyFitsWrapper::setterMethod(header_type::keyword_iterator i_key) {

  std::string varName = VariableName(i_key->name(), KEYWORD);
  std::string functionName = FunctionName(i_key->name(), KEYWORD);

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def set%s(%s) \n", functionName.c_str(), varName.c_str() + 2);
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Sets header keyword %s (type %s).", i_key->name().c_str(), i_key->data_type().c_str());
  keywordDescription(i_key);
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @param %s [%s] the value of header keyword %s\n", varName.c_str() + 2, i_key->data_type().c_str(), i_key->name().c_str());
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"set%s\", &%s::set%s)\n", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());
}


void PyFitsWrapper::setVisitId() {

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def setKeyVisitId() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Set visit ID, which consists of FITS header keywords PROGTYPE, PROG_ID, REQ_ID and VISITCTR");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @param visitId [VisitId] the value of the visit id\n");
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"setKeyVisitId\", &%s::setKeyVisitId)\n", m_wrapperClassName.c_str());
}


void PyFitsWrapper::getterMethod(header_type::keyword_iterator i_key) {

  std::string varName = VariableName(i_key->name(), KEYWORD);
  std::string functionName = FunctionName(i_key->name(), KEYWORD);

  // getKey* method
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def get%s() \n", functionName.c_str());
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns header keyword %s (type %s).", i_key->name().c_str(), i_key->data_type().c_str());
  keywordDescription(i_key);
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return [%s] the value of header keyword %s\n", i_key->data_type().c_str(), i_key->name().c_str());
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"get%s\", &%s::get%s)\n", functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());

  // getUnitOfKey* method
  std::string unitValue;
  if (i_key->unit().present()) {
    unitValue = i_key->unit().get();
  }
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getUnitOf%s() \n", functionName.c_str());
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns the unit of header keyword %s: \"%s\".\n", i_key->name().c_str(), unitValue.c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return the unit of header keyword %s: \"%s\"\n", i_key->name().c_str(), unitValue.c_str());
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"getUnitOf%s\", &%s::getUnitOf%s, boost::python::return_value_policy<boost::python::copy_const_reference>())\n",
                              functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());

  // getComOfKey* method
  std::string commentValue;
  if (i_key->comment().present()) {
    commentValue = i_key->comment().get();
  }
  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getComOf%s() \n", functionName.c_str());
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns the comment of header keyword %s: \"%s\".\n", i_key->name().c_str(), commentValue.c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return the comment of header keyword %s: \"%s\".\n", i_key->name().c_str(), commentValue.c_str());
  fprintf(m_docFile, "    */\n\n");
  fprintf(m_srcFile,"    .def(\"getComOf%s\", &%s::getComOf%s, boost::python::return_value_policy<boost::python::copy_const_reference>())\n",
                              functionName.c_str(), m_wrapperClassName.c_str(), functionName.c_str());
}


void PyFitsWrapper::getVisitId() {

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getKeyVisitId() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Get the visit ID , which consists of header keywords PROGTYPE, PROG_NUM, TARG_NUM and VISITCTR");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return [VisitId] the visit id.\n");
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"getKeyVisitId\", &%s::getKeyVisitId)\n", m_wrapperClassName.c_str());
}


void PyFitsWrapper::getFileName() {

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def getFileName() \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Returns the file name or an empty string if this class is not\n");
  fprintf(m_docFile, "    *         yet associated with a FITS extension.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @return the file name or an empty string if the class is not yet associated with a FITS extension.\n");
  fprintf(m_docFile, "    */\n\n");

  fprintf(m_srcFile,"    .def(\"getFileName\", &%s::GetFileName)\n", m_wrapperClassName.c_str());
}


void PyFitsWrapper::writeCurrentStatus() {

  fprintf(m_docFile, "   /** *************************************************************************\n");
  fprintf(m_docFile, "    *  @fn def writeCurrentStatus(filename) \n");
  fprintf(m_docFile, "    *  @memberof %s \n", ClassName().c_str());
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @brief Writes the current status into a new FITS file.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  This method should be used only for debugging purpose.\n");
  fprintf(m_docFile, "    *  Only the FITS table or the FITS image can be copied into the new\n");
  fprintf(m_docFile, "    *  FITS file. The status of the header keywords is not defined.\n\n");
  fprintf(m_docFile, "    *  A FITS table in the saved FITS file may have more rows as already\n");
  fprintf(m_docFile, "    *  created in this source table. The values in the additional rows are random.\n\n");
  fprintf(m_docFile, "    *  A FITS image will be copied into the primary array of the new FITS file.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @param filename of the file with the copied data. The file must not\n");
  fprintf(m_docFile, "    *                  exist before calling this method.\n");
  fprintf(m_docFile, "    *  \n");
  fprintf(m_docFile, "    *  @throw runtime_error if the new file cannot be created or when the\n");
  fprintf(m_docFile, "    *                       write operation fails.\n");
  fprintf(m_docFile, "    */\n");

  fprintf(m_srcFile,"    .def(\"writeCurrentStatus\", &%s::WriteCurrentStatus)\n", m_wrapperClassName.c_str());
}


bool PyFitsWrapper::hasVisitId() {

  HDU_type & hdu = m_fsd->HDU();

  std::vector<std::string>::size_type keywordCounter = 0;

  // search in all non-group keywords
  for (auto & keyword : hdu.header().keyword()) {
    if (std::find(m_visitIdKeywords.begin(), m_visitIdKeywords.end(), keyword.name())
        != m_visitIdKeywords.end()) {
      ++keywordCounter;  // found one of the visit id keywords
    }
  }

  //loop over all keyword groups
  header_type::group_iterator i_group = hdu.header().group().begin();
  header_type::group_iterator i_groupEnd = hdu.header().group().end();
  while (i_group != i_groupEnd) {

    // search in all header keywords in this group
    for (auto & keyword : i_group->keyword()) {
        if (std::find(m_visitIdKeywords.begin(), m_visitIdKeywords.end(), keyword.name())
            != m_visitIdKeywords.end()) {
          ++keywordCounter;  // found one of the visit id keywords
        }
      }

    ++i_group;
  }

  return keywordCounter == m_visitIdKeywords.size();
}


void PyFitsWrapper::visitIdConstructorForHdu(FILE * file) {

  if (hasVisitId()) {
    fprintf(file,"    .def(init<%s*>())\n", m_wrapperClassName.c_str());
  }
}


void PyFitsWrapper::associatedHdusWrapperFunctions() {

  if (!m_fsd->List_of_Associated_HDUs().present()) {
    return;
  }

  List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_hdu =
      m_fsd->List_of_Associated_HDUs().get().Associated_HDU().begin();
  List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_end =
      m_fsd->List_of_Associated_HDUs().get().Associated_HDU().end();

  while (i_hdu != i_end) {

    std::string assocHduClassName(ClassName(*i_hdu));
    std::string assocHduStructName(*i_hdu);
    std::string assocHduWrapperClassName(ClassName(*i_hdu) + "Wrapper");

    // Write a wrapper function for the function that appends this associated HDU.
    // This is necessary because the function exposed to Python must take an
    // object of the fits_data_model wrapper class instead of the
    // fits_data_model class itself as argument.
    fprintf(m_incFile, "inline %s * %s_append%s(%s * firstHdu",
            assocHduWrapperClassName.c_str(), ClassName().c_str(),
            assocHduClassName.c_str(), m_wrapperClassName.c_str());
    if (i_hdu->HDU_Type() == HDU_Type_type::image) {
      fprintf(m_incFile, ", const boost::python::list & imgSize");
    }
    fprintf(m_incFile, ") {\n");

    fprintf(m_incFile, "  %s * hdu = new %s(firstHdu->GetFileName(), \"APPEND\"",
            assocHduWrapperClassName.c_str(),  assocHduWrapperClassName.c_str());
    if (i_hdu->HDU_Type() == HDU_Type_type::image) {
      fprintf(m_incFile, ", imgSize");
    }
    fprintf(m_incFile, ");\n");

    fprintf(m_incFile, "  return hdu;\n");
    fprintf(m_incFile, "}\n\n");

    // Similarly, write a wrapper function for the function that opens an
    // associated HDU.
    fprintf(m_incFile, "inline %s * %s_open%s(%s * firstHdu) {\n",
               assocHduWrapperClassName.c_str(), ClassName().c_str(),
               assocHduClassName.c_str(), m_wrapperClassName.c_str());
    fprintf(m_incFile, "  try {\n");
    fprintf(m_incFile, "    return new %s(firstHdu->GetFileName() + \"[%s]\");\n",
               assocHduWrapperClassName.c_str(),  assocHduStructName.c_str());
    fprintf(m_incFile, "  }\n");
    fprintf(m_incFile, "  catch (std::exception & e) {}\n");
    fprintf(m_incFile, "  return nullptr;\n");
    fprintf(m_incFile, "}\n\n");

    i_hdu++;
  }
}


void PyFitsWrapper::exportAssociatedHdusFunctions(FILE * srcFile, FILE * docFile) {

  if (!m_fsd->List_of_Associated_HDUs().present()) {
    return;
  }

  List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_hdu =
      m_fsd->List_of_Associated_HDUs().get().Associated_HDU().begin();
  List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_end =
      m_fsd->List_of_Associated_HDUs().get().Associated_HDU().end();

  while (i_hdu != i_end) {

    std::string assocHduClassName(ClassName(*i_hdu));
    std::string assocHduWrapperClassName(ClassName(*i_hdu) + "Wrapper");
    std::string classNameMethodArgument = ClassName();
    classNameMethodArgument[0] = tolower(classNameMethodArgument[0]);

    // Export the the wrapper function for the Append_* function of this associated HDU to Python
    fprintf(docFile, "/** *************************************************************************\n");
    fprintf(docFile, "  *  @fn def append%s(%s", assocHduClassName.c_str(), classNameMethodArgument.c_str());
    if (i_hdu->HDU_Type() == HDU_Type_type::image) {
      fprintf(docFile, ", imgSize");
    }
    fprintf(docFile, ")\n");
    fprintf(docFile, "  *  @ingroup FitsDataModel\n");
    // Due to a bug in Doxygen, only the last @relates is shown in the documentation
    // https://bugzilla.gnome.org/show_bug.cgi?id=612617
    fprintf(docFile, "  *  @relates %s\n", assocHduClassName.c_str());
    fprintf(docFile, "  *  @relates %s\n", ClassName().c_str());
    fprintf(docFile, "  *  @author Reiner Rohlfs UGE\n");
    fprintf(docFile, "  *\n");
    fprintf(docFile, "  *  @brief Creates a new extension of class %s in the same file as \n", assocHduClassName.c_str());
    fprintf(docFile, "  *         the extension given in the method argument @b %s.\n", classNameMethodArgument.c_str());
    fprintf(docFile, "  *\n");
    fprintf(docFile, "  *  @param %s The first extension of a fits file, which must be of class %s.\n", classNameMethodArgument.c_str(), ClassName().c_str());
    fprintf(docFile, "  *            The new %s extension is create in the same FITS file.\n", assocHduClassName.c_str());
    if (i_hdu->HDU_Type() == HDU_Type_type::image) {
      fprintf(docFile, "  *  @param imgSize  defines the size of the image.\n");
    }
    fprintf(docFile, "  *\n");
    fprintf(docFile, "  *  @return  A %s object. \\n \n", assocHduClassName.c_str());
    fprintf(docFile, "  */\n");

    fprintf(srcFile,"  def(\"append%s\", fp_%s_append%s, return_value_policy<manage_new_object>());\n",
            assocHduClassName.c_str(), ClassName().c_str(), assocHduClassName.c_str());

    // Export the the wrapper function for the Open_* function of this associated HDU to Python
    fprintf(docFile, " /** *************************************************************************\n");
    fprintf(docFile, "  *  @fn def open%s(%s)\n", assocHduClassName.c_str(), classNameMethodArgument.c_str());
    fprintf(docFile, "  *  @ingroup FitsDataModel\n");
    // Due to a bug in Doxygen, only the last @relates is shown in the documentation
    // https://bugzilla.gnome.org/show_bug.cgi?id=612617
    fprintf(docFile, "  *  @relates %s\n", assocHduClassName.c_str());
    fprintf(docFile, "  *  @relates %s\n", ClassName().c_str());
    fprintf(docFile, "  *  @author Reiner Rohlfs UGE\n");
    fprintf(docFile, "  *  \n");
    fprintf(docFile, "  *  @brief Opens an extension of class %s in the same file as\n", assocHduClassName.c_str());
    fprintf(docFile, "  *         the extension given in the method argument @b %s.\n", classNameMethodArgument.c_str());
    fprintf(docFile, "  *  \n");
    fprintf(docFile, "  *  @param %s The first extension of a fits file, which must be of class %s.\n", classNameMethodArgument.c_str(), ClassName().c_str());
    fprintf(docFile, "  *            The extension will be opened in the same FITS file.\n");
    fprintf(docFile, "  *  \n");
    fprintf(docFile, "  *  @return  A %s object or\n", assocHduClassName.c_str());
    fprintf(docFile, "  *           None if the data structure does not exist in the FITS file.\n");
    fprintf(docFile, "  */\n");

    fprintf(srcFile,"  def(\"open%s\", fp_%s_open%s, return_value_policy<manage_new_object>());\n",
            assocHduClassName.c_str(), ClassName().c_str(), assocHduClassName.c_str());

    i_hdu++;
  }
}


void PyFitsWrapper::functionPointers(FILE * file) {

  if (!m_fsd->List_of_Associated_HDUs().present()) {
    return;
  }

  List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_hdu =
      m_fsd->List_of_Associated_HDUs().get().Associated_HDU().begin();
  List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_end =
      m_fsd->List_of_Associated_HDUs().get().Associated_HDU().end();

  fprintf(file, "// AppendHDU functions for class %s\n", ClassName().c_str());
  while (i_hdu != i_end) {

    std::string assocHduClassName(ClassName(*i_hdu));
    std::string assocHduWrapperClassName(ClassName(*i_hdu) + "Wrapper");

    // Function pointer to an append* function
    fprintf(file, "%s* (*fp_%s_append%s) (%s*",
            assocHduWrapperClassName.c_str(), ClassName().c_str(),
            assocHduClassName.c_str(), m_wrapperClassName.c_str());
    if (i_hdu->HDU_Type() == HDU_Type_type::image) {
      fprintf(file, ", const boost::python::list&");
    }
    fprintf(file, ") = &%s_append%s;\n", ClassName().c_str(), assocHduClassName.c_str());

    // Function pointer to an open* function
    fprintf(file, "%s* (*fp_%s_open%s) (%s*) = &%s_open%s;\n",
            assocHduWrapperClassName.c_str(), ClassName().c_str(),
            assocHduClassName.c_str(), m_wrapperClassName.c_str(),
            ClassName().c_str(), assocHduClassName.c_str());

    i_hdu++;
  }
  fprintf(file, "\n");
}


void PyFitsWrapper::exportClassToPythonFunction() {

  classDoc();
  constructorDoc();

  fprintf(m_srcFile, "void export%s() {\n\n", ClassName().c_str());

  exportClass();

  fprintf(m_srcFile, "}\n");
}


std::string PyFitsWrapper::getExportClassToPythonFunction() {

  return "export" + ClassName() +"()";
}


void PyFitsWrapper::validRefFileForHDU(FILE * file) {

  if (boost::starts_with(ClassName(), "RefApp") ||
      boost::starts_with(ClassName(), "SocApp")) {

    // Note: the documentation for this class is created in method classDoc()
    fprintf(file, "  class_<ValidRefFileWrapper<%s>>(\"Valid%s\", init<const list&>())\n", m_wrapperClassName.c_str(), ClassName().c_str());
    fprintf(file, "    .def(\"getFile\", &ValidRefFileWrapper<%s>::getFile, return_value_policy<reference_existing_object>())\n", m_wrapperClassName.c_str());
    fprintf(file, "    ;\n\n");
  }
}
