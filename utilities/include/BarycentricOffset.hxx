/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the BarycentricOffset class
 *
 *  @author  Reiner Rohlfs, UGE
 *
 *  @version 9.3 2018-06-04 #16346 improved documentation.
 *  @version 3.2 2015-04-17 RRO first released version
 */


#ifndef BARYCENTRIC_OFFSET_HXX_
#define BARYCENTRIC_OFFSET_HXX_

#include <string>
#include <vector>

/** ****************************************************************************
 *  @brief   A class to calculate the time difference of BJD - MJD
 *  @ingroup utilities
 *  @author  David Futyan, UGE
 *
 *  An instance of this class can calculate the time offset between
 *  Barycentic Julian Day BJD and Modified Julian Day MJD for a specific
 *  source in the sky, i.e. for a specific RA / DEC.
 *
 */
class BarycentricOffset
{
public:

   /** *************************************************************************
    *  @brief The class is initialized for a specific source, i.e. with its
    *         sky position RA / DEC
    *
    *  @param [in] ra  Right Ascension of the source for which the offset shall
    *                  be calculated. It has to be in the range [-180 .. 360[
    *                  The unit is deg.
    *  @param [in] dec Declination of the source for which the offset shall
    *                  be calculated. It has to be in the range [-90 .. 90]
    *                  The unit is deg.
    */
   BarycentricOffset(double ra, double dec);

   /** *************************************************************************
    *  @brief Copy constructor of the BarycentricOffset class.
    *
    */
   BarycentricOffset(const BarycentricOffset & barycentricOffset);

   /** *************************************************************************
    *  @brief Method to calculate the time offset between BJD and MJD
    *
    *  @param [in] mjd  MJD for which the time offset shall be calculated
    *
    *  @return the offset between BJD and MJD ( = BJD - MJD) in days
    */
   double Offset(double mjd);

   /** *************************************************************************
    *  @brief Defines the file containing the ephemeris data in a EXT_APP_DE1
    *  		  data structure. It is called by the CheopsInit() function.
    *
    */
   static void setEphemerisFileName(const std::string fileName) {m_ephemerisFilename = fileName;}

private:

   enum BODY{BARY_EARTH_MOON,BARY_MOON,BARY_SUN};

   /** *************************************************************************
    *  @brief Transforms a RA and Dec orientation in degrees into a unit vector
    *  in J2000 ECI coordinates
    *
    */
   void calcVector();

   /** *************************************************************************
    *  @brief Read physical constants from the ephemiris file and initialize
    *  some variables
    *
    */
   void initBarycent();

   /** *************************************************************************
    *  @brief Calculate the various vectors needed for the barycentric correction
    *  at the given input time, which is expected to be IJD.
    *
    */
   void getBaryVectors(double tt_ijd,
   				 double SolSysBaryCenter_to_Spacecraft[3],
   				 double Sun_to_Spacecraft[3],
   				 double EarthVelocity[3]);

   /** *************************************************************************
    *  @brief This function reads the ISDC DE200 ephemeris. Bodies are restricted
    *  to Earth-Moon barycenter (SSB), Moon (Geocentric) and Sun (SSB).
    *  Returns position and velocities in 3-vectors if pointer!=NULL
    *
    */
   void getDE200(BODY body, double ttime, double pos[3], double vel[3]);

   /** *************************************************************************
    *  @brief Calculates the shift in seconds between an observed time in MJD
    *  and the time of measurement at the solar system barycenter.
    *
    *  The correction from a measured time to solar system barycenter
    *  can be generally expressed as follows:
    *
    *  barycentered_time = observed_time
    *                      + clock corrections             [done before]
    *                      - dispersion                    [0 for X-rays]
    *                      + geometric light travel time   [up to ~500s]
    *                      + "Einstein corrections"        [millisec]
    *                      - "Shapiro delay"               [microsec]
    *
    *  We assume here that all corrections from the spacecraft clock
    *  to geocentric terrestrial time have already been done.
    *  The dispersion corrections are zero for X-rays.
    *  [See also http://lheawww.gsfc.nasa.gov/users/craigm/bary/]
    *
    *  All corrections below are expressed in _seconds_.
    */
    double TimeConvert_BaryCenter_Correction (
       double tt_mjd,
       double SolSysBaryCenter_to_Spacecraft[3],  /* in lightseconds! */
       double Sun_to_Spacecraft[3],               /* in lightseconds! */
       double EarthVelocity[3],                   /* in lightseconds! */
       double SolarMass) const;

    /** *************************************************************************
     *  @brief Calculate the difference in seconds between a TT time and a TDB time.
     *  The input TT time is expected to be in Modified Julian Days!
     *
     *  The order of magnitude for these corrections is typically ms
     *  and the drift of corrections within a single day is of order 10^-5 s.
     *
     *  It uses the coefficients from Fairhead & Bretagnon 1990,
     *  A&A 229, 240, as provided by TimeConvert_ctatv.
     *  The accuracy is better than 100 ns.
     *
     *  The proper way to do all this is to abandon TDB and use TCB.
     *
     *  The way this is done is as follows: TDB-TT and its derivative are
     *  calculated for the integer part of the Julian Day (when needed).
     *  The precise value is derived from the fractional part and derivative.
     */
    double TimeConvert_Difference_TTtoTDB(double tt_mjd) const;

    /** *************************************************************************
     *  @brief Computes the cumulative relativistic time correction to
     *  earth-based clocks, TDB-TDT, for a given time. Routine
     *  furnished by the Bureau des Longitudes, modified by
     *  removal of terms much smaller than 0.1 microsecond.
     *
     *  Method:
     *    Convert input time to millennia since 2000.0
     *    For each sinusoidal term of sufficient amplitude
     *       Compute value of term at input time
     *    End for
     *    Add together all terms and convert from microseconds to seconds
     *
     *  Results of this routine has been confirmed up to (1E-10)microsecond
     *  level compared with the calculation by the IBM machine: this seems
     *  to be within the 64-bit precision allowance, but still the original
     *  hardcopy should be kept as the right one. (M.M.)
     *
     *  Note for the C version: the accuracy is guaranteed to 100 ns.
    */
    double TimeConvert_ctatv(long JulianDayNumber, double FractionalJulianDay) const;

    double  m_ra;      ///< the RA of the source
    double  m_dec;     ///< the DEC of the source
    double 	m_sourceDirection[3]; ///< cartesian unit vector in the direction of the source

    bool m_initialized; ///< boolean to indicate whether or not the data members of the class have been initialized

    static std::string m_ephemerisFilename; ///< path to the ephemeris file


    /*****************************************************************************/
    /* DE200 ephemeris data                                                      */

    double       m_baryTstart   ;
    double       m_baryTstop    ;
    double       m_baryTimedel  ;

    int          m_baryPoint[3] ;
    int          m_baryCoeff[3] ;
    int          m_barySubin[3] ;
    double       m_baryEMratinv ;
    double       m_baryMsol     ;
    double       m_baryRadSol   ;
    double       m_baryInvTdel  ;
    double       m_baryAUfac    ;
    double       m_baryVELfac   ;

    std::vector<double> m_baryBuffer	;
    long 		 m_oldrow		;
};

#endif /* BARYCENTRIC_OFFSET_HXX_ */
