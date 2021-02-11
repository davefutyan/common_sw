/** ****************************************************************************
  *  @file
  *  @brief  the main function of the fits_data_model program
  *
  *  @author Reiner Rohlfs UGE
  *
  *
  *  @version 6.1   2016-07-14 ABE #11166: calling UsingDeclarations()
  *  @version 3.0   2015-01-20 RRO #7156:  calling StaticMethods()
  *  @version 1.0   2014-03-29 RRO         first version.
  *
  */

#include <stdexcept>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>

#include "FitsDataModel.hxx"

using namespace std;


/** ****************************************************************************
 *  @brief main function of the fits_data_model program
 */
int main(int argc, char * argv[] )
{
   try {

      ParamsPtr progParams = CheopsInit(argc, argv);

      // parsing the xml file
      string inputSchemaFile = progParams->GetAsString("input");
      cout << "Reading fits schema file  " << inputSchemaFile << endl;
      auto_ptr<Fits_schema_type> fsd (FITS_schema(inputSchemaFile));
      HDU_type & hdu = fsd->HDU();

      HeaderFile headerFile(hdu, progParams,
            boost::filesystem::path(inputSchemaFile).filename().string());
      headerFile.Top();
      headerFile.ClassTop();

      headerFile.HeaderVariables();
      if (hdu.table().present())
         headerFile.TableVariables();
      headerFile.UsingDeclarations();

      headerFile.ConstructorDestructor();
      headerFile.StaticMethods();
      headerFile.HeaderGeterSeter();
      if (hdu.table().present())
         headerFile.TableGeterSeter();

      headerFile.ClassBottom();
      headerFile.Associated_HDUs(fsd->List_of_Associated_HDUs());

      headerFile.Bottom();


      SourceFile sourceFile(hdu, progParams,
            boost::filesystem::path(inputSchemaFile).filename().string());
      sourceFile.Top();
      if (hdu.image().present())
         sourceFile.ImgConstructorTop();
      else
         sourceFile.TableConstructorTop();

      sourceFile.HeaderConstructor();

      if (hdu.table().present())
         sourceFile.TableConstructor();

      sourceFile.ConstructorBottom();

      if (hdu.table().present()) {
         // the second constructor to copy the data from an other table
         sourceFile.TableCopyConstructorTop();
         sourceFile.HeaderCopyConstructor();
         sourceFile.TableCopyConstructor();

         sourceFile.ConstructorBottom();

      }


      sourceFile.Destructor();

   }
   catch (exception & e)
   {
      cout << e.what() << endl;
      return 1;
   }

   return 0;
}

