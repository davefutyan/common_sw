/** ****************************************************************************
 *  @file
 *
 *  @ingroup ProgParam
 *  @brief Here all the functions to read the program parameter file *.xml
 *         are implemented.
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 5.0 2016-01-11 ABE #9942 allow for repeatable command line args
 *  @version 3.1 2015-02-10 RRO read program_params_schema.xsd always from
 *                              $CHEOPS_SW/resources
 *  @version 1.0 first released version
 *
 */

#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Logger.hxx"

#include "ProgramParams.hxx"
#include "program_params_schema.hxx"

using namespace boost::program_options;
using namespace boost::filesystem;
using namespace std;

/** ****************************************************************************
 * @brief Casts a string to the type defined by the template type.
 *
 * @param [in] paramName the parameter's name
 * @param [in] value     the value of parameter element
 * @param [in] valueName the name of the parameter element
 * @param [in] typeIndex the value's type
 *
 * @return The converted value.
 */
template <class T>
T CastStringValue(const string & paramName,
                         const string & value,
                         const string & valueName,
                         int   typeIndex) {

    T paramVal;
    try {
        paramVal = boost::lexical_cast<T>(value);
        return paramVal;
    }
    catch (const boost::bad_lexical_cast &) {
        throw runtime_error("Paramter [" + paramName +
                            "] : " + valueName + " = [" + value +
                            "] cannot be converted into its defined data type (" +
                            value_data_type::_xsd_value_data_type_literals_[typeIndex] + ")");
    }
}

/** ****************************************************************************
 * @brief Validates that a parameter value is within the accepted range of
 *        @b min and @b max.
 *
 * @param [in] paramName the parameter name
 * @param [in] val       the default value of the parameter
 * @param [in] min       the minimum value of the default value
 * @param [in] max       the maximum value of the default value
 * @param [in] typeIndex the type of the value
 *
* @return the parameter's default value, cast to its actual type
 *
 * @throw runtime_error if the value could not be cast to the correct type, or
 *                      if it is not within the min and max value.
 */
template <class T>
T ValidateParamValue(const string & paramName,
                       const param_type::value_optional & val,
                       const param_type::value_optional & min,
                       const param_type::value_optional & max,
                       int   typeIndex) {

  // a default value is defined in the parameter file
  T paramVal = CastStringValue<T>(paramName, val.get(), "value", typeIndex);

  if (min.present())
  {
      // a min value is defined in the parameter file
      // test the parameter value against it
      T minVal = CastStringValue<T>(paramName, min.get(), "min", typeIndex);

      if (paramVal < minVal)
          throw runtime_error("Paramter [" + paramName +
                              "] : the actual value = [" + val.get() +
                              "] is less than the accepted min value [" +
                              min.get() + "]." );
  }

  if (max.present())
  {
      // a max value is defined in the parameter file
      // test the parameter value against it
      T maxVal = CastStringValue<T>(paramName, max.get(), "max", typeIndex);

      if (paramVal > maxVal)
          throw runtime_error("Paramter [" + paramName +
              "] : the actual value = [" + val.get() +
              "] is greater than the accepted max value [" +
               max.get() + "]." );
  }

  return paramVal;
}

/** ****************************************************************************
 *  Validates a variable of data type bool and returns its value (true or false)
 *  Accepted values of a boolean variable are "on",  "yes", "1", "true" and
 *  "off", "no",  "0", "false".
 *
 * @param [in] paramName    the name of the parameter
 * @param [in] val          the parameter's value
 * @param [in] min          the minimum value of the default value
 * @param [in] max          the maximum value of the default value
 * @param [in] typeIndex    the parameter's type
 *
 * @return the boolean value of the parameter
 *
 * @throw runtime_error if the value could not be converted to bool.
 */
template <>
bool ValidateParamValue(const string & paramName,
                        const param_type::value_optional & val,
                        const param_type::value_optional & min,
                        const param_type::value_optional & max,
                        int   typeIndex)
{
    // we don't care for upper and lower case.
    // therefore we first convert everything to lower case
    string valCopy(val.get());
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
                        "] : value = ["+ val.get() +
                        "] cannot be converted into its defined data type (bool)");
}

/** ****************************************************************************
 * @brief Creates a valid boost program options semantic for a single command
 *        line parameter.
 *
 * @param [in] paramName    the name of the parameter
 * @param [in] val          the parameter's value
 * @param [in] min          the minimum value of the default value
 * @param [in] max          the maximum value of the default value
 * @param [in] isRepeatable true if the parameter can be repeated multiple time
 *                          on the command line
 * @param [in] typeIndex    the parameter's type
 *
 * @return the parameter's semantics
 */
template <class T>
value_semantic * SetSemantic(const string & paramName,
                             const param_type::value_optional & val,
                             const param_type::value_optional & min,
                             const param_type::value_optional & max,
                             bool isRepeatable,
                             int   typeIndex) {

    // if the parameter has a default value
    if (val.present()) {
        T paramVal = ValidateParamValue<T>(paramName, val, min, max, typeIndex);
        if (isRepeatable) {
            std::vector<T> defaultValues = { paramVal };

            // need to provide a textual representation of the default value to
            // default_value() when the value is a vector
            return value<std::vector<T>>()->default_value(defaultValues, boost::lexical_cast<std::string>(paramVal));
        }
        else {
            // create the semantic with correct data type and
            // with the value of the parameter as default value.
            return value<T>()->default_value(paramVal);
        }
    }
    else {
        if (isRepeatable) {
            return value<std::vector<T>>();
        }
        else {
            return value<T>();
        }
    }
}

/** ****************************************************************************
 *   Converts the attributes of one parameter form the xml file into a
 *   semantic variable and adds it to @b desc.
 *
 *   @param [in] path  The parent modules of this new parameter. May be
 *                     empty if the parameter does not belong to a module.
 *   @param [in] param Gives access to all attributes of the parameter, as
 *                     defined in the xml parameter file. It is filled by
 *                     while the xml file was parsed with the xsd library.
 *   @param [out] desc The new parameter is added to this variable.
 */
void ProgramParams::ReadParam(const string & path,
                              const param_type & param,
                              options_description & desc)
{
    // prepare name with optional short name of parameter
    string name = path + param.name();
    const param_type::short_name_optional & shortName = param.short_name();
    if (shortName.present())
        name += "," + shortName.get();
    
    // define the data type, set the default value and
    // verify it against accepted min and max value
    value_semantic * semantic;
    const param_type::value_optional & val = param.value();
    const param_type::type_optional & type = param.type();
    const param_type::min_optional & min = param.min();
    const param_type::max_optional & max = param.max();
    bool repeatable = param.repeatable().present() ?
            param.repeatable().get() : param.repeatable_default_value();

    if (!type.present() || type.get() == value_data_type::string) {
           semantic = SetSemantic<string>(name, val, min, max, repeatable,
                                          type.present() ? type.get() : value_data_type("string"));
    }
    else if (type.get() == value_data_type::int_) {
        semantic = SetSemantic<int>(name, val, min, max, repeatable, type.get());
    }
    else if (type.get() == value_data_type::double_) {
        semantic = SetSemantic<double>(name, val, min, max, repeatable, type.get());
    }
    else if (type.get() == value_data_type::bool_) {
        semantic = SetSemantic<bool>(name, val, min, max, repeatable, type.get());
    }

    // add parameter to list of parameters (desc) with optional help text
    const param_type::help_optional & help = param.help();
    if (help.present())
        desc.add_options()( name.c_str(), semantic, help.get().c_str() );
    else
        desc.add_options()( name.c_str(), semantic);
    
}

/** ****************************************************************************
 *  If first process all parameter of this module and processes than all
 *  sub-modules of this module.
 *
 *   @param [in] path   The parent modules of this new module. May be
 *                      empty if this module does not belong to a module.
 *   @param [in] module Gives access to all parameters and sub-modules of
 *                      this module, as
 *                      defined in the xml parameter file. It is filled by
 *                      while the xml file was parsed with the xsd library.
 *   @param [out] desc  All new parameter of this module and all sub-modules
 *                      are added to this variable.
 */
void ProgramParams::ReadModule(const string & path,
                               const module_type & module,
                               options_description & desc)
{

    // get and read all parameters of this module
    const module_type::param_sequence & params = module.param();
    module_type::param_const_iterator i_params = params.begin();
    while (i_params != params.end())
    {
        ReadParam(path + module.name() + ".", *i_params, desc);
        i_params++;
    }

    // get and process all sub-modules of this module
    const module_type::module_sequence & modules = module.module();
    module_type::module_const_iterator i_modules = modules.begin();
    while (i_modules != modules.end())
    {
        ReadModule(path + module.name() + ".", *i_modules, desc);
        i_modules++;
    }

}


/** ****************************************************************************
 *  First the function tries to open the parameter file in these locations:
 *  -# in the current directory
 *  -# in the conf sub-directory of the current directory
 *  -# in the ${CHOPS_SW}/conf directory.
 *
 *  Then is uses the XSD library to pars the parameter file and calls
 *  functions to store all parameters in @b desc.
 *
 *  The filename (without path) of the parameter file is programName.xml
 *
 *  @param [in] programName  It is used to define the file name of the
 *                           parameter file
 *  @param [out] desc        All parameters with their attributes are
 *                           stored in the variable.
 *
 *  @throw runtime_error if the parameter file cannot be found
 */
void ProgramParams::ReadParamFile(const string & programName,
                                  options_description & desc)
{
	// First look in the current directory. Its name has to be "program_name".xml
	string xmlFile = programName + ".xml";
    
    if (!boost::filesystem::exists(xmlFile))
    {
      // try in ./conf directory, this is useful if a test program is running
      xmlFile = "./conf/" +  programName + ".xml";
      if (!boost::filesystem::exists(xmlFile))
      {
         // try in ${CHEOPS_SW}/conf
         char * cheopsSw = getenv("CHEOPS_SW");
         if (cheopsSw == NULL)
            throw runtime_error("Environmenet variable CHEOPS_SW is not defined. "
                              " If is required to find the parameter file in "
                              "${CHEOPS_SW}/conf.");
         xmlFile = string(cheopsSw) + "/conf/" + programName + ".xml";
            if (!boost::filesystem::exists(xmlFile))
               throw runtime_error("Cannot find configuration file [" + programName +
                                 ".xml]. Neither in the current directory nor in ${CHEOPS_SW}/conf.");
      }

    }

    // get the schema file name of the program params file.
    // it has to be : $CHOEPS_SW/resources/program_params_schema.xsd
    char * cheopsSw = getenv("CHEOPS_SW");
    if (cheopsSw == NULL)
       throw runtime_error("Environment variable CHEOPS_SW is not defined. "
                           "It is used to access the file $CHEOPS_SW/resources/program_params_schema.xsd.");

    string schemaFileName(cheopsSw);
    schemaFileName += "/resources/program_params_schema.xsd";

    if (!boost::filesystem::exists(schemaFileName))
         throw runtime_error("Failed to find schema of program parameter file: " + schemaFileName);


    // parsing the xml file
    logger << debug << "Reading configuration file " << xmlFile << endl;
    
    // prepare to use schema file  $CHOEPS_SW/resources/program_params_schema.xsd
    xml_schema::properties props;
    props.no_namespace_schema_location (schemaFileName.c_str());
    props.schema_location ("http://www.w3.org/XML/1998/namespace", "xml.xsd");
    auto_ptr<program_params_type> xml;

    try {
      xml = auto_ptr<program_params_type>(program_params (xmlFile, 0, props));
    }
    // Error parsing the param file. Need to pass the error object to a stream
    // in order to get the complete error message that includes information
    // about the line on which the parse error occurred.
    catch (const xml_schema::exception& e) {
      std::stringstream message;
      message << e;
      throw runtime_error(message.str());
    }
    // get and read all parameters of first level
    const program_params_type::param_sequence & params = xml->param();
    program_params_type::param_const_iterator i_params = params.begin();
    while (i_params != params.end())
    {
        ReadParam(string(), *i_params, desc);
        i_params++;
    }
    
    
    // get and process all modules of first level
    const program_params_type::module_sequence & modules = xml->module();
    program_params_type::module_const_iterator i_modules = modules.begin();
    while (i_modules != modules.end())
    {
        ReadModule(string(), *i_modules, desc);
        i_modules++;
    }

    
}
