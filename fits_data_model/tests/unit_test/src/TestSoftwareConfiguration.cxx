#define BOOST_TEST_MAIN
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "boost/test/unit_test.hpp"
#include <fstream>
#include <stdint.h>
#include <stdio.h>

#include "FitsDalHeader.hxx"
#include "Logger.hxx"
#include "ProgramParams.hxx"
#include "SCI_COR_Lightcurve.hxx"

using namespace boost::unit_test;

struct FitsDalFixture {
   FitsDalFixture() {

     // ************************************************************************
     // Read the current svn revision from the svnversion command
     // ************************************************************************

//     FILE * stream;
//     char str[ 256 ] = { 0 };
//     stream = popen("svnversion", "r");
//     if (stream == NULL) {
//       throw std::runtime_error("Error calling svnversion");
//     }
//
//     // Read from the command until the output stream ends.
//     // svnversion returns one line
//     while( NULL != fgets( str, sizeof( str ) - 1, stream ) ) {
//       // Note the '\n' character will be stored in the string
//       currentSvnRevision = std::string(str);
//       boost::trim(currentSvnRevision);
//       std::cout << "svn revision: " << currentSvnRevision << "\n";
//      }
//
//     // Close the stream
//     int term_status = pclose( stream );
//     // Check the exit status
//     if (term_status != -1) {
//       // WEXITSTATUS is used to get the command's exit status
//       if (WEXITSTATUS(term_status) != 0) {
//         throw std::runtime_error("svnversion returned " + std::to_string(WEXITSTATUS(term_status)));
//       }
//     }
//     else {
//       throw std::runtime_error("pclose failed for svnversion");
//     }

     // ************************************************************************
     // Write the pipeline version used in this test to file
     // ************************************************************************

     const char * cheops_sw = getenv("CHEOPS_SW");
     if (cheops_sw == nullptr) {
       throw std::runtime_error("$CHEOPS_SW not defined. Cannot write $CHEOPS_SW/conf/pipeline_version");
     }

     pipelineVersionPath = std::string(cheops_sw) + "/conf/" + pipelineVersionFile;

     // If a pipeline version file already exists, read the version from it
     if (boost::filesystem::exists(pipelineVersionPath)) {
        std::ifstream file(pipelineVersionPath);
        std::string line;
        if (file.is_open()) {
           logger << info << "Reading file " << pipelineVersionPath << std::endl;
           std::getline(file, line);
           pipelineVersion = line;
           file.close();
        }
        else {
           throw std::runtime_error("Cannot read " + pipelineVersionPath);
        }
     }
     else {
       // Write the test pipeline version to file
       logger << info << "Writing file " << pipelineVersionPath << std::endl;
       std::ofstream file(pipelineVersionPath , std::ios::trunc);
       if (file.is_open()) {
          file << pipelineVersion << "\n";
          file.close();
          cleanPipelineVersionFile = true;
       }
       else {
         throw std::runtime_error("Cannot write to " + pipelineVersionPath);
       }
       logger << info << "Pipeline version is " << pipelineVersion << std::endl;

        unlink(m_fitsFile.c_str());
     }
   }

   ~FitsDalFixture() {
     unlink(m_fitsFile.c_str());

     if (cleanPipelineVersionFile && boost::filesystem::exists(pipelineVersionPath)) {
       logger << info << "Removing file " << pipelineVersionPath << std::endl;
       boost::filesystem::remove(pipelineVersionPath);
     }
   }

   ParamsPtr m_params;
   std::string pipelineVersion = "20180516T153438";
   std::string currentSvnRevision;
   std::string pipelineVersionFile = "pipeline_version";
   // Set in the constructor
   std::string pipelineVersionPath;
   bool cleanPipelineVersionFile = false;

   // The output file must not be deleted by the unit test because the software
   // configuration versions must be verified manually
   std::string m_fitsFile = "result/testSoftwareConfiguration_tmp.fits";
};

BOOST_FIXTURE_TEST_SUITE( testSoftwareConfiguration, FitsDalFixture )

////////////////////////////////////////////////////////////////////////////////
//  This test implements Infrastructure test specification
//  CHEOPS-SOC-TC-WP05-FUNC-002 Software configuration
//
//  The test specification verifies that a software product is tagged with the
//  svn revision of the software, and with the version number of the software
//  constituting the pipeline.
//
BOOST_AUTO_TEST_CASE( ProcessingVersion )
{

   m_params = CheopsInit(framework::master_test_suite().argc,
                        framework::master_test_suite().argv);

   std::string fileName = "result/testSoftwareConfiguration.fits";
   unlink(fileName.c_str());
   SciCorLightcurve * hdu = new SciCorLightcurve(fileName, "CREATE");
   BOOST_CHECK_EQUAL( hdu->getKeyPipeVer(), "N/A" );

   // Save the values of the PIPE_VER and SVN_REV keywords (they are written to
   // file in the destructor)
   delete hdu;

   FitsDalHeader * header = new FitsDalHeader(fileName);
   // Verify that a header keyword containing the svn revision exist
   BOOST_CHECK_NO_THROW( header->GetAttr<std::string>("SVN_REV") );
// If the unit tests are run without cleaning and recreating the *_init file,
// the revision in the *_init file will not necessarily reflect the actual
// svn revision.
//   BOOST_CHECK_EQUAL( header->GetAttr<std::string>("SVN_REV"), currentSvnRevision );

   // Verify that a header keyword containing the SOC release configuration base
   // line exist
   BOOST_CHECK_NO_THROW( header->GetAttr<std::string>("PIPE_VER") );
   BOOST_CHECK_EQUAL( header->GetAttr<std::string>("PIPE_VER"), pipelineVersion );
}

BOOST_AUTO_TEST_SUITE_END()
