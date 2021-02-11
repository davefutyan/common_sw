/** ****************************************************************************
 *  @file
 *
 *  @ingroup bookkeeping
 *  @brief   unit_test of the Bookkeeping module
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 7.0   2017-01-20 RRO first version
 *
 */

#define BOOST_TEST_MAIN


#include "boost/test/unit_test.hpp"

#include <stdexcept>
#include <string>
#include <initializer_list>

#include "LeapSeconds.hxx"
#include "DeltaTime.hxx"
#include "Bookkeeping.hxx"

using namespace boost::unit_test;
using namespace std;

struct BookkeepingFixture {
   BookkeepingFixture() {
   }

   ~BookkeepingFixture() {
   }

};

BOOST_FIXTURE_TEST_SUITE( testBookkeeping, BookkeepingFixture )

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( VisitCounter )
{

   system("rm -rf results/VisitCounter");

   list<string> leapSecondFile {string("./resources/CH_TU1972-01-01T00-00-00_SOC_APP_LeapSeconds_V0002.fits")};
   LeapSeconds::setLeapSecondsFileNames(leapSecondFile);

   std::vector<BKNumber> master = {{"PrType",   true,  bk_uint8},
                                   {"PrId",     true,  bk_uint16},
                                   {"OrId",     true,  bk_uint16},
                                   {"VisitCtr", false, bk_uint8} };

   //////////////   create a new bookkeeping group with one item

   Bookkeeping bookkeeping1("results", "VisitCounter", master);

   // looking for visit:
   uint8_t  progType = 2;
   uint16_t progId   = 234;
   uint16_t reqId    = 53;
   UTC      visitStart = UTC("2017-01-29T12:51:43.935464");
   UTC      visitStop  = UTC("2017-02-02T15:01:32.935464");


   BKItem visit {UTC(), UTC(),
                {{"PrType",   true,  bk_uint8,  progType},
                 {"PrId",     true,  bk_uint16, 0, progId},
                 {"OrId",     true,  bk_uint16, 0, reqId },
                 {"VisitCtr", false, bk_uint8} }               };

   std::list<BKItem> requestVisits = bookkeeping1.Find(visit);

   if (requestVisits.empty()) {
      // there does not yet exist a single visit for the request ID
      // set start end end time of visit, set VisitCtr to 1 and
      // insert a new Bookkeeping - Item
      visit.m_start = visitStart;
      visit.m_stop  = visitStop;
      visit.m_bkNumbers[3].m_uint8Number = 1;
      bookkeeping1.Insert(visit);

      BOOST_CHECK_EQUAL(true, true);
   }
   else {
      // should not happen in this test
      BOOST_CHECK_EQUAL(false, true);

   }

 ////////////////   looking for a next visit of the same observation request
 ////////////////   which is not jet available

   visitStart = UTC("2017-02-02T15:03:45.342520");
   visitStop  = UTC("2017-02-03T04:23:43.935464");
   uint8_t    visitCtr = 0;

   Bookkeeping bookkeeping2("results", "VisitCounter", master);
   requestVisits = bookkeeping2.Find(visit);
   if (requestVisits.empty()) {
      // now it should not be empty
      BOOST_CHECK_EQUAL(false, true);
   }
   else {
      UTC visitCenter = visitStart + (visitStop - visitStart) * 0.5;
      uint8_t currentHigestVisitCtr = 0;

      std::list<BKItem>::const_iterator i_requestedVisits = requestVisits.begin();
      while (i_requestedVisits != requestVisits.end()) {
         if (currentHigestVisitCtr < i_requestedVisits->m_bkNumbers[3].m_uint8Number)
            currentHigestVisitCtr = i_requestedVisits->m_bkNumbers[3].m_uint8Number;

         if (i_requestedVisits->m_start < visitCenter && i_requestedVisits->m_stop > visitCenter) {
            // this visit exists already. Take the visitCtr
            visitCtr = i_requestedVisits->m_bkNumbers[3].m_uint8Number;
            break;
         }
         i_requestedVisits++;
      }

      if (visitCtr == 0 ) {
         // this is a new visit
         visitCtr = currentHigestVisitCtr + 1;
         visit.m_start = visitStart;
         visit.m_stop  = visitStop;
         visit.m_bkNumbers[3].m_uint8Number = visitCtr;
         bookkeeping2.Insert(visit);

      }
      BOOST_CHECK_EQUAL(visitCtr, 2);
   }

   ////////////////   looking for a visit that already exist

   visitStart = UTC("2017-01-29T12:51:43.935464");
   visitStop  = UTC("2017-02-02T15:01:32.935464");
   visitCtr = 0;

   Bookkeeping bookkeeping3("results", "VisitCounter", master);
   requestVisits = bookkeeping3.Find(visit);
   if (requestVisits.empty()) {
      // now it should not be empty
      BOOST_CHECK_EQUAL(false, true);
   }
   else {
      UTC visitCenter = visitStart + (visitStop - visitStart) * 0.5;
      uint8_t currentHigestVisitCtr = 0;

      std::list<BKItem>::const_iterator i_requestedVisits = requestVisits.begin();
      while (i_requestedVisits != requestVisits.end()) {
         if (currentHigestVisitCtr < i_requestedVisits->m_bkNumbers[3].m_uint8Number)
            currentHigestVisitCtr = i_requestedVisits->m_bkNumbers[3].m_uint8Number;

         if (i_requestedVisits->m_start < visitCenter && i_requestedVisits->m_stop > visitCenter) {
            visitCtr = i_requestedVisits->m_bkNumbers[3].m_uint8Number;
            break;
         }
         i_requestedVisits++;
      }

     if (visitCtr == 0 ) {
        // now we should have found the visit
        BOOST_CHECK_EQUAL(false, true);
     }

     BOOST_CHECK_EQUAL(visitCtr, 1);
  }

}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( DataReductionChain )
{

   system("rm -rf results/DataReductionChain");


   std::vector<BKNumber> master = {{"PrType",    true, bk_uint8},
                                   {"PrId",      true, bk_uint16},
                                   {"OrId",      true, bk_uint16},
                                   {"VisitCtr",  true, bk_uint8},
                                   {"FirstData", false,bk_UTC,},
                                   {"ARCH_REV",  true, bk_uint8},
                                   {"PROC_NUM",  true, bk_uint8}};

   //////////////   create a new bookkeeping group with one item

   Bookkeeping bookkeeping1("results", "DataReductionChain", master);

   // looking for visit:
   uint8_t  progType = 2;
   uint16_t progId   = 234;
   uint16_t reqId    = 53;
   uint8_t  visitCtr = 5;
   uint8_t  arch_rev = 0;
   uint8_t  proc_num = 2;
   UTC      visitStart = UTC("2017-01-29T12:51:43.935464");
   UTC      visitEnd   = UTC("2017-02-02T15:01:32.935464");

   // for testing we define a visit of an other observation request
   UTC      firstData  = UTC("2017-01-29T19:34:43");
   BKItem test_visit {visitStart, visitEnd,
                {{"PrType",    true,  bk_uint8,  progType},
                 {"PrId",      true,  bk_uint16, 0, 233},
                 {"OrId",      true,  bk_uint16, 0, reqId },
                 {"VisitCtr",  true,  bk_uint8,  visitCtr},
                 {"FirstData", false, bk_UTC,  0, 0, firstData},
                 {"ARCH_REV",  true,  bk_uint8,  arch_rev},
                 {"PROC_NUM",  true,  bk_uint8,  proc_num}     } };
   bookkeeping1.Insert(test_visit);

   // define another visit one day later
   UTC      visitStart1 = UTC("2017-02-29T12:51:43.935464");
   UTC      visitEnd1   = UTC("2017-03-02T15:01:32.935464");
   BKItem visit {visitStart1, visitEnd1,
                {{"PrType",    true, bk_uint8,  progType},
                 {"PrId",      true, bk_uint16, 0, progId},
                 {"OrId",      true, bk_uint16, 0, reqId },
                 {"VisitCtr",  true, bk_uint8,  visitCtr},
                 {"FirstData", false, bk_UTC,  0, 0, firstData},
                 {"ARCH_REV",  true, bk_uint8,  arch_rev},
                 {"PROC_NUM",  true, bk_uint8,  proc_num}     } };

   std::list<BKItem> requestVisits1 = bookkeeping1.Find(visit);

   if (requestVisits1.empty()) {
      // this visit was not yet processed by Data Reduction Chain
      bookkeeping1.Insert(visit);

      BOOST_CHECK_EQUAL(true, true);
   }
   else {
      // should not happen in this test
      BOOST_CHECK_EQUAL(false, true);
   }

   ////////////////    test again for the same visit

   Bookkeeping bookkeeping2("results", "DataReductionChain", master);
   std::list<BKItem> requestVisits2 = bookkeeping2.Find(visit);

   if (requestVisits2.empty()) {
      // this visit was not yet processed by Data Reduction Chain
      bookkeeping2.Insert(visit);

      // should not happen in this test
      BOOST_CHECK_EQUAL(false, true);
   }
   else {
      // this visit was already processed by Data Reduction Chain

      BOOST_CHECK_EQUAL(true, true);
   }

}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( ErrorHandling )
{

   system("rm -rf results/ErrorHandling");

   std::vector<BKNumber> master = {{"N1",   true,  bk_uint8},
                                   {"N2",   true,  bk_uint16},
                                   {"N3",   false, bk_uint8}  };
   bool throw_exception = false;

   ///////////////   create a new bookkeeping group in a directory which does
   //////////////    not exist
   try {
      Bookkeeping bookkeeping("does_not_exist", "ErrorHandling", master);
   }
   catch (std::runtime_error & error) {
      throw_exception = true;
   }
   BOOST_CHECK_EQUAL(throw_exception, true);



   //////////////   create a new bookkeeping group

   Bookkeeping * bookkeeping = new Bookkeeping("results", "ErrorHandling", master);

   //////////////   open the bookkeeping group, but with a different master

   std::vector<BKNumber> master2 = {{"N1",  true,  bk_uint8},
                                    {"N3",  false, bk_uint8},
                                    {"N2",  true,  bk_uint16} };

   throw_exception = false;
   try {
     Bookkeeping * bookkeeping2 = new Bookkeeping("results", "ErrorHandling", master2);
     delete bookkeeping2;
   }
   catch (std::runtime_error & error) {
      throw_exception = true;
   }
   BOOST_CHECK_EQUAL(throw_exception, true);


   ////////////// insert a BKItem with different BKNumbers than the master

   BKItem visit {UTC(), UTC(),
                {{"N1",   true,  bk_uint8, 6},
                 {"N2",   true,  bk_uint16, 0, 245},
                 {"N3",   true,  bk_uint8, 89} }     };


   throw_exception = false;
   try {
      bookkeeping->Insert(visit);
   }
   catch (std::runtime_error & error) {
      throw_exception = true;
   }
   BOOST_CHECK_EQUAL(throw_exception, true);


   //////////////  insert twice the same BKItem
   BKItem visit2 {UTC("2017-01-29T12:51:43.935464"),
                  UTC("2017-01-29T14:51:43.935464"),
                 {{"N1",   true,  bk_uint8, 6},
                  {"N2",   true,  bk_uint16, 0, 245},
                  {"N3",   false, bk_uint8, 89} }     };


   throw_exception = false;
   try {
      bookkeeping->Insert(visit2);
      bookkeeping->Insert(visit2);
   }
   catch (std::runtime_error & error) {
      // this should be possible, no exception should be thrown
      throw_exception = true;
   }
   BOOST_CHECK_EQUAL(throw_exception, false);

   //////////    insert a new BKItem for a time range,which is at least
   //////////    partially already used by an other BKItem
   BKItem visit3 {UTC("2017-01-29T13:51:43.935464"),
                  UTC("2017-01-29T16:51:43.935464"),
                 {{"N1",   true,  bk_uint8, 6},
                  {"N2",   true,  bk_uint16, 0, 245},
                  {"N3",   false, bk_uint8, 02} }     };

   throw_exception = false;
   try {
      bookkeeping->Insert(visit3);
   }
   catch (std::runtime_error & error) {
      throw_exception = true;
   }
   BOOST_CHECK_EQUAL(throw_exception, true);



   delete bookkeeping;


}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( Unsigned32BitInteger )
{

   system("rm -rf results/Unsigned32BitInteger");


   std::vector<BKNumber> master = {{"PrType",    true, bk_uint8},
                                   {"PrId",      true, bk_uint16},
                                   {"OrId",      true, bk_uint16},
                                   {"VisitCtr",  true, bk_uint8},
                                   {"Obsid",     true, bk_uint32}};

   //////////////   create a new bookkeeping group with one item

   Bookkeeping bookkeeping1("results", "Unsigned32BitInteger", master);

   // looking for visit:
   uint8_t  progType = 2;
   uint16_t progId   = 234;
   uint16_t reqId    = 53;
   uint8_t  visitCtr = 5;

   uint32_t obsid1 = 1234567890;
   uint32_t obsid2 = 1234567891;

   UTC      visitStart1 = UTC("2017-01-29T12:51:43.935464");
   UTC      visitEnd1   = UTC("2017-02-02T15:01:32.935464");
   UTC      visitStart2 = UTC("2017-02-29T12:51:43.935464");
   UTC      visitEnd2   = UTC("2017-03-02T15:01:32.935464");

   // for testing we define a visit of an other observation request
   BKItem visitBk1 {visitStart1, visitEnd1,
                {{"PrType",    true,  bk_uint8,  progType},
                 {"PrId",      true,  bk_uint16, 0, progId},
                 {"OrId",      true,  bk_uint16, 0, reqId },
                 {"VisitCtr",  true,  bk_uint8,  visitCtr},
                 {"Obsid",     true,  bk_uint32, 0, 0, UTC(), obsid1}}};
   bookkeeping1.Insert(visitBk1);

   // define another visit one day later
   BKItem visitBk2 {visitStart2, visitEnd2,
                {{"PrType",    true, bk_uint8,  progType},
                 {"PrId",      true, bk_uint16, 0, progId},
                 {"OrId",      true, bk_uint16, 0, reqId },
                 {"VisitCtr",  true, bk_uint8,  visitCtr},
                 {"Obsid",     true, bk_uint32, 0, 0, UTC(), obsid2}}};

   std::list<BKItem> requestVisits1 = bookkeeping1.Find(visitBk2);
   BOOST_CHECK(requestVisits1.empty());

   bookkeeping1.Insert(visitBk2);
   std::list<BKItem> requestVisits2 = bookkeeping1.Find(visitBk2);
   BOOST_CHECK_EQUAL(requestVisits2.size(), 1);
}



BOOST_AUTO_TEST_SUITE_END()
