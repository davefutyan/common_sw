/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief   unit_test of the UTC class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 3.0   2015-01-24 RRO first version
 *
 */


#define BOOST_TEST_MAIN
#include "boost/test/unit_test.hpp"

#include <list>
#include <set>

//#include "REF_APP_Limits.hxx"
#include "FitsDalTable.hxx"

#include "ValidRefFile.hxx"

using namespace boost::unit_test;


/**
 * Helper method for creating a FITS data structure on disk. The template's type
 * specifies the class of the data structure.
 *
 * @param fileName      The name of the file
 * @param extname       The file's extension name
 * @param validityStart The file's validity start
 * @param validityStop  The file's validity stop
 * @param archRev       The file's archive revision number
 * @param procNum       The file's processing number
 */
template <class HDU>
void createRefFile(const std::string & fileName,
                   const std::string & extname,
                   const UTC & validityStart,
                   const UTC & validityStop,
                   int32_t archRev,
                   int32_t procNum) {

    FitsDalHeader::SetProgram("TestValidRefFile",
                             "", "", "undefined",
                             archRev, procNum);
    unlink(fileName.c_str());
    HDU * ref = new HDU(fileName, "CREATE");
    ref->SetAttr("V_STRT_U", validityStart.getUtc());
    ref->SetAttr("V_STOP_U", validityStop.getUtc());
    ref->SetAttr("ARCH_REV", archRev);
    ref->SetAttr("PROC_NUM", procNum);
    ref->SetAttr("EXTNAME", extname);

    delete ref;
}


struct ValidityPeriodFixture {

  std::string m_fileName1 = "resources/TestValidRefFile_RefAppLimits1.fits";
  std::string m_fileName2 = "resources/TestValidRefFile_RefAppLimits2.fits";
  std::string m_fileName3 = "resources/TestValidRefFile_RefAppLimits3.fits";

  ValidityPeriodFixture() {}

   ~ValidityPeriodFixture() {

     unlink(m_fileName1.c_str());
     unlink(m_fileName2.c_str());
     unlink(m_fileName3.c_str());
   }

};


BOOST_FIXTURE_TEST_SUITE( testValidityPeriod, ValidityPeriodFixture )

////////////////////////////////////////////////////////////////////////////////
//
// Test that ValidityPeriod objects are correctly ordered by their archive
// revision number and processing number.
//
BOOST_AUTO_TEST_CASE( test_ValidityPeriod_set )
{
  // Data for validity period 1
  UTC validityStart1(2020, 1, 1, 1, 1, 2);
  UTC validityStop1 (2020, 1, 1, 1, 1, 4);
  UTC creationDate1 (2020, 1, 1, 1, 1, 1);
  int32_t archRev1 = 0;
  int32_t procNum1 = 0;
  ValidityPeriod validityPeriod1(validityStart1, validityStop1, archRev1, procNum1, m_fileName1, m_fileName1);

  // Data for validity period 2
  UTC validityStart2(2020, 1, 1, 1, 1, 6);
  UTC validityStop2 (2020, 1, 1, 1, 1, 8);
  UTC creationDate2 (2020, 1, 1, 1, 1, 2);
  int32_t archRev2 = 0;
  int32_t procNum2 = 1;
  ValidityPeriod validityPeriod2(validityStart2, validityStop2, archRev2, procNum2, m_fileName2, m_fileName2);

  // Data for validity period 3
  UTC validityStart3(2020, 1, 1, 1, 1, 6);
  UTC validityStop3 (2020, 1, 1, 1, 1, 8);
  UTC creationDate3 (2020, 1, 1, 1, 1, 2);
  int32_t archRev3 = 1;
  int32_t procNum3 = 0;
  ValidityPeriod validityPeriod3(validityStart3, validityStop3, archRev3, procNum3, m_fileName3, m_fileName3);

  // The expected ordering
  std::vector<ValidityPeriod> expectedValidityPeriods;
  expectedValidityPeriods.push_back(validityPeriod1);
  expectedValidityPeriods.push_back(validityPeriod2);
  expectedValidityPeriods.push_back(validityPeriod3);

  // Create a set containing ValidityPeriods
  std::set<ValidityPeriod> validityPeriods;
  validityPeriods.insert(validityPeriod3);
  validityPeriods.insert(validityPeriod2);
  validityPeriods.insert(validityPeriod1);


  // Check that the validity periods are ordered by archive/processing number
  std::vector<ValidityPeriod>::size_type i = 0;
  for (auto & vp : validityPeriods) {
    BOOST_CHECK_EQUAL( vp.getFileName(), expectedValidityPeriods[i].getFileName() );
    i++;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Test that a ValidityPeriod correctly returns true or false for whether or
// not it contains a given time.
//
BOOST_AUTO_TEST_CASE( test_ValidityPeriod_contains )
{
  // Data for validity period 1
  UTC beforeValidityStart1 (2020, 1, 1, 1, 1, 1);
  UTC validityStart1       (2020, 1, 1, 1, 1, 2);
  UTC validityMiddle1      (2020, 1, 1, 1, 1, 3);
  UTC validityStop1        (2020, 1, 1, 1, 1, 4);
  UTC afterValidityStop1   (2020, 1, 1, 1, 1, 5);
  int32_t archRev1 = 0;
  int32_t procNum1 = 0;
  ValidityPeriod validityPeriod1(validityStart1, validityStop1, archRev1, procNum1, m_fileName1, m_fileName1);

  // Data for validity period 2
  UTC beforeValidityStart2 (2020, 1, 1, 1, 1, 5);
  UTC validityStart2       (2020, 1, 1, 1, 1, 6);
  UTC validityMiddle2      (2020, 1, 1, 1, 1, 7);
  UTC validityStop2        (2020, 1, 1, 1, 1, 8);
  UTC afterValidityStop2   (2020, 1, 1, 1, 1, 9);
  int32_t archRev2 = 1;
  int32_t procNum2 = 0;
  ValidityPeriod validityPeriod2(validityStart2, validityStop2, archRev2, procNum2, m_fileName2, m_fileName2);

  // Create a set containing ValidityPeriods
  std::set<ValidityPeriod> validityPeriods;
  validityPeriods.insert(validityPeriod2);
  validityPeriods.insert(validityPeriod1);

  ValidityPeriod vp1 = *(validityPeriods.begin());
  ValidityPeriod vp2 = *(++validityPeriods.begin());

  // Check that the method for checking if a validity period contains a specific time works correctly

  BOOST_CHECK_EQUAL( vp1.getFileName(), m_fileName1 );
  BOOST_CHECK_EQUAL( vp2.getFileName(), m_fileName2 );

  BOOST_CHECK( !vp1.contains(beforeValidityStart1) );
  BOOST_CHECK(  vp1.contains(validityStart1) );
  BOOST_CHECK(  vp1.contains(validityMiddle1) );
  BOOST_CHECK(  vp1.contains(validityStop1) );
  BOOST_CHECK( !vp1.contains(afterValidityStop1) );

  BOOST_CHECK( !vp2.contains(beforeValidityStart2) );
  BOOST_CHECK(  vp2.contains(validityStart2) );
  BOOST_CHECK(  vp2.contains(validityMiddle2) );
  BOOST_CHECK(  vp2.contains(validityStop2) );
  BOOST_CHECK( !vp2.contains(afterValidityStop2) );

}

BOOST_AUTO_TEST_SUITE_END()


struct ValidRefFileFixture {

  std::string m_fileName1 = "resources/TestValidRefFile_RefAppLimits1.fits";
  std::string m_fileName2 = "resources/TestValidRefFile_RefAppLimits2.fits";
  std::string m_fileName3 = "resources/TestValidRefFile_RefAppLimits3.fits";

  ValidRefFileFixture() {}

   ~ValidRefFileFixture() {

     unlink(m_fileName1.c_str());
     unlink(m_fileName2.c_str());
     unlink(m_fileName3.c_str());
   }

};


BOOST_FIXTURE_TEST_SUITE( testValidRefFile, ValidRefFileFixture )

////////////////////////////////////////////////////////////////////////////////
//
// Instantiate ValidRefFile with multiple input files.
//
BOOST_AUTO_TEST_CASE( test_ValidRefFile_constructor_multiple_files )
{
  // Data for validity period 1
  UTC validityStart1(2020, 1, 1, 1, 1, 2);
  UTC validityStop1 (2020, 1, 1, 1, 1, 4);
  UTC creationDate1 (2020, 1, 1, 1, 1, 1);
  int32_t archRev1 = 0;
  int32_t procNum1 = 0;
  ValidityPeriod validityPeriod1(validityStart1, validityStop1, archRev1, procNum1, m_fileName1, m_fileName1);

  // Data for validity period 2
  UTC validityStart2(2020, 1, 1, 1, 1, 6);
  UTC validityStop2 (2020, 1, 1, 1, 1, 8);
  UTC creationDate2 (2020, 1, 1, 1, 1, 2);
  int32_t archRev2 = 0;
  int32_t procNum2 = 1;
  ValidityPeriod validityPeriod2(validityStart2, validityStop2, archRev2, procNum2, m_fileName2, m_fileName2);

  // Data for validity period 3
  UTC validityStart3(2020, 1, 1, 1, 1, 6);
  UTC validityStop3 (2020, 1, 1, 1, 1, 8);
  UTC creationDate3 (2020, 1, 1, 1, 1, 2);
  int32_t archRev3 = 1;
  int32_t procNum3 = 0;
  ValidityPeriod validityPeriod3(validityStart3, validityStop3, archRev3, procNum3, m_fileName3, m_fileName3);

  std::vector<ValidityPeriod> validityPeriods = { validityPeriod1,
                                                  validityPeriod2,
                                                  validityPeriod3 };
  std::list<std::string> limitFileNames = { m_fileName1,
                                            m_fileName2,
                                            m_fileName3 };

  for (auto & vp : validityPeriods) {
        createRefFile<FitsDalTable>(vp.getFileName(),
                                    "REF_APP_Limits",
                                    vp.getValidityStart(),
                                    vp.getValidityStop(),
                                    vp.getArchRev(),
                                    vp.getProcNum());
  }

  ValidRefFile<FitsDalTable> * refAppLimits = new ValidRefFile<FitsDalTable>( limitFileNames );
  delete refAppLimits;

}

////////////////////////////////////////////////////////////////////////////////
//
// Instantiate ValidRefFile with a single input file.
//
BOOST_AUTO_TEST_CASE( test_ValidRefFile_constructor_one_file )
{
  // Test using a single reference file.

  std::list<std::string> fileNames = { m_fileName1 };
  UTC beforeValidityStart (2020, 1, 1, 1, 1, 1);
  UTC validityStart       (2020, 1, 1, 1, 1, 2);
  UTC validityMiddle      (2020, 1, 1, 1, 1, 3);
  UTC validityStop        (2020, 1, 1, 1, 1, 4);
  UTC afterValidityStop   (2020, 1, 1, 1, 1, 5);
  int32_t archRev = 0;
  int32_t procNum = 0;

  createRefFile<FitsDalTable>(m_fileName1,
                              "REF_APP_LImits",
                              validityStart,
                              validityStop,
                              archRev,
                              procNum);

  ValidRefFile<FitsDalTable> * refAppLimits = new ValidRefFile<FitsDalTable>(fileNames);

  // Check that error is thrown for data times outside of the validity period.
  BOOST_CHECK_THROW( refAppLimits->getFile(beforeValidityStart), std::runtime_error );
  BOOST_CHECK_THROW( refAppLimits->getFile(afterValidityStop),   std::runtime_error );

  // Check that the correct file was returned for data times within the validity period.
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStart)->GetFileName(),  m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityMiddle)->GetFileName(), m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStop)->GetFileName(),   m_fileName1 );


  delete refAppLimits;

}


////////////////////////////////////////////////////////////////////////////////
//
// Test with two reference files whose validity periods do not overlap.
//
// Time              ----------------->
// ValidityPeriod 1: |---|
// ValidityPeriod 2:       |---|
//
BOOST_AUTO_TEST_CASE( two_reference_files_no_overlap )
{

  // Data for validity period 1
  UTC beforeValidityStart1 (2020, 1, 1, 1, 1, 1);
  UTC validityStart1       (2020, 1, 1, 1, 1, 2);
  UTC validityMiddle1      (2020, 1, 1, 1, 1, 3);
  UTC validityStop1        (2020, 1, 1, 1, 1, 4);
  UTC afterValidityStop1   (2020, 1, 1, 1, 1, 5);
  int32_t archRev1 = 0;
  int32_t procNum1 = 0;

  // Data for validity period 2
  UTC beforeValidityStart2 (2020, 1, 1, 1, 1, 5);
  UTC validityStart2       (2020, 1, 1, 1, 1, 6);
  UTC validityMiddle2      (2020, 1, 1, 1, 1, 7);
  UTC validityStop2        (2020, 1, 1, 1, 1, 8);
  UTC afterValidityStop2   (2020, 1, 1, 1, 1, 9);
  int32_t archRev2 = 0;
  int32_t procNum2 = 1;

  std::list<std::string> fileNames = { m_fileName1, m_fileName2 };

  // Create FITS files on disk
  createRefFile<FitsDalTable>(m_fileName1, "REF_APP_LImits", validityStart1, validityStop1, archRev1, procNum1);
  createRefFile<FitsDalTable>(m_fileName2, "REF_APP_LImits", validityStart2, validityStop2, archRev2, procNum2);

  ValidRefFile<FitsDalTable> * refAppLimits = new ValidRefFile<FitsDalTable>(fileNames);

  // Check that error is thrown for data times outside of the validity periods.

  BOOST_CHECK_THROW( refAppLimits->getFile(beforeValidityStart1), std::runtime_error );
  BOOST_CHECK_THROW( refAppLimits->getFile(afterValidityStop1),   std::runtime_error );

  BOOST_CHECK_THROW( refAppLimits->getFile(beforeValidityStart2), std::runtime_error );
  BOOST_CHECK_THROW( refAppLimits->getFile(afterValidityStop2),   std::runtime_error );

  // Check that the correct file was returned for data times within the validity periods.

  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStart1)->GetFileName(),  m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityMiddle1)->GetFileName(), m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStop1)->GetFileName(),   m_fileName1 );

  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStart2)->GetFileName(),  m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityMiddle2)->GetFileName(), m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStop2)->GetFileName(),   m_fileName2 );

  delete refAppLimits;

}

////////////////////////////////////////////////////////////////////////////////
//
// Test with two reference files whose validity periods overlap partially.
//
// Time              ----------------->
// ValidityPeriod 1: |---|
// ValidityPeriod 2:    |---|
//
BOOST_AUTO_TEST_CASE( two_reference_files_partial_overlap )
{

  // Data for validity period 1
  UTC beforeValidityStart1 (2020, 1, 1, 1, 1, 1);
  UTC validityStart1       (2020, 1, 1, 1, 1, 2);
  UTC validityMiddle1      (2020, 1, 1, 1, 1, 3);
  UTC validityStop1        (2020, 1, 1, 1, 1, 4);
  UTC afterValidityStop1   (2020, 1, 1, 1, 1, 5);
  int32_t archRev1 = 0;
  int32_t procNum1 = 0;

  // Data for validity period 2
  UTC beforeValidityStart2 (2020, 1, 1, 1, 1, 3);
  UTC validityStart2       (2020, 1, 1, 1, 1, 4);
  UTC validityMiddle2      (2020, 1, 1, 1, 1, 5);
  UTC validityStop2        (2020, 1, 1, 1, 1, 6);
  UTC afterValidityStop2   (2020, 1, 1, 1, 1, 7);
  int32_t archRev2 = 0;
  int32_t procNum2 = 1;

  std::list<std::string> fileNames = { m_fileName1, m_fileName2 };

  // Create FITS files on disk
  createRefFile<FitsDalTable>(m_fileName1, "REF_APP_LImits", validityStart1, validityStop1, archRev1, procNum1);
  createRefFile<FitsDalTable>(m_fileName2, "REF_APP_LImits", validityStart2, validityStop2, archRev2, procNum2);

  ValidRefFile<FitsDalTable> * refAppLimits = new ValidRefFile<FitsDalTable>(fileNames);


  // Check that error is thrown for data times outside of the validity periods.

  BOOST_CHECK_THROW( refAppLimits->getFile(beforeValidityStart1), std::runtime_error );
  BOOST_CHECK_THROW( refAppLimits->getFile(afterValidityStop2),   std::runtime_error );

  // Check that the correct file was returned for data times within the validity periods.

  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStart1)->GetFileName(),  m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityMiddle1)->GetFileName(), m_fileName1 );

  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStop1)->GetFileName(),   m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStart2)->GetFileName(),  m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityMiddle2)->GetFileName(), m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStop2)->GetFileName(),   m_fileName2 );

  delete refAppLimits;

}


////////////////////////////////////////////////////////////////////////////////
//
// Test with two reference files where the second file overlaps a period inside
// the first file.
//
// Time              ----------------->
// ValidityPeriod 1: |-------|
// ValidityPeriod 2:   |---|
//
BOOST_AUTO_TEST_CASE( two_reference_files_full_overlap )
{

  // Data for validity period 1
  UTC beforeValidityStart1 (2020, 1, 1, 1, 1, 1);
  UTC validityStart1       (2020, 1, 1, 1, 1, 2);
  UTC validityMiddle1      (2020, 1, 1, 1, 1, 5);
  UTC validityStop1        (2020, 1, 1, 1, 1, 8);
  UTC afterValidityStop1   (2020, 1, 1, 1, 1, 9);
  int32_t archRev1 = 0;
  int32_t procNum1 = 0;

  // Data for validity period 2
  UTC beforeValidityStart2 (2020, 1, 1, 1, 1, 3);
  UTC validityStart2       (2020, 1, 1, 1, 1, 4);
  UTC validityMiddle2      (2020, 1, 1, 1, 1, 5);
  UTC validityStop2        (2020, 1, 1, 1, 1, 6);
  UTC afterValidityStop2   (2020, 1, 1, 1, 1, 7);
  int32_t archRev2 = 0;
  int32_t procNum2 = 1;

  std::list<std::string> fileNames = { m_fileName1, m_fileName2 };

  // Create FITS files on disk
  createRefFile<FitsDalTable>(m_fileName1, "REF_APP_LImits", validityStart1, validityStop1, archRev1, procNum1);
  createRefFile<FitsDalTable>(m_fileName2, "REF_APP_LImits", validityStart2, validityStop2, archRev2, procNum2);

  ValidRefFile<FitsDalTable> * refAppLimits = new ValidRefFile<FitsDalTable>(fileNames);

  // Check that error is thrown for data times outside of the validity periods.

  BOOST_CHECK_THROW( refAppLimits->getFile(beforeValidityStart1), std::runtime_error );
  BOOST_CHECK_THROW( refAppLimits->getFile(afterValidityStop1),   std::runtime_error );

  // Check that the correct file was returned for data times within the validity periods.

  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStart1)->GetFileName(),       m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(beforeValidityStart2)->GetFileName(), m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityMiddle1)->GetFileName(),      m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStart2)->GetFileName(),       m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityMiddle2)->GetFileName(),      m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStop2)->GetFileName(),        m_fileName2 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(afterValidityStop2)->GetFileName(),   m_fileName1 );
  BOOST_CHECK_EQUAL( refAppLimits->getFile(validityStop1)->GetFileName(),        m_fileName1 );

  delete refAppLimits;

}

BOOST_AUTO_TEST_SUITE_END()
