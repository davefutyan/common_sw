/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the MJD class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 3.2 2015-04-07 RRO  first released version
 */

// first include system header files


// third include the data model header files


// last include the header files of this module
#include "Mjd.hxx"
#include "Bjd.hxx"
#include "Utc.hxx"
#include "BarycentricOffset.hxx"


////////////////////////////////////////////////////////////////////////////////
UTC MJD::getUtc() const  {

   // get number of seconds since 1. 1. 1970
   // MJD at 1. 1. 1970 = 40587
   double _1970 = (m_mjd - 40587) * 24 * 3600;
   int32_t seconds = _1970;

   return UTC(seconds, _1970 - seconds);
}


////////////////////////////////////////////////////////////////////////////////
BJD MJD::operator + (BarycentricOffset & barycentricOffset ) const {

   return BJD(m_mjd + barycentricOffset.Offset(m_mjd));
}

