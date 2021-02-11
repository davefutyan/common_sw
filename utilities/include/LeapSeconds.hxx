/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the LeapSeconds class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 3.3   2015-05-26 RRO first version
 */

#ifndef _LEAP_SECONDS_HXX_
#define _LEAP_SECONDS_HXX_

#include <map>
#include <string>
#include <list>

#include "Utc.hxx"

/** ****************************************************************************
 *  @brief   To read the leap seconds from a REF file and provide number of
 *           leap seconds for a specific time.
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  This class can read the number of leap seconds from the SOC_APP_LeapSeconds
 *  data structure and can return the number of leap seconds that have been
 *  introduced until a specific time.
 *
 *  Currently only the UTC class is using this class to retrieve the number
 *  of leap seconds.
 */
class LeapSeconds {

private:
   /// the UTC time and the number of leap seconds until the UTC. These
   /// values are directly read from the SOC_APP_LeapSeconds data structure.
   static std::map<UTC, int16_t>  m_leapSeconds;

   /** *************************************************************************
    *  @brief   returns the number of leap seconds that are introduced until
    *           @b utc.
    *
    *  If at time @b utc a leap second was introduced than this leap second
    *  is also counted in the returned value.
    *
    *  @return number of leap seconds that are introduced until @b utc.
    *
    *  @throw runtime_error if this class was not yet initialized with a leap
    *         second file, i.e. if the method setLeapSecondsFileNames was not
    *         called before.
    */
   static int16_t  getNumLeapSeconds(const UTC & utc);

friend class UTC;

public:

   /** *************************************************************************
    *  @brief   Reads the leap seconds from a SOC_APP_LeapSeconds data structure.
    *
    *  If in the list of @b fileNames are more than 1 leap second file the most
    *  up to data file at the time when this function is called is used.
    *
    *  This function has to be called for the first call of getNumLeapSeconds().
    *  It is called by the CheopsInit() function.
    *
    *  The FITS extension, define by fileNames has to have two rows: UTC_TIME
    *  and TAI_UTC.
    */
   static void    setLeapSecondsFileNames(const std::list< std::string > & fileNames);
};



#endif /* _LEAP_SECONDS_HXX_ */
