/** ****************************************************************************
 *  @file
 *  @brief  Declaration of the three class of the fits_data_model program
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 10.0.0 2018-08-03 RRO #16271: New methods to create the "copy" -
 *                                         constructor of tables.
 *  @version 6.1   2016-07-14 ABE #11166: New method for writing using
 *                                        declarations making
 *                                        FitsDalHeader::Get/SetAttr protected.
 *  @version 6.0   2016-07-08 ABE #11163: Support vector column size, 
 *                                        setter and getter methods
 *  @version 3.3   2015-05-05 RRO #8130:  Support PassId header keywords
 *  @version 3.2   2015-04-08 RRO #7717:  Support OBT, UTC, MJD and BJD
 *                                        header keywords and columns.
 *  @version 3.1   2015-01-30 RRO #7278:  New method: Associated_HDUs().
 *  @version 3.0   2015-01-20 RRO #7156:  New method: StaticMethods()
 *  @version 3.0   2014-12-22 RRO #7054:  Support of NULL values in FITS columns
 *  @version 1.3   2014-08-14 RRO #6229:  size of DATA_TYPE_STR changed fromo
 *                                        8 to 16
 *  @version 1.2   2014-07-31 RRO #6101:  Grouping of header keywords
 *  @version 1.1   2014-05-05 RRO distinguish between keyword and cell for
 *                                function names and variable names.
 *  @version 1.0   2014-03-29 RRO first version
 *
 */


#ifndef FITS_DATA_MODEL_HXX_
#define FITS_DATA_MODEL_HXX_

#include <stdio.h>

#include "CreateFitsFile.hxx"

#include "ProgramParams.hxx"

#include "fits_data_model_schema.hxx"

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Base directory of HeaderFile and SourceFile
 *
 *  The class consist of some helper methods which provides the conversion
 *  of the data structure name into file name, and class name as well as
 *  conversion from the keyword name to variable name and function name.
 */
class OutputFile
{
protected:
  	FILE *                m_file;      ///< file handler
	HDU_type &            m_hdu;       ///< the top level class of the schema
 	const ParamsPtr       m_progParam; ///< all program parameters
  	std::string           m_schemaFileName; ///< file name of input schema file


   /** *************************************************************************
    *  @brief Used to specify the type for which the functions VariableName
    *         and FunctionName are used, either for keywords or for cells
    *         of a table.
    */
  	enum NameType {
  	      KEYWORD,    ///< name defines a header keyword
  	      CELL,       ///< name is used to define a cell of a table
  	      NULL_V,     ///< name is used to define a NULL variable
  	      SIZE,       ///< name is used to define the bin size of variable
  	      VECTOR_CELL ///< name is used to define a vector cell of a table
  	      };

public:
   /** *************************************************************************
    *  @brief Initialize the varialbes.
    */
	OutputFile(HDU_type & hdu, const ParamsPtr progParam,
	           const std::string & schemaFileName);

   /** *************************************************************************
    *  @brief Returns the filename without path and without extension.
    */
	std::string   BaseFileName();

   /** *************************************************************************
    *  @brief Returns the extension name as it is defined in the fits schema.
    */
	std::string & ExtName()  {return m_hdu.extname();}

	/** *************************************************************************
    *  @brief Returns the class name. It is constructed from the extension name.
    */
	std::string   ClassName()  {return ClassName(ExtName());}

	/** *************************************************************************
	  *  @brief Returns the class name that is constructoed from @b extName.
	  */
	std::string   ClassName(const std::string & extName);

   /** *************************************************************************
    *  @brief  Returns the variable name of a keyword.
    */
	std::string   VariableName(const std::string & keyname, NameType nameType);

   /** *************************************************************************
    *  @brief Returns the function name of a keyword.
    */
	std::string   FunctionName(const std::string & keyname, NameType nameType);
};


/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief All methods to write the *.hxx header file
 */
class HeaderFile : public OutputFile
{

   void Header1Variable(header_type::keyword_iterator i_key);
   void SetMethod(header_type::keyword_iterator i_key);
   void GetMethod(header_type::keyword_iterator i_key);
   void GetVisitMethod();
   void SetVisitMethod();

public:
   /** *************************************************************************
    *  @brief Creates the output header file.
    */
	HeaderFile(HDU_type & hdu, const ParamsPtr progParam,
	           const std::string & schemaFileName);

   /** *************************************************************************
    *  @brief write the final part into the header file and closes it.
    */
	~HeaderFile();

   /** *************************************************************************
    *  @brief Writes the top of the header file.
    */
	void Top();

   /** *************************************************************************
    *  @brief Writes the top of the class definition.
    */
	void ClassTop();

   /** *************************************************************************
    *  @brief Writes the variable definition for each FITS header keyword.
    */
	void HeaderVariables();

   /** *************************************************************************
    *  @brief Write the variable of the table column.
    */
	void TableVariables();

   /** *************************************************************************
    *  @brief Write using-declarations for super class methods.
    */
	void UsingDeclarations();

   /** *************************************************************************
    *  @brief Writes the constructor and the destructor.
    */
	void ConstructorDestructor();

   /** *************************************************************************
    *  @brief Writes static methods in the header
    *
    *  Currently there is only one statix method: getExtName().
    */
	void StaticMethods();

   /** *************************************************************************
    *  @brief Writes the Get and the Set functions for each FITS header keyword.
    */
	void HeaderGeterSeter();

   /** *************************************************************************
    *  @brief Writes the Get and the Set functions for each FITS table column.
    */
	void TableGeterSeter();


   /** *************************************************************************
    *  @brief Writes the bottom of the class definition.
    */
	void ClassBottom();

  /** *************************************************************************
    *  @brief Writes the functions to create associated HDUs
    */
	void Associated_HDUs(const Fits_schema_type::List_of_Associated_HDUs_optional & hduList);

	/** *************************************************************************
    *  @brief Writes the bottom of the header file.
    */
	void Bottom();

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief All methods to write the *.cxx source file
 */
class SourceFile : public OutputFile
{

  /** *************************************************************************
   *  @brief Creates a setter method for one header keyword.
   */
   void Set1Attr(header_type::keyword_iterator & i_key);


public:
   /** *************************************************************************
    *  @brief Creates the output source file.
    */
	SourceFile(HDU_type & hdu, const ParamsPtr progParam,
         const std::string & schemaFileName);
   /** *************************************************************************
    *  @brief write the final part into the header file and closes it.
    */
	~SourceFile();

   /** *************************************************************************
    *  @brief Returns the data structure version as defined in the fsd file
    */
	std::string GetDataStructureVersion();

   /** *************************************************************************
    *  @brief Write the top part of the source file.
    */
	void Top();

   /** *************************************************************************
    *  @brief Writes the top of the image constructor.
    */
	void ImgConstructorTop();

   /** *************************************************************************
    *  @brief Writes the top of the table constructor.
    */
   void TableConstructorTop();

   /** *************************************************************************
    *  @brief Writes the top of the table constructor that copies the data
    *         from an other table.
    */
   void TableCopyConstructorTop();

   /** *************************************************************************
    *  @brief Writes the constructor with the initialization of the header
    *         keywords.
    */
   void HeaderConstructor();

   /** *************************************************************************
    *  @brief Writes the constructor with the initialization of the header
    *         keywords from the source table.
    */
   void HeaderCopyConstructor();

   /** *************************************************************************
    *  @brief Writes the constructor with the initialization of the
    *         column variable.
    */
   void TableConstructor();

   /** *************************************************************************
    *  @brief Writes the constructor with the initialization of the
    *         column variable for the "copy" - constructor
    */
   void TableCopyConstructor();

   /** *************************************************************************
    *  @brief Writes the bottom ot the constructor
    */
   void ConstructorBottom();

   /** *************************************************************************
    *  @brief Writes the destructor.
    */
	void Destructor();
};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief All possible data types of an image pixel
 */
extern const char * DATA_TYPE_STR [16];

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief All possible data types of a keyword
 */
extern const char * KEYWORD_DATA_TYPE[10];

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief All possible data types of a keyword as they appear in the FITS
 *         header.
 */
extern const char * KEYWORD_NATIVE_DATA_TYPE[10];


/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief All possible data types of a column
 */
extern const char * COLUMN_DATA_TYPE[31];



#endif /* FITS_DATA_MODEL_HXX_ */
