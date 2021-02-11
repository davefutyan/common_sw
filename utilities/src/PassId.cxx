/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the PassId class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 6.1 2018-05-29 ABE #15780 == operator now returns true when
 *                                     comparing two invalid PassIds.
 *  @version 6.1 2016-07-14 RRO        change  operator std::string (); to
 *                                            operator std::string () const;
 *  @version 4.3 2015-10-21 RRO #9407  operator== and operator< implemented
 *  @version 3.3 2015-05-05 RRO #8130  update to be able to read / write a
 *                                     PassId to a FITS header
 *  @version 3.0 2015-01-26 RRO first released version
 */

// first include system header files
#include <stdexcept>
#include <string.h>

// third include the data model header files


// last include the header files of this module
#include "PassId.hxx"

using namespace std;


////////////////////////////////////////////////////////////////////////////////
void PassId::Validate() {

   if ((m_year < 10 || m_year > 30) && m_year != 0)
      throw runtime_error("The year of a PassID has to be in the range from 10 to 30."
                          " PassId - constructor found " + to_string(m_year) );

   if (m_month < 0 || m_month > 12)
      throw runtime_error("The month of a PassID has to be in the range from 1 to 12."
                          " PassId - constructor found " + to_string(m_month) );

   if (m_day < 0 || m_day > 31)
      throw runtime_error("The day of a PassID has to be in the range from 1 to 31."
                          " PassId - constructor found " + to_string(m_day) );

   if (m_hour < 0 || m_hour > 23)
      throw runtime_error("The hour of a PassID has to be in the range from 0 to 23."
                          " PassId - constructor found " + to_string(m_hour) );


}

////////////////////////////////////////////////////////////////////////////////
PassId::PassId(uint16_t year, uint8_t month, uint8_t day, uint8_t hour)
   : m_year(year > 2000 ? year - 2000 : year), m_month(month),
     m_day(day), m_hour(hour) , m_valid(true) {

   Validate();
}


////////////////////////////////////////////////////////////////////////////////
PassId::PassId(const std::string & passId) {

   try {
      int16_t offset = 0;
      if (passId.substr(0,2) == "PS")
         offset = 2;

      m_year  = stoi(passId.substr(offset + 0,2));
      m_month = stoi(passId.substr(offset + 2,2));
      m_day   = stoi(passId.substr(offset + 4,2));
      m_hour  = stoi(passId.substr(offset + 6,2));
      m_valid = true;

      Validate();
   }
   catch (invalid_argument) {
      m_valid = false;
   }

}

////////////////////////////////////////////////////////////////////////////////
PassId::operator std::string () const {

   char hstr[100];
   if (m_valid)
      sprintf(hstr, "%02hhu%02hhu%02hhu%02hhu",
            m_year, m_month, m_day, m_hour);
   else
      hstr[0] = 0;

   return string(hstr);
}

////////////////////////////////////////////////////////////////////////////////
bool PassId::operator < (const PassId & passId) const {

   if (!m_valid || !passId.m_valid)
      return false;

   if (m_year != passId.m_year)
      return m_year < passId.m_year;

   if (m_month != passId.m_month)
      return m_month < passId.m_month;

   if (m_day != passId.m_day)
      return m_day < passId.m_day;

   return m_hour < passId.m_hour;
}

////////////////////////////////////////////////////////////////////////////////
bool PassId::operator == (const PassId & passId) const {

   if (!m_valid && !passId.m_valid) {
     return true;
   }
   return m_valid && passId.m_valid &&
          m_year  == passId.m_year  &&
          m_month == passId.m_month &&
          m_day   == passId.m_day   &&
          m_hour  == passId.m_hour;
}

////////////////////////////////////////////////////////////////////////////////
string PassId::getFileNamePattern() const {

   char hstr[100];

   if (m_valid)
      sprintf(hstr, "PS%02hhu%02hhu%02hhu%02hhu",
            m_year, m_month, m_day, m_hour);
   else
      strcpy(hstr, "PS00000000");

   return string(hstr);

}
