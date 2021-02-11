/** ****************************************************************************
 *  @file
 *  @ingroup FitsDataModel
 *  @author  Anja Bekkelien UGE
 *
 *  @brief   Class declarations for py_fits_data_model.
 *
 *  Contains declarations for classes used to generate Python bindings for
 *  fits_data_model.
 *
 *  @version 6.4.4 2016-02-03 ABE #12552 Add additional Doxygen documentation
 *                                       for fits header keywords and table columns
 *  @version 6.3   2016-10-24 ABE       Remove OBS_ID and add REQ_ID to the list
 *                                      of keyword names that constitutes a
 *                                      visit id
 *  @version 5.2   2016-06-07 ABE       Export method
 *                                      FitsDalHeader::WriteCurrentStatus()
 *  @version 4.4   2015-12-03 ABE #9857 Export Open_* functions to Python.
 *  @version 4.3.1 2015-11-30 ABE #9844 Access FITS images as numpy arrays in
 *                                      Python fits_data_model.
 *  @version 4.1                        Split python bindings into separate
 *                                      files for each fits data model class to
 *                                      increase performance.
 *  @version 1.0                        First released version.
 *
 */

#ifndef PYFITSDATAMODEL_HXX_
#define PYFITSDATAMODEL_HXX_

#include <memory>
#include <vector>
#include <stdio.h>

#include "Logger.hxx"
#include "ProgramParams.hxx"
#include "FitsDataModel.hxx"
#include "fits_data_model_schema.hxx"

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author  Anja Bekkelien UGE
 *
 *  @brief   This class uses XSD-to-C++ bindings generated from .fsd files to
 *           create a Python wrapper for a specific .fsd file.
 *
 *  This class contains functionality that is common to both FITS images and
 *  FITS tables, such as getter and setter methods for header keywords.
 *
 */
class PyFitsWrapper : public OutputFile {

 protected:

  FILE * m_incFile;             ///< file handle for the output include file
  FILE * m_srcFile;             ///< file handle for the output source file
  FILE * m_docFile;             ///< file handle for the Doxygen output file

  std::string m_outputCodeDir;  ///< The base directory to which code files are
                                ///< written. Source files are put in
                                ///< m_outputCodeDir/src and header files in
                                ///< m_outputCodeDir/include
  std::string m_outputDocDir;   ///< The directory to which doxygen
                                ///< documentation files are written

  std::auto_ptr<Fits_schema_type> m_fsd;  ///< pointer to the fsd

  std::string m_wrapperClassName;   ///< the class name of the class that
                                    ///< extends the fits data model class

  std::vector<std::string> m_visitIdKeywords = ///< The keywords that make up the visit ID
      { "PROGTYPE", "PROG_ID", "REQ_ID", "VISITCTR" };

  /** **************************************************************************
   *  @brief Returns the the base file name for this fits data structure, for
   *         instance "REF_APP_Limits".
   */
  std::string BaseFileName();

  /** **************************************************************************
   *  @brief Opens the three output files for this data structure: a source
   *         file, a header file and a doxygen .txt file.
   */
  void openOutputFiles();

  /** **************************************************************************
   *  @brief Closes all output files.
   */
  void closeOutputFiles();

  /** **************************************************************************
   *  @brief Writes the include file for this data structure.
   */
  void includeFile();

  /** **************************************************************************
   *  @brief Writes the source file for this data structure.
   */
  void sourceFile();

  /** **************************************************************************
   *  @brief Writes the Python Doxygen documentation for this file.
   */
  void fileDoc();

  /** **************************************************************************
   *  @brief Writes Python class documentation to the output Doxygen file.
   */
  void classDoc();

  /** **************************************************************************
   *  @brief Writes Python Doxygen documentation for the class constructor.
   */
  virtual void constructorDoc() {};

  /** **************************************************************************
   *  @brief Writes a description of a header keyword.
   *
   *  @param [in] i_key the iterator pointing to the header keyword
   */
  void keywordDescription(header_type::keyword_iterator i_key);

  /** **************************************************************************
   *  @brief Writes the getter method of one header keyword.
   *
   *  @param [in] i_key the iterator pointing to the header keyword
   */
  void getterMethod(header_type::keyword_iterator i_key);

  /** **************************************************************************
   *  @brief Writes the setter method of one header keyword.
   *
   *  @param [in] i_key the iterator pointing to the header keyword
   */
  void setterMethod(header_type::keyword_iterator i_key);

  /** *************************************************************************
   *  @brief Writes the get-method of the VisitId class
   *
   *  Creates a VisitId from the header Keywords PROGTYPE, PROG_NUM, TARG_NUM
   *  and VISITCTR and returns it
   */
  void getVisitId();

  /** *************************************************************************
   *  @brief Writes the set-method of the VisitId class.
   *
   *  Updates the header Keywords PROGTYPE, PROG_NUM, TARG_NUM and VISITCTR from
   *  a VisitId.
   */
  void setVisitId();

  /** **************************************************************************
   *  @brief Returns true if this structure has a VisitId.
   *
   *  This is the case if the following keywords are defined in the FITS HDU:
   *  - PROGTYPE,
   *  - PROG_ID,
   *  - REQ_ID and
   *  - VISITCTR.
   */
  bool hasVisitId();

  /** **************************************************************************
   *  @brief Writes the class declaration of the wrapper class that extends the
   *         fits data structure class.
   */
  virtual void wrapperClassDeclaration() {};

  /** **************************************************************************
   *  @brief Writes the class definition of the wrapper class that extends the
   *         fits data structure class.
   */
  virtual void wrapperClassDefinition() {};

  /** **************************************************************************
   *  @brief Writes the FitsDalHeader::GetFileName() method.
   */
  void getFileName();

  /** **************************************************************************
   *  @brief Writes the FitsDalHeader::WriteCurrentStatus() method.
   */
  void writeCurrentStatus();

  /** **************************************************************************
   *  @brief Writes a getter and setter method for every header keyword to the
   *  Boost Python class_.
   */
  void headerGettersSetters();

  /** **************************************************************************
   *  @brief Writes the definition of the method that exports the fits data
   *         structure class to python.
   */
  virtual void exportClassToPythonFunction();

  /** **************************************************************************
   *  @brief Writes the Boost Python class_ for this fits data structure class.
   *         Must be overridden in sub classes.
   */
  virtual void exportClass() {};

  /** ***************************************************************************
   *  @brief Writes wrapper functions for the Append_[dataStructure] and
   *         Open_[dataStructure] functions of fits data model.
   */
  void associatedHdusWrapperFunctions();

 public:

  /**
   * The prefix used for all generated files.
   */
  static const std::string FILE_NAME_PREFIX;

  /** **************************************************************************
   *  @brief Initializes the variables of the class
   *
   *  @param [in] fsd            an instance of Fits_schema_type from which to
   *                             generate Python bindings.
   *  @param [in] progParam      All program parameters, read from the program
   *                             parameter file common_sw_example.xml and from
   *                             the job - order file. (The job order file is
   *                             defined in the command line).
   *  @param [in] schemaFileName The name of an fsd file including it's path.
   *  @param [in] outputCodeDir  the directory to which code files are written.
   *                             Header files are put in the sub directory
   *                             'include', and source files in the sub
   *                             directory 'src'
   *  @param [in] outputDocDir   the directory to which Doxygen documentation is
   *                             writtenion for the generated Python code will be
   *                             written.
   */
  PyFitsWrapper(std::auto_ptr<Fits_schema_type> & fsd,
                const ParamsPtr progParam,
                const std::string & schemaFileName,
                const std::string & outputCodeDir,
                const std::string & outputDocDir);

  /** **************************************************************************
*  @brief Destructor.
   */
  virtual ~PyFitsWrapper();

  /** ***************************************************************************
   *  @brief Factory method for creating instances of subclasses of this class.
   *
   *  @param [in] fsd the fsd of the data structure
   *  @param [in] progParam      the program parameters
   *  @param [in] schemaFileName the name of the fsd file including its path
   *  @param [in] outputCodeDir  the directory to which code files are written.
   *                             Header files are put in the sub directory
   *                             'include', and source files in the sub
   *                             directory 'src'
   *  @param [in] outputDocDir   the directory to which Doxygen documentation is
   *                             written
   */
  static std::unique_ptr<PyFitsWrapper> factory(std::auto_ptr<Fits_schema_type> & fsd,
                                                const ParamsPtr progParam,
                                                const std::string & schemaFileName,
                                                const std::string & outputCodeDir,
                                                const std::string & outputDocDir);

  /** **************************************************************************
   *  @brief Creates a header file, source file and Doxygen file that exports
   *         the fits data structure to python.
   */
  void create();

  /** **************************************************************************
   *  @brief Returns the name of the function that exports the data structure to
   *         python, without return type and ().
   *
   *  @return function name, without return type and ()
   */
  std::string getExportClassToPythonFunction();

  /** **************************************************************************
   *  @brief Writes a VisitId constructor which takes the class of this data
   *         structure as argument, to the exposed Python VisitId class.
   *
   *  The output of this method is
   *  .def(init<[class name]*>())
   *  and should be included in the class_ definition of VisitId.
   *
   *  @param [in] file the file object to which the constructor is written.
   */
  void visitIdConstructorForHdu(FILE * file);

  /** **************************************************************************
   *  @brief Writes the Boost Python def(...) to export all Append_[HDU]
   *         and Open_[HDU] functions for this data structure.
   *
   *  @param [in] srcFile the source file object which the function is written
   *  @param [in] docFile the doxygen file object to which the function's
   *                      documentation is written.
   */
  void exportAssociatedHdusFunctions(FILE * srcFile, FILE * docFile);

  /**
   *  @brief Writes function pointers needed to export the Append_[HDU] functions
   *         of this data structure to python.
   *
   *  @param [in] file the file object to which the function pointers will be
   *                   written
   */
  virtual void functionPointers(FILE * file);

  /**
   *  @brief Writes the class_ declaration for a ValidRefFile<HDU> with this HDU
   *        as its template type. The declaration is only created if the
   *        fits_data_model class name starts with REF_APP or SOC_APP.
   *
   *  @param [in] file the file object to which the class_ will be written
   */
  void validRefFileForHDU(FILE * file);

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author  Anja Bekkelien UGE
 *
 *  @brief   This class uses XSD-to-C++ bindings generated from .fsd files to
 *  create a Python wrapper for a specific image .fsd file.
 *
 *  The Python wrappers for images exposes additional functionality to Python by
 *  extending the generated fits_data_model classes and defining additional methods.
 */
class PyImageWrapper : public PyFitsWrapper  {

 protected:

  /** **************************************************************************
   *  @brief Writes Doxygen documentation for the constructor of the Python
   *         class.
   */
  void constructorDoc();

  /** **************************************************************************
   *  @brief Writes the class declaration of the wrapper class that extends the
   *         fits data structure class.
   */
  void wrapperClassDeclaration();

  /** **************************************************************************
   *  @brief Writes the class definition of the wrapper class that extends the
   *         fits data structure class.
   */
  void wrapperClassDefinition();

  /** **************************************************************************
   *  @brief Writes the Boost Python class_ for this fits data structure class.
   */
  void exportClass();

  /** **************************************************************************
   *  @brief Writes the definition of the method that exports the fits data
   *         structure class to python.
   *
   *  This overridden method adds additional functionality for exporting the
   *  image data as a numpy array.
   */
  void exportClassToPythonFunction();

 public:

  /** **************************************************************************
   *  @brief Initializes the variables of the class
   *
   *  @param [in] fsd            an instance of Fits_schema_type from which to
   *                             generate Python bindings.
   *  @param [in] progParam      All program parameters, read from the program
   *                             parameter file common_sw_example.xml and from
   *                             the job - order file. (The job order file is
   *                             defined in the command line).
   *  @param [in] schemaFileName The name of an fsd file including it's path.
   *  @param [in] outputCodeDir  the directory to which code files are written.
   *                             Header files are put in the sub directory
   *                             'include', and source files in the sub
   *                             directory 'src'
   *  @param [in] outputDocDir   the directory to which Doxygen documentation is
   *                             writtenion for the generated Python code will be
   *                             written.
   */
  PyImageWrapper(std::auto_ptr<Fits_schema_type> & fsd,
                 const ParamsPtr progParam,
                 const std::string & schemaFileName,
                 const std::string & outputCodeDir,
                 const std::string & outputDocDir);

};


/** ****************************************************************************
 *  @brief   This class uses XSD-to-C++ bindings generated from .fsd files to
 *  create a Python wrapper for a specific table .fsd file.
 *
 *  @ingroup FitsDataModel
 *  @author  Anja Bekkelien UGE
 *
 */
class PyTableWrapper : public PyFitsWrapper  {

 protected:

  /** **************************************************************************
     *  @brief Writes a description of a table column.
     *
     *  @param [in] i_col the iterator of the table column
     */
  void columnDescription(table_type::column_iterator i_col);

  /** **************************************************************************
   *  @brief Writes the getter method of one table column.
   *
   *  @param [in] i_col the iterator of the table column
   */
  void getterMethod(table_type::column_iterator i_col);

  /** **************************************************************************
   *  @brief Writes the setter method of one table column.
   *
   *  @param [in] i_col the iterator of the table column
   */
  void setterMethod(table_type::column_iterator i_col);

  /** **************************************************************************
   *  @brief Writes the class declaration of the wrapper class that extends the
   *         fits data structure class.
   */
  void wrapperClassDeclaration();

  /** **************************************************************************
   *  @brief Generates code that extends the original table class and adds
   *  additional methods to it.
   */
  void wrapperClassDefinition();

  /** **************************************************************************
   *  @brief Writes Doxygen documentation for the constructor of the Python
   *         class.
   */
  void constructorDoc();

  /** **************************************************************************
   *  @brief Writes the Boost Python class_ for this fits data structure class.
   */
  void exportClass();

  /** **************************************************************************
   *  @brief Generates getters, setters, getNull and isNull methodsfor each
   *  table column.
   */
  void tableGettersSetters();

 public:

  /** **************************************************************************
   *  @brief Initializes the variables of the class
   *
   *  @param [in] fsd            an instance of Fits_schema_type from which to
   *                             generate Python bindings.
   *  @param [in] progParam      All program parameters, read from the program
   *                             parameter file common_sw_example.xml and from
   *                             the job - order file. (The job order file is
   *                             defined in the command line).
   *  @param [in] schemaFileName The name of an fsd file including it's path.
   *  @param [in] outputCodeDir  the directory to which code files are written.
   *                             Header files are put in the sub directory
   *                             'include', and source files in the sub
   *                             directory 'src'
   *  @param [in] outputDocDir   the directory to which Doxygen documentation is
   *                             writtenion for the generated Python code will be
   *                             written.
   */
  PyTableWrapper(std::auto_ptr<Fits_schema_type> & fsd,
                 const ParamsPtr progParam,
                 const std::string & schemaFileName,
                 const std::string & outputCodeDir,
                 const std::string & outputDocDir);
};


/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author  Anja Bekkelien UGE
 *
 *  @brief   This class uses XSD-to-C++ bindings generated from .fsd files to
 *           create a single Python extension module that contains all classes
 *           defined by the .fsd files.
 *
 */
class PyFitsDataModelExtension {

 private:

  /**
   * The name of the generated Python extension module
   */
  const std::string MODULE_NAME = "libpy_fits_data_model";
  /**
   *The program parameters
   */
  const ParamsPtr m_progParam;
  /**
   * The names, including the path, of the fsd files to process
   */
  std::vector<std::string> m_fsdFiles;
  /**
   * The name of the directory to which generated C++ source files will be written
   */
  std::string m_outputCodeDir;
  /**
   * The name of the directory to which generated Doxygen documentation files will be written.
   */
  std::string m_outputDocDir;
  /**
   * A handle to the output source file.
   */
  FILE * m_srcFile;
  /**
   * A handle to the output file for documentation.
   */
  FILE * m_docFile;

  /**
   * A vector containing fits wrapper objects for all input fsd files.
   */
  std::vector<std::unique_ptr<PyFitsWrapper>> m_fitsWrappers;

  /** **************************************************************************
   * @brief Writes include statements to the output file.
   */
  void includes();

  /** **************************************************************************
   * @brief Create function pointers to overloaded functions for creating FITS
   * files.
   */
  void createFunctionPointers();

  /** **************************************************************************
   * @brief Export fits_dal classes.
   */
  void exportFitsDal();

  /** **************************************************************************
   * @brief Export classes from the utility module.
   */
  void exportUtility();

  /** **************************************************************************
   * @brief Substitutes the Append_[HDU] functions defines in the
   * fits_data_model headers with ones that uses wrapper classes as function
   * argument and return type.
   */
  void appendHduWrapperFunctions();

  /** **************************************************************************
   * @brief Writes converters used to transform data structures from C++ to
   * Python or vice versa.
   */
  void converters();

  /** **************************************************************************
   *  @brief Creates the code for class wrappers that expose functionality not
   *  available in the C++ fits_data_model API.
   */
  void classWrappers();

  /** **************************************************************************
   *  @brief Creates the code for the Python extension module.
   */
  void pythonModule();

  /** **************************************************************************
   *  @brief Creates the header of a separate file containing Doxygen
   *         documentation.
   */
  void fileDoc();

 public:

 /** ***************************************************************************
  *  @brief Initializes the variables of the class
  *
  *  @param [in] progParam     All program parameters, read from the program
  *                            parameter file common_sw_example.xml and from
  *                            the job - order file. (The job order file is
  *                            defined in the command line).
  *  @param [in] fsdFiles      A list containing the file names (including the
  *                            path) of all .fsd files for which to generate
  *                            Python bindings.
  *  @param [in] outputCodeDir the directory to which code files are written.
  *                            Header files are put in the sub directory
  *                            'include', and source files in the sub
  *                            directory 'src'
  *  @param [in] outputDocDir  the directory to which Doxygen documentation is
  *                            writtenion for the generated Python code will be
  *                            written.
  */
  PyFitsDataModelExtension(const ParamsPtr progParam,
                           std::vector<std::string> fsdFiles,
                           const std::string & outputCodeDir,
                           const std::string & outputDocDir);

  /** **************************************************************************
   *  @brief Closes file handles.
   */
  ~PyFitsDataModelExtension();

  /** **************************************************************************
   *  @brief For each fsd file given to the constructor ths method creates a
   *         source, header and doxygen file that exports that data structure to
   *         Python. It also creates a main file containing the Boost Python
   *         BOOST_PYTHON_MODULE macro that creates the python extension module.
   */
  void create();
};


#endif /* PYFITSDATAMODEL_HXX_ */
