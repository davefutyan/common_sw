/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the BJD class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 9.1 2018-02-09 RRO  #15456 BJD is related to JD and not to
 *                               modified JD, i.e. an offset of 2400000.5 is
 *                               applied.
 *  @version 3.3 2015-04-07 RRO  first released version
 */

// first include system header files


// third include the data model header files


// last include the header files of this module
#include "Mjd.hxx"
#include "Bjd.hxx"
#include "BarycentricOffset.hxx"

#define TIMECONVERT_MJD_TO_JD    2400000.5


////////////////////////////////////////////////////////////////////////////////
MJD BJD::operator - (BarycentricOffset & barycentricOffset ) const {

	// The following implementation has a small loss of accuracy (~1ms)
	// because the argument of BarycentricOffset::Offset should be MJD,
	// which would require an iterative implementation.
	return MJD(m_bjd - barycentricOffset.Offset(m_bjd - TIMECONVERT_MJD_TO_JD));
}

