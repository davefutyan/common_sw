/** ****************************************************************************
    @file
    @brief  Implementation of the OutputFile class

    @author Reiner Rohlfs UGE

 *  @version 6.0   2016-07-08 ABE #11163: Support vector column size, 
 *                                        setter and getter methods
 *  @version 3.0   2014-12-22 RRO #7054:  Support of NULL values in FITS columns
 *  @version 1.0   2014-03-29 RRO first version
 *  @version 1.1   2014-05-05 RRO distinguish between keyword and cell for
                                  function names and variable names.

*/

#include <locale>

#include <boost/filesystem.hpp>

#include "FitsDataModel.hxx"

using namespace boost::filesystem;
using namespace std;


OutputFile::OutputFile(HDU_type & hdu, const ParamsPtr  progParam,
                        const std::string & schemaFileName)
   :  m_hdu(hdu), m_progParam (progParam), m_schemaFileName(schemaFileName)
{
   m_file = NULL;
}

string  OutputFile::BaseFileName()
{
   // the input fits schema file name
   string fsd( m_progParam->GetAsString("input") );
   // file name without path and without extension
   return path(fsd).stem().replace_extension("").string();
}

string OutputFile::ClassName(const std::string & extName)
{
   std::locale loc;
   string className;
   bool   newWord = true;


   string::const_iterator i_extName = extName.begin();
   string::const_iterator i_end     = extName.end();
   while (i_extName != i_end)
      {
      if (*i_extName == '-' || *i_extName == '_')
         newWord = true;
      else
         {
         if (newWord)
            className += std::toupper(*i_extName, loc);
         else
            className += std::tolower(*i_extName, loc);
         newWord = false;
         }
      ++i_extName;
      }


   return className;
}

/** *************************************************************************
  *  The @b keyname is modified to create a valid variable name, which
  *  follows the coding guide.
  *
  *  @param [in] keyname   the keyword name of a header card or the column
  *                        name as defined in the fsd file.
  *  @param [in] nameType  KEYWORD:  the variable name with start with
  *                        m_key\n
  *                        CELL: the variable name will start with
  *                        m_cell
  *                        NULL_V: the variable name will start with m_null
  */
string  OutputFile::VariableName(const string & keyname, NameType nameType)
{
   std::locale loc;

   string variableName("m_");
   if      (nameType == KEYWORD)   variableName += "key";
   else if (nameType == CELL)      variableName += "cell";
   else if (nameType == NULL_V)    variableName += "null";
   bool   newWord = true;

   string::const_iterator i_keyName = keyname.begin();
   string::const_iterator i_end     = keyname.end();
   while (i_keyName != i_end)
      {
      if (*i_keyName == '-' || *i_keyName == '_')
         newWord = true;
      else
         {
         if (newWord)
            variableName += toupper(*i_keyName, loc);
         else
            variableName += tolower(*i_keyName, loc);
         newWord = false;
         }
      ++i_keyName;
      }


   return variableName;
}

/** *************************************************************************
  *  The @b keyname is modified to create a valid function name, which
  *  follows the coding guide.
  *
  *  @param [in] keyname   the keyword name of a header card or the column
  *                        name as defined in the fsd file.
  *  @param [in] nameType  KEYWORD:  the function name with start with Key\n
  *                        CELL: the function name will start with Cell
  *                        NULL_V: the function name will start with Null
  *                        SIZE: the function name will start with Size
  *                        VECTOR: the function name will start with CellVector
  */
string  OutputFile::FunctionName(const string & keyname, NameType nameType)
{
   std::locale loc;

   string functionName;
   if (nameType == KEYWORD)     { functionName = "Key"; }
   else if (nameType == CELL)   { functionName = "Cell"; }
   else if (nameType == NULL_V) { functionName = "Null"; }
   else if (nameType == SIZE)   { functionName = "Size"; }
   else if (nameType == VECTOR_CELL) { functionName = "CellVector"; }

   bool   newWord = true;

   string::const_iterator i_keyName = keyname.begin();
   string::const_iterator i_end     = keyname.end();
   while (i_keyName != i_end)
      {
      if (*i_keyName == '-' || *i_keyName == '_')
         newWord = true;
      else
         {
         if (newWord)
            functionName += toupper(*i_keyName, loc);
         else
            functionName += tolower(*i_keyName, loc);
         newWord = false;
         }
      ++i_keyName;
      }


   return functionName;
}
