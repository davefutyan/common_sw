/** ****************************************************************************
 *  @file
 *
 *  @ingroup ProgParam
 *  @brief Declaration of the ModuleParams class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 1.0 first released version
 *
 */

#ifndef _MODULE_PARAMS_HXX_
#define _MODULE_PARAMS_HXX_

#include <string>


class ProgramParams;

/** ****************************************************************************
 *  @ingroup ProgParam
 *  @author Reiner Rohlfs UGE
 *
 *  @brief A class to access all parameters and sub-modules of a
 *         module.
 *
 *  The only possible way to create an instance of this class is the
 *  method ProgramParams::Module(). Or if an instance of ModuleParams
 *  exists already the method ModuleParams::Module() can be used to
 *  create a ModuleParams. \n
 *  This class is useful if the values of several parameters of a
 *  sub-module should be read. The module name of this module must not
 *  be specified in the full path of the parameter name. Following two
 *  code snippets are identical and illustrate the usage of this class:
 *  <pre> ParamsPtr prog = CheopsInit(argc, argv);
 *   auto myIntVar = prog->GetAsInt("module1.param1");
 *   auto myBoolVar = prog->GetAsBool("module1.param2"); </pre>
 *  and
 *  <pre> ParamsPtr prog = CheopsInit(argc, argv);
 *   const ModuleParams module = prog->module("module1");
 *   auto myIntVar = module->GetAsInt("param1");
 *   auto myBoolVar = module->GetAsBool("param2"); </pre>
 */
class ModuleParams {


          std::string     m_path;          ///<  a dot (.) separated list of parent modules
    const ProgramParams * m_programParams; ///< the only ProgramParams instantiation in a program


   /** ************************************************************************
    *  @brief The constructor is private, as only the ProgramParams class and
    *         this class can create a new ModuleParams.
    */
   ModuleParams(const std::string & path, const ProgramParams * programParams)
       : m_path(path), m_programParams(programParams) {}

public:

   /** ************************************************************************
    *  @brief Copy constructor
    */
   ModuleParams(const ModuleParams & moduleParams)
      : m_path(moduleParams.m_path), m_programParams(moduleParams.m_programParams) {}


   /** ************************************************************************
    *  @brief Assignment operator
    */
   const ModuleParams & operator = (const ModuleParams & moduleParams)
      {m_path = moduleParams.m_path; m_programParams = moduleParams.m_programParams; return *this;}


   /** ************************************************************************
    *  @brief Creates and returns a ModuleParams to access all parameters
    *         of a sub-module.
    */
   const ModuleParams Module(const std::string & module) const;


   /** ************************************************************************
    *  @brief Returns the value of an int32_t - parameter
    */
   int32_t      GetAsInt    (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns the value of a double-parameter
    */
   double       GetAsDouble (const std::string & name) const;


   /** ************************************************************************
    *  @brief Returns the value of a bool-parameter
    */
    bool        GetAsBool   (const std::string & name) const;


    /** *************************************************************************
     *  @brief Returns the value of a string-parameter
     */
    std::string GetAsString (const std::string & name) const;


    /** *************************************************************************
     *  @brief to be able to create an instance of this ModuleParams class
     */
    friend  const ModuleParams ProgramParams::Module(const std::string & module) const;
};




#endif /* _MODULE_PARAMS_HXX_ */
