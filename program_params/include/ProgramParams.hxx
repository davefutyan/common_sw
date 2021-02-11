/** ****************************************************************************
 *  @file
 *
 *  @ingroup ProgParam
 *  @brief Declaration of the ProgramParams class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 10.5.1 ABE 2018-12-11 #17373 New optional argument to CheopsInit
 *                                        for turning off verbose logging
 *  @version  9.1.2 ABE 2018-03-23 #15770 log program params and file names
 *                                        after the log file stream has be created
 *  @version  5.2   ABE 2016-05-03 #10691 log program parameters
 *  @version  5.0   ABE 2016-01-20  #9990 move CheopsExit() implementation to
 *                                        makefiles module, remove intermediate
 *                                        output dir
 *  @version  5.0   ABE 2016-01-11  #9942 allow for repeatable command line args
 *  @version  3.3   ABE 2015-05-05  #8408 store output files in temporary output
 *                                        dir
 *                      2015-06-08  #8408 add CheopsExit() method
 *  @version  3.1   RRO 2015-02-10  #7280 new methods to provide data of new
 *                                        job-order file
 *  @version  1.0                         first released version
 *
 */

#ifndef _PROGRAM_PARAMS_H_
#define _PROGRAM_PARAMS_H_

#include <memory>
#include <string>
#include <list>
#include <set>
#include <map>

#include <boost/program_options.hpp>

class program_params_type;
class module_type;
class param_type;
class ModuleParams;


/** ****************************************************************************
 *  @ingroup ProgParam
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Top level interface class to access all parameters of
 *         a program
 *
 *  An application program should not create an instance of this class by
 *  itself, instead it shall call the CheopsInit() function, which returns a
 *  smart pointer of this class.\n
 *
 */
class ProgramParams  {

   std::string m_progName;  ///< Name of current program, set in the constructor

   /// All program parameters.
   boost::program_options::variables_map m_params;

   /** ************************************************************************
    *  @brief Parameters defined in the job order file.
    *
    *  The key is either the value of the Name element if the parameter comes
    *  from the dynamic processing parameters, otherwise it is the name of the
    *  element containing the value.
    */
   std::map< std::string, std::string>  m_jobOrderParams;

   /** ************************************************************************
    *  @brief All input FITS extensions for the current task as defined in the
    *         job order file.
    *
    *  The key of the map is the File_Type in the job order, which is
    *  identical to a FITS data structure name.
    *  The value of the map is a list of all FITS extensions found in the job
    *  order belonging to that File_Type, i.e. which are constructed
    *  with that FITS data structure.
    */
   std::map< std::string, std::list<std::string> > m_inputFiles;

   /** ************************************************************************
    *  @brief The output directory for the current task as defined in the
    *         job order file.
    */
   std::string  m_outDir;

   /** ************************************************************************
     *  @brief The tmp directory for the current task as defined in the
     *         job order file.
     */
   std::string  m_tmpDir;

   /** ************************************************************************
    * @brief Reads the job order file and stores the filenames in
    *        m_inputFiles and m_outputFiles
    */
   void ReadJobOrderFile(const std::string & jobOrderFileName);


   /** ************************************************************************
    * @brief Reads the parameter file and stores them in desc
    */
   void ReadParamFile(const std::string & programName,
                      boost::program_options::options_description & desc);
    

   /** ************************************************************************
    * @brief Reads the parameters of one module and stores them in desc
    */
   void ReadModule(const std::string & path,
                   const module_type & module,
                   boost::program_options::options_description & desc);


   /** ************************************************************************
    * @brief Reads one parameter form the parameter file and stores its
    *        attributes in desc
    */
   void ReadParam(const std::string & path,
                  const param_type & param,
                  boost::program_options::options_description & desc);

public:

   /** ************************************************************************
    * @brief Logs all program parameters, including default values provided by
    *        the program parameter conf file, and input files.
    */
   void LogProgramParams(const std::map<std::string, std::string> & externalParams = std::map<std::string, std::string>());

   /** ************************************************************************
    *  @brief Reads the parameter file the job order file and gets the
    *         command line parameters
    */
   ProgramParams(int argc, char * argv[],
                 const char * progName, const char * version);


   /** ************************************************************************
    *  @brief Creates and returns a ModuleParams to access all parameters
    *         of a sub-module.
    */
   const ModuleParams Module(const std::string & module) const;


   /** ************************************************************************
    *  @brief Returns the value of an int32_t-parameter
    */
   int32_t     GetAsInt    (const std::string & name) const;


   /** *************************************************************************
    *  @brief Returns the value of a double-parameter
    */
   double      GetAsDouble (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns the value of a bool-parameter
    */
   bool        GetAsBool   (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns the value of a string-parameter
    */
   std::string GetAsString (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns all values of a repeatable int-parameter
    */
   std::vector<int> GetAsIntVector (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns all values of a repeatable double-parameter
    */
   std::vector<double> GetAsDoubleVector (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns all values of a repeatable bool-parameter
    */
   std::vector<bool> GetAsBoolVector (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns all values of a repeatable string-parameter
    */
   std::vector<std::string> GetAsStringVector (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns a list of all available input structure name, found
    *         in the job order file
    */
   const std::set<std::string>   GetInputStructNames();


   /** ************************************************************************
    *  @brief Returns a list of all input file names of data structure
    *         @b dataStructName, found in the job order file
    */
   const std::list<std::string> & GetInputFiles(const std::string & dataStructName);


   /** ************************************************************************
    *  @brief Returns the output directory that shall be used to store all
    *         files, that shall be archived.
    *
    *  If the program runs successfully, these files will be archived by the
    *  Monitor4EO framework after the program has terminated.
    */
    const std::string & GetOutDir() const { return m_outDir; }


   /** ************************************************************************
    *  @brief Returns a temporary directory that should be used to store all
    *         files that can be deleted after the execution of the program.
    *
    *  If is read from the job-order file. The "File_TYPE" in the job-order
    *  file is TMP and the "File_Name_Type" is Directory. All files, stored
    *  in this directory will be deleted by the Monitor4EO framework after the
    *  execution of the program.
    */
    const std::string & GetTmpDir() const  { return m_tmpDir;}


private:

   /** ************************************************************************
    *  @brief There exist no implementation. It cannot be used
    */
   ProgramParams( const ProgramParams &);


   /** ************************************************************************
    *  @brief There exist no implementation. It cannot be used
    */
   ProgramParams & operator = (const ProgramParams &);
};


#include "ModuleParams.hxx"
#include "ProgramParamsInl.hxx"


/** ****************************************************************************
 *  @ingroup ProgParam
 *  @brief Defined for convenience.
 */
typedef std::shared_ptr<ProgramParams> ParamsPtr;


/** ****************************************************************************
 *  @ingroup ProgParam
 *  @brief Initializes various infrastructure modules using the program
 *  arguments.
 *
 *  This function shall be called as very first function in every main()
 *  function. It is automatically coded by the makefiles module, when
 *  a program is build.
 *
 *  @param [in] argc           the number of command line parameters. Should be
 *                             passed from the main() function.
 *  @param [in] argv           the command line parameters of the program.
 *                             Should be passed from the main() function.
 *  @param [in] verboseLogging optional argument defining whether or not program
 *                             parameters and input files are written to the
 *                             log. Defaults to true.
 *
 *  @return A shared pointer to an instantiation of ProgramParams. It
 *          has to be used to get the values of the program parameters
 *          and the input and output file names as they are defined in
 *          the job order file.
 */
extern ParamsPtr CheopsInit(int argc, char* argv[], bool verboseLogging=true);



/** ************************************************************************
 *  @brief Performs exit routines before program termination.
 *
 *  This function shall be called as the very last function in main().
 *
 *  By default, the function creates an output trigger file containing values
 *  read from an input trigger file defined in the job order. If no input
 *  trigger file found in the job order, the output trigger file will contain
 *  no values.
 *
 *  The output trigger file will be created in the output directory defined in
 *  the job order. If no job order was provided, it will be created in the
 *  current directory.
 *
 *  @param [in] createTriggerFile if true, an output trigger file is created, if
 *                                false, no output trigger file is created. The
 *                                default value is true.
 */
extern void CheopsExit(bool createTriggerFile = true);


#endif /* PROGRAM_PARAMS_H_ */
