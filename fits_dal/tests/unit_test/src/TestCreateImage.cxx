/*
 * test_parameters.cxx
 *
 * @version 9.3 #15574 RRO
 *       - new test cases: IncreaseSize, DecreaseSize
 *
 *  Created on: May 8, 2014
 *      Author: rohlfs
 */

#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <stdint.h>

#include "ProgramParams.hxx"
#include "FitsDalImage.hxx"

using namespace boost::unit_test;

struct FitsDalFixture {
   FitsDalFixture() {
      m_params = CheopsInit(framework::master_test_suite().argc,
                            framework::master_test_suite().argv);


   }

   ~FitsDalFixture() {
   }

   ParamsPtr m_params;
};

BOOST_FIXTURE_TEST_SUITE( testCreateImage, FitsDalFixture )

////////////////////////////////////////////////////////////////////////////////
// Create a first simple image
BOOST_AUTO_TEST_CASE( FirstImage )
{
   unlink("results/simpleImage.fits");

   FitsDalImage<uint16_t> * image = new FitsDalImage<uint16_t>(
           "results/simpleImage.fits", "CREATE", {2, 3, 4});

   image->SetAttr("EXTNAME", std::string("TST-IMA-SIMPLE"));

   BOOST_CHECK_EQUAL(3, image->GetNumDim());

   uint16_t pixelValue = 0;
   for (int32_t z = 0; z < 4; z++) {
      for (int32_t y = 0; y < 3; y++) {
         for (int32_t x = 0; x < 2; x++) {
            (*image)[z][y][x] = ++pixelValue;
         }
      }
   }

   delete image;
}

////////////////////////////////////////////////////////////////////////////////
// Create a second image and define some header keywords
BOOST_AUTO_TEST_CASE( SecondImage )
{
   FitsDalImage<double> * image = new FitsDalImage<double>(
           "results/simpleImage.fits", "APPEND", {5, 10});

   image->SetAttr("EXTNAME", std::string("TST-IMA-SECOND"));

   //set new keywords of every supported type
   image->SetAttr("int8_t",   (int8_t)1,   "int8_t",   "int8_t");
   image->SetAttr("uint8_t",  (uint8_t)1,  "uint8_t",  "uint8_t");
   image->SetAttr("int16_t",  (int16_t)1,  "int16_t",  "int16_t");
   image->SetAttr("uint16_t", (uint16_t)1, "uint16_t", "uint16_t");
   image->SetAttr("int32_t",  (int32_t)1,  "int32_t",  "int32_t");
   image->SetAttr("uint32_t", (uint32_t)1, "uint32_t", "uint32_t");
   image->SetAttr("int64_t",  (int64_t)1,  "int64_t",  "int64_t");
   image->SetAttr("uint64_t", (uint64_t)1, "uint64_t", "uint64_t");
   image->SetAttr("bool",     (bool)1,     "bool",     "bool");
   image->SetAttr("float",    (float)1,    "float",    "float");
   image->SetAttr("double",   (double)1,   "double",   "double");
   image->SetAttr("string",   std::string("1"),  "string", "string");
   image->SetAttr("TOOLONGSTRING",  "5",  "TooLongString");

   // update existing keyword
   image->SetAttr("int32_t",  (int32_t)-2  );
   image->SetAttr("uint32_t", (uint32_t)2, "new comment");
   image->SetAttr("bool",     false,       "",     "new unit");
   image->SetAttr("float",   (float)2.2,     "new comment",    "new unit");
   image->SetAttr("double",  (double)-2.2,    "double", "double");
   image->SetAttr("string",  std::string("new"),  "string", "string");

   // insert a comment
   image->InsertCommentLine("");
   image->InsertCommentLine("test comment");
   image->InsertCommentLine("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

   delete image;
}

////////////////////////////////////////////////////////////////////////////////
// Create a fits image of each type
BOOST_AUTO_TEST_CASE( FitsImageAllTypes )
{
   std::string fileName = "results/FitsImageAllTypes.fits";

   unlink(fileName.c_str());
   FitsDalImage<uint8_t> * image_uint8_t = new FitsDalImage<uint8_t>(
       fileName, "CREATE", {2, 3, 4});
   delete image_uint8_t;
   image_uint8_t = new FitsDalImage<uint8_t>(fileName);
   delete image_uint8_t;

   unlink(fileName.c_str());
   FitsDalImage<int16_t> * image_int16_t = new FitsDalImage<int16_t>(
       fileName, "CREATE", {2, 3, 4});
   delete image_int16_t;
   image_int16_t = new FitsDalImage<int16_t>(fileName);
   delete image_int16_t;

   unlink(fileName.c_str());
   FitsDalImage<uint16_t> * image_uint16_t = new FitsDalImage<uint16_t>(
       fileName, "CREATE", {2, 3, 4});
   delete image_uint16_t;
   image_uint16_t = new FitsDalImage<uint16_t>(fileName);
   delete image_uint16_t;

   unlink(fileName.c_str());
   FitsDalImage<int32_t> * image_int32_t = new FitsDalImage<int32_t>(
       fileName, "CREATE", {2, 3, 4});
   delete image_int32_t;
   image_int32_t = new FitsDalImage<int32_t>(fileName);
   delete image_int32_t;

   unlink(fileName.c_str());
   FitsDalImage<uint32_t> * image_uint32_t = new FitsDalImage<uint32_t>(
       fileName, "CREATE", {2, 3, 4});
   delete image_uint32_t;
   image_uint32_t = new FitsDalImage<uint32_t>(fileName);
   delete image_uint32_t;

   unlink(fileName.c_str());
   FitsDalImage<int64_t> * image_int64_t = new FitsDalImage<int64_t>(
       fileName, "CREATE", {2, 3, 4});
   delete image_int64_t;
   image_int64_t = new FitsDalImage<int64_t>(fileName);
   delete image_int64_t;

   unlink(fileName.c_str());
   FitsDalImage<float> * image_float = new FitsDalImage<float>(
       fileName, "CREATE", {2, 3, 4});
   delete image_float;
   image_float = new FitsDalImage<float>(fileName);
   delete image_float;

   unlink(fileName.c_str());
   FitsDalImage<double> * image_double = new FitsDalImage<double>(
       fileName, "CREATE", {2, 3, 4});
   delete image_double;
   image_double = new FitsDalImage<double>(fileName);
   delete image_double;

}

////////////////////////////////////////////////////////////////////////////////
// Create a simple image and increase the 3rd Dimension
BOOST_AUTO_TEST_CASE( IncreaseSize )
{
   unlink("results/ResizedImage.fits");

   FitsDalImage<uint16_t> * image = new FitsDalImage<uint16_t>(
           "results/ResizedImage.fits", "CREATE", {2, 3, 4});

   image->SetAttr("EXTNAME", std::string("TST-IMA-SIMPLE"));

   uint16_t pixelValue = 0;
   for (int32_t z = 0; z < 4; z++) {
      for (int32_t y = 0; y < 3; y++) {
         for (int32_t x = 0; x < 2; x++) {
            (*image)[z][y][x] = ++pixelValue;
         }
      }
   }

   image->ResizeThirdDimension(6);
   (*image)[4][1][1] = 40;
   (*image)[5][0][0] = 50;


   delete image;
}

////////////////////////////////////////////////////////////////////////////////
// Create a simple image and decreas the 3rd Dimension
BOOST_AUTO_TEST_CASE( DecreaseSize )
{
   unlink("results/DecreasedImage.fits");

   FitsDalImage<uint16_t> * image = new FitsDalImage<uint16_t>(
           "results/DecreasedImage.fits", "CREATE", {2, 3, 4});

   image->SetAttr("EXTNAME", std::string("TST-IMA-SIMPLE"));

   uint16_t pixelValue = 0;
   for (int32_t z = 0; z < 4; z++) {
      for (int32_t y = 0; y < 3; y++) {
         for (int32_t x = 0; x < 2; x++) {
            (*image)[z][y][x] = ++pixelValue;
         }
      }
   }

   image->ResizeThirdDimension(2);
   (*image)[1][1][1] = 40;


   delete image;
}


BOOST_AUTO_TEST_SUITE_END()
