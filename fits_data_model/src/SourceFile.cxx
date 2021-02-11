/** ****************************************************************************
 *  @file
 *  @brief  Implementation of the SourceFile class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 12.0   2019-10-23 RRO #19772  Implement the decoding of TC(196,1)
 *  @version 10.3   2018-10-26 RRO #17343  Do not copy the header keyword EXT_VER
 *                                         while a new table is created from an
 *                                         existing one.
 *  @version 10.0.0 2018-08-02 RRO #16271: For tables: new constructor to copy
 *                                         header keywords and all columns from
 *                                         an other table.
 *  @version 9.0   2017-12-18  RRO #15057: Provide unit and comments by get*methods
 *  @version 3.4   2015-05-11  RRO UTC string has 6 digits after the comma,
 *                                 i.e. in total 36 digits.
 *  @version 3.3   2015-05-05  RRO #8130: Support PassId header keywords
 *  @version 3.2   2015-04-05  RRO #7717: read and write OBT, UTC, MJd and BJD
 *                                        keywords and columns.\n
 *                 2015-04-08  RRO #7721: The size of the axis is defined by a
 *                                        std::vector<long>
 *  @version 3.0   2014-12-22  RRO #7054: Support of NULL values in FITS columns\n
 *           3.0   2015-01-05  RRO #7057: Column variables are initialized
 *                                        with the NULL values or with 0.
 *  @version 1.6   2014-11-24  RRO #6947: keyword values were not read since
 *                                        grouping was implemented.
 *  @version 1.5   2014-08-25  RRO #6280: string type can now be A or string
 *  @version 1.4   2014-07-31  RRO #6101: Grouping of header keywords
 *  @version 1.3   2014-07-30  RRO #????: verify the data structure version
 *  @version 1.2   2014-05-13  RRO Can define the size of the axis of an image
 *                                 at runtime
 *  @version 1.1   2014-05-05  RRO distinguish between keyword and cell for
 *                                 function names and variable names.
 *  @version 1.0   2014-03-29  RRO first version
*/

#include <stdexcept>
#include <iostream>
#include <string>

#include "FitsDataModel.hxx"

using namespace std;



/** ****************************************************************************
 * @param [in] hdu        Gives access to all data of the HDU tag in the
 *                        fits schema file.
 * @param [in] progParam  All program parameters.
 * @param [in] schemaFileName  File name of input schema file.
 */
SourceFile::SourceFile(HDU_type & hdu, const ParamsPtr progParam,
                       const std::string & schemaFileName)
	: OutputFile(hdu, progParam, schemaFileName)
{
   // the output source file name
     string filename = progParam->GetAsString("source_output_dir") +
                       "/" + BaseFileName() + ".cxx";

	m_file = fopen(filename.c_str(), "w");

	if (m_file == NULL)
		throw runtime_error("Failed to create source file " + filename);
};

/** *************************************************************************
 *  Looks for the header keyword "EXT_VER" and returns the default value
 *  as defined in the fsd file.
 *  The functions returns an empty string, if the keyword EXT_VER is not
 *  defined in the fsd file or if no default value is defined.
 */
string SourceFile::GetDataStructureVersion()
{
   header_type::keyword_iterator i_key = m_hdu.header().keyword().begin();
   header_type::keyword_iterator i_keyEnd = m_hdu.header().keyword().end();

   while (i_key != i_keyEnd) {
      if (i_key->name() == "EXT_VER")  {
         return i_key->default_().present() ? i_key->default_().get() : string();
         }
      ++i_key;
   }
   return string();
}


void SourceFile::Top()
{
   fprintf(m_file, "/** ****************************************************************************\n");
   fprintf(m_file, " *  @file\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  @ingroup FitsDataModel\n");
   fprintf(m_file, " *  @brief Implementation of the %s class\n", ClassName().c_str());
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  @author Reiner Rohlfs UGE\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  This is an automatically created file. Do not modify it!\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " *  @version 10.0 2018-08-02 RRO #16271: For tables: new constructor to copy\n");
   fprintf(m_file, " *                                       header keywords and all columns from\n");
   fprintf(m_file, " *                                       an other table.\n");
   fprintf(m_file, " *  @version 6.0 2016-07-08 ABE #11163: fill vector column members with NULL values\n");
   fprintf(m_file, " *  @version 3.0 2014-12-22 RRO #7054:  Support NULL values of columns\n");
   fprintf(m_file, " *  @version 1.0            RRO         first released version\n");
   fprintf(m_file, " *\n");
   fprintf(m_file, " */\n\n");


   fprintf(m_file, "#include <string>\n");
   fprintf(m_file, "#include <string.h>\n");
	fprintf(m_file, "#include \"%s.hxx\"\n\n", BaseFileName().c_str()   );
   fprintf(m_file, "using namespace std;\n\n");
}

/** ****************************************************************************
 *  @brief Throws a runtime_error about an not defined dimension in the
 *         FITS schema file *.fsd
 */
void DefError(int numDim, int notDef)
{
	throw runtime_error("Image is defined with " + to_string(numDim) +
			            " dimensions. But the size of axis number " +
			            to_string(notDef) + " is not defined.");
}

static bool AxisSizeDefined(int32_t axisIndex, image_type & img) {
   switch (axisIndex) {
   case 0 : return img.axis1().present();break;
   case 1 : return img.axis2().present();break;
   case 2 : return img.axis3().present();break;
   case 3 : return img.axis4().present();break;
   case 4 : return img.axis5().present();break;
   case 5 : return img.axis6().present();break;
   case 6 : return img.axis7().present();break;
   case 7 : return img.axis8().present();break;
   case 8 : return img.axis9().present();break;
   default: return false;
   }
}

static uint32_t AxisSize(int32_t axisIndex, image_type & img) {
   switch (axisIndex) {
   case 0 : return (uint32_t)img.axis1().get();break;
   case 1 : return (uint32_t)img.axis2().get();break;
   case 2 : return (uint32_t)img.axis3().get();break;
   case 3 : return (uint32_t)img.axis4().get();break;
   case 4 : return (uint32_t)img.axis5().get();break;
   case 5 : return (uint32_t)img.axis6().get();break;
   case 6 : return (uint32_t)img.axis7().get();break;
   case 7 : return (uint32_t)img.axis8().get();break;
   case 8 : return (uint32_t)img.axis9().get();break;
   default: return 0;
   }
}

/// definition of the initializer_list
void DimSize(FILE * file, image_type & img)  {

   fprintf(file, "                        {");

   for (int32_t dim = 0; dim < img.naxis(); dim++) {
      if (!AxisSizeDefined(dim, img))
         DefError(img.naxis(), dim + 1);
      if (dim > 0)
         fprintf(file, ",\n                         ");
      uint32_t predefinedSize = AxisSize(dim, img);
      if (predefinedSize == 0) {
         fprintf(file, "axisSize.size() > %d ? axisSize[%d] : 0",
                  dim, dim);
      }
      else {
         fprintf(file, "%u", predefinedSize);
      }
  }


   fprintf(file, "}   ");

}

static void AxisSizeDocumentation(FILE * file, image_type & img) {

   for (int32_t dim = 0; dim < img.naxis(); dim++) {
      fprintf(file, " *                       @b axisSize[%d] ", dim);
      if (AxisSizeDefined(dim, img) && AxisSize(dim, img) != 0 )
         fprintf(file, "is not uses as size of this axis is defined in the .fsd file.\\n\n");
      else
         fprintf(file, "defines the size of AXIS-%d\\n\n", dim + 1);
   }

}

void SourceFile::ImgConstructorTop()
{

	image_type & img = m_hdu.image().get();


	// constructor
   fprintf(m_file, "/** ****************************************************************************\n");
   fprintf(m_file, " *  @param [in] filename file of be created / opened \n");
   fprintf(m_file, " *  @param [in] mode     can be READONLY, CREATE or APPEND\n");
   fprintf(m_file, " *  @param [in] axisSize array of values, defining the size of axes, for\n");
   fprintf(m_file, " *                       which the size is not defined in the fsd file.\n");
   fprintf(m_file, " *                       For example: {200, 200} or {sizeA, sizeB}.\n");
   fprintf(m_file, " *                       Parameter is used only if @b mode is either\n");
   fprintf(m_file, " *                       CREATE or APPEND.\\n\n");
   AxisSizeDocumentation(m_file, img);
   fprintf(m_file, " */ \n");
	fprintf(m_file, "%s::%s(const std::string & filename, "
			        "const char * mode,\n", ClassName().c_str(), ClassName().c_str());
	fprintf(m_file, "         std::vector<long> axisSize)\n");

	image_data_type & dataType = img.data_type();
	const char * dataTypeStr = DATA_TYPE_STR[ (int)dataType ];

	// the initialization of the FitsDaLImage
	//  : FitsDalImage<T>(filename, mode
	fprintf(m_file, "   : FitsDalImage<%s>(filename, mode,\n" ,dataTypeStr);

	// prepare the initializer list
	DimSize(m_file, img);


	fprintf(m_file, " )\n" );

	fprintf(m_file, "{\n");
}


void SourceFile::TableConstructorTop()
{
   // constructor
   fprintf(m_file, "/** ****************************************************************************\n");
   fprintf(m_file, " *  @param [in] filename file of be created / opened \n");
   fprintf(m_file, " *  @param [in] mode     can be READONLY, CREATE or APPEND\n");
   fprintf(m_file, " */ \n");

   fprintf(m_file, "%s::%s(const std::string & filename, "
                 "const char * mode)\n", ClassName().c_str(), ClassName().c_str());

   // the initialization of the FitsDaLTable
   //  : FitsDalTable(filename, mode)
   fprintf(m_file, "   : FitsDalTable(filename, mode)\n" );

   fprintf(m_file, "{\n");

}
void SourceFile::TableCopyConstructorTop()
{
   // constructor
   fprintf(m_file, "\n\n/** ****************************************************************************\n");
   fprintf(m_file, " *  @param [in] filename    file of be created / opened \n");
   fprintf(m_file, " *  @param [in] sourceFilename all data are copied from this table\n");
   fprintf(m_file, " */ \n");

   fprintf(m_file, "%s::%s(const std::string & filename, "
                   " const std::string & sourceFilename)\n\n",
                   ClassName().c_str(), ClassName().c_str());

   // the initialization of the FitsDaLTable
   //  : FitsDalTable(filename, "CREATE")
   fprintf(m_file, "   : FitsDalTable(filename, \"CREATE\")\n" );

   fprintf(m_file, "{\n\n");

   fprintf(m_file, "   FitsDalTable * sourceTable = new FitsDalTable(sourceFilename);\n\n");

}


void SourceFile::HeaderConstructor()
{

	fprintf(m_file, "   if (strcmp(mode, \"READONLY\") == 0)\n");
	fprintf(m_file, "   {\n");

	// Check the version of the data structure
	fprintf(m_file, "      // compare the data structure version used to create this source code\n");
	fprintf(m_file, "      // with the version of the FITS extension.\n");
	fprintf(m_file, "      CompareDataStructureVersion(\"%s\", \"%s\");\n",
	                         GetDataStructureVersion().c_str(), ExtName().c_str());
	fprintf(m_file, "\n");

   // do not check the EXTNAMA if we have opened the primary array

	fprintf(m_file, "      int hduNum;\n");
	fprintf(m_file, "      if (fits_get_hdu_num(m_fitsFile, &hduNum) > 1)\n");
	fprintf(m_file, "      {\n");

	fprintf(m_file, "         // throw an exception if the EXTNAME in the FITS file is not identical\n");
	fprintf(m_file, "         // with the name of the data structure of this class\n");
	fprintf(m_file, "         if (GetAttr<string>(\"EXTNAME\") != \"%s\" )\n",  ExtName().c_str());
   fprintf(m_file, "            throw runtime_error(\"Expected extension named %s, but opened \" + \n", ExtName().c_str());
   fprintf(m_file, "                    GetAttr<string>(\"EXTNAME\") + \" in file \" + GetFileName());\n");
   fprintf(m_file, "      }\n\n");

   // loop over all header keywords
   fprintf(m_file, "      // we accept if the attribute does not exist any more\n");
   fprintf(m_file, "      // and catch the exception.\n\n");

   header_type::keyword_iterator i_key = m_hdu.header().keyword().begin();
   header_type::keyword_iterator i_keyEnd = m_hdu.header().keyword().end();
   while (i_key != i_keyEnd)
   {
      // try { variable = GetAttr<dataType>("KeyName");}
      // catch(exception) {}
      std::string varName = VariableName(i_key->name(), KEYWORD);

      fprintf(m_file, "      try { %s = GetAttr<%s>(\"%s\"",
            varName.c_str(),
            KEYWORD_NATIVE_DATA_TYPE[ i_key->data_type() ],
            i_key->name().c_str() );

      // the comment if available
      fprintf(m_file, ", & %sCom", varName.c_str());

      // the unit if available
      fprintf(m_file, ", & %sUnit", varName.c_str());


      fprintf(m_file, ");}\n");
      fprintf(m_file, "      catch(exception) {}\n\n");

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

         // try { variable = GetAttr<dataType>("KeyName");}
         // catch(exception) {}
         std::string varName = VariableName(i_key->name(), KEYWORD);

         fprintf(m_file, "      try { %s = GetAttr<%s>(\"%s\"",
                 varName.c_str(),
                 KEYWORD_NATIVE_DATA_TYPE[ i_key->data_type() ],
                 i_key->name().c_str() );

         // the comment if available
         fprintf(m_file, ", & %sCom", varName.c_str());

         // the unit if available
         fprintf(m_file, ", & %sUnit", varName.c_str());

         fprintf(m_file, ");}\n");
         fprintf(m_file, "      catch(exception) {}\n\n");

         ++i_key;
      }

      ++i_group;
   }

   fprintf(m_file, "   }\n");

   if (m_hdu.image().present()) {
      image_type & img = m_hdu.image().get();
      if (img.null().present()  || img.data_type() == image_data_type::float_ ||
                                   img.data_type() == image_data_type::double_   ) {
         fprintf(m_file, "   else {\n");
         if (img.data_type() == image_data_type::float_  ) {
            fprintf(m_file, "      m_null        = std::numeric_limits<float>::quiet_NaN();\n");
         }
         else if (img.data_type() == image_data_type::double_   ) {
            fprintf(m_file, "      m_null        = std::numeric_limits<double>::quiet_NaN();\n");
         }
         else {
            fprintf(m_file, "      m_null        = %lld;\n", img.null().get());
         }

         fprintf(m_file, "      m_nullDefined = true;\n");
         fprintf(m_file, "   }\n");
      }
   }
}

void SourceFile::HeaderCopyConstructor()
{


   // loop over all header keywords
   fprintf(m_file, "   // we accept if the attribute does not exist in the source table\n");
   fprintf(m_file, "   // and catch the exception.\n\n");

   header_type::keyword_iterator i_key = m_hdu.header().keyword().begin();
   header_type::keyword_iterator i_keyEnd = m_hdu.header().keyword().end();
   while (i_key != i_keyEnd)
   {
      if (i_key->name() == "EXT_VER") {
         // do not copy the EXT_VER keyword
         ++i_key;
         continue;
      }

      // try { variable = sourceTable->GetAttr<dataType>("KeyName");}
      // catch(exception) {}
      std::string varName = VariableName(i_key->name(), KEYWORD);

      fprintf(m_file, "   try { %s = sourceTable->GetAttr<%s>(\"%s\"",
            varName.c_str(),
            KEYWORD_NATIVE_DATA_TYPE[ i_key->data_type() ],
            i_key->name().c_str() );

      // the comment if available
      fprintf(m_file, ", & %sCom", varName.c_str());

      // the unit if available
      fprintf(m_file, ", & %sUnit", varName.c_str());


      fprintf(m_file, ");}\n");
      fprintf(m_file, "   catch(exception) {}\n\n");

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

         if (i_key->name() == "EXT_VER") {
            // do not copy the EXT_VER keyword
            ++i_key;
            continue;
         }

         // try { variable = GetAttr<dataType>("KeyName");}
         // catch(exception) {}
         std::string varName = VariableName(i_key->name(), KEYWORD);

         fprintf(m_file, "   try { %s = sourceTable->GetAttr<%s>(\"%s\"",
                 varName.c_str(),
                 KEYWORD_NATIVE_DATA_TYPE[ i_key->data_type() ],
                 i_key->name().c_str() );

         // the comment if available
         fprintf(m_file, ", & %sCom", varName.c_str());

         // the unit if available
         fprintf(m_file, ", & %sUnit", varName.c_str());

         fprintf(m_file, ");}\n");
         fprintf(m_file, "   catch(exception) {}\n\n");

         ++i_key;
      }

      ++i_group;
   }
}

void SourceFile::TableConstructor()
{
   // initialize the column variables
   fprintf(m_file, "\n   // initialize the column variables with its NULL value, or with 0.\n");
   fprintf(m_file, "   if (strcmp(mode, \"READONLY\") != 0)\n");
   fprintf(m_file, "   {\n");

   bool hasVectorColumns = false;

   // loop over all columns
   table_type::column_iterator i_col = m_hdu.table().get().column().begin();
   table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();
   while (i_col != i_colEnd)
   {
      if ( i_col->data_type() != column_data_type::A &&
           i_col->data_type() != column_data_type::string) {
         //  std::fill_n(varName, size, value)
         fprintf(m_file, "      std::fill_n(");
         if (i_col->bin_size() == 1) {
            fprintf(m_file, "&");
         }
         else {
            fprintf(m_file, " ");
            hasVectorColumns = true;
         }
         string varName = VariableName(i_col->name(), CELL);
         fprintf(m_file, "%s, " , varName.c_str());
         if (varName.length() < 16)
            fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

         fprintf(m_file, " %3lld,", i_col->bin_size());
         if (i_col->data_type() == column_data_type::E ||
             i_col->data_type() == column_data_type::float_)
            fprintf(m_file, " std::numeric_limits<float>::quiet_NaN()");
         else if (i_col->data_type() == column_data_type::D ||
                  i_col->data_type() == column_data_type::double_ ||
                  i_col->data_type() == column_data_type::MJD     ||
                  i_col->data_type() == column_data_type::BJD        )
            fprintf(m_file, " std::numeric_limits<double>::quiet_NaN()");
         else if (i_col->data_type() == column_data_type::L ||
                  i_col->data_type() == column_data_type::bool_)
            fprintf(m_file, " false");
         else if (i_col->data_type() == column_data_type::UTC )
            fprintf(m_file, " \"1970-01-01T00:00:00.000000\"");
         else if (i_col->null().present())
            fprintf(m_file, " %s", i_col->null().get().c_str());
         else if (i_col->data_type() == column_data_type::OBT ||
                  i_col->data_type() == column_data_type::CUC)
            fprintf(m_file, " -1");
         else
            fprintf(m_file, " 0");


         fprintf(m_file, ");\n");
      }

      ++i_col;
   }
   fprintf(m_file, "   }\n\n");

   if (hasVectorColumns) {

      fprintf(m_file, "   // Read-only mode\n");
      fprintf(m_file, "   else {\n");
      fprintf(m_file, "      \n");
      fprintf(m_file, "      // For each vector column, check if the bin size of the column in this\n");
      fprintf(m_file, "      // file is less than the bin size defined in the fsd. If yes, it means\n");
      fprintf(m_file, "      // that the file is being read with a different version of the fsd than it\n");
      fprintf(m_file, "      // was created with, and that the size in the fsd has been increased.\n");
      fprintf(m_file, "      // \n");
      fprintf(m_file, "      // In this case, the elements of the member variable array holding the\n");
      fprintf(m_file, "      // value of the vector column are filled with NULL values so that when a\n");
      fprintf(m_file, "      // cell of this column is read from the file, the end of the array will\n");
      fprintf(m_file, "      // not contain garbage values.\n");
      fprintf(m_file, "      \n");
      fprintf(m_file, "      std::list<FitsColMetaData> colMetaData = GetFitsColMetaData();\n");
      fprintf(m_file, "      for (auto & col : colMetaData) {\n");

      table_type::column_iterator i_col = m_hdu.table().get().column().begin();
      table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();
      while (i_col != i_colEnd)
      {
         if ( i_col->bin_size() > 1 &&
              i_col->data_type() != column_data_type::A &&
              i_col->data_type() != column_data_type::string)
         {
            string varName = VariableName(i_col->name(), CELL);

            fprintf(m_file, "         if (col.m_colName == \"%s\" && col.m_arraySize < %u) {\n", varName.c_str(), (uint32_t)(i_col->bin_size()));
            fprintf(m_file, "            std::fill_n( %s, %u, ", varName.c_str(), (uint32_t)(i_col->bin_size()));

           if (i_col->data_type() == column_data_type::E ||
               i_col->data_type() == column_data_type::float_)
           fprintf(m_file, " std::numeric_limits<float>::quiet_NaN()");
           else if (i_col->data_type() == column_data_type::D ||
                    i_col->data_type() == column_data_type::double_ ||
                    i_col->data_type() == column_data_type::MJD     ||
                    i_col->data_type() == column_data_type::BJD        )
              fprintf(m_file, " std::numeric_limits<double>::quiet_NaN()");
           else if (i_col->data_type() == column_data_type::L ||
                    i_col->data_type() == column_data_type::bool_)
              fprintf(m_file, " false");
           else if (i_col->data_type() == column_data_type::UTC )
              fprintf(m_file, " \"1970-01-01T00:00:00.000000\"");
           else if (i_col->null().present())
              fprintf(m_file, " %s", i_col->null().get().c_str());
           else if (i_col->data_type() == column_data_type::OBT ||
                    i_col->data_type() == column_data_type::CUC)
              fprintf(m_file, " -1");
           else
              fprintf(m_file, " 0");
           fprintf(m_file, ");\n");
           fprintf(m_file, "         }\n");

       }
       ++i_col;
     }
     fprintf(m_file, "      }\n");
     fprintf(m_file, "   }\n\n");
   }

   // loop over all columns
    i_col = m_hdu.table().get().column().begin();
    while (i_col != i_colEnd)
    {
       // Assign(name, &variable, 1, "comment", "unit", &nullVariable);
       fprintf(m_file, "   Assign(\"%s\", ", i_col->name().c_str());
          if (i_col->name().length() < 16)
               fprintf(m_file, "%*s", (int32_t)(16 - i_col->name().length()), " ");

       if (i_col->bin_size() == 1 || i_col->data_type() == column_data_type::A ||
                                     i_col->data_type() == column_data_type::string)
          fprintf(m_file, "& ");
       else
          fprintf(m_file, "  ");

       string varName = VariableName(i_col->name(), CELL);
       fprintf(m_file, "%s, " , varName.c_str());
       if (varName.length() < 16)
            fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

       if (i_col->data_type() == column_data_type::UTC) {
          fprintf(m_file, " 26");  /// UTC is actually a string of length 26
       }
       else {
          fprintf(m_file, "%3u", (uint32_t)(i_col->bin_size()));
       }


       if (i_col->comment().present())
          fprintf(m_file, ",  \"%s\"", i_col->comment().get().c_str());
       else if (i_col->unit().present() || i_col->null().present() ||
                i_col->data_type() == column_data_type::OBT ||
                i_col->data_type() == column_data_type::CUC)
          fprintf(m_file, ",  \"\"");

       if (i_col->unit().present())
          fprintf(m_file, ",  \"%s\"", i_col->unit().get().c_str());
       else if (i_col->null().present() ||
                i_col->data_type() == column_data_type::OBT ||
                i_col->data_type() == column_data_type::CUC)
           fprintf(m_file, ",  \"\"");

       if (i_col->data_type() == column_data_type::OBT ||
           i_col->data_type() == column_data_type::CUC ||
           (i_col->null().present() &&
            i_col->data_type() >= 4 && i_col->data_type() <= 19) ) {
          varName = VariableName(i_col->name(), NULL_V);
          fprintf(m_file, ", &%s" , varName.c_str());
       }

       fprintf(m_file, ");\n");
       ++i_col;
    }
    fprintf(m_file, "\n");

    //get the units of the columns
    fprintf(m_file, "   // get the units of all columns.\n");
    i_col = m_hdu.table().get().column().begin();
    while (i_col != i_colEnd)
    {
       string varName = VariableName(i_col->name(), CELL);

       // m_colnameUnit = GetColUnit("colName");
       fprintf(m_file, "   %sUnit " , varName.c_str());
       if (varName.length() < 20)
            fprintf(m_file, "%*s", (int32_t)(20 - varName.length()), " ");
       fprintf(m_file, " = GetColUnit(\"%s\");\n", i_col->name().c_str());

       ++i_col;
    }

}



void SourceFile::TableCopyConstructor()
{
   // initialize the column variables
   fprintf(m_file, "\n   // initialize the column variables with its NULL value, or with 0.\n");


   // loop over all columns
   table_type::column_iterator i_col = m_hdu.table().get().column().begin();
   table_type::column_iterator i_colEnd = m_hdu.table().get().column().end();
   while (i_col != i_colEnd)
   {
      if ( i_col->data_type() != column_data_type::A &&
           i_col->data_type() != column_data_type::string) {
         //  std::fill_n(varName, size, value)
         fprintf(m_file, "   std::fill_n(");
         if (i_col->bin_size() == 1) {
            fprintf(m_file, "&");
         }
         else {
            fprintf(m_file, " ");
         }
         string varName = VariableName(i_col->name(), CELL);
         fprintf(m_file, "%s, " , varName.c_str());
         if (varName.length() < 16)
            fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

         fprintf(m_file, " %3lld,", i_col->bin_size());
         if (i_col->data_type() == column_data_type::E ||
             i_col->data_type() == column_data_type::float_)
            fprintf(m_file, " std::numeric_limits<float>::quiet_NaN()");
         else if (i_col->data_type() == column_data_type::D ||
                  i_col->data_type() == column_data_type::double_ ||
                  i_col->data_type() == column_data_type::MJD     ||
                  i_col->data_type() == column_data_type::BJD        )
            fprintf(m_file, " std::numeric_limits<double>::quiet_NaN()");
         else if (i_col->data_type() == column_data_type::L ||
                  i_col->data_type() == column_data_type::bool_)
            fprintf(m_file, " false");
         else if (i_col->data_type() == column_data_type::UTC )
            fprintf(m_file, " \"1970-01-01T00:00:00.000000\"");
         else if (i_col->null().present())
            fprintf(m_file, " %s", i_col->null().get().c_str());
         else if (i_col->data_type() == column_data_type::OBT ||
                  i_col->data_type() == column_data_type::CUC)
            fprintf(m_file, " -1");
         else
            fprintf(m_file, " 0");


         fprintf(m_file, ");\n");
      }

      ++i_col;
   }
   fprintf(m_file, "\n");


   // loop over all columns
    i_col = m_hdu.table().get().column().begin();
    while (i_col != i_colEnd)
    {
       // Assign(name, &variable, 1, "comment", "unit", &nullVariable);
       fprintf(m_file, "                Assign(\"%s\", ", i_col->name().c_str());
          if (i_col->name().length() < 16)
               fprintf(m_file, "%*s", (int32_t)(16 - i_col->name().length()), " ");

       if (i_col->bin_size() == 1 || i_col->data_type() == column_data_type::A ||
                                     i_col->data_type() == column_data_type::string)
          fprintf(m_file, "& ");
       else
          fprintf(m_file, "  ");

       string varName = VariableName(i_col->name(), CELL);
       fprintf(m_file, "%s, " , varName.c_str());
       if (varName.length() < 16)
            fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

       if (i_col->data_type() == column_data_type::UTC) {
          fprintf(m_file, " 26");  /// UTC is actually a string of length 26
       }
       else {
          fprintf(m_file, "%3u", (uint32_t)(i_col->bin_size()));
       }


       if (i_col->comment().present())
          fprintf(m_file, ",  \"%s\"", i_col->comment().get().c_str());
       else if (i_col->unit().present() || i_col->null().present() ||
                i_col->data_type() == column_data_type::OBT ||
                i_col->data_type() == column_data_type::CUC    )
          fprintf(m_file, ",  \"\"");

       if (i_col->unit().present())
          fprintf(m_file, ",  \"%s\"", i_col->unit().get().c_str());
       else if (i_col->null().present() ||
                i_col->data_type() == column_data_type::OBT ||
                i_col->data_type() == column_data_type::CUC    )
           fprintf(m_file, ",  \"\"");

       if (i_col->data_type() == column_data_type::OBT ||
           i_col->data_type() == column_data_type::CUC ||
           (i_col->null().present() &&
            i_col->data_type() >= 4 && i_col->data_type() <= 19) ) {
          varName = VariableName(i_col->name(), NULL_V);
          fprintf(m_file, ", &%s" , varName.c_str());
       }

       fprintf(m_file, ");\n");


       // sourceTable->Assign(name, &variable, 1);
       fprintf(m_file, "   sourceTable->Assign(\"%s\", ", i_col->name().c_str());
          if (i_col->name().length() < 16)
               fprintf(m_file, "%*s", (int32_t)(16 - i_col->name().length()), " ");

       if (i_col->bin_size() == 1 || i_col->data_type() == column_data_type::A ||
                                     i_col->data_type() == column_data_type::string)
          fprintf(m_file, "& ");
       else
          fprintf(m_file, "  ");

       varName = VariableName(i_col->name(), CELL);
       fprintf(m_file, "%s, " , varName.c_str());
       if (varName.length() < 16)
            fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

       if (i_col->data_type() == column_data_type::UTC) {
          fprintf(m_file, " 26");  /// UTC is actually a string of length 26
       }
       else {
          fprintf(m_file, "%3u", (uint32_t)(i_col->bin_size()));
       }
       fprintf(m_file, ");\n");

       ++i_col;
    }
    fprintf(m_file, "\n");

    //get the units of the columns
    fprintf(m_file, "   // get the units of all columns.\n");
    i_col = m_hdu.table().get().column().begin();
    while (i_col != i_colEnd)
    {
       string varName = VariableName(i_col->name(), CELL);

       // m_colnameUnit = GetColUnit("colName");
       fprintf(m_file, "   %sUnit " , varName.c_str());
       if (varName.length() < 20)
            fprintf(m_file, "%*s", (int32_t)(20 - varName.length()), " ");
       fprintf(m_file, " = GetColUnit(\"%s\");\n", i_col->name().c_str());

       ++i_col;
    }

    // copy all rows from the sourceTable to this table
    fprintf(m_file, "\n//  copy all rows from the sourceTable to this table.\n");
    fprintf(m_file, "   while(sourceTable->ReadRow())\n" );
    fprintf(m_file, "      WriteRow();\n\n");


    // delete  the source table
    fprintf(m_file, "  delete sourceTable;\n");
}


void SourceFile::ConstructorBottom()
{
	fprintf(m_file, "}\n");
}

/** *************************************************************************
 *  @param i_key the iterator from which to get the header keyword.
 */
void SourceFile::Set1Attr(header_type::keyword_iterator & i_key)
{
   // SetAttr("keyword", variable, "comment", "unit");

   // SetAttr("keyname",
   fprintf(m_file, "      SetAttr(\"%s\",", i_key->name().c_str());
   if (i_key->name().length() < 8)
      fprintf(m_file, "%*s", (int32_t)(8 - i_key->name().length()), " ");

   // cast to its native data type for OBT, UTC, MJD, BJD
   if (i_key->data_type() == keyword_data_type::OBT    ||
       i_key->data_type() == keyword_data_type::UTC    ||
       i_key->data_type() == keyword_data_type::MJD    ||
       i_key->data_type() == keyword_data_type::BJD    ||
       i_key->data_type() == keyword_data_type::PassId      )
      fprintf(m_file, " (%s)", KEYWORD_NATIVE_DATA_TYPE[ i_key->data_type() ]);

   // variable
   string varName = VariableName(i_key->name(), KEYWORD);
   fprintf(m_file, " %s", varName.c_str());
   if (varName.length() < 16)
        fprintf(m_file, "%*s", (int32_t)(16 - varName.length()), " ");

   // , "comment"
   if (i_key->comment().present())
      fprintf(m_file, ", \"%s\"",  i_key->comment().get().c_str());
   else if (i_key->unit().present())
      fprintf(m_file, ", \"\"");

   // , "unit"
   if (i_key->unit().present())
           fprintf(m_file, ", \"%s\"",  i_key->unit().get().c_str());

   fprintf(m_file, ");\n");

}

void SourceFile::Destructor()
{

   // destructor
   fprintf(m_file, "\n\n");
   fprintf(m_file, "%s::~%s()\n", ClassName().c_str(), ClassName().c_str());
   fprintf(m_file, "{\n");


   fprintf(m_file, "   if (m_update)\n");
   fprintf(m_file, "   {\n");

   // the extension name
   fprintf(m_file, "      SetAttr(\"EXTNAME\",  string(\"%s\"), \"extension name\");\n",
                    ExtName().c_str());

   // the fits schema file
   fprintf(m_file, "      SetAttr(\"SCHEMA\",   string(\"%s\"), \"filename of schema defining this data structure\");\n",
                   m_schemaFileName.c_str());

   // loop over all header keywords
   header_type::keyword_iterator i_key = m_hdu.header().keyword().begin();
   header_type::keyword_iterator i_keyEnd = m_hdu.header().keyword().end();
   while (i_key != i_keyEnd)
   {
      Set1Attr(i_key);

      ++i_key;
   }

   //loop over all keyword groups
   header_type::group_iterator i_group = m_hdu.header().group().begin();
   header_type::group_iterator i_groupEnd = m_hdu.header().group().end();
   while (i_group != i_groupEnd) {

      fprintf(m_file, "      InsertCommentLine(std::string() );\n");
      fprintf(m_file, "      InsertCommentLine(\"%s\");\n", i_group->description().c_str());
      fprintf(m_file, "      InsertCommentLine(std::string() );\n");

      // loop over all header keywords in this group
      i_key = i_group->keyword().begin();
      i_keyEnd = i_group->keyword().end();
      while (i_key != i_keyEnd) {
         Set1Attr(i_key);

         ++i_key;
      }

      ++i_group;
   }

   fprintf(m_file, "      InsertCommentLine(std::string() );\n");
   fprintf(m_file, "   }\n");
   fprintf(m_file, "}\n");

}


///////////////////////////////////////////////////////////////////////////////
SourceFile::~SourceFile()
{
	if (m_file)
	{
		fclose(m_file);
	}

}

