/*
 * test_parameters.cxx
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

BOOST_FIXTURE_TEST_SUITE( testReadImage, FitsDalFixture )

////////////////////////////////////////////////////////////////////////////////
// Create a first simple image
BOOST_AUTO_TEST_CASE( FirstImage )
{

   FitsDalImage<uint16_t> * image = new FitsDalImage<uint16_t>(
           "results/simpleImage.fits");

   BOOST_CHECK_EQUAL(3, image->GetNumDim());

   long size[3];
   image->GetSize(size);
   BOOST_CHECK_EQUAL(2, size[0]);
   BOOST_CHECK_EQUAL(3, size[1]);
   BOOST_CHECK_EQUAL(4, size[2]);

   std::vector<long> sizeVector = image->GetSize();
   BOOST_CHECK_EQUAL(3, sizeVector.size());
   BOOST_CHECK_EQUAL(2, sizeVector[0]);
   BOOST_CHECK_EQUAL(3, sizeVector[1]);
   BOOST_CHECK_EQUAL(4, sizeVector[2]);

   uint16_t pixelValue = 0;
   for (int32_t z = 0; z < 4; z++) {
      for (int32_t y = 0; y < 3; y++) {
         for (int32_t x = 0; x < 2; x++) {
            BOOST_CHECK_EQUAL(++pixelValue, (*image)[z][y][x]);
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
           "results/simpleImage.fits+2");

   BOOST_CHECK_EQUAL("FitsDalImage", image->getClassName());

   BOOST_CHECK_EQUAL(image->GetAttr<std::string>("EXTNAME"), "TST-IMA-SECOND");

//   //test keywords of every supported type
   std::string comment, unit;

   BOOST_CHECK_EQUAL(image->GetAttr<int8_t>("INT8_T", &comment, &unit), 1);
   BOOST_CHECK_EQUAL(comment, "int8_t");
   BOOST_CHECK_EQUAL(unit.c_str(), "int8_t");

   BOOST_CHECK_EQUAL(image->GetAttr<uint64_t>("UINT64_T", &comment, &unit), 1);
   BOOST_CHECK_EQUAL(comment, "uint64_t");
   BOOST_CHECK_EQUAL(unit.c_str(), "uint64_t");

   BOOST_CHECK_CLOSE(image->GetAttr<float>("FLOAT", &comment, &unit), 2.2, 0.001);
   BOOST_CHECK_EQUAL(comment, "new comment");
   BOOST_CHECK_EQUAL(unit.c_str(), "new unit");

   BOOST_CHECK_EQUAL(image->GetAttr<double>("DOUBLE", &comment, &unit), -2.2);
   BOOST_CHECK_EQUAL(comment, "double");
   BOOST_CHECK_EQUAL(unit.c_str(), "double");

   BOOST_CHECK_EQUAL(image->GetAttr<bool>("BOOL", &comment, &unit), false);
   BOOST_CHECK_EQUAL(comment, "bool");
   BOOST_CHECK_EQUAL(unit.c_str(), "new unit");

   BOOST_CHECK_EQUAL(image->GetAttr<std::string>("TOOLONGSTRING", &comment, &unit), "5");
   BOOST_CHECK_EQUAL(comment, "TooLongString");
   BOOST_CHECK_EQUAL(unit.c_str(), "");

   // accessing a nonexistent keyword shall fail
   BOOST_CHECK_THROW(image->GetAttr<uint64_t>("TEST", &comment, &unit), std::runtime_error);

   // converting to the wrong type shall fail
   BOOST_CHECK_THROW(image->GetAttr<int8_t>("STRING", &comment, &unit), std::runtime_error);
   BOOST_CHECK_THROW(image->GetAttr<uint64_t>("STRING", &comment, &unit), std::runtime_error);
   BOOST_CHECK_THROW(image->GetAttr<float>("STRING", &comment, &unit), std::runtime_error);
   BOOST_CHECK_THROW(image->GetAttr<double>("STRING", &comment, &unit), std::runtime_error);
   BOOST_CHECK_THROW(image->GetAttr<bool>("STRING", &comment, &unit), std::runtime_error);

   delete image;
}

////////////////////////////////////////////////////////////////////////////////
// Open just the header to read some keywords
BOOST_AUTO_TEST_CASE( HEADER )
{
   FitsDalHeader * header = new FitsDalHeader("results/simpleImage.fits");

   BOOST_CHECK_EQUAL(header->GetAttr<std::string>("EXTNAME"), "TST-IMA-SIMPLE");

   delete header;
}

////////////////////////////////////////////////////////////////////////////////
// Read an image that does not exist
BOOST_AUTO_TEST_CASE( NonExistentImage )
{

   BOOST_CHECK_THROW(
       new FitsDalImage<uint16_t>(
           "results/nonexistentImage.fits"),
       std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
