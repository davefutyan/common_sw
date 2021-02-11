/** ****************************************************************************
 *  @file
 *  @ingroup FitsDataModel
 *  @author  Anja Bekkelien UGE
 *
 *  @brief   Method definitions for the PyFitsDataModelExtension class.
 *
 *  The methods are used to generate Boost Python bindings in C++ that exposes
 *  the fits_data_model C++ API to Python.
 *  
 *  @version 10.2   ABE 2018-10-12 #16882 TriggerFile interface update for
 *                                        passing in the job order validity time.
 *  @version  9.2   ABE 2018-05-15 #15807 Set PIPE_VER in data product headers
 *  @version  9.2   ABE 2018-05-09 #15652 new mandatory attribute in trigger file
 *                                        schema, PREV_PASS_ID
 *  @version  8.0   ABE 2017-08-02 #12370 Implement __hash__() for utility classes
 *                                        so they can be used as dict keys and in
 *                                        sets. Remove clear() from UTC class.
 *  @version  7.4   ABE 2017-05-31 #12948 Read and write the creation date
 *                                        of the trigger file.
 *  @version  7.3   ABE 2017-05-10 #12806 Read and write the new element OBSID
 *                                        from the trigger file.
 *  @version  7.3   2017-05-03 ABE #12927 Add new method Empty() for checking if
 *                                        a trigger file has been read.
 *  @version  6.3   2016-10-24 ABE        Remove deprecated methods
 *                                        TriggerFile::getProcessingVersion(),
 *                                        TriggerFile::getObservationId()
 *  @version  6.2   2016-09-13 ABE        Add BarycentricOffset::setEphemerisFileName()
 *  @version  6.2   2016-08-31 ABE #11463 Add archive parameters reivision number
 *                                        and processing number to
 *                                        FitsDalHeader::SetProgram()
 *  @version  6.2   2016-08-31 ABE #11514 Add
 *                                        TriggerFile::getInputProcessingNumber(),
 *                                        TriggerFile::getOutputProcessingNumber(),
 *                                        TriggerFile::getRequestId() to python
 *  @version  6.1   2016-08-18 ABE #11327 Add VisitId::getRequestId() to python
 *  @version  6.0   2016-06-29 ABE #11081 Added __str__() methods to utility
 *                                        classes for automatic string
 *                                        representation
 *  @version  6.0   2016-05-23 ABE #11081 Enable pickling and deep copy of fits
 *                                        header data types
 *  @version  4.4   2015-12-03 ABE #9857  Export Open_* functions to Python.
 *  @version  4.3.1 2015-11-30 ABE #9844  Access FITS images as numpy arrays in
 *                                        Python fits_data_model.
 *  @version  4.1                         Split python bindings into separate
 *                                        files for each fits data model class to
 *                                        increase performance.
 *  @version  1.0                         First released version.
 *
 */

#include <algorithm>
#include <vector>
#include <string>
#include <stdexcept>
#include <boost/filesystem.hpp>

#include "PyFitsDataModel.hxx"

/** **************************************************************************
 *  @brief Creates a directory.
 *
 *  @param path The path to the directory to be created.
 *
 *  @throw runtim_error if the directory could not be created.
 *
 */
void createDirectory(const std::string & path) {

  // create the output directories if they don't exist
  if (!boost::filesystem::exists(path)) {

    if (!boost::filesystem::create_directories(path)) {
      throw std::runtime_error("Failed to create directory [" + path + "]");
    }
    else {
      logger << debug << "Created directory [" + path + "]" << std::endl;
    }
  }
}


PyFitsDataModelExtension::PyFitsDataModelExtension(
    const ParamsPtr progParam, std::vector<std::string> fsdFiles,
    const std::string & outputCodeDir, const std::string & outputDocDir) :
    m_progParam(progParam), m_fsdFiles(fsdFiles), m_outputCodeDir(outputCodeDir),
    m_outputDocDir(outputDocDir) {

  createDirectory(m_outputCodeDir + "/include");
  createDirectory(m_outputCodeDir + "/src");
  createDirectory(m_outputDocDir);

  std::string srcFileName = outputCodeDir + "/src/" + PyFitsWrapper::FILE_NAME_PREFIX + "Main.cxx";
  std::string docFileName = outputDocDir + "/" + PyFitsWrapper::FILE_NAME_PREFIX + "Main.txt";

  m_srcFile = fopen(srcFileName.c_str(), "w");
  if (m_srcFile == NULL) {
    throw std::runtime_error("Failed to create source file [" + srcFileName + "]");
  }

  m_docFile = fopen(docFileName.c_str(), "w");
  if (m_docFile == NULL) {
    throw std::runtime_error("Failed to create Doxygen file [" + docFileName + "]");
  }
}


PyFitsDataModelExtension::~PyFitsDataModelExtension() {

  if (m_srcFile) {
    fclose(m_srcFile);
  }

  if (m_docFile) {
    fclose(m_docFile);
  }
}


void PyFitsDataModelExtension::create() {

  logger << info << "Generating Python interface for fits_data_model" << std::endl;

  for (auto schemaFile : m_fsdFiles) {
    std::auto_ptr<Fits_schema_type> fsd (FITS_schema(schemaFile));
    std::unique_ptr<PyFitsWrapper> wrapper = PyFitsWrapper::factory(fsd, m_progParam, schemaFile, m_outputCodeDir, m_outputDocDir);
    m_fitsWrappers.push_back(std::move(wrapper));
  }

  // Create the include, source and doxygen doc file for eahc data structure
  for (auto & wrapper : m_fitsWrappers) {
    wrapper->create();
  }

  fileDoc();
  includes();
  converters();
  createFunctionPointers();
  pythonModule();

}


void PyFitsDataModelExtension::fileDoc() {

//  fprintf(m_docFile, "/** ****************************************************************************\n");
//  fprintf(m_docFile, " *  @file\n");
//  fprintf(m_docFile, " *  @ingroup FitsDataModel\n");
//  fprintf(m_docFile, " *\n");
//  fprintf(m_docFile, " */\n\n");
}


void PyFitsDataModelExtension::includes() {

  fprintf(m_srcFile, "/** ****************************************************************************\n");
  fprintf(m_srcFile, " *  \n");
  fprintf(m_srcFile, " *  This is an auto generated file. Do not modify.\n");
  fprintf(m_srcFile, " */\n\n");

  fprintf(m_srcFile, "#include <functional>\n"); // for std::hash
  fprintf(m_srcFile, "\n");
  fprintf(m_srcFile, "#include <boost/python.hpp>\n");
  fprintf(m_srcFile, "#include <boost/python/stl_iterator.hpp>\n");
  fprintf(m_srcFile, "\n");
  fprintf(m_srcFile, "#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION\n");
  fprintf(m_srcFile, "#include <numpy/noprefix.h>\n");
  fprintf(m_srcFile, "\n");
  fprintf(m_srcFile, "#include \"BarycentricOffset.hxx\"\n");
  fprintf(m_srcFile, "#include \"Bjd.hxx\"\n");
  fprintf(m_srcFile, "#include \"CreateFitsFile.hxx\"\n");
//  fprintf(m_srcFile, "#include \"DeltaTime.hxx\"\n");
  fprintf(m_srcFile, "#include \"LeapSeconds.hxx\"\n");
  fprintf(m_srcFile, "#include \"Mjd.hxx\"\n");
  fprintf(m_srcFile, "#include \"Obt.hxx\"\n");
  fprintf(m_srcFile, "#include \"ObtUtcCorrelation.hxx\"\n");
  fprintf(m_srcFile, "#include \"PassId.hxx\"\n");
  fprintf(m_srcFile, "#include \"TriggerFile.hxx\"\n");
  fprintf(m_srcFile, "#include \"Utc.hxx\"\n");
  fprintf(m_srcFile, "#include \"ValidRefFile.hxx\"\n");
  fprintf(m_srcFile, "#include \"VisitId.hxx\"\n");
  fprintf(m_srcFile, "#include \"FitsDalHeader.hxx\"\n");
  fprintf(m_srcFile, "\n");

  // Include the auto generated include files for all data structures
  for (auto & schemaFile : m_fsdFiles) {
    fprintf(m_srcFile, "#include \"%s.hxx\"\n", std::string(PyFitsWrapper::FILE_NAME_PREFIX + boost::filesystem::path(schemaFile).stem().string()).c_str());
  }

  fprintf(m_srcFile, "\n");
  fprintf(m_srcFile, "using namespace boost::python;\n\n");
  fprintf(m_srcFile, "using namespace boost::python::numeric;\n\n");
}


void PyFitsDataModelExtension::createFunctionPointers() {

  fprintf(m_srcFile, "//////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "// Functions pointers and wrappers used to expose utility classes.\n");
  fprintf(m_srcFile, "//////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");

  for (auto & wrapper : m_fitsWrappers) {
    wrapper->functionPointers(m_srcFile);
  }


  // Wrapped in order to hide the optional pointer arguments of GetAttr from the python interface
  fprintf(m_srcFile, "std::string FitsDalHeader_getAttr(FitsDalHeader & self, const std::string & name) {\n");
  fprintf(m_srcFile, "  return self.GetAttr<std::string>(name);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void FitsDalHeader_SetProgram_3attr(const std::string & programName,\n");
  fprintf(m_srcFile, "                                           const std::string & programVersion,\n");
  fprintf(m_srcFile, "                                           const std::string & svnRevision) {\n");
  fprintf(m_srcFile, "  FitsDalHeader::SetProgram(programName, programVersion, svnRevision);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void FitsDalHeader_SetProgram_4attr(const std::string & programName,\n");
  fprintf(m_srcFile, "                                           const std::string & programVersion,\n");
  fprintf(m_srcFile, "                                           const std::string & svnRevision,\n");
  fprintf(m_srcFile, "                                           const std::string & procChain) {\n");
  fprintf(m_srcFile, "  FitsDalHeader::SetProgram(programName, programVersion, svnRevision, procChain);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void FitsDalHeader_SetProgram_5attr(const std::string & programName,\n");
  fprintf(m_srcFile, "                                           const std::string & programVersion,\n");
  fprintf(m_srcFile, "                                           const std::string & svnRevision,\n");
  fprintf(m_srcFile, "                                           const std::string & procChain,\n");
  fprintf(m_srcFile, "                                           uint16_t revisionNumber) {\n");
  fprintf(m_srcFile, "  FitsDalHeader::SetProgram(programName, programVersion, svnRevision, procChain, revisionNumber);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void FitsDalHeader_SetProgram_6attr(const std::string & programName,\n");
  fprintf(m_srcFile, "                                           const std::string & programVersion,\n");
  fprintf(m_srcFile, "                                           const std::string & svnRevision,\n");
  fprintf(m_srcFile, "                                           const std::string & procChain,\n");
  fprintf(m_srcFile, "                                           uint16_t revisionNumber,\n");
  fprintf(m_srcFile, "                                           uint16_t processingNumber) {\n");
  fprintf(m_srcFile, "  FitsDalHeader::SetProgram(programName, programVersion, svnRevision, procChain, revisionNumber, processingNumber);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void FitsDalHeader_SetProgram_7attr(const std::string & programName,\n");
  fprintf(m_srcFile, "                                           const std::string & programVersion,\n");
  fprintf(m_srcFile, "                                           const std::string & svnRevision,\n");
  fprintf(m_srcFile, "                                           const std::string & procChain,\n");
  fprintf(m_srcFile, "                                           uint16_t revisionNumber,\n");
  fprintf(m_srcFile, "                                           uint16_t processingNumber,\n");
  fprintf(m_srcFile, "                                           const std::string & pipelineVersion) {\n");
  fprintf(m_srcFile, "  FitsDalHeader::SetProgram(programName, programVersion, svnRevision, procChain, revisionNumber, processingNumber, pipelineVersion);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( UTC_getUtc_overloads, getUtc, 0, 1 )\n\n");

  fprintf(m_srcFile, "static std::string BJD__str__(BJD & self) {\n");
  fprintf(m_srcFile, "  return std::to_string(self);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "std::size_t BJD__hash__(BJD & self) {\n");
  fprintf(m_srcFile, "  std::size_t h = std::hash<double>{}(self.getBjd());\n");
  fprintf(m_srcFile, "  return h;\n");
  fprintf(m_srcFile, "}\n");

  fprintf(m_srcFile, "static MJD BJD__sub__(BJD & self, BarycentricOffset & barycentricOffset) {\n");
  fprintf(m_srcFile, "  return self - barycentricOffset;\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static std::string MJD__str__(MJD & self) {\n");
  fprintf(m_srcFile, "  return std::to_string(self);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "std::size_t MJD__hash__(MJD & self) {\n");
  fprintf(m_srcFile, "  std::size_t h = std::hash<double>{}(self.getMjd());\n");
  fprintf(m_srcFile, "  return h;\n");
  fprintf(m_srcFile, "}\n");

  fprintf(m_srcFile, "static BJD MJD__add__(MJD & self, BarycentricOffset & barycentricOffset) {\n");
  fprintf(m_srcFile, "  return self + barycentricOffset;\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static std::string OBT__str__(OBT & self) {\n");
  fprintf(m_srcFile, "  return std::to_string(self);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "std::size_t OBT__hash__(OBT & self) {\n");
  fprintf(m_srcFile, "  std::size_t h = std::hash<long long>{}(self.getObt());\n");
  fprintf(m_srcFile, "  return h;\n");
  fprintf(m_srcFile, "}\n");

  fprintf(m_srcFile, "static std::string PassId__str__(PassId & self) {\n");
  fprintf(m_srcFile, "  return std::string(self);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "std::size_t PassId__hash__(PassId & self) {\n");
  fprintf(m_srcFile, "  std::size_t h = std::hash<std::string>{}(self.getFileNamePattern());\n");
  fprintf(m_srcFile, "  return h;\n");
  fprintf(m_srcFile, "}\n");

  fprintf(m_srcFile, "static std::string UTC__str__(UTC & self) {\n");
  fprintf(m_srcFile, "  return std::string(self);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "std::size_t UTC__hash__(UTC & self) {\n");
  fprintf(m_srcFile, "  std::size_t h = std::hash<std::string>{}(self.getUtc());\n");
  fprintf(m_srcFile, "  return h;\n");
  fprintf(m_srcFile, "}\n");

  fprintf(m_srcFile, "static std::string VisitId__str__(VisitId & self) {\n");
  fprintf(m_srcFile, "  return self.getFileNamePattern();\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "std::size_t VisitId__hash__(VisitId & self) {\n");
  fprintf(m_srcFile, "  std::size_t h = std::hash<std::string>{}(self.getFileNamePattern());\n");
  fprintf(m_srcFile, "  return h;\n");
  fprintf(m_srcFile, "}\n");

  fprintf(m_srcFile, "static void LeapSeconds_setLeapSecondsFileNames(const boost::python::list & fileNames) {\n");
  fprintf(m_srcFile, "  stl_input_iterator<std::string> begin(fileNames), end;\n");
  fprintf(m_srcFile, "  std::list<std::string> files(begin, end);\n");
  fprintf(m_srcFile, "  LeapSeconds::setLeapSecondsFileNames(files);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void OBTUTCCorrelation_SetCorrelationFileNames(const boost::python::list & fileNames) {\n");
  fprintf(m_srcFile, "  stl_input_iterator<std::string> begin(fileNames), end;\n");
  fprintf(m_srcFile, "  std::list<std::string> files(begin, end);\n");
  fprintf(m_srcFile, "  OBTUTCCorrelation::SetCorrelationFileNames(files);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void OBT_SetResetCounterFileNames(const boost::python::list & fileNames) {\n");
  fprintf(m_srcFile, "  stl_input_iterator<std::string> begin(fileNames), end;\n");
  fprintf(m_srcFile, "  std::list<std::string> files(begin, end);\n");
  fprintf(m_srcFile, "  OBT::SetResetCounterFileNames(files);\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "template <class HDU>\n");
  fprintf(m_srcFile, "class ValidRefFileWrapper : public ValidRefFile<HDU> {\n\n");
  fprintf(m_srcFile, " private:\n\n");
  fprintf(m_srcFile, "  std::list<std::string> convertList(const boost::python::list & list) {\n");
  fprintf(m_srcFile, "    stl_input_iterator<std::string> begin(list), end;\n");
  fprintf(m_srcFile, "    return std::list<std::string>(begin, end);\n");
  fprintf(m_srcFile, "  }\n\n");
  fprintf(m_srcFile, " public:\n\n");
  fprintf(m_srcFile, "  ValidRefFileWrapper(const boost::python::list & fileNames) :\n");
  fprintf(m_srcFile, "    ValidRefFile<HDU>(convertList(fileNames)) {}\n\n");
  fprintf(m_srcFile, "  HDU * getFile(const UTC & dataTime) {\n");
  fprintf(m_srcFile, "    return ValidRefFile<HDU>::getFile(dataTime);\n");
  fprintf(m_srcFile, "  }\n");
  fprintf(m_srcFile, "};\n\n");

  fprintf(m_srcFile, "static void TriggerFile_initialize_3arg(\n");
  fprintf(m_srcFile, "    const std::string & outDir,\n");
  fprintf(m_srcFile, "    const std::string & processorName,\n");
  fprintf(m_srcFile, "    const boost::python::list & triggerFiles\n");
  fprintf(m_srcFile, "    ) {\n");
  fprintf(m_srcFile, "  stl_input_iterator<std::string> begin(triggerFiles), end;\n");
  fprintf(m_srcFile, "  std::list<std::string> files(begin, end);\n");
  fprintf(m_srcFile, "  TriggerFile::Initialize(\n");
  fprintf(m_srcFile, "      outDir,\n");
  fprintf(m_srcFile, "      processorName,\n");
  fprintf(m_srcFile, "      files\n");
  fprintf(m_srcFile, "      );\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void TriggerFile_initialize_4arg(\n");
  fprintf(m_srcFile, "    const std::string & outDir,\n");
  fprintf(m_srcFile, "    const std::string & processorName,\n");
  fprintf(m_srcFile, "    const UTC & jobOrderStartTime,\n");
  fprintf(m_srcFile, "    const UTC & jobOrderStopTime\n");
  fprintf(m_srcFile, "    ) {\n");
  fprintf(m_srcFile, "  TriggerFile::Initialize(\n");
  fprintf(m_srcFile, "      outDir,\n");
  fprintf(m_srcFile, "      processorName,\n");
  fprintf(m_srcFile, "      jobOrderStartTime,\n");
  fprintf(m_srcFile, "      jobOrderStopTime\n");
  fprintf(m_srcFile, "      );\n");
  fprintf(m_srcFile, "}\n\n");

  fprintf(m_srcFile, "static void TriggerFile_readTriggerFile(\n");
  fprintf(m_srcFile, "    const std::string & path) {\n");
  fprintf(m_srcFile, "  TriggerFile::ReadTriggerFile(path);\n");
  fprintf(m_srcFile, "}\n\n");

  // Wrapper method for WriteTriggerFile that does not use utility classes as
  // input arguments, without the optional obsid and dataName arguments
  // TODO: this should be removed when obsid is no longer optional!
//  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_noObsid(\n");
//  fprintf(m_srcFile, "    const std::string & directory,\n");
//  fprintf(m_srcFile, "    const std::string & processorName,\n");
//  fprintf(m_srcFile, "    const std::string & passId,\n");
//  fprintf(m_srcFile, "    uint8_t programType,\n");
//  fprintf(m_srcFile, "    uint16_t programId,\n");
//  fprintf(m_srcFile, "    uint16_t requestId,\n");
//  fprintf(m_srcFile, "    uint8_t visitCounter,\n");
//  fprintf(m_srcFile, "    const std::string & validityStart,\n");
//  fprintf(m_srcFile, "    const std::string & validityStop,\n");
//  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint16_t outputProcessingNumber) {\n");
//  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
//  fprintf(m_srcFile, "      directory, processorName,\n");
//  fprintf(m_srcFile, "      passId, programType, programId, requestId, visitCounter,\n");
//  fprintf(m_srcFile, "      validityStart, validityStop, \n");
//  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber);\n");
//  fprintf(m_srcFile, "}\n\n");


  // Wrapper method for WriteTriggerFile that does not use utility classes as
  // input arguments, with the optional dataName argument
  // TODO: this should be removed when obsid is no longer optional!
//  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_dataName_noObsid(\n");
//  fprintf(m_srcFile, "    const std::string & directory,\n");
//  fprintf(m_srcFile, "    const std::string & processorName,\n");
//  fprintf(m_srcFile, "    const std::string & passId,\n");
//  fprintf(m_srcFile, "    uint8_t programType,\n");
//  fprintf(m_srcFile, "    uint16_t programId,\n");
//  fprintf(m_srcFile, "    uint16_t requestId,\n");
//  fprintf(m_srcFile, "    uint8_t visitCounter,\n");
//  fprintf(m_srcFile, "    const std::string & validityStart,\n");
//  fprintf(m_srcFile, "    const std::string & validityStop,\n");
//  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
//  fprintf(m_srcFile, "    const std::string & dataName) {\n");
//  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
//  fprintf(m_srcFile, "      directory, processorName,\n");
//  fprintf(m_srcFile, "      passId, programType, programId, requestId, visitCounter,\n");
//  fprintf(m_srcFile, "      validityStart, validityStop,\n");
//  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, 0, dataName);\n");
//  fprintf(m_srcFile, "}\n\n");


  // Wrapper method for WriteTriggerFile that does not use utility classes as
  // input arguments, without the optional dataName argument
//  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile(\n");
//  fprintf(m_srcFile, "    const std::string & directory,\n");
//  fprintf(m_srcFile, "    const std::string & processorName,\n");
//  fprintf(m_srcFile, "    const std::string & passId,\n");
//  fprintf(m_srcFile, "    uint8_t programType,\n");
//  fprintf(m_srcFile, "    uint16_t programId,\n");
//  fprintf(m_srcFile, "    uint16_t requestId,\n");
//  fprintf(m_srcFile, "    uint8_t visitCounter,\n");
//  fprintf(m_srcFile, "    const std::string & validityStart,\n");
//  fprintf(m_srcFile, "    const std::string & validityStop,\n");
//  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint32_t obsid) {\n");
//  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
//  fprintf(m_srcFile, "      directory, processorName,\n");
//  fprintf(m_srcFile, "      passId, programType, programId, requestId, visitCounter,\n");
//  fprintf(m_srcFile, "      validityStart, validityStop, \n");
//  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, obsid);\n");
//  fprintf(m_srcFile, "}\n\n");

  // Wrapper method for WriteTriggerFile that does not use utility classes as
  // input arguments, with the optional dataName argument
//  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_1_opt_arg(\n");
//  fprintf(m_srcFile, "    const std::string & directory,\n");
//  fprintf(m_srcFile, "    const std::string & processorName,\n");
//  fprintf(m_srcFile, "    const std::string & passId,\n");
//  fprintf(m_srcFile, "    uint8_t programType,\n");
//  fprintf(m_srcFile, "    uint16_t programId,\n");
//  fprintf(m_srcFile, "    uint16_t requestId,\n");
//  fprintf(m_srcFile, "    uint8_t visitCounter,\n");
//  fprintf(m_srcFile, "    const std::string & validityStart,\n");
//  fprintf(m_srcFile, "    const std::string & validityStop,\n");
//  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint32_t obsid,\n");
//  fprintf(m_srcFile, "    const std::string & dataName) {\n");
//  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
//  fprintf(m_srcFile, "      directory, processorName,\n");
//  fprintf(m_srcFile, "      passId, programType, programId, requestId, visitCounter,\n");
//  fprintf(m_srcFile, "      validityStart, validityStop,\n");
//  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, obsid, dataName);\n");
//  fprintf(m_srcFile, "}\n\n");

  // Wrapper method for WriteTriggerFile that does not use utility classes as
  // input arguments, with the optional dataName argument
//  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_2_opt_arg(\n");
//  fprintf(m_srcFile, "    const std::string & directory,\n");
//  fprintf(m_srcFile, "    const std::string & processorName,\n");
//  fprintf(m_srcFile, "    const std::string & passId,\n");
//  fprintf(m_srcFile, "    uint8_t programType,\n");
//  fprintf(m_srcFile, "    uint16_t programId,\n");
//  fprintf(m_srcFile, "    uint16_t requestId,\n");
//  fprintf(m_srcFile, "    uint8_t visitCounter,\n");
//  fprintf(m_srcFile, "    const std::string & validityStart,\n");
//  fprintf(m_srcFile, "    const std::string & validityStop,\n");
//  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
//  fprintf(m_srcFile, "    uint32_t obsid,\n");
//  fprintf(m_srcFile, "    const std::string & dataName,\n");
//  fprintf(m_srcFile, "    const std::string & creationDate) {\n");
//  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
//  fprintf(m_srcFile, "      directory, processorName,\n");
//  fprintf(m_srcFile, "      passId, programType, programId, requestId, visitCounter,\n");
//  fprintf(m_srcFile, "      validityStart, validityStop,\n");
//  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, obsid, dataName, creationDate);\n");
//  fprintf(m_srcFile, "}\n\n");

  // Wrapper methods for WriteTriggerFile that uses utility classes as input
  // arguments, without the optional dataName argument
  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_utilities_0_opt_arg(\n");
  fprintf(m_srcFile, "    const std::string & directory,\n");
  fprintf(m_srcFile, "    const std::string & processorName,\n");
  fprintf(m_srcFile, "    const PassId & passId,\n");
  fprintf(m_srcFile, "    const VisitId & visitId,\n");
  fprintf(m_srcFile, "    const UTC & validityStart,\n");
  fprintf(m_srcFile, "    const UTC & validityStop,\n");
  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint32_t obsid) {\n");
  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
  fprintf(m_srcFile, "      directory, processorName,\n");
  fprintf(m_srcFile, "      passId, visitId,\n");
  fprintf(m_srcFile, "      validityStart, validityStop,\n");
  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, obsid);\n");
  fprintf(m_srcFile, "}\n\n");

  // Wrapper methods for WriteTriggerFile that uses utility classes as input
  // arguments, with the optional dataName argument
  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_utilities_1_opt_arg(\n");
  fprintf(m_srcFile, "    const std::string & directory,\n");
  fprintf(m_srcFile, "    const std::string & processorName,\n");
  fprintf(m_srcFile, "    const PassId & passId,\n");
  fprintf(m_srcFile, "    const VisitId & visitId,\n");
  fprintf(m_srcFile, "    const UTC & validityStart,\n");
  fprintf(m_srcFile, "    const UTC & validityStop,\n");
  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint32_t obsid,\n");
  fprintf(m_srcFile, "    const std::string & dataName) {\n");
  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
  fprintf(m_srcFile, "      directory, processorName,\n");
  fprintf(m_srcFile, "      passId, visitId,\n");
  fprintf(m_srcFile, "      validityStart, validityStop,\n");
  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, obsid, dataName);\n");
  fprintf(m_srcFile, "}\n\n");

  // Wrapper methods for WriteTriggerFile that uses utility classes as input
  // arguments, with the optional dataName argument
  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_utilities_2_opt_arg(\n");
  fprintf(m_srcFile, "    const std::string & directory,\n");
  fprintf(m_srcFile, "    const std::string & processorName,\n");
  fprintf(m_srcFile, "    const PassId & passId,\n");
  fprintf(m_srcFile, "    const VisitId & visitId,\n");
  fprintf(m_srcFile, "    const UTC & validityStart,\n");
  fprintf(m_srcFile, "    const UTC & validityStop,\n");
  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint32_t obsid,\n");
  fprintf(m_srcFile, "    const std::string & dataName, \n");
  fprintf(m_srcFile, "    const UTC & creationDate) {\n");
  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
  fprintf(m_srcFile, "      directory, processorName,\n");
  fprintf(m_srcFile, "      passId, visitId,\n");
  fprintf(m_srcFile, "      validityStart, validityStop,\n");
  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, obsid, dataName, creationDate);\n");
  fprintf(m_srcFile, "}\n\n");

  // Wrapper methods for WriteTriggerFile that uses utility classes as input
  // arguments, with the optional dataName argument
  fprintf(m_srcFile, "static std::string TriggerFile_writeTriggerFile_utilities_3_opt_arg(\n");
  fprintf(m_srcFile, "    const std::string & directory,\n");
  fprintf(m_srcFile, "    const std::string & processorName,\n");
  fprintf(m_srcFile, "    const PassId & passId,\n");
  fprintf(m_srcFile, "    const VisitId & visitId,\n");
  fprintf(m_srcFile, "    const UTC & validityStart,\n");
  fprintf(m_srcFile, "    const UTC & validityStop,\n");
  fprintf(m_srcFile, "    uint16_t inputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint16_t outputProcessingNumber,\n");
  fprintf(m_srcFile, "    uint32_t obsid,\n");
  fprintf(m_srcFile, "    const std::string & dataName, \n");
  fprintf(m_srcFile, "    const UTC & creationDate,\n");
  fprintf(m_srcFile, "    const PassId & prevPassId) {\n");
  fprintf(m_srcFile, "  return TriggerFile::WriteTriggerFile(\n");
  fprintf(m_srcFile, "      directory, processorName,\n");
  fprintf(m_srcFile, "      passId, visitId,\n");
  fprintf(m_srcFile, "      validityStart, validityStop,\n");
  fprintf(m_srcFile, "      inputProcessingNumber, outputProcessingNumber, obsid, dataName, creationDate, prevPassId);\n");
  fprintf(m_srcFile, "}\n\n");

  // Pickling Boost Python objects, see http://www.boost.org/doc/libs/1_57_0/libs/python/doc/v2/pickle.html
  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "  // Enable pickling and deep copy for fits header data types.\n");
  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");
  fprintf(m_srcFile, "struct BJD_pickle_suite : boost::python::pickle_suite {\n");
  fprintf(m_srcFile, "   static boost::python::tuple getinitargs(const BJD & obj) {\n");
  fprintf(m_srcFile, "      return boost::python::make_tuple(obj.getBjd());\n");
  fprintf(m_srcFile, "   }\n");
  fprintf(m_srcFile, "};\n\n");

  fprintf(m_srcFile, "struct MJD_pickle_suite : boost::python::pickle_suite {\n");
  fprintf(m_srcFile, "   static boost::python::tuple getinitargs(const MJD & obj) {\n");
  fprintf(m_srcFile, "      return boost::python::make_tuple(obj.getMjd());\n");
  fprintf(m_srcFile, "   }\n");
  fprintf(m_srcFile, "};\n\n");

  fprintf(m_srcFile, "struct OBT_pickle_suite : boost::python::pickle_suite {\n");
  fprintf(m_srcFile, "   static boost::python::tuple getinitargs(const OBT & obj) {\n");
  fprintf(m_srcFile, "      return boost::python::make_tuple(obj.getObt());\n");
  fprintf(m_srcFile, "   }\n");
  fprintf(m_srcFile, "};\n\n");

  fprintf(m_srcFile, "struct UTC_pickle_suite : boost::python::pickle_suite {\n");
  fprintf(m_srcFile, "   static boost::python::tuple getinitargs(const UTC & obj) {\n");
  fprintf(m_srcFile, "      return boost::python::make_tuple(obj.getFileNamePattern());\n");
  fprintf(m_srcFile, "   }\n");
  fprintf(m_srcFile, "};\n\n");

  fprintf(m_srcFile, "struct VisitId_pickle_suite : boost::python::pickle_suite {\n");
  fprintf(m_srcFile, "   static boost::python::tuple getinitargs(const VisitId & obj) {\n");
  fprintf(m_srcFile, "      return boost::python::make_tuple(obj.getProgramType(),\n");
  fprintf(m_srcFile, "                                       obj.getProgramId(),\n");
  fprintf(m_srcFile, "                                       obj.getRequestId(),\n");
  fprintf(m_srcFile, "                                       obj.getVisitCounter());\n");
  fprintf(m_srcFile, "   }\n");
  fprintf(m_srcFile, "};\n\n");

  fprintf(m_srcFile, "struct PassId_pickle_suite : boost::python::pickle_suite {\n");
  fprintf(m_srcFile, "   static boost::python::tuple getinitargs(const PassId & obj) {\n");
  fprintf(m_srcFile, "      if (obj.isValid()) {\n");
  fprintf(m_srcFile, "        return boost::python::make_tuple(obj.getYear(),\n");
  fprintf(m_srcFile, "                                         obj.getMonth(),\n");
  fprintf(m_srcFile, "                                         obj.getDay(),\n");
  fprintf(m_srcFile, "                                         obj.getHour());\n");
  fprintf(m_srcFile, "      } else {\n");
  fprintf(m_srcFile, "        return boost::python::make_tuple();\n");
  fprintf(m_srcFile, "      }\n");
  fprintf(m_srcFile, "   }\n");
  fprintf(m_srcFile, "};\n\n");
}

void PyFitsDataModelExtension::exportFitsDal() {

  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "  // Export fits_dal classes.\n");
  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");

  fprintf(m_docFile, "/** ****************************************************************************\n");
  fprintf(m_docFile, " *  @class FitsDalHeader\n");
  fprintf(m_docFile, " *  @ingroup FitsDataModel\n");
  fprintf(m_docFile, " *  @author Reiner Rohlfs UGE\n");
  fprintf(m_docFile, " *\n");
  fprintf(m_docFile, " *  @brief Representing a FITS header with access functions to the attributes.\n");
  fprintf(m_docFile, " *  Most of the times this FitsDalHeader is used as the parent class of\n");
  fprintf(m_docFile, " *  the FitsDalImage and FitsDalTable classes to access a FTIS extension.\n");
  fprintf(m_docFile, " *  But it is also possible to open any FITS extension, using this class\n");
  fprintf(m_docFile, " *  to read the header keyword. In this approach it is not possible to\n");
  fprintf(m_docFile, " *  update the header keywords or to access the data of the HDU\n");
  fprintf(m_docFile, " */\n\n");

  fprintf(m_docFile, "/** *************************************************************************\n");
  fprintf(m_docFile, " *  @fn def setProgram(programName, programVersion, svnRevision, procChain, revisionNumber, processingNumber, pipelineVersion) \n");
  fprintf(m_docFile, " *  \n");
  fprintf(m_docFile, " *  @memberof FitsDalHeader\n");
  fprintf(m_docFile, " *  @brief These program specific values will be written in every header,\n");
  fprintf(m_docFile, " *         that is create after calling this function..\n");
  fprintf(m_docFile, " *\n");
  fprintf(m_docFile, " *  @param programName [string] The program name\n");
  fprintf(m_docFile, " *  @param programVersion [string] The program's version\n");
  fprintf(m_docFile, " *  @param svnRevision [string] The SVN revision number\n");
  fprintf(m_docFile, " *  @param procChain [string] (Optional) the processing chain\n");
  fprintf(m_docFile, " *  @param revisionNumber [uint16_t] (Optional) the archive revision number\n");
  fprintf(m_docFile, " *  @param processingNumber [uint16_t] (Optional) the processing number\n");
  fprintf(m_docFile, " *  @param pipelineVersion [string] (Optional) the pipeline version\n");
  fprintf(m_docFile, " */\n\n");

  fprintf(m_docFile, "/** *************************************************************************\n");
  fprintf(m_docFile, " *  @fn def getAttr(name) \n");
  fprintf(m_docFile, " *  \n");
  fprintf(m_docFile, " *  @memberof FitsDalHeader\n");
  fprintf(m_docFile, " *  @brief Returns the value of a header keyword as a string.\n");
  fprintf(m_docFile, " *\n");
  fprintf(m_docFile, " *  @param name [string] The name of a header keyword\n");
  fprintf(m_docFile, " *  @return The value of the header keyword\n");
  fprintf(m_docFile, " */\n\n");

  fprintf(m_srcFile, "  class_<FitsDalHeader>(\"FitsDalHeader\", init<const std::string&>())\n");
  fprintf(m_srcFile, "    .def(\"setProgram\", FitsDalHeader_SetProgram_3attr)\n");
  fprintf(m_srcFile, "    .def(\"setProgram\", FitsDalHeader_SetProgram_4attr)\n");
  fprintf(m_srcFile, "    .def(\"setProgram\", FitsDalHeader_SetProgram_5attr)\n");
  fprintf(m_srcFile, "    .def(\"setProgram\", FitsDalHeader_SetProgram_6attr)\n");
  fprintf(m_srcFile, "    .def(\"setProgram\", FitsDalHeader_SetProgram_7attr)\n");
//  fprintf(m_srcFile, "    .def(\"setProgram\", (void (*)(const std::string&, const std::string&, const std::string&))&FitsDalHeader::SetProgram)\n");
//  fprintf(m_srcFile, "    .def(\"setProgram\", (void (*)(const std::string&, const std::string&, const std::string&, const std::string&))&FitsDalHeader::SetProgram)\n");
  fprintf(m_srcFile, "    .staticmethod(\"setProgram\")\n");
  fprintf(m_srcFile, "    .def(\"getDataVersion\", &FitsDalHeader::getDataVersion)\n");
  fprintf(m_srcFile, "    .staticmethod(\"getDataVersion\")\n");
  fprintf(m_srcFile, "    .def(\"getAttr\", FitsDalHeader_getAttr)\n");
  fprintf(m_srcFile, "    ;\n");
  fprintf(m_srcFile, "\n");
}

void PyFitsDataModelExtension::exportUtility() {

  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "  // Export utility classes.\n");
  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");

//  // Class DeltaTime
//  fprintf(m_srcFile, "  class_<DeltaTime>(\"DeltaTime\", init<double>())\n");
//  fprintf(m_srcFile, "    .def(\"getSeconds\", &DeltaTime::getSeconds)\n");
//  fprintf(m_srcFile, "    .def(self * double())\n");             // __mul__
//  fprintf(m_srcFile, "    .def(self / double())\n");             // __div__
//  fprintf(m_srcFile, "    ;\n");

  // Class OBTUTCCorrelation
  fprintf(m_srcFile, "  class_<OBTUTCCorrelation>(\"OBTUTCCorrelation\")\n");
  fprintf(m_srcFile, "    .def(\"setCorrelationFileNames\", &OBTUTCCorrelation_SetCorrelationFileNames)\n");
  fprintf(m_srcFile,"     .staticmethod(\"setCorrelationFileNames\")\n");
  fprintf(m_srcFile, "    ;\n");

  // Class OBT
  fprintf(m_srcFile, "  class_<OBT>(\"OBT\", init<>())\n");
  fprintf(m_srcFile, "    .setattr(\"__hash__\", boost::python::object())\n");
  fprintf(m_srcFile, "    .def(init<const OBT&>())\n");
  fprintf(m_srcFile, "    .def(init<int64_t, optional<UTC>>())\n");
  fprintf(m_srcFile, "    .def(init<int64_t, bool, optional<UTC>>())\n");
  fprintf(m_srcFile, "    .def(\"getObt\", &OBT::getObt)\n");
  fprintf(m_srcFile, "    .def(\"getUtc\", &OBT::getUtc)\n");
  fprintf(m_srcFile, "    .def(\"isSynchronized\", &OBT::isSynchronized)\n");
  fprintf(m_srcFile, "    .def(\"getResetCounter\", &OBT::getResetCounter)\n");
  fprintf(m_srcFile, "    .def(\"setResetCounterFileNames\", &OBT_SetResetCounterFileNames)\n");
  // Define special operators.
  fprintf(m_srcFile, "    .def(\"__str__\", OBT__str__)\n");      // __str__
  fprintf(m_srcFile, "    .def(\"__hash__\", OBT__hash__)\n");    // __str__
  fprintf(m_srcFile, "    .def(self < self)\n");                  // __lt__
  fprintf(m_srcFile, "    .def(self <= self)\n");                 // __le__
  fprintf(m_srcFile, "    .def(self > self)\n");                  // __gt__
  fprintf(m_srcFile, "    .def(self >= self)\n");                 // __ge__
  fprintf(m_srcFile, "    .def(self == self)\n");                 // __eq__
//  fprintf(m_srcFile, "    .def(self - self)\n");                // __sub__
//  fprintf(m_srcFile, "    .def(self + other<DeltaTime>())\n");  // __add__
  fprintf(m_srcFile, "    .def(int_(self))\n");                   // __int__
  // Pickling
  fprintf(m_srcFile, "    .def_pickle(OBT_pickle_suite())\n");
  fprintf(m_srcFile, "    ;\n");

  // Class LeapSeconds
  fprintf(m_srcFile, "  class_<LeapSeconds>(\"LeapSeconds\", init<>())\n");
  fprintf(m_srcFile, "    .def(\"setLeapSecondsFileNames\", &LeapSeconds_setLeapSecondsFileNames)\n");
  fprintf(m_srcFile,"     .staticmethod(\"setLeapSecondsFileNames\")\n");
  fprintf(m_srcFile, "    ;\n");

  // Class BarycentricOffset
  fprintf(m_srcFile, "  class_<BarycentricOffset>(\"BarycentricOffset\", init<double, double>())\n");
  fprintf(m_srcFile, "    .def(init<const BarycentricOffset&>())\n");
  fprintf(m_srcFile, "    .def(\"setEphemerisFileName\", &BarycentricOffset::setEphemerisFileName)\n");
  fprintf(m_srcFile, "    ;\n");

  // Class BJD
  fprintf(m_srcFile, "  class_<BJD>(\"BJD\", init<>())\n");
  fprintf(m_srcFile, "    .def(init<const BJD&>())\n");
  fprintf(m_srcFile, "    .def(init<double>())\n");
  fprintf(m_srcFile, "    .def(\"getBjd\", &BJD::getBjd)\n");
  // Define special operators.
  fprintf(m_srcFile, "    .def(\"__str__\", BJD__str__)\n");              // __str__
  fprintf(m_srcFile, "    .def(\"__hash__\", BJD__hash__)\n");            // __hash__
  fprintf(m_srcFile, "    .def(float_(self))\n");                         // __float__
  fprintf(m_srcFile, "    .def(self < self)\n");                          // __lt__
  fprintf(m_srcFile, "    .def(self <= self)\n");                         // __le__
  fprintf(m_srcFile, "    .def(self > self)\n");                          // __gt__
  fprintf(m_srcFile, "    .def(self >= self)\n");                         // __ge__
//  fprintf(m_srcFile, "    .def(self == double())\n");                   // __eq__
  fprintf(m_srcFile, "    .def(self == self)\n");                         // __eq__
  fprintf(m_srcFile, "    .def(self != self)\n");                         // __ne__
  // fprintf(m_srcFile, "    .def(self - other<BarycentricOffset>())\n"); // __sub__
  fprintf(m_srcFile, "    .def(\"__sub__\", BJD__sub__)\n");              // __sub__
  // Pickling
  fprintf(m_srcFile, "    .def_pickle(BJD_pickle_suite())\n");
  fprintf(m_srcFile, "    ;\n");

  // Class MJD
  fprintf(m_srcFile, "  class_<MJD>(\"MJD\", init<>())\n");
  fprintf(m_srcFile, "    .def(init<const MJD&>())\n");
  fprintf(m_srcFile, "    .def(init<double>())\n");
  fprintf(m_srcFile, "    .def(\"getMjd\", &MJD::getMjd)\n");
  fprintf(m_srcFile, "    .def(\"getUtc\", &MJD::getUtc)\n");
  // Define special operators.
  fprintf(m_srcFile, "    .def(\"__str__\", MJD__str__)\n");      // __str__
  fprintf(m_srcFile, "    .def(\"__hash__\", MJD__hash__)\n");     // __hash__
  fprintf(m_srcFile, "    .def(float_(self))\n");                 // __float__
  fprintf(m_srcFile, "    .def(self == double())\n");             // __eq__
  fprintf(m_srcFile, "    .def(self < self)\n");                  // __lt__
  fprintf(m_srcFile, "    .def(self <= self)\n");                 // __le__
  fprintf(m_srcFile, "    .def(self > self)\n");                  // __gt__
  fprintf(m_srcFile, "    .def(self >= self)\n");                 // __ge__
  fprintf(m_srcFile, "    .def(self != self)\n");                 // __ne__
  fprintf(m_srcFile, "    .def(self == self)\n");                 // __eq__
  // fprintf(m_srcFile, "    .def(self + other<BarycentricOffset>())\n"); // __add__
  fprintf(m_srcFile, "    .def(\"__add__\", MJD__add__)\n");              // __add__
  // Pickling
  fprintf(m_srcFile, "    .def_pickle(MJD_pickle_suite())\n");
  fprintf(m_srcFile, "    ;\n");

  // Class UTC
  fprintf(m_srcFile, "  class_<UTC>(\"UTC\", init<>())\n");
  fprintf(m_srcFile, "    .def(init<const std::string&>())\n");
  fprintf(m_srcFile, "    .def(init<uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, optional<double>>())\n");
  // UTC(time_t seconds, double secFraction);

  // The clear() method has been removed in order to make UTC objects immutable,
  // to allow for an implementation of the __hash__() method that corresponds
  // to the __eq__() method. Only immutable objects shall implement __hash__()
  // See https://redmine.isdc.unige.ch/issues/12370
  // fprintf(m_srcFile, "    .def(\"clear\", &UTC::clear)\n");
  fprintf(m_srcFile, "    .def(\"empty\", &UTC::empty)\n");
  fprintf(m_srcFile, "    .def(\"getUtc\", &UTC::getUtc, UTC_getUtc_overloads())\n");
  fprintf(m_srcFile, "    .def(\"getFileNamePattern\", &UTC::getFileNamePattern)\n");
  fprintf(m_srcFile, "    .def(\"getYear\", &UTC::getYear)\n");
  fprintf(m_srcFile, "    .def(\"getMonth\", &UTC::getMonth)\n");
  fprintf(m_srcFile, "    .def(\"getDay\", &UTC::getDay)\n");
  fprintf(m_srcFile, "    .def(\"getHour\", &UTC::getHour)\n");
  fprintf(m_srcFile, "    .def(\"getMinute\", &UTC::getMinute)\n");
  fprintf(m_srcFile, "    .def(\"getSecond\", &UTC::getSecond)\n");
  fprintf(m_srcFile, "    .def(\"getSecFraction\", &UTC::getSecFraction)\n");
  fprintf(m_srcFile, "    .def(\"getMjd\", &UTC::getMjd)\n");
  fprintf(m_srcFile, "    .def(\"getObt\", &UTC::getObt)\n");
  // Define special operators.
  fprintf(m_srcFile, "    .def(\"__str__\", UTC__str__)\n");      // __str__
  fprintf(m_srcFile, "    .def(\"__hash__\", UTC__hash__)\n");    // __hash__
  fprintf(m_srcFile, "    .def(self < self)\n");                  // __lt__
  fprintf(m_srcFile, "    .def(self <= self)\n");                 // __le__
  fprintf(m_srcFile, "    .def(self > self)\n");                  // __gt__
  fprintf(m_srcFile, "    .def(self >= self)\n");                 // __ge__
  fprintf(m_srcFile, "    .def(self != self)\n");                 // __ne__
  fprintf(m_srcFile, "    .def(self == self)\n");                 // __eq__
//  fprintf(m_srcFile, "    .def(self - self)\n");                // __sub__
//  fprintf(m_srcFile, "    .def(self + other<DeltaTime>())\n");  // __add__
//  fprintf(m_srcFile, "    .def(self == str())\n");              // __eq__
  // Pickling
  fprintf(m_srcFile, "    .def_pickle(UTC_pickle_suite())\n");
  fprintf(m_srcFile, "    ;\n");

  // Class PassId
  fprintf(m_srcFile, "  class_<PassId>(\"PassId\", init<>())\n");
  fprintf(m_srcFile, "    .def(init<uint8_t, uint8_t, uint8_t, uint8_t>())\n");
  fprintf(m_srcFile, "    .def(init<const std::string&>())\n");
  fprintf(m_srcFile, "    .def(\"isValid\", &PassId::isValid)\n");
  fprintf(m_srcFile, "    .def(\"getYear\", &PassId::getYear)\n");
  fprintf(m_srcFile, "    .def(\"getMonth\", &PassId::getMonth)\n");
  fprintf(m_srcFile, "    .def(\"getDay\", &PassId::getDay)\n");
  fprintf(m_srcFile, "    .def(\"getHour\", &PassId::getHour)\n");
  fprintf(m_srcFile, "    .def(\"getFileNamePattern\", &PassId::getFileNamePattern)\n");
  // Define special operators.
  fprintf(m_srcFile, "    .def(\"__str__\", PassId__str__)\n");   // __str__
  fprintf(m_srcFile, "    .def(\"__hash__\", PassId__hash__)\n"); // __hash__
  fprintf(m_srcFile, "    .def(self < self)\n");                  // __lt__
  fprintf(m_srcFile, "    .def(self != self)\n");                 // __ne__
  fprintf(m_srcFile, "    .def(self == self)\n");                 // __eq__
  // Pickling
  fprintf(m_srcFile, "    .def_pickle(PassId_pickle_suite())\n");
  fprintf(m_srcFile, "    ;\n");

  // Class VisitId
  fprintf(m_srcFile, "  class_<VisitId>(\"VisitId\", init<>())\n");
  fprintf(m_srcFile, "    .def(init<uint8_t, uint16_t, uint16_t, uint8_t>())\n");
  fprintf(m_srcFile, "    .def(init<const std::string&>())\n");

  // Create one VisitID(HDU) constructor for each data structure in fits data model
  for (auto & wrapper : m_fitsWrappers) {
    wrapper->visitIdConstructorForHdu(m_srcFile);
  }

  fprintf(m_srcFile, "    .def(\"isValid\", &VisitId::isValid)\n");
  fprintf(m_srcFile, "    .def(\"getProgramType\", &VisitId::getProgramType)\n");
  fprintf(m_srcFile, "    .def(\"getProgramId\", &VisitId::getProgramId)\n");
  fprintf(m_srcFile, "    .def(\"getRequestId\", &VisitId::getRequestId)\n");
  fprintf(m_srcFile, "    .def(\"getVisitCounter\", &VisitId::getVisitCounter)\n");
  fprintf(m_srcFile, "    .def(\"getFileNamePattern\", &VisitId::getFileNamePattern)\n");
  // Define special operators.
  fprintf(m_srcFile, "    .def(\"__str__\", VisitId__str__)\n");   // __str__
  fprintf(m_srcFile, "    .def(\"__hash__\", VisitId__hash__)\n"); // __hash__
  fprintf(m_srcFile, "    .def(self < self)\n");                   // __lt__
  fprintf(m_srcFile, "    .def(self != self)\n");                  // __ne__
  fprintf(m_srcFile, "    .def(self == self)\n");                  // __eq__
  // Pickling
  fprintf(m_srcFile, "    .def_pickle(VisitId_pickle_suite())\n");
  fprintf(m_srcFile, "    ;\n\n");

  // Expose a ValidRefFile<HDU> class for each data structure in fits data model
  for (auto & wrapper : m_fitsWrappers) {
    wrapper->validRefFileForHDU(m_srcFile);
  }

  fprintf(m_srcFile, "\n");

  // Class TriggerFile
  fprintf(m_srcFile, "  class_<TriggerFile>(\"TriggerFile\", init<>())\n");
  // TODO: this method is deprecated and shall be removed in a future release
//  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_noObsid)\n");
  // TODO: this method is deprecated and shall be removed in a future release
//  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_dataName_noObsid)\n");
//  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile)\n");
//  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_1_opt_arg)\n");
//  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_2_opt_arg)\n");
  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_utilities_0_opt_arg)\n");
  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_utilities_1_opt_arg)\n");
  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_utilities_2_opt_arg)\n");
  fprintf(m_srcFile, "    .def(\"writeTriggerFile\", TriggerFile_writeTriggerFile_utilities_3_opt_arg)\n");
  fprintf(m_srcFile,"     .staticmethod(\"writeTriggerFile\")\n");

  fprintf(m_srcFile, "    .def(\"readTriggerFile\", TriggerFile_readTriggerFile)\n");
  fprintf(m_srcFile,"     .staticmethod(\"readTriggerFile\")\n");

  fprintf(m_srcFile, "    .def(\"initialize\", TriggerFile_initialize_3arg)\n");
  fprintf(m_srcFile, "    .def(\"initialize\", TriggerFile_initialize_4arg)\n");
  fprintf(m_srcFile,"     .staticmethod(\"initialize\")\n");

  fprintf(m_srcFile, "    .def(\"getOutputDirectory\", &TriggerFile::GetOutputDirectory)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getOutputDirectory\")\n");
  fprintf(m_srcFile, "    .def(\"setOutputDirectory\", &TriggerFile::SetOutputDirectory)\n");
  fprintf(m_srcFile,"     .staticmethod(\"setOutputDirectory\")\n");

  fprintf(m_srcFile, "    .def(\"getProcessorName\", &TriggerFile::GetProcessorName)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getProcessorName\")\n");
  fprintf(m_srcFile, "    .def(\"setProcessorName\", &TriggerFile::SetProcessorName)\n");
  fprintf(m_srcFile,"     .staticmethod(\"setProcessorName\")\n");

  fprintf(m_srcFile, "    .def(\"getPassId\", &TriggerFile::GetPassId)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getPassId\")\n");

  fprintf(m_srcFile, "    .def(\"getPrevPassId\", &TriggerFile::GetPrevPassId)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getPrevPassId\")\n");

  fprintf(m_srcFile, "    .def(\"getVisitId\", &TriggerFile::GetVisitId)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getVisitId\")\n");

  fprintf(m_srcFile, "    .def(\"getProgramType\", &TriggerFile::GetProgramType)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getProgramType\")\n");

  fprintf(m_srcFile, "    .def(\"getProgrammeType\", &TriggerFile::GetProgrammeType)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getProgrammeType\")\n");

  fprintf(m_srcFile, "    .def(\"getProgramId\", &TriggerFile::GetProgramId)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getProgramId\")\n");

  fprintf(m_srcFile, "    .def(\"getProgrammeId\", &TriggerFile::GetProgrammeId)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getProgrammeId\")\n");

  fprintf(m_srcFile, "    .def(\"getRequestId\", &TriggerFile::GetRequestId)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getRequestId\")\n");

  fprintf(m_srcFile, "    .def(\"getCreationDate\", &TriggerFile::GetCreationDate)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getCreationDate\")\n");

  fprintf(m_srcFile, "    .def(\"getVisitCounter\", &TriggerFile::GetVisitCounter)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getVisitCounter\")\n");

  fprintf(m_srcFile, "    .def(\"getValidityStart\", &TriggerFile::GetValidityStart)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getValidityStart\")\n");

  fprintf(m_srcFile, "    .def(\"getValidityStop\", &TriggerFile::GetValidityStop)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getValidityStop\")\n");

  fprintf(m_srcFile, "    .def(\"getInputProcessingNumber\", &TriggerFile::GetInputProcessingNumber)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getInputProcessingNumber\")\n");

  fprintf(m_srcFile, "    .def(\"getOutputProcessingNumber\", &TriggerFile::GetOutputProcessingNumber)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getOutputProcessingNumber\")\n");

  fprintf(m_srcFile, "    .def(\"getObsid\", &TriggerFile::GetObsid)\n");
  fprintf(m_srcFile,"     .staticmethod(\"getObsid\")\n");

  fprintf(m_srcFile, "    .def(\"empty\", &TriggerFile::Empty)\n");
  fprintf(m_srcFile,"     .staticmethod(\"empty\")\n");

  fprintf(m_srcFile, "    ;\n\n");

}


void PyFitsDataModelExtension::converters() {

  fprintf(m_srcFile, "//////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "// Data structure converters.\n");
  fprintf(m_srcFile, "//////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");
  fprintf(m_srcFile, "// Converts a std::pair instance to a Python tuple.\n");
  fprintf(m_srcFile, "template <typename T1, typename T2>\n");
  fprintf(m_srcFile, "struct std_pair_to_tuple {\n");
  fprintf(m_srcFile, "  static PyObject* convert(std::pair<T1, T2> const& p) {\n");
  fprintf(m_srcFile, "    return boost::python::incref(\n");
  fprintf(m_srcFile, "        boost::python::make_tuple(p.first, p.second).ptr());\n");
  fprintf(m_srcFile, "  }\n");
  fprintf(m_srcFile, "  static PyTypeObject const *get_pytype () {return &PyTuple_Type; }\n");
  fprintf(m_srcFile, "};\n");
  fprintf(m_srcFile, "\n");
  fprintf(m_srcFile, "// Helper for convenience.\n");
  fprintf(m_srcFile, "template <typename T1, typename T2>\n");
  fprintf(m_srcFile, "struct std_pair_to_python_converter {\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter() {\n");
  fprintf(m_srcFile, "    boost::python::to_python_converter<\n");
  fprintf(m_srcFile, "    std::pair<T1, T2>,\n");
  fprintf(m_srcFile, "    std_pair_to_tuple<T1, T2>,\n");
  fprintf(m_srcFile, "    true //std_pair_to_tuple has get_pytype\n");
  fprintf(m_srcFile, "    >();\n");
  fprintf(m_srcFile, "  }\n");
  fprintf(m_srcFile, "};\n");
  fprintf(m_srcFile, "\n");
}


void PyFitsDataModelExtension::pythonModule() {

  fprintf(m_srcFile, "//////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "// Python extension module.\n");
  fprintf(m_srcFile, "//////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");

  // https://groups.google.com/forum/#!topic/astropy-dev/6_AesAsCauM
  fprintf(m_srcFile, "#if (PY_VERSION_HEX < 0x03000000)\n");
  fprintf(m_srcFile, "void init_numpy(void) {\n");
  fprintf(m_srcFile, "  import_array();\n");
  fprintf(m_srcFile, "}\n");
  fprintf(m_srcFile, "#else\n");
  fprintf(m_srcFile, "int* init_numpy(void) {\n");
  fprintf(m_srcFile, "  import_array();\n");
  fprintf(m_srcFile, "  return NULL;\n");
  fprintf(m_srcFile, "}\n");
  fprintf(m_srcFile, "#endif\n\n");

  fprintf(m_srcFile, "BOOST_PYTHON_MODULE(%s) {\n", MODULE_NAME.c_str());
  fprintf(m_srcFile, "\n");

  fprintf(m_srcFile, "  init_numpy();\n");
  fprintf(m_srcFile, "  array::set_module_and_type(\"numpy\", \"ndarray\");\n");

  fprintf(m_srcFile, "  std_pair_to_python_converter<int8_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<uint8_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<int16_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<uint16_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<int32_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<uint32_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<int64_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<uint64_t, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<bool, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<float, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<double, bool>();\n");
  fprintf(m_srcFile, "  std_pair_to_python_converter<std::string, bool>();\n");
  fprintf(m_srcFile, "\n");

  exportFitsDal();
  exportUtility();

  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "  // Export fits data model classes.\n");
  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");

  for (auto & wrapper : m_fitsWrappers) {
    fprintf(m_srcFile, "  %s;\n", wrapper->getExportClassToPythonFunction().c_str());
  }
  fprintf(m_srcFile, "\n");

  fprintf(m_srcFile, "  /////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "  // Export global functions\n");
  fprintf(m_srcFile, "  //////////////////////////////////////////////////////////////////////////////\n");
  fprintf(m_srcFile, "\n");

  // Open_*() and Append_*() functions
  for (auto & wrapper : m_fitsWrappers) {
    wrapper->exportAssociatedHdusFunctions(m_srcFile, m_docFile);
  }
  fprintf(m_srcFile, "\n");

  // buildFileName() function
  fprintf(m_docFile, "/** ****************************************************************************\n");
  fprintf(m_docFile, " *  @fn def buildFileName(path, utc, visitId, passId, dataName, structName)\n");
  fprintf(m_docFile, " *  @ingroup FitsDataModel\n");
  fprintf(m_docFile, " *  @relates FitsDalHeader\n");
  fprintf(m_docFile, " *  @author Reiner Rohlfs UGE\n");
  fprintf(m_docFile, " *  \n");
  fprintf(m_docFile, " *  @brief Creates a CHEOPS compatible file-name, depending on the input parameters.\n");
  fprintf(m_docFile, " *  The format is:\n");
  fprintf(m_docFile, " *  path/CH_visitId_passId_utc_structName-dataName_V0000.fits\n");
  fprintf(m_docFile, " *  visitId, passId and dataName are optional and will be part of the filename\n");
  fprintf(m_docFile, " *  if their data structure is valid (visitId, passId) or not empty (dataName).\n");
  fprintf(m_docFile, " *  \n");
  fprintf(m_docFile, " *  @param path The directory of the file\n");
  fprintf(m_docFile, " *  @param utc The validity start time of the file\n");
  fprintf(m_docFile, " *  @param visitId The visit id of the file. Use VisitId() for files that do not have a visit id. \n");
  fprintf(m_docFile, " *  @param passId The pass id of the file. Use PassId() for files that do not have a pass id.\n");
  fprintf(m_docFile, " *  @param dataName The data name of the file. Use an empty string for files that do not have a dta name\n");
  fprintf(m_docFile, " *  @param structName The struct name/data type of the file.\n");
  fprintf(m_docFile, " *  @return A file path.\n");
  fprintf(m_docFile, " */\n\n");
  fprintf(m_srcFile, "  def(\"buildFileName\", buildFileName);\n\n");

  fprintf(m_srcFile, "}\n");
}

