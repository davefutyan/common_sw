/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the LeapSeconds() class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *
 *  @version 10.2 2018-10-11 RRO # 17327 There are no leap seconds before the
 *                                       first enty in the leap-second file.
 *  @version 3.3  2015-05-06 RRO first version
 */

// first include system header files
#include <string>
#include <time.h>

// third include the data model header files
#include "FitsDalTable.hxx"

// last include the header files of this module
#include "Utc.hxx"
#include "Mjd.hxx"
#include "LeapSeconds.hxx"
#include "ValidRefFile.hxx"

using namespace std;


std::map<UTC, int16_t> LeapSeconds::m_leapSeconds;

////////////////////////////////////////////////////////////////////////////////
void  LeapSeconds::setLeapSecondsFileNames(const std::list< std::string > & fileNames) {

   if (fileNames.size() == 0)
      // we accept this and do nothing in the moment.
      return;

   m_leapSeconds.clear();

   ValidRefFile<FitsDalTable> validRefFile(fileNames);

   time_t nowTime = time(NULL);
   struct tm * ptm1 = gmtime(&nowTime);
   UTC now(ptm1->tm_year + 1900, ptm1->tm_mon+1, ptm1->tm_mday,
           ptm1->tm_hour, ptm1->tm_min, ptm1->tm_sec, 0);

   FitsDalTable * table = validRefFile.getFile(now);

   UTC      utc;
   int16_t  taiUtc;

   table->Assign("UTC_TIME", &utc,   26);
   table->Assign("TAI_UTC",  &taiUtc);

   while (table->ReadRow()) {
      m_leapSeconds.insert(pair<UTC, int16_t>(utc, taiUtc));
   }

}

////////////////////////////////////////////////////////////////////////////////
int16_t  LeapSeconds::getNumLeapSeconds(const UTC & utc) {

   if (m_leapSeconds.size() == 0)
      throw runtime_error("There is no leap second file defined (SOC_APP_LeapSeconds)");

   map<UTC, int16_t>::reverse_iterator ir_leapSecond = m_leapSeconds.rbegin();
   while (ir_leapSecond != m_leapSeconds.rend() &&
          utc < ir_leapSecond->first) {

      ++ir_leapSecond;
   }

   if (ir_leapSecond == m_leapSeconds.rend())
      // we are before the first leap second
      return 0;

   return ir_leapSecond->second;
}
