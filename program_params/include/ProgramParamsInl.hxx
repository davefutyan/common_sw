/** ****************************************************************************
 *  @file
 *
 *  @ingroup ProgParam
 *  @brief Some inline methods of the classes ProgramParams and ModuleParams
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 1.0 first released version
 *
 */
#ifndef _PROGRAM_PARAMS_INL_HXX_
#define _PROGRAM_PARAMS_INL_HXX_


/** ****************************************************************************
 *  The parameters (and sub-modules) of @b module can be accessed with
 *  the returned ModuleParams without specifying the @b module name in
 *  its path. See also Detailed Description of ModuleParams.
 *
 *  @param [in] module  name of the module.
 */
inline const ModuleParams ProgramParams::Module(const std::string & module) const
        { return ModuleParams(module, this); }


/** ****************************************************************************
 *  The parameters (and sub-modules) of @b module can be accessed with
 *  the returned ModuleParams without specifying the @b module name in
 *  its path. See also Detailed Description of ModuleParams.
 *
 *  @param [in] module  name of the sub-module.
 */
inline const ModuleParams ModuleParams::Module(const std::string & module) const
      { return ModuleParams(m_path + "." + module, m_programParams); }


/** ****************************************************************************
 *  The value of parameters with data type == @b int32_t can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.
 *
 *  @param [in] name the partial name of a parameter. If the parameter is
 *                   defined in a sub-module of this module the sub-module
 *                   name must be part of the @b name. For example:
 *                   @c "sub-module.parameter". But the name of this
 *                   module must be omitted.
 *
 *  @throw runtime_error if the full parameter @b name (including the
 *                       module name of this module) does not exist
 *                       or if the value of the parameter cannot be
 *                       converted into a int variable.
 *  @throw exception     if the parameter @b name is not of data type int32_t
 *
 *  @return    value of the parameter.
 */
inline int32_t ModuleParams::GetAsInt (const std::string & name) const
                       {return m_programParams->GetAsInt(m_path + "." + name);}


/** ****************************************************************************
 *  The value of parameters with data type == @b double can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.
 *
 *  @param [in] name the partial name of a parameter. If the parameter is
 *                   defined in a sub-module of this module the sub-module
 *                   name must be part of the @b name. For example:
 *                   @c "sub-module.parameter". But the name of this
 *                   module must be omitted.
 *
 *  @throw runtime_error if the full parameter @b name (including the
 *                       module name of this module) does not exist
 *                       or if the value of the parameter cannot be
 *                       converted into a double variable.
 *  @throw exception     if the parameter @b name is not of data type double
 *
 *  @return    value of the parameter.
 */
inline double ModuleParams::GetAsDouble (const std::string & name) const
                              {return m_programParams->GetAsDouble(m_path + "." + name);}


/** *****************************************************************************
 *  The value of parameters with data type == @b bool can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.\n
 *  Following case insensitive strings are interpreted as true:\n
 *        "true", "on", "yes", "1".\n
 *  Following case insensitive strings are interpreted as false:\n
 *        "false", "off", "no", "0".
 *
 *  @param [in] name the partial name of a parameter. If the parameter is
 *                   defined in a sub-module of this module the sub-module
 *                   name must be part of the @b name. For example:
 *                   @c "sub-module.parameter". But the name of this
 *                   module must be omitted.
 *
 *  @throw runtime_error if the full parameter @b name (including the
 *                       module name of this module) does not exist
 *                       or if the value of the parameter cannot be
 *                       understood to be a bool variable.
 *  @throw exception     if the parameter @b name is not of data type bool
 *
 *  @return    value of the parameter.
 */
inline bool  ModuleParams::GetAsBool(const std::string & name) const
                    {return m_programParams->GetAsBool(m_path + "." + name);}


/** ****************************************************************************
 *  The value of parameters with data type == @b string can be accessed with
 *  this function. Note: the data type is defined in the XML parameter file.
 *
 *  @param [in] name the partial name of a parameter. If the parameter is
 *                   defined in a sub-module of this module the sub-module
 *                   name must be part of the @b name. For example:
 *                   @c "sub-module.parameter". But the name of this
 *                   module must be omitted.
 *
 *  @throw runtime_error if the full parameter @b name (including the
 *                       module name of this module) does not exist
 *  @throw exception     if the parameter @b name is not of data type string
 *
 *  @return    value of the parameter.
 */
inline std::string ModuleParams::GetAsString (const std::string & name) const
                           {return m_programParams->GetAsString(m_path + "." + name);}


#endif /* _PROGRAM_PARAMS_INL_HXX_ */
