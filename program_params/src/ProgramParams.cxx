/** ****************************************************************************
 *  @file
 *
 *  @ingroup ProgParam
 *  @brief Implementation of the ProgramParams class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 10.2   ABE 2018-10-10 #17260 All of ProgramParams' GetAs*() methods
 *                                        now work for job order parameters
 *  @version  9.1.2 ABE 2018-03-23 #15770 log program params and file names
 *                                        after the log file stream has be created
 *  @version  9.0   ABE 2018-01-10 #15293 log input files
 *  @version  5.2   ABE 2016-05-03 #10691 log program parameters
 *  @version  5.0   ABE 2016-01-20 #9990  move CheopsExit() implementation to
 *                                        makefiles module, remove intermediate
 *                                        output dir
 *  @version  5.0   ABE 2016-01-11 #9942  allow for repeatable command line args
 *  @version  3.3   ABE 2015-05-05 #8408  store output files in temporary output dir
 *                      2015-06-08 #8408  add CheopsExit() method
 *  @version  1.1   RRO 2014-05-08        improved error message
 *  @version  1.0                         first released version
 *
 */


#include <algorithm>    // std::sort
#include <cctype>       // std::isspace
#include <iostream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "Logger.hxx"
#include "ProgramParams.hxx"

using namespace std;
using namespace boost::program_options;


/** ****************************************************************************
 *  Reads the XML parameter file and overwrites the parameter values
 *  with the values given in the command line. The input and output file names
 *  are read from the job order file. The job order filename has to be
 *  specified on the command line as a parameter and not as an option. For
 *  example: program_name job_order_file_name
 *
 *  Prints a help text
 *  with a short documentation of every parameter if the user
 *  specifies the @c -h or @c \--help option. Prints the version of
 *  the program if the user specifies the @c -v or @c \--version
 *  option. After printing the help text or the version number it
 *  terminates the program by calling exit(0);
 *
 *  @param [in] argc number of arguments in argv, should be copied
 *                   directly from the main() function.
 *  @param [in] argv list of command line parameters. Should be
 *                   copied directly from the main() function.
 *  @param [in] progName  name of the program. It is used to find the
 *                        parameter file, which has to have the file name
 *                        progName.xml.
 *  @param [in] version   the version number of the program. Will
 *                        be printed by calling the program with the
 *                        @c -v or @c \--version option.
 *
 *  @throw exception if something fails. For example:
 *    - the xml file cannot be read
 *    - the xml schema cannot be read
 *    - the xml file is not fully valid
 *    - a not defined parameter is given in the command line
 *    - a parameter value cannot be converted in its data type
 */
ProgramParams::ProgramParams(int argc, char * argv[],
                             const char * progName, const char * version)
            : m_progName(progName)
{
    options_description desc("Complete list of program parameters");

    // the parameters that are always available
    desc.add_options()
    ("help,h",    "displays this help message")
    ("version,v", "displays the version of this component")
    ("job_order", value<string>(), "filename of job order");


    ReadParamFile(progName, desc);

    // the job_order does not need a key in the command line. It is the only
    // valid parameter in the command line.
    positional_options_description noKey;
    noKey.add("job_order", -1);

    store(command_line_parser(argc, argv).options(desc).positional(noKey).run(), m_params);
    notify(m_params);

    // print help message if user defines -h or --help
    if (m_params.count("help"))
    {
        cout << desc;
        exit(0);
    }
    
    // print version number if user specifies -v or --version
    if (m_params.count("version"))
    {
       cout << argv[0] << " version " << version << endl;
       exit(0);
    }

    // read the job order file if it is define in the command line
    if (m_params.count("job_order"))
       ReadJobOrderFile(GetAsString("job_order"));
    else
       logger << warn << "No job order file defined." << endl;

}

/** ****************************************************************************
 *  Does case-insensitive string comparison, intended to be used with standard 
 *  sorting functions.
 *
 *  @param [in] first  a string to compare
 *  @param [in] second a string to compare
 *  @return the result of the comparison
 */
bool CompareStringNocase (const std::string& first, const std::string& second)
{
  unsigned int i=0;
  while ( (i<first.length()) && (i<second.length()) )
  {
    if (tolower(first[i])<tolower(second[i])) return true;
    else if (tolower(first[i])>tolower(second[i])) return false;
    ++i;
  }
  return ( first.length() < second.length() );
}

/** ****************************************************************************
 *  @param [in] externalParams optional argument for passing in additional
 *                             key/value pairs to be written to the "Program
 *                             Parameters" section of the log file in addition
 *                             to those defined in the job order, on the
 *                             command line or in the config file.
 */
void ProgramParams::LogProgramParams(const std::map<std::string, std::string> & externalParams) {

  boost::format formatString("%s=%s");
  vector<string> params;

  // Loop over command line/config file parameters
  for (auto & param : m_params) {

    // If this parameter also exists in the job order, ignore it and print the
    // value found in the job order instead. This is in line with the
    // implementation of GetAsString, in which the value found in the job order
    // takes precedence if the parameter exists both in the job order and on
    // the command line/in the config file.
    //
    // If the parameter is defined in the externalParameters, the value in
    // externalParameters takes precedence
    if (m_jobOrderParams.count(param.first) == 0 &&
    	externalParams.count(param.first) == 0) {

      if (((boost::any)param.second.value()).type() == typeid(bool)) {
        params.push_back(boost::str(formatString % param.first % param.second.as<bool>()));
      }
      else if (((boost::any)param.second.value()).type() == typeid(int)) {
        params.push_back(boost::str(formatString % param.first % param.second.as<int>()));
      }
      else if (((boost::any)param.second.value()).type() == typeid(string)) {
        params.push_back(boost::str(formatString % param.first % param.second.as<string>()));
      }
      else if (((boost::any)param.second.value()).type() == typeid(double)) {
        params.push_back(boost::str(formatString % param.first % param.second.as<double>()));
      }
      else if (((boost::any)param.second.value()).type() == typeid(std::vector<int>)) {
        for (auto & p : param.second.as<std::vector<int>>()) {
          params.push_back(boost::str(formatString % param.first % p));
        }
      }
      else if (((boost::any)param.second.value()).type() == typeid(std::vector<bool>)) {
        for (bool p : param.second.as<std::vector<bool>>()) {
          params.push_back(boost::str(formatString % param.first % p));
        }
      }
      else if (((boost::any)param.second.value()).type() == typeid(std::vector<double>)) {
        for (auto & p : param.second.as<std::vector<double>>()) {
          params.push_back(boost::str(formatString % param.first % p));
        }
      }
      else if (((boost::any)param.second.value()).type() == typeid(std::vector<string>)) {
        for (auto & p : param.second.as<std::vector<string>>()) {
          params.push_back(boost::str(formatString % param.first % p));
        }
      }
      else {
        throw runtime_error("Unknown type found for program parameter [" + param.first + "]");
      }
    }
  }

  // Loop over job order parameters
  // If a parameter is defined in the externalParameters, the value in the
  // externalParameters takes precedence.
  for (auto & param : m_jobOrderParams) {
    if(externalParams.count(param.first) == 0) {
      params.push_back(boost::str(formatString % param.first % param.second));
    }
  }

  for (auto & param : externalParams) {
    params.push_back(boost::str(formatString % param.first % param.second));
  }

  if (params.size() > 0) {
    // Sort the parameters alphabetically, ignoring case
    std::sort(params.begin(), params.end(), CompareStringNocase);
    logger << info << "Program parameters:" << std::endl;
    for (auto & param : params) {
      logger << info << "  " << param << std::endl;
    }
  }

  //
  // Log input files
  //

  // Sort the struct names alphabetically, ignoring case
  vector<string> files;
  vector<string> structNames;
  for (auto & structName : GetInputStructNames()) {
    structNames.push_back(structName);
  }
  std::sort(structNames.begin(), structNames.end(), CompareStringNocase);

  for (auto & structName : structNames) {
    logger << info << "Input files of type " << structName << ":" << std::endl;
    files.clear();

    // Sort the file names alphabetically, ignoring case
    for (auto & file : GetInputFiles(structName)) {
      files.push_back(file);
    }
    std::sort(files.begin(), files.end(), CompareStringNocase);

    for(auto & file : files) {
      // Get the file name without path since the path provided in the job order
      // is only valid during program execution
      std::string baseName = boost::filesystem::path(file).filename().string();

      // If this is a fits file, remove the name of the first extension from
      // the end of the file name
      std::size_t extStartIndex = baseName.find("[");
      if (extStartIndex != std::string::npos) {
        baseName = baseName.substr(0, extStartIndex);
      }
      logger << info << " " << baseName << std::endl;
    }
  }
}

/** ****************************************************************************
 *  The structure names in the returned list can be used as input parameter of
 *  the GetInputFiles() method to retrieve all file names of a given data
 *  structure.
 *
 *  @return List of all available input structure names in the job order file.
 */
const set<string> ProgramParams::GetInputStructNames()
{
   set<string> structNames;

   map<string, list<string> >::const_iterator i_files = m_inputFiles.begin();
   map<string, list<string> >::const_iterator i_filesEnd = m_inputFiles.end();
   while (i_files != i_filesEnd)
      {
      structNames.insert(i_files->first);
      ++i_files;
      }

   return structNames;
}


/** ****************************************************************************
 *  @throw runtime_error if the data structure is not defined in the job order.
 */
const std::list<std::string> & ProgramParams::GetInputFiles(const std::string & dataStructName)
{
   if (m_inputFiles.count(dataStructName)) {
      return m_inputFiles[dataStructName];
   }
   else {
      throw std::runtime_error("Input data structure [" + dataStructName
                               + "] is not defined in the job order file.");
   }
}

/** ****************************************************************************
 *  Casts a string value to a different type.
 *
 * @param [in] paramName    the name of the parameter. Used for logging purposes.
 * @param [in] value        the parameter's value
 *
 * @return the converted value of the parameter
 *
 * @throw runtime_error if the value could not be converted to the specified type
 */
template <class T>
T CastStringValue(const std::string & paramName, const std::string & value) {
    try {
        return boost::lexical_cast<T>(value);
    }
    catch (const boost::bad_lexical_cast &) {
        throw runtime_error ("Parameter [" + paramName +
                             "] : value [" + value +
                             "] cannot be converted to the specified type.");
    }
}

/** ****************************************************************************
 *  Converts a string value to boolean. Accepted string values are
 *  "on",  "yes", "1", "true" and
 *  "off", "no",  "0", "false".
 *
 * @param [in] paramName    the name of the parameter. Used for logging purposes.
 * @param [in] value        the parameter's value
 *
 * @return the boolean value of the parameter
 *
 * @throw runtime_error if the value could not be converted to bool.
 */
template <>
bool CastStringValue(const std::string & paramName, const std::string & value) {
    // we don't care for upper and lower case.
    // therefore we first convert everything to lower case
    string valCopy(value);
    boost::algorithm::to_lower(valCopy);

    // We accept these stings. We take the same list as the boost library.
    // boost accepts these on the command line for a bool option.
    static const vector<string> trueStr  {"on",  "yes", "1", "true"};
    static const vector<string> falseStr {"off", "no",  "0", "false"};

    if (find(trueStr.begin(), trueStr.end(), valCopy) != trueStr.end())
        return true;

    if (find(falseStr.begin(), falseStr.end(), valCopy) != falseStr.end())
      return false;

    throw runtime_error("Paramter [" + paramName +
                        "] : value = ["+ value +
                        "] cannot be converted to type bool");
}

/** ****************************************************************************
 *  Converts a string value to a vector of the specified type by splitting the
 *  string on whitespace and/or commas and casting each element. The splitting
 *  takes quotes into account, i.e. substrings enclosed within quotes are
 *  treated as single elements even if they contain the allowed separators.
 *
 * @param [in] paramName    the name of the parameter. Used for logging purposes.
 * @param [in] str          the parameter's value. Whitespace and/or commas are
 *                          allowed as separators.
 *
 * @return a vector containing the result of splitting the input string.
 *
 * @throw runtime_error if the value could not be converted.
 */
template <class T>
std::vector<T> SplitQuotedString(const std::string & paramName, const std::string & str) {
  std::vector<T> values;
  std::string word;
  bool insideQuotes = false;

  for (size_t i = 0; i < str.size(); i++) {
    char c = str[i];

    if (c == '"' || c == '\'') { insideQuotes = !insideQuotes; }
    else if (std::isspace(static_cast<unsigned char>(c)) || c == ',') {
      if (!insideQuotes) {
        if (word.size() > 0) {
          values.push_back(CastStringValue<T>(paramName, word));
          word = "";
        }
      }
      else { word += c; }
    }
    else { word += c; }
  }

  if (insideQuotes) {
    throw std::runtime_error("Parameter [" + paramName + "] : value [" +
                             "] has unclosed quotation mark.");
  }
  else {
    if (word.size() > 0) {
      values.push_back(CastStringValue<T>(paramName, word));
    }
  }

  return values;
}

/** ****************************************************************************
 *  The value of parameters with data type == @b int32_t can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *                       or if the value of the parameter cannot be
 *                       converted into a int32_t variable.
 *  @throw exception     if the parameter @b name is not of data type int32_t
 *
 *  @return    value of the parameter.
 */
int32_t ProgramParams::GetAsInt(const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
        return CastStringValue<int>(name, m_jobOrderParams.at(name));
    }
    else if (m_params.count(name))
        try {return m_params[name].as<int>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsInt'.");}
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
}

/** ****************************************************************************
 *  The value of parameters with data type == @b double can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *                       or if the value of the parameter cannot be
 *                       converted into a double variable.
 *  @throw exception     if the parameter @b name is not of data type double
 *
 *  @return    value of the parameter.
 */
double ProgramParams::GetAsDouble(const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
        return CastStringValue<double>(name, m_jobOrderParams.at(name));
    }
    else if (m_params.count(name))
        try {return m_params[name].as<double>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsDouble'.");}
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
    
}


/** ****************************************************************************
 *  The value of parameters with data type == @b bool can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.\n
 *  Following case insensitive strings are interpreted as true:\n
 *        "true", "on", "yes", "1".\n
 *  Following case insensitive strings are interpreted as false:\n
 *        "false", "off", "no", "0".\n
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *                       or if the value of the parameter cannot be
 *                       understood to be a bool variable.
 *  @throw exception     if the parameter @b name is not of data type bool
 *
 *  @return    value of the parameter.
 */
bool ProgramParams::GetAsBool(const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
        return CastStringValue<bool>(name, m_jobOrderParams.at(name));
    }
    else if (m_params.count(name))
        try {return m_params[name].as<bool>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsBool'.");}
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
}

/** ****************************************************************************
 *  The value of parameters with data type == @b string can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *  @throw exception     if the parameter @b name is not of data type string
 *
 *  @return    value of the parameter.
 */
std::string ProgramParams::GetAsString(const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
        return m_jobOrderParams.at(name);
    }
    else if (m_params.count(name)) {
        try {return m_params[name].as<string>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsString'.");}
    }
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
}

/** ************************************************************************
 *  Repeatable parameters are parameters that can be given multiple times on the
 *  command line. The values of all occurrences of a repeatable parameter are
 *  returned as a vector.
 *
 *  The value of repeatable parameters with data type == @b int32_t can be
 *  accessed with this function. Note: the data type is defined in the XML
 *  parameter file.
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  NOTE: monitor4EO does not allow that a parameter is defined multiple times
 *  in the job order. Therefore, to specify multiple values for a parameter in
 *  the job order, all values must be put into the Value element of the
 *  parameter, separated by whitespace and/or commas.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *                       or if the value of the parameter cannot be
 *                       converted into a int32_t variable.
 *  @throw exception     if the parameter @b name is not of data type int32_t
 *
 *  @return    value of the parameter.
 */
std::vector<int> ProgramParams::GetAsIntVector (const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
        return SplitQuotedString<int>(name, m_jobOrderParams.at(name));
    }
    else if (m_params.count(name))
        try {return m_params[name].as<std::vector<int>>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsIntVector'.");}
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
}

/** ************************************************************************
 *  Repeatable parameters are parameters that can be given multiple times on the
 *  command line. The values of all occurrences of a repeatable parameter are
 *  returned as a vector.
 *
 *  The value of repeatable parameters with data type == @b double can be
 *  accessed with this function. Note: the data type is defined in the XML
 *  parameter file.
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  NOTE: monitor4EO does not allow that a parameter is defined multiple times
 *  in the job order. Therefore, to specify multiple values for a parameter in
 *  the job order, all values must be put into the Value element of the
 *  parameter, separated by whitespace and/or commas.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *                       or if the value of the parameter cannot be
 *                       converted into a double variable.
 *  @throw exception     if the parameter @b name is not of data type double
 *
 *  @return    value of the parameter.
 */
std::vector<double> ProgramParams::GetAsDoubleVector (const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
        return SplitQuotedString<double>(name, m_jobOrderParams.at(name));
    }
    else if (m_params.count(name))
        try {return m_params[name].as<std::vector<double>>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsDoubleVector'.");}
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
}

/** ************************************************************************
 *  Repeatable parameters are parameters that can be given multiple times on the
 *  command line. The values of all occurrences of a repeatable parameter are
 *  returned as a vector.
 *
 *  The value of repeatable parameters with data type == @b bool can be
 *  accessed with this function. Note: the data type is defined in the XML
 *  parameter file.
 *
 *  Following case insensitive strings are interpreted as true:\n
 *        "true", "on", "yes", "1".\n
 *  Following case insensitive strings are interpreted as false:\n
 *        "false", "off", "no", "0".\n
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  NOTE: monitor4EO does not allow that a parameter is defined multiple times
 *  in the job order. Therefore, to specify multiple values for a parameter in
 *  the job order, all values must be put into the Value element of the
 *  parameter, separated by whitespace and/or commas.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *                       or if the value of the parameter cannot be
 *                       understood to be a bool variable.
 *  @throw exception     if the parameter @b name is not of data type bool
 *
 *  @return    value of the parameter.
 */
std::vector<bool> ProgramParams::GetAsBoolVector (const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
        return SplitQuotedString<bool>(name, m_jobOrderParams.at(name));
    }
    else if (m_params.count(name))
        try {return m_params[name].as<std::vector<bool>>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsBoolVector'.");}
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
}

/** ************************************************************************
 *  Repeatable parameters are parameters that can be given multiple times on the
 *  command line. The values of all occurrences of a repeatable parameter are
 *  returned as a vector.
 *
 *  The value of repeatable parameters with data type == @b string can be
 *  accessed with this function. Note: the data type is defined in the XML
 *  parameter file.
 *
 *  The method will first look for the parameter in the job order file and
 *  return it if found. Otherwise, it looks in the parameter file and command
 *  line parameters. If it is not found, an exception is thrown.
 *
 *  NOTE: monitor4EO does not allow that a parameter is defined multiple times
 *  in the job order. Therefore, to specify multiple values for a parameter in
 *  the job order, all values must be put into the Value element of the
 *  parameter, separated by whitespace and/or commas.
 *
 *  @param [in] name the full name of a parameter. If the parameter is
 *                   defined in a module or sub-module the module name
 *                   must be part of the @b name. For example:
 *                   @c "module.sub-module.parameter"
 *
 *  @throw runtime_error if the parameter @b name does not exist
 *  @throw exception     if the parameter @b name is not of data type string
 *
 *  @return    value of the parameter.
 */
std::vector<std::string> ProgramParams::GetAsStringVector (const std::string & name) const
{
    if (m_jobOrderParams.find(name) != m_jobOrderParams.end()) {
      return SplitQuotedString<std::string>(name, m_jobOrderParams.at(name));
    }
    else if (m_params.count(name))
        try {return m_params[name].as<std::vector<std::string>>();}
        catch (boost::bad_any_cast &)
            {throw runtime_error ("Parameter [" + name + "] cannot be read 'AsStringVector'.");}
    else
        throw runtime_error("Parameter [" + name + "] itself or its value is not defined.");
}


