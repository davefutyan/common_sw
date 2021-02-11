/** ****************************************************************************
 *   @file
 *   @brief  Implementation of the HeaderFile class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 12.0  2019-10-23 RRO #19772  Implement the decoding of TC(196,1)
 *  @version 10.0.0 2018-08-02 RRO #16271: For tables: new constructor to copy
 *                                         header keywords and all columns from
 *                                         an other table.
 *  @version 9.0.1 2018-01-26 RRO #15340: change function names to get units and
 *                                        comments of header keywords and columns
 *  @version 9.0   2017-12-18 RRO #15057: Provide unit and comments by get*methods
 *  @version 8.1   2017-10-22 RRO #14854: The parameter of setCellVector.. () is
 *                                        now a const ref argument.
 *  @version 7.3   2017-05-15 RRO #12925: remove const of returned vectors
 *  @version 6.3   2016-10-21 RRO       : remove deprecated OBS_ID
 *  @version 6.1   2016-07-29 RRO #11327: use REQ_ID keyword to fill the VisitId
 *  @version 6.1   2016-07-14 ABE #11166: New method for writing using
 *                                        declarations making
 *                                        FitsDalHeader::Get/SetAttr protected.
 *  @version 4.4   2015-12-02 RRO #9856:  New open function to open associated extensions.
 *  @version 3.3   2015-05-05 RRO #8130:  Support PassId header keywords
 *  @version 3.2   2015-04-05 RRO #7717:  read and write OBT, UTC, MJd and BJD
 *                                        keywords and columns\n
 *                 2015-04-08 RRO #7721:  The size of the axis is defined by a
 *                                        std::vector<long>
 *  @version 3.1   2015-01-30 RRO #7278:  code of the Append_* functions
 *                                        implemented. see Associated_HDUs().
 *  @version 3.0   2015-01-20 RRO #7156:  new static function: getExtName()
 *  @version 3.0   2014-12-22 RRO #7054:  Support of NULL values in FITS columns
 *  @version 1.5   2014-08-25 RRO #6280:  string type can now be A or string
 *  @version 1.4   2014-08-06 RRO #6141:  use brief description and full description
 *  @version 1.3   2014-07-31 RRO #6101:  Grouping of header keywords
 *  @version 1.2   2014-05-13 RRO Can define the size of the axis of an image
 *                                at runtime
 *  @version 1.1   2014-05-05 RRO distinguish between keyword and cell for
 *                                function names and variable names.
 *  @version 1.0   2014-03-14 RRO first version
 *
 */

#include <stdexcept>
#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "FitsDataModel.hxx"

using namespace std;

/// possible data types of images
const char * DATA_TYPE_STR [16] {
	"uint8_t",
   "uint8_t",
	"int16_t",
   "int16_t",
	"uint16_t",
   "uint16_t",
	"int32_t",
   "int32_t",
	"uint32_t",
   "uint32_t",
	"int64_t",
   "int64_t",
	"float",
   "float",
	"double",
   "double"
};

const char * KEYWORD_DATA_TYPE[10] {
     "std::string", "int32_t", "uint32_t", "double", "bool",
     "OBT", "UTC", "MJD", "BJD", "PassId"
};

const char * KEYWORD_NATIVE_DATA_TYPE[10] {
     "std::string", "int32_t", "uint32_t", "double", "bool",
     "int64_t", "std::string", "double", "double", "std::string"
};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief All possible data types of a table column
 */
const char * COLUMN_DATA_TYPE[31] {
     "std::string",
     "std::string",
     "bool",
     "bool",
     "int8_t",
     "int8_t",
     "uint8_t",
     "uint8_t",
     "int16_t",
     "int16_t",
     "uint16_t",
     "uint16_t",
     "int32_t",
     "int32_t",
     "int32_t",
     "uint32_t",
     "uint32_t",
     "uint32_t",
     "int64_t",
     "int64_t",
     "uint64_t",
     "uint64_t",
     "float",
     "float",
     "double",
     "double",
     "OBT",
     "OBT",
     "UTC",
     "MJD",
     "BJD"
};



/** ****************************************************************************
 * @param [in] hdu        Gives access to all data of the HDU tag in the
 *                        fits schema file.
 * @param [in] progParam  All program parameters.
 * @param [in] schemaFileName  File name of input schema file.
 */
HeaderFile::HeaderFile(HDU_type & hdu, const ParamsPtr progParam,
                       const std::string & schemaFileName)
	: OutputFile(hdu, progParam, schemaFileName)
{
   // the output header file name
	string filename = progParam->GetAsString("header_output_dir") +
	                  "/" + BaseFileName() + ".hxx";

	m_file = fopen(filename.c_str(), "w");

	if (m_file == NULL)
		throw runtime_error("Failed to create header file " + filename);
};

void HeaderFile::Top()
{
   // the file description

   fprintf(m_file, "/** ****************************************************************************\n");
   fprintf(m_file, " *  @file\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  @ingroup FitsDataModel\n");
   fprintf(m_file, " *  @brief Declaration of the %s class\n", ClassName().c_str());
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  @author Reiner Rohlfs UGE\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  This is an automatically created file. Do not modify it!\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  @version 10.0 2018-08-02 RRO #16271: For tables: new constructor to copy\n");
   fprintf(m_file, " *                                       header keywords and all columns from\n");
   fprintf(m_file, " *                                       an other table.\n");
   fprintf(m_file, " *  @version 6.0 2016-07-08 ABE #11163: new getter, setter and size methods for \n");
   fprintf(m_file, " *                                      vector columns\n");
   fprintf(m_file, " *  @version 4.4 2015-12-02 RRO #9856:  new open_StructName(%s * firstHdu) function\n", ClassName().c_str());
   fprintf(m_file, " *  @version 3.0 2015-01-20 RRO #7156:  new static function: getExtName()\n");
   fprintf(m_file, " *  @version 3.0 2014-12-22 RRO #7054:  Support NULL values of columns\n");
   fprintf(m_file, " *  @version 1.0            RRO         first released version\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " */\n\n");

	fprintf(m_file, "#ifndef _%s_HXX_\n", boost::to_upper_copy(BaseFileName()).c_str());
	fprintf(m_file, "#define _%s_HXX_\n", boost::to_upper_copy(BaseFileName()).c_str());
	fprintf(m_file, "\n");
	fprintf(m_file, "#include <cstdint>\n");
   fprintf(m_file, "#include <limits>\n");
   fprintf(m_file, "#include <cmath>\n");
   fprintf(m_file, "#include <string>\n");
   fprintf(m_file, "#include <vector>\n");
   fprintf(m_file, "#include <string.h>\n");
   fprintf(m_file, "\n");
   fprintf(m_file, "#include <Obt.hxx>\n");
   fprintf(m_file, "#include <Utc.hxx>\n");
   fprintf(m_file, "#include <Mjd.hxx>\n");
   fprintf(m_file, "#include <Bjd.hxx>\n");
   fprintf(m_file, "#include <VisitId.hxx>\n");
   fprintf(m_file, "#include <PassId.hxx>\n");
   fprintf(m_file, "\n");
	fprintf(m_file, "#include \"FitsDal%s.hxx\"\n", m_hdu.image().present() ? "Image" : "Table");
	fprintf(m_file, "\n");
}

void HeaderFile::ClassTop()
{

   fprintf(m_file, "/** ****************************************************************************\n");
   fprintf(m_file, " *  @ingroup FitsDataModel\n");
   fprintf(m_file, " *  @author Reiner Rohlfs UGE\n");
   fprintf(m_file, " *\n");
   if (m_hdu.brief().present()) {
      fprintf(m_file, " *  @brief %s\n", m_hdu.brief().get().c_str());
   }
   if (m_hdu.description().present()) {
      fprintf(m_file, " *\n");
      fprintf(m_file, " *  %s\n", m_hdu.description().get().c_str());
   }

   fprintf(m_file, " */\n");

	if (m_hdu.image().present())
	{
		image_data_type & dataType = m_hdu.image().get().data_type();
		const char * dataTypeStr = DATA_TYPE_STR[ (int)dataType ];

		fprintf(m_file, "class %s : public FitsDalImage<%s>\n",  ClassName().c_str(), dataTypeStr);
	}
	else
		fprintf(m_file, "class %s : public FitsDalTable\n",  ClassName().c_str());

	fprintf(m_file, "{\n");
}

/** *************************************************************************
 *  @brief Writes the definition of one variable in the header file
 *
 *  @param [in] i_key  an iterator to all data of the variable.
 */
void HeaderFile::Header1Variable(header_type::keyword_iterator i_key) {

   // the data type
   fprintf(m_file, "   %-12s", KEYWORD_DATA_TYPE[ i_key->data_type() ]);

   // the variable name
   string varName = VariableName(i_key->name(), KEYWORD);
   fprintf(m_file, "%s", varName.c_str());
   if (varName.length() < 19)
      fprintf(m_file, "%*s", (int32_t)(19 - varName.length()), " ");

   // the default value
   if (i_key->data_type() == keyword_data_type::string)
      fprintf(m_file, " {\"%s\"}; ", i_key->default_().present() ?
                                     i_key->default_().get().c_str() :
                                     "" );
   else if (i_key->data_type() == keyword_data_type::boolean)
      fprintf(m_file, " {%s}; ", i_key->default_().present() &&
                                 i_key->default_().get() == "F"   ?
                                 "false" : "true" );
   else if (i_key->data_type() == keyword_data_type::UTC)
      fprintf(m_file, " {%s}; ", i_key->default_().present()   ?
                                 i_key->default_().get().c_str() :
                                 "UTC()");
   else if (i_key->data_type() == keyword_data_type::PassId)
       fprintf(m_file, " {\"%s\"}; ", i_key->default_().present()   ?
                                  i_key->default_().get().c_str() :
                                  "");
   else
      fprintf(m_file, " {%s}; ", i_key->default_().present() ?
                                 i_key->default_().get().c_str() :
                                 "0" );

   // the comment as ///< [unit] comment
   if (i_key->comment().present() || i_key->unit().present())
      fprintf(m_file, "///< ");
   if (i_key->unit().present())
      fprintf(m_file, "[%s] ", i_key->unit().get().c_str());
   if (i_key->comment().present())
            fprintf(m_file, "%s ", i_key->comment().get().c_str());

   fprintf(m_file, "\n");

   // the unit if available
   string unit;
   if (i_key->unit().present())
      unit = i_key->unit().get();

   fprintf(m_file, "   std::string %sUnit", varName.c_str());
   if (varName.length() < 16)
      fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");
   fprintf(m_file, "{\"%s\"}; ///< unit of keyword %s\n",
                   unit.c_str(), i_key->name().c_str());

   // the comment if available
   string comment;
   if (i_key->comment().present())
      comment = i_key->comment().get();

   fprintf(m_file, "   std::string %sCom", varName.c_str());
   if (varName.length() < 17)
      fprintf(m_file, "%*s", (int32_t)(17 - varName.length()), " ");
   fprintf(m_file, "{\"%s\"}; ///< comment of keyword %s\n",
                   i_key->comment().get().c_str(), i_key->name().c_str());

   fprintf(m_file, "\n");

}

///////////////////////////////////////////////////////////////////////////////
/// Writes the variable definition for each FITS header keyword
void HeaderFile::HeaderVariables()
{
	fprintf(m_file, "protected:\n\n");

	fprintf(m_file, "   // Variables of the FITS header keywords\n\n");

	// loop over all header keywords
	header_type::keyword_iterator i_key = m_hdu.header().keyword().begin();
	header_type::keyword_iterator i_keyEnd = m_hdu.header().keyword().end();
	while (i_key != i_keyEnd)	{
		Header1Variable(i_key);
		++i_key;
	}

   //loop over all keyword groups
   header_type::group_iterator i_group = m_hdu.header().group().begin();
   header_type::group_iterator i_groupEnd = m_hdu.header().group().end();
   while (i_group != i_groupEnd) {

      // loop over all header keywords in this group
      i_key = i_group->keyword().begin();
      i_keyEnd = i_group->keyword().end();
      while (i_key != i_keyEnd) {
         Header1Variable(i_key);
         ++i_key;
      }
     ++i_group;
   }

   fprintf(m_file, "\n");
}

/** *************************************************************************
  *  @brief Writes the definition of all column variables
  */
void HeaderFile::TableVariables()
{
   fprintf(m_file, "   // Variables of the columns of the FITS table.\n\n");

   // loop over all columns
   table_type::column_iterator i_col = m_hdu.table().get().column().begin();
   table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();
   while (i_col != i_colEnd)
   {
      // the data type
      fprintf(m_file, "   %-15s", COLUMN_DATA_TYPE[ i_col->data_type() ]);

      // the variable name
      string varName = VariableName(i_col->name(), CELL);
      fprintf(m_file, "%s", varName.c_str());

      // the array size
      if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                                   i_col->data_type() != column_data_type::string)
         {
         fprintf(m_file, "[%u];", (uint32_t)i_col->bin_size());
         if (i_col->bin_size() < 10)
            fprintf(m_file, " ");
         if (i_col->bin_size() < 100)
            fprintf(m_file, " ");
         }
      else
         fprintf(m_file, ";     ");

      if (varName.length() < 16)
         fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

      // the comment as ///< comment
      fprintf(m_file, "///< %s ", i_col->comment().present() ?
                                       i_col->comment().get().c_str() :
                                       "no comment");

      fprintf(m_file, "\n");

      //the variable to store the NULL value
      if (i_col->data_type() == column_data_type::OBT ||
          i_col->data_type() == column_data_type::CUC ||
          (i_col->null().present() &&
           i_col->data_type() >= 4 && i_col->data_type() <= 19) ) {

         // the data type
          fprintf(m_file, "   %-15s", COLUMN_DATA_TYPE[ i_col->data_type() ]);

          // the variable name
          string varName = VariableName(i_col->name(), NULL_V);
          fprintf(m_file, "%s     ", varName.c_str());

          if (varName.length() < 16)
              fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

          // the null value itself
          if (i_col->data_type() == column_data_type::OBT ||
              i_col->data_type() == column_data_type::CUC ) {
             fprintf(m_file, "{%s};", i_col->null().present() ?
                                      i_col->null().get().c_str() : "-1" );
          }
          else
             fprintf(m_file, "{%s};  ", i_col->null().get().c_str());

          // the comment as ///< NULL value of column name
          fprintf(m_file, "///< NULL value of column %s ", i_col->name().c_str());

          fprintf(m_file, "\n");
      }

      // the unit
      fprintf(m_file, "   std::string    %sUnit;", varName.c_str());
      if (varName.length() < 17)
         fprintf(m_file, "%*s", (int32_t)(17 - varName.length()), " ");
      // the comment as ///< unit of column name
      fprintf(m_file, "///< unit of column %s\n\n", i_col->name().c_str());

      ++i_col;
   }
   fprintf(m_file, "\n");
}

///////////////////////////////////////////////////////////////////////////////
void HeaderFile::UsingDeclarations()
{
   fprintf(m_file, "   using FitsDalHeader::GetAttr;\n");
   fprintf(m_file, "   using FitsDalHeader::SetAttr;\n");
   fprintf(m_file, "\n");
}

///////////////////////////////////////////////////////////////////////////////
void HeaderFile::ConstructorDestructor()
{
	fprintf(m_file, "public:\n\n");

   fprintf(m_file, "   /** *************************************************************************\n");
	fprintf(m_file, "    *  @brief Constructor, initialize the variables \n");
   fprintf(m_file, "    */ \n");
	fprintf(m_file, "   %s(const std::string & filename, "
			        "const char * mode = \"READONLY\"", ClassName().c_str());
   if (m_hdu.image().present())  {
      fprintf(m_file, ",\n         std::vector<long> axisSize  = {}");
   }

	fprintf(m_file, ");\n\n");

	if (m_hdu.table().present()) {

	   fprintf(m_file, "   /** *************************************************************************\n");
	   fprintf(m_file, "    *  @brief Similar as a copy constructor, but initialise data from an other table. \n");
	   fprintf(m_file, "    *  \n");
      fprintf(m_file, "    *  Copies all header keywords and all columns from the @b sourceFilename\n");
      fprintf(m_file, "    *  if they are available in the @b sourceFilename.\n");
      fprintf(m_file, "    */ \n");
	   fprintf(m_file, "   %s(const std::string & filename, "
	                   " const std::string & sourceFilename);\n\n", ClassName().c_str());

	}

	fprintf(m_file, "   ~%s();\n", ClassName().c_str());
	fprintf(m_file, "\n");

}

void HeaderFile::StaticMethods()
{

   // function will look like
   // static std::string getExtName() {return std::string("name");}

   fprintf(m_file, "   ///Returns the name of the FITS extension.\n");
   fprintf(m_file, "   static std::string  getExtName()  {return std::string(\"%s\");}\n\n",
                   ExtName().c_str());

   fprintf(m_file, "   ///Returns the class name.\n");
   fprintf(m_file, "   static std::string  getClassName()  {return std::string(\"%s\");}\n\n",
                   ClassName().c_str());

}

/** *************************************************************************
 *  @brief Writes the set-method of one variable in the header file
 *
 *  @param [in] i_key  an iterator to all data of the variable.
 */
void HeaderFile::SetMethod(header_type::keyword_iterator i_key) {

   string varName = VariableName(i_key->name(), KEYWORD);
   string functionName = FunctionName(i_key->name(), KEYWORD);

   fprintf(m_file, "   /// Set value of FITS header attribute %s\n", i_key->name().c_str());
   //the SetFunctions looks like
   // void setName(type name) {m_name = name;}
   fprintf(m_file,"   void set%s ", functionName.c_str());
   if (functionName.length() < 16)
      fprintf(m_file, "%*s", (int32_t)(16 - functionName.length()), " ");

   fprintf(m_file,"(%-12s %s) ", KEYWORD_DATA_TYPE[ i_key->data_type() ],
                                 varName.c_str() + 2);
   if (varName.length() < 16)
      fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

   fprintf(m_file, "{%s = %s;}", varName.c_str(), varName.c_str() + 2);

   fprintf(m_file, "\n\n");
}

/** *************************************************************************
 *  @brief Writes the set-method of the VisitId class
 *
 *  Updatedes the header Keywords PROGTYPE, PROG_NUM, TARG_NUM
 *  and VISITCTR from a VisitId
 */
void HeaderFile::SetVisitMethod() {

   fprintf(m_file, "   /// Set values of FITS header attribute PROGTYPE, PROG_ID, REQ_ID and VISITCTR\n");
   //the SetFunctions looks like
   // void setKeyVisitId(const VisitId & visitId) {m_keyProgtype = visitId.getProgramType();
   //                                              m_keyProgId   = visitId.getProgramId();
   //                                              m_keyReqId    = visitId.getRequestId();
   //                                              m_keyVisitctr = visitId.getVisitCounter();}
   fprintf(m_file,"   void setKeyVisitId       (const VisitId & visitId)     {");
   fprintf(m_file,"%s = visitId.getProgramType();\n",
                   VariableName("PROGTYPE", KEYWORD).c_str());
   fprintf(m_file,"%58s %s = visitId.getProgramId();\n",
                   " ", VariableName("PROG_ID", KEYWORD).c_str());
   fprintf(m_file,"%58s %s = visitId.getRequestId();\n",
                   " ", VariableName("REQ_ID", KEYWORD).c_str());
   fprintf(m_file,"%58s %s = visitId.getVisitCounter(); }\n\n",
                   " ", VariableName("VISITCTR", KEYWORD).c_str());
}

/** *************************************************************************
 *  @brief Writes the get-method of one variable in the header file
 *
 *  @param [in] i_key  an iterator to all data of the variable.
 */
void HeaderFile::GetMethod(header_type::keyword_iterator i_key) {

   string varName = VariableName(i_key->name(), KEYWORD);
   string functionName = FunctionName(i_key->name(), KEYWORD);

   fprintf(m_file, "   /// Get value of FITS header attribute %s\n", i_key->name().c_str());

   //the GetFunctions looks like
   // type getName() const {return m_name;}
   fprintf(m_file,"         %-12s  get%s() const ", KEYWORD_DATA_TYPE[ i_key->data_type() ],
                                             functionName.c_str());
   if (functionName.length() < 18)
      fprintf(m_file, "%*s", (int32_t)(18 - functionName.length()), " ");

   fprintf(m_file," {return %s;}\n", varName.c_str());

   // the unit
   fprintf(m_file, "   /// Get the unit of FITS header attribute %s\n", i_key->name().c_str());
   fprintf(m_file, "   const std::string & getUnitOf%s() const", functionName.c_str());
   if (functionName.length() < 14)
      fprintf(m_file, "%*s", (int32_t)(14 - functionName.length()), " ");
   fprintf(m_file, "{return %sUnit;}\n", varName.c_str());

   // the comment
   fprintf(m_file, "   /// Get the comment of FITS header attribute %s\n", i_key->name().c_str());
   fprintf(m_file, "   const std::string & getComOf%s() const", functionName.c_str());
   if (functionName.length() < 15)
      fprintf(m_file, "%*s", (int32_t)(15 - functionName.length()), " ");
   fprintf(m_file, "{return %sCom;}\n", varName.c_str());


   fprintf(m_file, "\n");
}

/** *************************************************************************
 *  @brief Writes the get-method of the VisitId class
 *
 *  Creates a VisitId from the header Keywords PROGTYPE, PROG_NUM, TARG_NUM
 *  and VISITCTR and returns it
 */
void HeaderFile::GetVisitMethod() {


   fprintf(m_file, "   /// Get the VisitId defined by the header keywords PROGTYPE, PROG_ID, REQ_ID and VISITCTR\n");

   //the GetFunctions looks like
   // VisitId getKeyVisitId() const {return VisitId(m_keyProgtype, m_keyProgId, m_keyReqId, m_keyVisitctr);}
   fprintf(m_file,"   VisitId      getKeyVisitId() const ");

   fprintf(m_file,"       {return VisitId(%s, %s, %s, %s);}\n\n ",
                     VariableName("PROGTYPE", KEYWORD).c_str(),
                     VariableName("PROG_ID",  KEYWORD).c_str(),
                     VariableName("REQ_ID",   KEYWORD).c_str(),
                     VariableName("VISITCTR", KEYWORD).c_str()     );
}
///////////////////////////////////////////////////////////////////////////////
void HeaderFile::HeaderGeterSeter()
{

	fprintf(m_file, "   // Set functions for all header keywords\n\n");
	// loop over all header keywords
	header_type::keyword_iterator i_key = m_hdu.header().keyword().begin();
	header_type::keyword_iterator i_keyEnd = m_hdu.header().keyword().end();
	while (i_key != i_keyEnd)
	{
	   SetMethod(i_key);
	   ++i_key;
	}
   //loop over all keyword groups
   header_type::group_iterator i_group = m_hdu.header().group().begin();
   header_type::group_iterator i_groupEnd = m_hdu.header().group().end();
   while (i_group != i_groupEnd) {

      // loop over all header keywords in this group
      i_key = i_group->keyword().begin();
      i_keyEnd = i_group->keyword().end();
      while (i_key != i_keyEnd) {
         SetMethod(i_key);
         ++i_key;
      }
      if (i_group->description() == "Visit"          ||
          i_group->description() == "Pass and Visit"    )
         SetVisitMethod();
      ++i_group;
   }
	fprintf(m_file, "\n");

	fprintf(m_file, "   // Get functions for all header keywords\n\n");
	// loop over all header keywords
	i_key = m_hdu.header().keyword().begin();
	i_keyEnd = m_hdu.header().keyword().end();
	while (i_key != i_keyEnd)
	{
	   GetMethod(i_key);
	   ++i_key;
	}
   //loop over all keyword groups
   i_group = m_hdu.header().group().begin();
   i_groupEnd = m_hdu.header().group().end();
   while (i_group != i_groupEnd) {

      // loop over all header keywords in this group
      i_key = i_group->keyword().begin();
      i_keyEnd = i_group->keyword().end();
      while (i_key != i_keyEnd) {
         GetMethod(i_key);
         ++i_key;
      }
      if (i_group->description() == "Visit"          ||
          i_group->description() == "Pass and Visit"    )
         GetVisitMethod();
     ++i_group;
   }
	fprintf(m_file, "\n");
}

///////////////////////////////////////////////////////////////////////////////
void HeaderFile::TableGeterSeter()
{

   fprintf(m_file, "   // Set functions for all table columns\n\n");
   // loop over all columns
   table_type::column_iterator i_col = m_hdu.table().get().column().begin();
   table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();
   while (i_col != i_colEnd)
      {
      string varName = VariableName(i_col->name(), CELL);
      string functionName = FunctionName(i_col->name(), CELL);

      fprintf(m_file, "   /// Set value of FITS table column %s\n", i_col->name().c_str());
      //the SetFunctions looks like
      // void setName(type name) {m_name = name;}
      // or
      // void setName(const std::string & name) {m_name = name;}
      // or
      // void setName(type * name) {memcpy(m_name, name, size * sizeof(type));}

      fprintf(m_file,"   void set%s ", functionName.c_str());
      if (functionName.length() < 16)
         fprintf(m_file, "%*s", (int32_t)(16 - functionName.length()), " ");

      if (i_col->data_type() == column_data_type::A ||
          i_col->data_type() == column_data_type::string) {
         fprintf(m_file,"(const %-10s & %s) ", COLUMN_DATA_TYPE[ i_col->data_type() ],
                                               varName.c_str() + 2);
      }
      else {
         fprintf(m_file,"(%-16s %s %s) ", COLUMN_DATA_TYPE[ i_col->data_type() ],
            (i_col->bin_size() > 1) ? "*" : " ",  varName.c_str() + 2);
      }

      if (varName.length() < 16)
         fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");
      if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                                   i_col->data_type() != column_data_type::string)
         fprintf(m_file, "{memcpy(%s, %s, %u * sizeof(%s));}",
                           varName.c_str(), varName.c_str() + 2,
                           (uint32_t)(i_col->bin_size()), COLUMN_DATA_TYPE[ i_col->data_type() ] );
      else
         fprintf(m_file, "{%s = %s;}", varName.c_str(), varName.c_str() + 2);

      fprintf(m_file, "\n");

      // for vector columns that are not strings, add a second set function that
      // takes a vector as argument. It looks like
      // void setName(const std::vector<type> name) {
      //   std::copy(name.begin(),
      //       name.size() < size ? name.end() : name.begin()+size,
      //       m_name);}
      if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                                   i_col->data_type() != column_data_type::string) {
        functionName = FunctionName(i_col->name(), VECTOR_CELL);

        fprintf(m_file, "   /// Set value of FITS table column %s\n", i_col->name().c_str());
        fprintf(m_file,"   void set%s ", functionName.c_str());
        if (functionName.length() < 16)
           fprintf(m_file, "%*s", (int32_t)(16 - functionName.length()), " ");

        fprintf(m_file,"(const std::vector<%s> & %s) ", COLUMN_DATA_TYPE[ i_col->data_type() ],
                       varName.c_str() + 2);

        if (varName.length() < 16)
           fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

        fprintf(m_file, "{std::copy(%s.begin(), %s.size() <= %u ? %s.end() : %s.begin()+%u, %s);}",
                        varName.c_str() + 2, varName.c_str() + 2,
                        (uint32_t)(i_col->bin_size()), varName.c_str() + 2,
                        varName.c_str() + 2, (uint32_t)(i_col->bin_size()), varName.c_str());

        fprintf(m_file, "\n");
      }

      // function to set the NULL value
      // the SetNullFunctions looks like
      // void SetNullName()          {m_cellName = m_nullName;}
      // or
      // void SetNullName()          {m_cellName.clear();}
      // or
      // void SetName(int32_t index) {m_cellName[index] = n_nullName;}
      functionName = FunctionName(i_col->name(), NULL_V);

      if (i_col->data_type()  == column_data_type::A       ||
          i_col->data_type()  == column_data_type::string  ||
          i_col->data_type()  == column_data_type::E       ||
          i_col->data_type()  == column_data_type::float_  ||
          i_col->data_type()  == column_data_type::D       ||
          i_col->data_type()  == column_data_type::double_ ||
          i_col->data_type()  == column_data_type::OBT     ||
          i_col->data_type()  == column_data_type::CUC     ||
          i_col->data_type()  == column_data_type::UTC     ||
          i_col->data_type()  == column_data_type::MJD     ||
          i_col->data_type()  == column_data_type::BJD     ||
          (i_col->null().present() &&
           i_col->data_type() >= 4 && i_col->data_type() <= 19)  ) {
         fprintf(m_file, "   /// Set the NULL value in the current row of column %s\n", i_col->name().c_str());


         fprintf(m_file,"   void set%s ", functionName.c_str());
         if (functionName.length() < 16)
            fprintf(m_file, "%*s", (int32_t)(16 - functionName.length()), " ");

         if (i_col->data_type() == column_data_type::A       ||
             i_col->data_type() == column_data_type::string  ||
             i_col->bin_size() == 1 ) {
             fprintf(m_file, "() %*s", 33, " ");
         }
         else {
             fprintf(m_file, "(int32_t index) %*s", 20, " ");
         }

         fprintf(m_file, "{%s", varName.c_str());
         if (i_col->data_type() == column_data_type::A       ||
             i_col->data_type() == column_data_type::string  ||
             i_col->data_type() == column_data_type::UTC        )
            fprintf(m_file, ".clear()");
         else {
            if (i_col->bin_size() > 1)
               fprintf(m_file, "[index]");
            if (i_col->data_type() >= 20 &&
                i_col->data_type() != column_data_type::OBT &&
                i_col->data_type() != column_data_type::CUC    )
               fprintf(m_file, " = std::numeric_limits<%s>::quiet_NaN()",
                     COLUMN_DATA_TYPE[i_col->data_type()]);
            else
               fprintf(m_file, " = %s", VariableName(i_col->name(), NULL_V).c_str());
         }
         fprintf(m_file, ";}\n");
      }
      fprintf(m_file, "\n");

      ++i_col;
      }
   fprintf(m_file, "\n");

   fprintf(m_file, "   // Get functions for all table columns\n\n");
   // loop over all header columns
   i_col = m_hdu.table().get().column().begin();
   while (i_col != i_colEnd)
   {
      string varName = VariableName(i_col->name(), CELL);
      string functionName = FunctionName(i_col->name(), CELL);

      fprintf(m_file, "   /// Get value of FITS table columns %s\n", i_col->name().c_str());

      //the GetFunctions looks like
      // type getName() const {return m_name;}
      if (i_col->bin_size() > 1 || i_col->data_type() == column_data_type::A ||
                                   i_col->data_type() == column_data_type::string)
         fprintf(m_file, "   const ");
      else
         fprintf(m_file, "         ");

      fprintf(m_file,"%s " , COLUMN_DATA_TYPE[ i_col->data_type() ]);
      if (i_col->data_type() == column_data_type::A || i_col->data_type() == column_data_type::string)
         fprintf(m_file, "& ");
      else if (i_col->bin_size() > 1)
         fprintf(m_file, "* ");
      else
         fprintf(m_file, "  ");

      int type_length = strlen(COLUMN_DATA_TYPE[ i_col->data_type() ]);
      if (type_length < 14)
            fprintf(m_file, "%*s", 14 - type_length, " ");


      fprintf(m_file, "get%s() const ", functionName.c_str());
      if (functionName.length() < 16)
         fprintf(m_file, "%*s", (int32_t)(16 - functionName.length()), " ");

      fprintf(m_file," {return %s;}\n", varName.c_str());

      // for vector columns that are not strings, add a second get function that
      // returns a vector. It looks like
      // std::vector<type> getCellName() const { return std::vector<type>(m_cellName, m_cellName+size);}
      if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                                   i_col->data_type() != column_data_type::string) {
        functionName = FunctionName(i_col->name(), VECTOR_CELL);

        fprintf(m_file, "   /// Get value of FITS table column %s\n", i_col->name().c_str());
        fprintf(m_file, "   std::vector<%s> " , COLUMN_DATA_TYPE[ i_col->data_type() ]);

        fprintf(m_file, "get%s() const ", functionName.c_str());
        if (functionName.length() < 16)
           fprintf(m_file, "%*s", (int32_t)(16 - functionName.length()), " ");

        fprintf(m_file, "{ return std::vector<%s>(%s, %s+%u); }",
                        COLUMN_DATA_TYPE[ i_col->data_type() ], varName.c_str(),
                        varName.c_str(), (uint32_t)(i_col->bin_size()));

        fprintf(m_file, "\n");
      }

      // function to test the data against its NULL value
      // the isNullFunctions looks like
      // bool isNullName()          {return m_cellName == m_nullName;}
      // or
      // bool isNullName()          {return m_cellName.empty();}
      // or
      // bool isNullName(int32_t index) {return m_cellName[index] = n_nullName;}
      // or
      // bool isNullName()          {return std::isnan(m_cellName);}
      functionName = FunctionName(i_col->name(), NULL_V);

      if (i_col->data_type()  == column_data_type::A       ||
          i_col->data_type()  == column_data_type::string  ||
          i_col->data_type()  == column_data_type::E       ||
          i_col->data_type()  == column_data_type::float_  ||
          i_col->data_type()  == column_data_type::D       ||
          i_col->data_type()  == column_data_type::double_ ||
          i_col->data_type()  == column_data_type::OBT     ||
          i_col->data_type()  == column_data_type::CUC     ||
          i_col->data_type()  == column_data_type::UTC     ||
          i_col->data_type()  == column_data_type::MJD     ||
          i_col->data_type()  == column_data_type::BJD     ||
          (i_col->null().present() &&
           i_col->data_type() >= 4 && i_col->data_type() <= 19)  ) {
         fprintf(m_file, "   /// Test the value of current row in column %s against its NULL value\n", i_col->name().c_str());


         fprintf(m_file,"         bool             is%s", functionName.c_str());

         if (i_col->data_type() == column_data_type::A       ||
             i_col->data_type() == column_data_type::string  ||
             i_col->bin_size() == 1 ) {
             fprintf(m_file, "()");
             if (functionName.length() < 25)
                 fprintf(m_file, "%*s", (int32_t)(25 - functionName.length()), " ");

         }
         else {
             fprintf(m_file, "(int32_t index)");
             if (functionName.length() < 12)
                 fprintf(m_file, "%*s", (int32_t)(12 - functionName.length()), " ");

         }

         fprintf(m_file, "{return ");
         if (i_col->data_type()  == column_data_type::E       ||
             i_col->data_type()  == column_data_type::float_  ||
             i_col->data_type()  == column_data_type::D       ||
             i_col->data_type()  == column_data_type::double_ ||
             i_col->data_type()  == column_data_type::MJD     ||
             i_col->data_type()  == column_data_type::BJD          )
            fprintf(m_file, "std::isnan(" );
         fprintf(m_file, "%s", varName.c_str());

         if (i_col->data_type() == column_data_type::A       ||
             i_col->data_type() == column_data_type::string  ||
             i_col->data_type() == column_data_type::UTC         )
            fprintf(m_file, ".empty()");
         else {
            if (i_col->bin_size() > 1)
               fprintf(m_file, "[index]");
            if (i_col->data_type()  == column_data_type::E       ||
                i_col->data_type()  == column_data_type::float_  ||
                i_col->data_type()  == column_data_type::D       ||
                i_col->data_type()  == column_data_type::double_ ||
                i_col->data_type()  == column_data_type::MJD     ||
                i_col->data_type()  == column_data_type::BJD          )
               fprintf(m_file, ")");
            else
               fprintf(m_file, " == %s", VariableName(i_col->name(), NULL_V).c_str());
         }
         fprintf(m_file, ";}\n");
      }

      // for vector columns that are not strings, add a method that returns the
      // size of the vector column as defined in the fsd. The method looks like
      // type getSizeName() const  { return size; }
      if (i_col->bin_size() > 1 && i_col->data_type() != column_data_type::A &&
                                   i_col->data_type() != column_data_type::string) {
        functionName = FunctionName(i_col->name(), SIZE);

        fprintf(m_file, "   /// Get size of FITS table vector column %s\n", i_col->name().c_str());
        fprintf(m_file, "         int32_t ");

        int type_length = strlen(COLUMN_DATA_TYPE[ i_col->data_type() ]);
        if (type_length < 14)
           fprintf(m_file, "%*s", 14 - type_length, " ");

        fprintf(m_file, "get%s() const", functionName.c_str());
        if (functionName.length() < 16)
          fprintf(m_file, "%*s", (int32_t)(16 - functionName.length()), " ");
        fprintf(m_file," { return %u; }\n", (uint32_t)(i_col->bin_size()));
        fprintf(m_file, "\n");
      }

      // the GetUnit functions of the columns
      functionName = FunctionName(i_col->name(), CELL);

      fprintf(m_file, "   /// Get unit of FITS table column %s\n", i_col->name().c_str());

      // const std::string & getUnitOfCellColnameUnit() const  {return m_cellColNameUnit;}
      fprintf(m_file, "   const std::string &    getUnitOf%s() const ", functionName.c_str());
      if (functionName.length() < 11)
         fprintf(m_file, "%*s", (int32_t)(11 - functionName.length()), " ");
      fprintf(m_file, "{return %sUnit;}\n", varName.c_str());

      ++i_col;
      fprintf(m_file, "\n");
   }
   fprintf(m_file, "\n");


}

///////////////////////////////////////////////////////////////////////////////
void HeaderFile::ClassBottom()
{
	fprintf(m_file, "};\n");
}

///////////////////////////////////////////////////////////////////////////////
void HeaderFile::Associated_HDUs(const Fits_schema_type::List_of_Associated_HDUs_optional & hduList) {

   if (!hduList.present())
      // nothing to do, there is no list of associated HDUs defined.
      return;


   fprintf(m_file, "\n#include <stdexcept>\n");

   List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_hdu =
         hduList.get().Associated_HDU().begin();
   List_of_Associated_HDUs_type::Associated_HDU_const_iterator i_end =
         hduList.get().Associated_HDU().end();
   while (i_hdu != i_end) {
      string assClassName(ClassName(*i_hdu));

      fprintf(m_file, "#include \"%s.hxx\"\n\n", i_hdu->c_str());

      // the doxygen documentation
      fprintf(m_file, "   /** *************************************************************************\n");
      fprintf(m_file, "    *  @ingroup FitsDataModel\n");
      fprintf(m_file, "    *  @author Reiner Rohlfs UGE\n");
      fprintf(m_file, "    *\n");
      fprintf(m_file, "    *  @brief Creates a %s class in the same file as \n", assClassName.c_str());
      fprintf(m_file, "    *         the data of @b firstHdu of data type\n");
      fprintf(m_file, "    *         %s are stored. \n", ClassName().c_str());
      fprintf(m_file, "    *\n");
      fprintf(m_file, "    *  @param [in] firstHdu  should be the first HDU in a FITS file.\n");
      fprintf(m_file, "    *                        The new extension is create in the same FITS file.\n");
      if (i_hdu->HDU_Type() == HDU_Type_type::image)
         fprintf(m_file, "    *  @param [in] imgSize  defines the size of the image.\n");
      fprintf(m_file, "    *\n");
      fprintf(m_file, "    *  @return  A %s object. \\n \n", assClassName.c_str());
      fprintf(m_file, "    *           <b> The returned object has to be deleted by\n");
      fprintf(m_file, "    *           the application program.</b>\n");
      fprintf(m_file, "    */\n");


      // the code shall look like:
      // inline ClassName * Append_ClassName(ThisClassName * firstHdu) {
      //    ClassName * hdu = new ClassName(firstHdu->GetFileName(), "APPEND");
      //    return hdu;
      //  }

      fprintf(m_file, "   inline %s * Append_%s(%s * firstHdu",
            assClassName.c_str(),  assClassName.c_str(),
            ClassName().c_str());
      if (i_hdu->HDU_Type() == HDU_Type_type::image)
         fprintf(m_file, ", const std::vector<long> & imgSize");
      fprintf(m_file, ") {\n");

      fprintf(m_file, "      %s * hdu = new %s(firstHdu->GetFileName(), \"APPEND\"",
            assClassName.c_str(),  assClassName.c_str());
      if (i_hdu->HDU_Type() == HDU_Type_type::image)
         fprintf(m_file, ", imgSize");
      fprintf(m_file, ");\n");

      fprintf(m_file, "      return hdu;\n");
      fprintf(m_file, "   }\n\n");


      // the doxygen documentation
       fprintf(m_file, "   /** *************************************************************************\n");
       fprintf(m_file, "    *  @ingroup FitsDataModel\n");
       fprintf(m_file, "    *  @author Reiner Rohlfs UGE\n");
       fprintf(m_file, "    *\n");
       fprintf(m_file, "    *  @brief Opens a %s class in the same file as \n", assClassName.c_str());
       fprintf(m_file, "    *         the data of @b firstHdu of data type\n");
       fprintf(m_file, "    *         %s is located. \n", ClassName().c_str());
       fprintf(m_file, "    *\n");
       fprintf(m_file, "    *  @param [in] firstHdu  should be the first HDU in a FITS file.\n");
       fprintf(m_file, "    *                        The extension will be opened in the same FITS file\n");
       fprintf(m_file, "    *                        as @b firstHdu is located.\n");
       fprintf(m_file, "    *\n");
       fprintf(m_file, "    *  @return  A %s object or\n", assClassName.c_str());
       fprintf(m_file, "    *           a nullptr if the data structure does not exist in the FITS file.\\n \n");
       fprintf(m_file, "    *           <b> The returned object has to be deleted by\n");
       fprintf(m_file, "    *           the application program.</b>\n");
       fprintf(m_file, "    */\n");


       // the code shall look like:
       // inline ClassName * Open_ClassName(ThisClassName * firstHdu) {
       //    try {
       //      return new ClassName(firstHdu->GetFileName() + "[EXTNAME]");
       //    }
       //    catch (std::exception & e) {
       //    }
       //    return nullptr;
       //  }

       fprintf(m_file, "   inline %s * Open_%s(%s * firstHdu) {\n",
             assClassName.c_str(),  assClassName.c_str(),
             ClassName().c_str());

       fprintf(m_file, "      try { \n");
       fprintf(m_file, "         return new %s(firstHdu->GetFileName() + \"[%s]\");\n",
              assClassName.c_str(), i_hdu->c_str());
       fprintf(m_file, "      }\n");
       fprintf(m_file, "      catch (std::exception & e) {\n");
       fprintf(m_file, "      }\n");
       fprintf(m_file, "      return nullptr;\n");
       fprintf(m_file, "   }\n\n");

      i_hdu++;
   }

}

///////////////////////////////////////////////////////////////////////////////
void HeaderFile::Bottom()
{

	fprintf(m_file, "\n");
	fprintf(m_file, "#endif /* _%s_HXX_ */\n",  boost::to_upper_copy(BaseFileName()).c_str());
}

///////////////////////////////////////////////////////////////////////////////
HeaderFile::~HeaderFile()
{
	if (m_file)
	{
		fclose(m_file);
	}

}
