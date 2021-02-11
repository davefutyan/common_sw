/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the UTC() class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 9.2  2018-10-11 RRO #17327 the offset of 32.184 shall not applied
 *                                      for UTC times before 1977
 *  @version 8.0  2017-08-03 RRO #14321 include in the tests the rounding offset
 *  @version 5.1  2016-03-19 RRO set maximum year to 2037
 *  @version 4.3  2015-10-27 RRO #9302: new +, += and -= operators
 *  @version 3.2  2015-03-10 RRO #7620: Implement the new filename convention
 *  @version 3.0  2015-01-24 RRO first version
 */

// first include system header files
#include <stdexcept>
#include <time.h>

// third include the data model header files


// last include the header files of this module
#include "Utc.hxx"
#include "Mjd.hxx"
#include "Obt.hxx"
#include "LeapSeconds.hxx"
#include "ObtUtcCorrelation.hxx"
#include "DeltaTime.hxx"

using namespace std;


static const uint8_t MAX_DAY[13] =
      {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static string MONTH_NAME[13] = {
      "",
      "January",
      "February",
      "March",
      "April",
      "May",
      "June",
      "July",
      "August",
      "September",
      "October",
      "November",
      "December"
};

static int64_t getNumDays(int64_t year, int64_t month, int64_t day) {

   // number of days since MJD = 0
   // code taken from INTEGRAL DAL3GEN

   /* The algorithm is taken directly from Fliegel and Van Falndern           */
   /* (1968): "Explanatory Supplement to the Astronomical Almanac"            */

   int64_t j1 = 1461*(year + 4800 + (month - 14)/12)/4;
   int64_t j2 = 367*(month - 2 - (month - 14)/12*12)/12;
   int64_t j3 = 3*((year + 4900 + (month - 14)/12)/100)/4;
   // it is 2400001 and not 240000.5 because the algorithm assumes 12h00
   // and not midnight (comment taken from DAL3Gen of INTEGRAL code)
   int64_t numDays = (day - 32075 + j1 + j2 - j3) - 2400001;

   return numDays;

}

////////////////////////////////////////////////////////////////////////////////
UTC::UTC(const std::string & utc)  {

   if (utc.compare(0,2, "TU") == 0) {
      // it is a UTC from a filename
      if (utc.length() != 21 || utc[6] != '-' || utc[9] != '-' ||
          utc[12] != 'T' || utc[15] != '-' || utc[18] != '-')
          throw runtime_error("The format of the UTC as string is not as expected."
                              "Expected: TUyyyy-mm-ddThh-mm-ss, but found: " + utc);

      m_utc = utc.substr(2);
      m_utc[13] = ':';
      m_utc[16] = ':';
      m_utc += ".000000";
   }
   else {
      // it is a standard UTC string
      if (utc.length() == 19 && utc[4] == '-' && utc[7] == '-' &&
          utc[10] == 'T' && utc[13] == ':' && utc[16] == ':') {
         m_utc = utc + ".000000";
      }
      else if (utc.length() > 19 && utc[4] == '-' && utc[7] == '-' &&
            utc[10] == 'T' && utc[13] == ':' && utc[16] == ':'&& utc[19] == '.') {
         m_utc = utc;
         m_utc.resize(26, '0');
      }
      else {
         throw runtime_error("The format of the UTC as string is not as expected."
                             "Expected: yyyy-mm-ddThh:mm:ss.ffffff, but found: " + utc);
      }

   }
}

////////////////////////////////////////////////////////////////////////////////
UTC::UTC(uint16_t year, uint8_t month, uint8_t day,
         uint8_t hour, uint8_t min, uint8_t sec, double secFraction) {


   init(year, month, day, hour, min, sec, secFraction);

}

////////////////////////////////////////////////////////////////////////////////
UTC::UTC(time_t seconds, double secFraction) {

   // transform TT time into TAI if the time is after 1. 1. 1977
   // 1. 1. 1977 = 2557 days * 86400 sec / day = 220924800 sec
   if (seconds >=  220924800) {
      seconds -= 32;
      secFraction -= 0.184;
   }

   // #14321 In the init function a rounding offset of 0.0000005 is added
   if (secFraction  < -0.0000005) {
      secFraction += 1;
      seconds -= 1;
   }

   // assume first that TAI - UTC = 0 and get number of leap seconds,
   // which may be wrong by 1
   struct tm * ptm1 = gmtime(&seconds);
   init(ptm1->tm_year + 1900, ptm1->tm_mon+1, ptm1->tm_mday,
        ptm1->tm_hour, ptm1->tm_min, ptm1->tm_sec, secFraction);

   int16_t leapSeconds1 = LeapSeconds::getNumLeapSeconds(*this);

   // now TAI - UTC = leapseconds
   seconds -= leapSeconds1;
   struct tm * ptm2 = gmtime(&seconds);
   init(ptm2->tm_year + 1900, ptm2->tm_mon+1, ptm2->tm_mday,
        ptm2->tm_hour, ptm2->tm_min, ptm2->tm_sec, secFraction);

   // test again number of leap second
   int16_t leapSeconds2 = LeapSeconds::getNumLeapSeconds(*this);

   if (leapSeconds1 != leapSeconds2) {
      // the previously derived number of leap seconds was indeed wrong
      // take the correct number of leap seconds
      seconds = seconds + leapSeconds1 - leapSeconds2;
      ptm2 = gmtime(&seconds);
      if(ptm2->tm_min == 0 &&ptm2->tm_sec == 0) {
         // we are exactly at a leap second
         // reduce the seconds by 1 for the gmtime function and
         // increase the tm_sec by 1, which will become 60
         seconds -= 1;
         ptm2 = gmtime(&seconds);
         ptm2->tm_sec +=1;
      }

      init(ptm2->tm_year + 1900, ptm2->tm_mon+1, ptm2->tm_mday,
           ptm2->tm_hour, ptm2->tm_min, ptm2->tm_sec, secFraction);
   }


}

////////////////////////////////////////////////////////////////////////////////
void UTC::init(uint16_t year, uint8_t month, uint8_t day,
               uint8_t hour, uint8_t min, uint8_t sec, double secFraction) {

   if (year < 1970 || year > 2037)
      throw runtime_error("The year of a utc has to be in the range from 1970 to 2037."
                          " UTC - constructor found " + to_string(year) );

   if (month < 1 || month > 12)
      throw runtime_error("The month of a utc has to be in the range from 1 to 12."
                          " UTC - constructor found " + to_string(month) );

   if (day < 1 || day > MAX_DAY[month])
       throw runtime_error("The day of a utc for month " + MONTH_NAME[month] +
                           " has to be in the range from 1 to " +
                           to_string(MAX_DAY[month]) +
                           ". UTC - constructor found " + to_string(day) );

   if (hour > 23)
      throw runtime_error("The hour of a utc has to be in the range from 0 to 23."
                           " UTC - constructor found " + to_string(hour) );

   if (min > 59)
      throw runtime_error("The minute of a utc has to be in the range from 0 to 59."
                           " UTC - constructor found " + to_string(min) );

   if (sec > 60)
      throw runtime_error("The second of a utc has to be in the range from 0 to 60."
                           " UTC - constructor found " + to_string(sec) );

   // #14321 In the next sprintf function a rounding offset of 0.0000005 is added
   if (secFraction >= 1.0 || secFraction < -0.0000005)
      throw runtime_error("The fraction of a second of a utc has to be less than 1.0."
                           " UTC - constructor found " + to_string(secFraction) );

   char hstr[100];
   sprintf(hstr, "%04hu-%02hhu-%02hhuT%02hhu:%02hhu:%02hhu.%06d",
                 year, month, day, hour, min, sec, int32_t(secFraction * 1000000 + 0.5));

   m_utc = hstr;
}

////////////////////////////////////////////////////////////////////////////////
DeltaTime UTC::operator - (const UTC & utc) const {

   int64_t thisNumDays = getNumDays(getYear(), getMonth(), getDay());
   int64_t inNumDays   = getNumDays(utc.getYear(), utc.getMonth(), utc.getDay());

   double thisNumSeconds = getHour() * 3600.0 + getMinute() * 60.0 +
                           getSecond() + getSecFraction() +
                           LeapSeconds::getNumLeapSeconds(*this);

   double inNumSeconds   = utc.getHour() * 3600.0 + utc.getMinute() * 60.0 +
                           utc.getSecond() + utc.getSecFraction() +
                           LeapSeconds::getNumLeapSeconds(utc);

   return DeltaTime( (thisNumDays - inNumDays) * 86400.0 +
                     thisNumSeconds - inNumSeconds);

}

////////////////////////////////////////////////////////////////////////////////
UTC UTC::operator + (const DeltaTime & deltaTime) const {

   int64_t numDays = getNumDays(getYear(), getMonth(), getDay());


   // number of days since 1. 1. 1970
   numDays -= 40587;

   double seconds = getHour() * 3600.0 + getMinute() * 60.0 +
                    getSecond() + getSecFraction() +
                    LeapSeconds::getNumLeapSeconds(*this);

   seconds += numDays * 86400.0 + deltaTime.getSeconds();

   if (getYear() >= 1977)
      // the 32.184 sec are introduced 1. 1. 1977
      seconds += 32.184;

   time_t numSeconds = seconds;

   return UTC(numSeconds, seconds - numSeconds);

}

////////////////////////////////////////////////////////////////////////////////
UTC UTC::operator - (const DeltaTime & deltaTime) const {

   int64_t numDays = getNumDays(getYear(), getMonth(), getDay());

   // number of days since 1. 1. 1970
   numDays -= 40587;

   double seconds = getHour() * 3600.0 + getMinute() * 60.0 +
                    getSecond() + getSecFraction() +
                    LeapSeconds::getNumLeapSeconds(*this);

   seconds += numDays * 86400.0 - deltaTime.getSeconds();

   if (getYear() >= 1977)
      // the 32.184 sec are introduced 1. 1. 1977
      seconds += 32.184;


   time_t numSeconds = seconds;

   return UTC(numSeconds, seconds - numSeconds);

}

////////////////////////////////////////////////////////////////////////////////
UTC & UTC::operator += (const DeltaTime & deltaTime) {

   int64_t numDays = getNumDays(getYear(), getMonth(), getDay());

   // number of days since 1. 1. 1970
   numDays -= 40587;

   double seconds = getHour() * 3600.0 + getMinute() * 60.0 +
                    getSecond() + getSecFraction() +
                    LeapSeconds::getNumLeapSeconds(*this);

   seconds += numDays * 86400.0 + deltaTime.getSeconds();

   if (getYear() >= 1977)
      // the 32.184 sec are introduced 1. 1. 1977
      seconds += 32.184;


   time_t numSeconds = seconds;

   *this = UTC(numSeconds, seconds - numSeconds);

   return *this;
}

////////////////////////////////////////////////////////////////////////////////
UTC & UTC::operator -= (const DeltaTime & deltaTime) {

   int64_t numDays = getNumDays(getYear(), getMonth(), getDay());

   // number of days since 1. 1. 1970
   numDays -= 40587;

   double seconds = getHour() * 3600.0 + getMinute() * 60.0 +
                    getSecond() + getSecFraction() +
                    LeapSeconds::getNumLeapSeconds(*this);

   seconds += numDays * 86400.0 - deltaTime.getSeconds();

   if (getYear() >= 1977)
      // the 32.184 sec are introduced 1. 1. 1977
      seconds += 32.184;

   time_t numSeconds = seconds;

   *this = UTC(numSeconds, seconds - numSeconds);

   return *this;

}


////////////////////////////////////////////////////////////////////////////////
string UTC::getUtc(bool withoutSecFraction) const {

   if (withoutSecFraction)
      return m_utc.substr(0, 19);

   return m_utc;

}

////////////////////////////////////////////////////////////////////////////////
std::string UTC::getFileNamePattern() const {

   string utc = getUtc(true);
   utc[13] = '-';
   utc[16] = '-';
   return "TU" + utc;
}

////////////////////////////////////////////////////////////////////////////////
MJD UTC::getMjd() const {

   int64_t year = getYear();
   int64_t month = getMonth();
   int64_t day   = getDay();

   // number of days since MJD = 0
   // code taken from INTEGRAL DAL3GEN

   /* The algorithm is taken directly from Fliegel and Van Falndern           */
   /* (1968): "Explanatory Supplement to the Astronomical Almanac"            */

   int64_t j1 = 1461*(year + 4800 + (month - 14)/12)/4;
   int64_t j2 = 367*(month - 2 - (month - 14)/12*12)/12;
   int64_t j3 = 3*((year + 4900 + (month - 14)/12)/100)/4;
   // it is 2400001 and not 240000.5 because the algorithm assumes 12h00
   // and not midnight (comment taken from DAL3Gen of INTEGRAL code)
   int64_t numDays = (day - 32075 + j1 + j2 - j3) - 2400001;

   int16_t leapSeconds = LeapSeconds::getNumLeapSeconds(*this);

   double mjd = numDays;
   /// 32.184 is the delay between TT and TAI time system
   mjd += getHour() / 24.0 + getMinute() / 1440.0 +
          (getSecond()+getSecFraction() + leapSeconds +
                ((year >= 1977) ? 32.184 : 0.0)) / 86400.0;

   return MJD(mjd);
}

OBT UTC::getObt() const {
   return OBTUTCCorrelation::getObt(*this);
}
