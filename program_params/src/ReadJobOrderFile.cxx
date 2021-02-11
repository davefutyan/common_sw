/** ****************************************************************************
 *  @file
 *
 *  @ingroup ProgParam
 *  @brief Here all the functions to read the job order file
 *         are implemented.
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 6.1 2016-07-26 ABE #10658 format sensing time as valid UTC time
 *  @version 5.2 2016-05-03 ABE #10691 log program parameters
 *  @version 5.0 2016-01-20 ABE #9990  remove intermediate output dir
 *  @version 4.2 2015-09-09 RRO #9178  new function: replaceEnvVar()
 *  @version 3.3 2015-06-08 ABE #8408  add CheopsExit() method
 *  @version 3.1 2015-02-10 RRO #7280  read new job-order file
 *  @version 1.1 13.08.14 RRO   #6265  log a message if there is no entry for this
 *                                     program in the job-order file.
 *  @version 1.0          RRO          first released version
 *
 */

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sstream>

#include "Logger.hxx"

#include "ProgramParams.hxx"
#include "job_order_schema.hxx"

using namespace std;


/** ****************************************************************************
 *  Replaces all environment variables in @b name by their value.
 *
 *  The environment variables has to be specified like this: ${NAME}
 *  The function is a recursive function to replace one environment
 *  variable per call.
 *
 *  @param [in] name  the string that may include environment variables, that
 *                    will be replaced by their value.
 *
 *  @return a string where all environment variables are replaced by their
 *          value. The function returns @b name if there is no environment
 *          variable used in the string of @b name.
 *
 * @throw runtime_error if an environment variable that is use in @b name is
 *                      not defined.
 *
 */
static string replaceEnvVar(std::string & name) {

   string resolvedName = name;

   string::size_type pos1 = resolvedName.find("${");
   if (pos1 == string::npos) {
      // there is not environment variable used in name we do nothing
      return resolvedName;
   }

   // find end of environment variable
   string::size_type pos2 = resolvedName.find("}", pos1);
   if (pos2 == string::npos) {
      // it is strange, the environment variable does not end. Anyhow, we
      // do nothing.
      return resolvedName;
   }

   // try to get the value of the environment variable
   string env = resolvedName.substr(pos1 + 2, pos2 - pos1 - 2);
   char * envValue = getenv(env.c_str());

   if (envValue == NULL) {
      throw runtime_error("The environment variable " + env +
             " is not defined. It is used in the job-order file in this filename / directory: " +
             name);
   }

   // replace the environment variable with its value
   resolvedName.replace(pos1, pos2-pos1 + 1, envValue);

   // call this function again to replace the next environment variable in
   // the same file name
   return replaceEnvVar(resolvedName);
}

/** ****************************************************************************
 *  Reads all input files as defined in the job order file and stores them
 *  in @b inputFiles.
 *
 *  @param [in] inputList   part of the tree generated from the job order
 *                          xml file storing the list of input files of the
 *                          current program
 *  @param [in]  process    name of the process for which the input files are
 *                          read from the job-order file. It is used only in
 *                          a warning message.
 *  @param jobOrderFileName  name of the job order file. It is used only in
 *                          a warning message.
 *  @param [out] inputFiles the filenames are stored in this map.
 *
 */
static void ReadInputFiles(const List_of_Inputs & inputList,
                           const string & process,
                           const std::string & jobOrderFileName,
                           map<string, list<string> > & inputFiles )
   {

   // loop over all input types
   List_of_Inputs::Input_const_iterator i_input = inputList.Input().begin();
   List_of_Inputs::Input_const_iterator i_inputEnd = inputList.Input().end();

   uint32_t inputCounter = 0;
   while (i_input != i_inputEnd)
      {
      // we are only interested in files on disk, i.e. File_Name_Type == Physical
      if (i_input->File_Name_Type() == File_Name_type::Physical) {

         // loop over all filename for this input type
         List_of_File_Names::File_Name_const_iterator i_file =
               i_input->List_of_File_Names().File_Name().begin();
         List_of_File_Names::File_Name_const_iterator i_fileEnd =
               i_input->List_of_File_Names().File_Name().end();

         uint32_t fileNameCounter = 0;

         if (inputFiles.count(i_input->File_Type()) == 0) {
           inputFiles[i_input->File_Type()] = std::list<std::string>();
         }

         while(i_file != i_fileEnd)
            {

            // replace all environment variables in the filename
            string dol = *i_file;
            dol = replaceEnvVar(dol);

            // append the fits extension to the file if it ends with .fits or
            // fits.gz
            if (!dol.empty() &&
                (boost::algorithm::ends_with(dol, ".fits") ||
                 boost::algorithm::ends_with(dol, ".fits.gz"))) {
               dol.append("[" + i_input->File_Type() +"]");
            }

            inputFiles[i_input->File_Type()].push_back(dol);

            ++fileNameCounter;
            ++i_file;
            }

         if (fileNameCounter != i_input->List_of_File_Names().count())
            logger << warn << "Found " << fileNameCounter << " file names of File_Type "
                           << i_input->File_Type() << " of process "
                           << process << " in job-order file " << jobOrderFileName
                           << " but expected " << uint64_t(i_input->List_of_File_Names().count())
                           << " file names." << endl;

      }

      ++inputCounter;
      i_input++;
      }
   if (inputCounter != inputList.count())
      logger << warn << "Fond " << inputCounter << " types of Inputs of process "
                     << process << " in job-order file " << jobOrderFileName
                     << " but expected " << uint64_t(inputList.count())
                     << " types of Inputs." << endl;


   }


/** ****************************************************************************
 *  Reads all output files as defined in the job order file and stores them
 *  in @b outputFiles.
 *
 *  @param [in] outputList       part of the tree generated from the job order
 *                               xml file storing the list of output files of
 *                               the current program
 *  @param [in] process          the name of the process
 *  @param [in] jobOrderFileName the name of the job order file
 *  @param [out] outDir          the OUT directory of the job order is stored in
 *                               this string.
 *  @param [out] tmpDir          the TMP  directory of the job order is stored
 *                               in this string.
 */
static void ReadOutputDirs(const List_of_Outputs & outputList,
                           const string & process,
                           const std::string & jobOrderFileName,
                           string & outDir, string & tmpDir)
{
   // loop over all output types
   List_of_Outputs::Output_const_iterator i_output = outputList.Output().begin();
   List_of_Outputs::Output_const_iterator i_outputEnd = outputList.Output().end();

   uint32_t outputCounter = 0;
   bool     foundOutDir   = false;
   bool     foundTmpDir   = false;
   while (i_output != i_outputEnd)
      {
      // we are only interested in directories, i.e. File_Name_Type == Directory
      if (i_output->File_Name_Type() == File_Name_type::Directory) {

         if (i_output->File_Type() == "OUT") {
            outDir = i_output->File_Name();
            outDir = replaceEnvVar(outDir);
            foundOutDir = true;
         }

         else if (i_output->File_Type() == "TMP") {
            tmpDir = i_output->File_Name();
            tmpDir = replaceEnvVar(tmpDir);
            foundTmpDir = true;
         }

      }

       ++outputCounter;
       i_output++;
       }
    if (outputCounter != outputList.count())
       logger << warn << "Found " << outputCounter << " types of Outputs of process "
                      << process << " in job-order file " << jobOrderFileName
                      << " but expected " << uint64_t(outputList.count())
                      << " types of Outputs." << endl;

    if (!foundOutDir)
       throw runtime_error("OUT directory for process " + process +
                           " is not defined in job-order file " + jobOrderFileName);

    if (!foundTmpDir)
       throw runtime_error("TMP directory for process " + process +
                           " is not defined in job-order file " + jobOrderFileName);

}

/** ****************************************************************************
 *  @brief Takes a time string of the format yyyymmdd_hhmmssffffff and returns
 *  it as a valid UTC time string of the fomat yyyy-mm-ddThh:mm:ss.ffffff.
 *
 *  @param [in] time a time formatted as yyyymmdd_hhmmssffffff
 *  @return the input time converted to a valid UTC time of the format
 *          yyyy-mm-ddThh:mm:ss.ffffff
 *  @throw runtime_error if the input time is not correctly formatted
 */
static std::string ConvertSensingTimeToUtc(const std::string & time) {

  // Check that the format is yyyymmdd_hhmmssffffff
  bool valid = true;
  if (time.size() != 21) {
    valid = false;
  }
  else {
    for (int i = 0; i < 21; i++) {
      if ((i == 8 && time[i] != '_') ||
          (i != 8 && !isdigit(time[i]))) {
        valid = false;
        break;
      }
    }
  }

  if (!valid) {
    throw runtime_error("The format of the UTC as string is not as expected."
                        "Expected: yyyymmdd_hhmmssffffff but found: " + time);
  }

  // Convert the time to a valid UTC time with the format
  // yyyy-mm-ddThh:mm:ss.ffffff
  std::string utcString = time.substr(0, 4) + "-"
                        + time.substr(4, 2) + "-"
                        + time.substr(6, 2) + "T"
                        + time.substr(9, 2) + ":"
                        + time.substr(11, 2) + ":"
                        + time.substr(13, 2) + "."
                        + time.substr(15, 6);

  return utcString;
}

/** ****************************************************************************
 *  Reads parameters from the job-order file's Ipf_Conf element and stores them
 *  in @b jobOrderParams.
 *
 *  @param [in] ipfConf         the Ipf_Conf element of the tree generated from
 *                              the job order xml file.
 *  @param [out] jobOrderParams the parameters of the job order are stored in
 *                              this map.
 */
static void ReadIpfConf(const Ipf_Conf & ipfConf,
                        map< string, string> & jobOrderParams) {

   jobOrderParams["Processor_Name"] = ipfConf.Processor_Name();
   jobOrderParams["Version"] = ipfConf.Version();
   jobOrderParams["Stdout_Log_Level"] = ipfConf.Stdout_Log_Level();
   jobOrderParams["Stderr_Log_Level"] = ipfConf.Stderr_Log_Level();
   jobOrderParams["Test"] = ipfConf.Test() ? "1" : "0";
   jobOrderParams["Breakpoint_Enable"] = ipfConf.Breakpoint_Enable() ? "1" : "0";
   jobOrderParams["Processing_Station"] = ipfConf.Processing_Station();

   if (ipfConf.Sensing_Time().present()) {

       jobOrderParams["Start"] = ConvertSensingTimeToUtc(ipfConf.Sensing_Time().get().Start());
       jobOrderParams["Stop"] =  ConvertSensingTimeToUtc(ipfConf.Sensing_Time().get().Stop());

   }

   if (ipfConf.Dynamic_Processing_Parameters().present()) {

     Dynamic_Processing_Parameters::Processing_Parameter_const_iterator i_param =
           ipfConf.Dynamic_Processing_Parameters().get().Processing_Parameter().begin();
     Dynamic_Processing_Parameters::Processing_Parameter_const_iterator i_paramEnd =
           ipfConf.Dynamic_Processing_Parameters().get().Processing_Parameter().end();

     while (i_param != i_paramEnd) {
        jobOrderParams[i_param->Name()] = i_param->Value();
        ++i_param;
     }
   }
}


/** ****************************************************************************
 *  Parses the job order file, using the xsd library and looks for the section
 *  where the input and output files for this program are defined. The
 *  program name is a parameter to the constructor of ProgramParams. Only
 *  the files specified for this program are read and stored in m_inputFiles
 *  and m_outputFiles.
 *
 *  @param jobOrderFileName   name of the job order file
 *
 *  @throw runtime_error if the job order file does not exist
 *
 */
void ProgramParams::ReadJobOrderFile(const std::string & jobOrderFileName)
{

   if (!boost::filesystem::exists(jobOrderFileName))
      throw runtime_error("Failed to find job order file " + jobOrderFileName);

   // get the schema file name of the job-order file.
   // it has to be : $CHOEPS_SW/resources/job_order_schema.xsd
   char * cheopsSw = getenv("CHEOPS_SW");
   if (cheopsSw == NULL)
      throw runtime_error("Environment variable CHEOPS_SW is not defined. "
                          "It is used to access the file $CHEOPS_SW/resources/job_order_schema.xsd.");

   string schemaFileName(cheopsSw);
   schemaFileName += "/resources/job_order_schema.xsd";

   if (!boost::filesystem::exists(schemaFileName))
        throw runtime_error("Failed to find schema of job order file: " + schemaFileName);

   logger << info << "Reading the job order file " << jobOrderFileName << endl;

   // prepare to use schema file  $CHOEPS_SW/resources/job_order_schema.xsd
   xml_schema::properties props;
   props.no_namespace_schema_location (schemaFileName.c_str());
   props.schema_location ("http://www.w3.org/XML/1998/namespace", "xml.xsd");
   auto_ptr<Ipf_Job_Order> jobOrder;

   try {
      jobOrder = auto_ptr<Ipf_Job_Order>(Ipf_Job_Order_ (jobOrderFileName, 0, props));
   }
   // Error parsing the job order file. Need to pass the error object to a stream
   // in order to get the complete error message that includes information about
   // the line on which the parse error occurred.
   catch (const xml_schema::exception& e) {
     std::stringstream message;
     message << e;
     throw runtime_error(message.str());
   }

   const List_of_Ipf_Procs & procs = jobOrder->List_of_Ipf_Procs();

   List_of_Ipf_Procs::Ipf_Proc_const_iterator i_procs = procs.Ipf_Proc().begin();
   List_of_Ipf_Procs::Ipf_Proc_const_iterator i_procsEnd = procs.Ipf_Proc().end();
   bool procFound = false;
   uint32_t numProcesses = 0;
   while (i_procs != i_procsEnd)
      {
      if (m_progName == i_procs->Task_Name())
         {
         ReadInputFiles(i_procs->List_of_Inputs (), i_procs->Task_Name(),
                        jobOrderFileName, m_inputFiles);

         ReadOutputDirs(i_procs->List_of_Outputs(), i_procs->Task_Name(),
                        jobOrderFileName, m_outDir, m_tmpDir);
         procFound = true;
         }

      ++numProcesses;
      ++i_procs;
      }

   if (!procFound)
      logger << warn << "There is no entry for program " << m_progName <<
                        " in the job order file " << jobOrderFileName << endl;
   if (numProcesses != procs.count())
      logger << warn << "Fond data for " << numProcesses << " processes in job-order file "
                     << jobOrderFileName << ", but expected data for " << uint64_t(procs.count())
                     << " processes." << endl;

   ReadIpfConf(jobOrder->Ipf_Conf(), m_jobOrderParams);

 }



