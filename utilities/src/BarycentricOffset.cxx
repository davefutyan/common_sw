/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the BarycentricOffset() class
 *
 *  @author David Futyan, UGE
 *
 *  @version 9.1  2018-02-09 RRO #15456 BJD is related to JD and not to
 *                               modified JD, i.e. an offset of 2400000.5 is
 *                               applied.
 *  @version 6.2  2016-08-31 DFU first version
 */

// first include system header files
#include <stdexcept>
#include <time.h>
#include <iomanip>
#include <cmath>

// third include the data model header files
#include "FitsDalTable.hxx"

// last include the header files of this module
#include "Mjd.hxx"
#include "Bjd.hxx"
#include "Utc.hxx"
#include "BarycentricOffset.hxx"

using namespace std;

string BarycentricOffset::m_ephemerisFilename;

#define SCALAR_PRODUCT(A, B)        (A[0]*B[0] + A[1]*B[1] + A[2]*B[2])

/*****************************************************************************/
/*                                                                           */
/* The following lines define several constants used in time conversions.    */
/* These are taken from various sources                                      */
/*                                                                           */
#define MJDREF						0
#define TIMECONVERT_SECONDS_IN_DAY	86400.0
#define TIMECONVERT_TT_TAI_DELAY	32.184
#define TIMECONVERT_INT_MJD_TO_JD	2400001
#define TIMECONVERT_MJD_TO_JD		2400000.5
/*                                                                           */
/*****************************************************************************/

////////////////////////////////////////////////////////////////////////////////
BarycentricOffset::BarycentricOffset(double ra, double dec) : m_ra(ra), m_dec(dec) {

	// In case RA is provided in range -180 to 180 instead of 0 to 360
	if (m_ra < 0.) m_ra += 360.;

    // First a bit of sanity checking
    if (m_ra<0.0 || m_ra>=360.0) {
 	   throw runtime_error("BarycentricOffset::Offset: invalid RA: "+to_string(m_ra));
    }
    if (m_dec<-90.0 || m_dec>90.0) {
 	   throw runtime_error("BarycentricOffset::Offset: invalid DEC: "+to_string(m_dec));
    }

	// Calculate a unit vector in Cartesian coordinates from the spherical coordinates.
	calcVector();

	m_initialized = false;
    m_oldrow=-1;
}

////////////////////////////////////////////////////////////////////////////////
BarycentricOffset::BarycentricOffset(const BarycentricOffset & barycentricOffset) {

	m_ra = barycentricOffset.m_ra;
	m_dec = barycentricOffset.m_dec;

    for (int i=0;i<3;i++) m_sourceDirection[i] = barycentricOffset.m_sourceDirection[i];

    m_baryTstart    = barycentricOffset.m_baryTstart;
    m_baryTstop     = barycentricOffset.m_baryTstop;
    m_baryTimedel   = barycentricOffset.m_baryTimedel;
	for (int i=0;i<3;i++) {
		m_baryPoint[i]  = barycentricOffset.m_baryPoint[i];
		m_baryCoeff[i]  = barycentricOffset.m_baryCoeff[i];
		m_barySubin[i]  = barycentricOffset.m_barySubin[i];
	}
    m_baryEMratinv  = barycentricOffset.m_baryEMratinv;
    m_baryMsol      = barycentricOffset.m_baryMsol;
    m_baryRadSol    = barycentricOffset.m_baryRadSol;
    m_baryInvTdel   = barycentricOffset.m_baryInvTdel;
    m_baryAUfac     = barycentricOffset.m_baryAUfac;
    m_baryVELfac    = barycentricOffset.m_baryVELfac;

	m_initialized = true;
    m_oldrow=-1;
}

////////////////////////////////////////////////////////////////////////////////
double BarycentricOffset::Offset(double mjd) {

	// If not already done, read DE200 constants and initialize some variables
	if (!m_initialized) initBarycent();

	double EarthVelocity[3];
	double SolSysBaryCenter_to_Spacecraft[3],Sun_to_Spacecraft[3];
	double baryCenterShift=0.0;

	// Derive all the required vectors for the given time
	getBaryVectors(mjd,
			SolSysBaryCenter_to_Spacecraft,
			Sun_to_Spacecraft,
			EarthVelocity);

	/*
	 * Now we have all the pieces together and can calculate
	 * the overall time shift as function of the vectors
	 * the current earth velocity, the speed of light and
	 * a factor for the solar mass.
	 * The input time must be shifted to MJD first.
	 */
	baryCenterShift = TimeConvert_BaryCenter_Correction(
					MJDREF + mjd,  /* time in MJD */
					SolSysBaryCenter_to_Spacecraft,
					Sun_to_Spacecraft,
					EarthVelocity,
					m_baryMsol);

   // Return the total correction, converting the returned seconds to days.
	// #15459 BJD is related to JD and not to modified JD, i.e. an offset
	// of 2400000.5 is applied.
   return baryCenterShift/TIMECONVERT_SECONDS_IN_DAY + TIMECONVERT_MJD_TO_JD;
}

////////////////////////////////////////////////////////////////////////////////
void BarycentricOffset::calcVector() {

  // Transform everything to RADIANS
  m_ra*=(M_PI/180.);
  m_dec*=(M_PI/180.);

  double q0=cos(m_dec);
  m_sourceDirection[0]=cos(m_ra)*q0;
  m_sourceDirection[1]=sin(m_ra)*q0;
  m_sourceDirection[2]=sin(m_dec);

  for (int i=0;i<3;i++) {
    if (fabs(m_sourceDirection[i]) < 1e-10) m_sourceDirection[i] = 0;
  }

}

////////////////////////////////////////////////////////////////////////////////
void BarycentricOffset::initBarycent() {

    vector<string> Cname,Objname;
    vector<double> Cvalue;
    vector<int> Objpoint,Objcoeff,Objsubin;

	string Cname_temp,Objname_temp;
	double Cvalue_temp;
	int Objpoint_temp,Objcoeff_temp,Objsubin_temp;

	double clight,timex;

	// Open the DE200 ephemeris file and read the physical constant from DE1
	FitsDalTable * de200_de1 = new FitsDalTable(m_ephemerisFilename,"READONLY");
	de200_de1->Assign("Cname", &Cname_temp, 6);
	de200_de1->Assign("Cvalue", &Cvalue_temp);
	while (de200_de1->ReadRow()) {
		Cname.push_back(Cname_temp);
		Cvalue.push_back(Cvalue_temp);
	}
	long NumVals = Cname.size();

	// Read the data pointers from DE2
	FitsDalTable * de200_de2 = new FitsDalTable(de200_de1->GetFileName() + "[EXT_APP_DE2]");
	de200_de2->Assign("Object", &Objname_temp, 22);
	de200_de2->Assign("Pointer", &Objpoint_temp);
	de200_de2->Assign("NumCoeff", &Objcoeff_temp);
	de200_de2->Assign("NumSubIntv", &Objsubin_temp);
	while (de200_de2->ReadRow()) {
		Objname.push_back(Objname_temp);
		Objpoint.push_back(Objpoint_temp);
		Objcoeff.push_back(Objcoeff_temp);
		Objsubin.push_back(Objsubin_temp);
	}
	long NumObjs = Objname.size();

	// Read the time information from DE3
	FitsDalTable * de200_de3 = new FitsDalTable(de200_de1->GetFileName() + "[EXT_APP_DE3]");
	m_baryTstart = de200_de3->GetAttr<double>("TSTART");
	m_baryTstop = de200_de3->GetAttr<double>("TSTOP");
	m_baryTimedel = de200_de3->GetAttr<double>("TIMEDEL");
	long NumRows = 0;
	while (de200_de3->ReadRow()) NumRows++;

	delete de200_de3;
	delete de200_de2;
	delete de200_de1;

	if (NumRows!=(long) ((m_baryTstop - m_baryTstart + 0.5 ) / m_baryTimedel)) {
		throw runtime_error("BarycentricOffset::initBarycent: Number of rows in DE1 ephemeris file ("
				+to_string(NumRows)+") inconsistent with the Tstart ("+to_string(m_baryTstart)+"), Tstop ("
				+to_string(m_baryTstop)+") and Timedel ("+to_string(m_baryTimedel)+") header keywords");
	}

	// Calculate the derived physical constants
	int i=0; while (i<NumVals && Cname[i].compare("CLIGHT")) i++;
	if (i>=NumVals) throw runtime_error("BarycentricOffset::initBarycent: CLIGHT parameter not found in DE1 ephemeris file");
	clight=Cvalue[i];

	i=0; while (i<NumVals && Cname[i].compare("EMRAT")) i++;
	if (i>=NumVals) throw runtime_error("BarycentricOffset::initBarycent: EMRAT parameter not found in DE1 ephemeris file");
	m_baryEMratinv=1.0/(1.0 + Cvalue[i]);

	i=0; while (i<NumVals && Cname[i].compare("AU")) i++;
	if (i>=NumVals) throw runtime_error("BarycentricOffset::initBarycent: AU parameter not found in DE1 ephemeris file");
	timex=Cvalue[i]/clight;

	i=0; while (i<NumVals && Cname[i].compare("GMS")) i++;
	if (i>=NumVals) throw runtime_error("BarycentricOffset::initBarycent: GMS parameter not found in DE1 ephemeris file");
	m_baryMsol=Cvalue[i]*timex*timex*timex/(TIMECONVERT_SECONDS_IN_DAY*TIMECONVERT_SECONDS_IN_DAY);
	timex=Cvalue[i]/clight;

	i=0; while (i<NumVals && Cname[i].compare("RADS")) i++;
	if (i>=NumVals) {
		i=0; while (i<NumVals && Cname[i].compare("ASUN")) i++;
		if (i>=NumVals) throw runtime_error("BarycentricOffset::initBarycent: RADS and ASUN parameters not found in DE1 ephemeris file");
	}
	m_baryRadSol=Cvalue[i]/clight;
	m_baryRadSol=clight * 1000.0 ;
	m_baryInvTdel=1.0/m_baryTimedel;
	m_baryAUfac=1.0/clight ;
	m_baryVELfac=2.0/(m_baryTimedel * TIMECONVERT_SECONDS_IN_DAY) ;

	// Find the EARTH, MOON and SUN data
	i=0; while (i<NumObjs && Objname[i].compare("Earth-Moon Barycenter")) i++;
	if (i>=NumObjs) throw runtime_error("BarycentricOffset::initBarycent: Earth-Moon Barycenter not found in DE2 ephemeris file");
	m_baryPoint[BARY_EARTH_MOON]=Objpoint[i];
	m_baryCoeff[BARY_EARTH_MOON]=Objcoeff[i];
	m_barySubin[BARY_EARTH_MOON]=Objsubin[i];

	i=0; while (i<NumObjs && Objname[i].compare("Moon (geocentric)")) i++;
	if (i>=NumObjs) throw runtime_error("BarycentricOffset::initBarycent: Moon (geocentric) not found in DE2 ephemeris file");
	m_baryPoint[BARY_MOON]=Objpoint[i];
	m_baryCoeff[BARY_MOON]=Objcoeff[i];
	m_barySubin[BARY_MOON]=Objsubin[i];

	i=0; while (i<NumObjs && Objname[i].compare("Sun")) i++;
	if (i>=NumObjs) throw runtime_error("BarycentricOffset::initBarycent: Sun not found in DE2 ephemeris file");
	m_baryPoint[BARY_SUN]=Objpoint[i];
	m_baryCoeff[BARY_SUN]=Objcoeff[i];
	m_barySubin[BARY_SUN]=Objsubin[i];

	m_initialized = true;

}

////////////////////////////////////////////////////////////////////////////////
void BarycentricOffset::getBaryVectors(double tt_ijd,
				 double SolSysBaryCenter_to_Spacecraft[3],
				 double Sun_to_Spacecraft[3],
				 double EarthVelocity[3]) {

    double EarthMoonPosition[3],EarthMoonVelocity[3];
    double MoonPosition[3],MoonVelocity[3],SunPosition[3];

    // Find the location of Earth, Moon and Sun
    // for the current event time from the DE200 ephemeris
    getDE200(BARY_MOON,tt_ijd,MoonPosition,MoonVelocity);
    getDE200(BARY_EARTH_MOON,tt_ijd,EarthMoonPosition,EarthMoonVelocity);
    getDE200(BARY_SUN,tt_ijd,SunPosition,nullptr);

    /*
     * Calculate the corr. vectors
     * Solar System Barycenter-to-Spacecraft
     *    SSBC->SC = EARTH - MOON/(1+Me/Mb)
     * and Sun-to-Spacecraft
     *    Sun->SC  = SSBC->SC - SUN
     * as well as the relative velocity of the earth
     *
     * The positions of various bodies are given as vectors
     * from the Solar System Barycenter
     * m_baryEMratinv is the Earth/Moon inverse mass ratio
     */
    for (int i=0;i<3; i++) {
    	SolSysBaryCenter_to_Spacecraft[i] = EarthMoonPosition[i] - MoonPosition[i]*m_baryEMratinv;
    	Sun_to_Spacecraft[i] = SolSysBaryCenter_to_Spacecraft[i]-SunPosition[i];
    	EarthVelocity[i] = EarthMoonVelocity[i] - MoonVelocity[i]*m_baryEMratinv;
    }
    //cout << "SolSysBaryCenter_to_Spacecraft: "; for (int i=0; i<3; i++) cout << SolSysBaryCenter_to_Spacecraft[i] << " " ; cout << endl;

}

////////////////////////////////////////////////////////////////////////////////
void BarycentricOffset::getDE200(BODY body,
			   double   ttime,
			   double   pos[3],
			   double   vel[3]) {

	long newrow;
	static double time_1=0.0;
	double time_2,temp_time,dtime1,tc,twot,vfac,pc[18],vc[18];
	int intnum,i,j,np;
	vector<double>::iterator bufptr;

	// Set time in JD and check time consistency
	time_1=floor(ttime)+TIMECONVERT_MJD_TO_JD+MJDREF-0.5;
	time_2=ttime-floor(ttime)+0.5;
	if ( ( time_1 < m_baryTstart ) || ( time_1+time_2 > m_baryTstop ) ) {
		throw runtime_error("BarycentricOffset::m_getDE200: Ephemeris data time out of range");
	}

	// Find the row where the data is located. Read if it's a new row
	newrow = (time_1 - m_baryTstart) * m_baryInvTdel + 1 ;
	if (time_1 == m_baryTstop) newrow--;

	if (newrow!=m_oldrow) {

	    // Open the DE200 ephemeris file
		FitsDalTable * de200 = new FitsDalTable(m_ephemerisFilename,"READONLY");
		FitsDalTable * de200_de3 = new FitsDalTable(de200->GetFileName() + "[EXT_APP_DE3]");
		double chebCoeffs[826];
		de200_de3->Assign("ChebCoeffs", chebCoeffs, 826);

		// Read the required row
		de200_de3->ReadRow(newrow);
		m_baryBuffer.clear();
		for (unsigned i=0;i<826;i++) {
			m_baryBuffer.push_back(chebCoeffs[i]);
		}

	    // Close the DE200 ephemeris file
	    delete de200_de3;
	    delete de200;

		m_oldrow=newrow;

	}

	// Calculate the relative time interval
	time_1 = ((time_1-((newrow-1)*m_baryTimedel+m_baryTstart))+time_2)*m_baryInvTdel;

	// Interpolate the ephemeris data for relative time time_1

	// Get correct sub-interval number for this set of coefficients
	// and then get normalized Chebyshev time within that subinterval
	temp_time=m_barySubin[body]*time_1;
	dtime1=floor(time_1);
	intnum = temp_time-dtime1;

	// tc is the normalized chebyshev time (-1 <= tc <= 1)
	tc = 2.0*(temp_time-floor(temp_time)+dtime1)-1.0;

	// Compute polynomial values
	twot = tc+tc;

	// Calculate the position; Be sure that at least 'm_baryCoeff[body]' polynomials have
	// been evaluated and are stored in the array 'pc'.
	np =2;
	pc[0] = 1.0;
	pc[1] = tc;
	if ( np<m_baryCoeff[body] ) {
		for (i=np; i<m_baryCoeff[body]; i++)
			pc[i] = twot * pc[i-1] - pc[i-2] ;
		np = m_baryCoeff[body] ;
	}

	// Interpolate to get position for each component
	bufptr = m_baryBuffer.begin()+m_baryPoint[body]+intnum*np*3 -1;

	for (i=0; i<3; i++) {
		pos[i]=0.0;
		for (j=0; j<np; j++)
			pos[i]+=pc[j]*(*(bufptr++));
	}

	// Calculate the velocity if requested
	if (vel!=nullptr) {
		// If velocity interpolation is wanted, be sure enough
		// derivative polynomials have been generated and stored.
		np =3;
		vc[0] =0.0;
		vc[1] =1.0;
		vc[2] =twot+twot;

		vfac = m_barySubin[body]*m_baryVELfac;
		if ( np<m_baryCoeff[body] ) {
			for (i=np; i<m_baryCoeff[body]; i++) {
				vc[i] = twot*vc[i-1]+2.0*pc[i-1]-vc[i-2];
			}

			np = m_baryCoeff[body];
		}

		//Interpolate to get velocity for each component
		bufptr = m_baryBuffer.begin()+m_baryPoint[body]+intnum*np*3;
		for (i=0; i<3; i++) {
			vel[i] =0.0;
			for (j=1; j<np; j++) {
				vel[i] += vc[j]*(*(bufptr++));
			}

			vel[i] *=vfac;
			bufptr++ ;
		}
	}

	// Scale with AUfac
	for(i=0;i<3;i++) pos[i]*=m_baryAUfac;
	if (vel!=nullptr) for(i=0;i<3;i++) vel[i]*=m_baryAUfac;

}

double BarycentricOffset::TimeConvert_BaryCenter_Correction(
    double tt_mjd,
    double SolSysBaryCenter_to_Spacecraft[3],  // in lightseconds!
    double Sun_to_Spacecraft[3],               // in lightseconds!
    double EarthVelocity[3],                   // in lightseconds!
    double SolarMass) const {

    double GeometricCorrection=0.0;
    double EinsteinCorrection=0.0;
    double ShapiroCorrection=0.0;
    double SunDistance=0.0;
    double cosTheta=0.0;


    /* ------------------------------------------------------------
     *  COMPUTE GEOMETRIC CORRECTION
     *  ------------------------------------------------------------
     *  This is the barycentric correction due to the travelling of light
     *  from the observatory to the solar system bary center. We must project
     *  the observatory position to the object direction to get the time
     *  difference (because this difference immediately depends on the
     *  object position in respect of the earth-bary center connection).
     *  This correction can be up to 500 s.
     *
     *  geometric_correction = ( vector(SSBC->SC) * unitvector(Source) ) / c
     *
     *  NOTE: The factor of 1/c is here factored into the vector units
     *
     */

    GeometricCorrection =
	SCALAR_PRODUCT(m_sourceDirection, SolSysBaryCenter_to_Spacecraft);
	//cout << "Source direction "; for (int i=0; i<3; i++) cout << m_sourceDirection[i] << " "; cout << endl;

    /* ------------------------------------------------------------
     *  COMPUTE EINSTEIN CORRECTION
     *  ------------------------------------------------------------
     *
     *  The "Einstein" delay is a combination of gravitational redshift
     *  and time dilation due to the movement of earth and other bodies,
     *  and is caused by the time-varying gravitational potential and
     *  doppler shifts experienced by the observatory clock.
     *  To an accuracy of 1 ns, this effect can be further broken down
     *  into two components:
     *        ("Einstein") = ("Einstein")_geo + (rsat . vearth)/c^2
     *  where
     *    ("Einstein")_geo is the effect seen at the earth center
     *                     which can be calculated numerically
     *                     [TimeConvert_TTtoTDB]
     *    rsat is the vector earth->satellite
     *    vearth the earth velocity relative to the Solar System Barycenter.
     *
     *  The overall correction is typically at the millisecond level.
     *
     *  NOTE: A factor of 1/c is hidden in the vector units which are
     *        in lightseconds here instead of m/s!
     */

    EinsteinCorrection =
	TimeConvert_Difference_TTtoTDB(tt_mjd); //(rsat . vearth)/c^2 component assumed negligible

    /* ------------------------------------------------------------
     *  COMPUTE SHAPIRO CORRECTION
     *  ------------------------------------------------------------
     *
     * The "Shapiro delay" is the time delay analog to the well-known bending
     * of light near the sun: I.I. Shapiro, Phys. Rev. Lett. 13, 789 (1964)
     * It is calculated from the distance between the Sun and the satellite
     * and the cosine of the unit vectors Sun->Satellite and Satellite->Source
     *
     * In normal units it's magnitude is
     *   -(2G * M_Sun / c^3) * log (1 + cos theta)
     * An order of magnitude estimate shows that this usually in microseconds.
     */

     SunDistance = sqrt(SCALAR_PRODUCT(Sun_to_Spacecraft, Sun_to_Spacecraft));

     cosTheta = SCALAR_PRODUCT(m_sourceDirection, Sun_to_Spacecraft)
	        / SunDistance;

     ShapiroCorrection = 2 * SolarMass * log(1+cosTheta);

     // Finally return the sum of all applied corrections
     //cout << "Corrections (Geometric, Einstein, Shapiro): " << GeometricCorrection << " " << EinsteinCorrection << " " << ShapiroCorrection << endl;
     return ( GeometricCorrection + EinsteinCorrection + ShapiroCorrection);
}

double BarycentricOffset::TimeConvert_Difference_TTtoTDB(double tt_mjd) const {

  static long oldday=0;
  long day;
  static double tdbtdt;
  static double tdbtdtdot;

  day=TIMECONVERT_INT_MJD_TO_JD + (int) tt_mjd;

  if (day!=oldday) {
    oldday=day;

    tdbtdt = TimeConvert_ctatv(day, 0.0) ;
    tdbtdtdot = TimeConvert_ctatv(day, 0.5) - TimeConvert_ctatv(day, -0.5) ;
  }

  return( tdbtdt + (fmod(tt_mjd,1.0) - 0.5) * tdbtdtdot );
}

double BarycentricOffset::TimeConvert_ctatv(long   JulianDayNumber,
				double FractionalJulianDay) const {

  double t, tt, t1, t2, t3, t4, t5, t24, t25, t29, t30 ;

  t = ((JulianDayNumber-2451545) + FractionalJulianDay)/(365250.0) ;
  tt = t*t ;

  t1  =       1656.674564 * sin(  6283.075943033*t + 6.240054195)
    +        22.417471 * sin(  5753.384970095*t + 4.296977442)
    +        13.839792 * sin( 12566.151886066*t + 6.196904410)
    +         4.770086 * sin(   529.690965095*t + 0.444401603)
    +         4.676740 * sin(  6069.776754553*t + 4.021195093)
    +         2.256707 * sin(   213.299095438*t + 5.543113262)
    +         1.694205 * sin(    -3.523118349*t + 5.025132748)
    +         1.554905 * sin( 77713.772618729*t + 5.198467090)
    +         1.276839 * sin(  7860.419392439*t + 5.988822341)
    +         1.193379 * sin(  5223.693919802*t + 3.649823730)
    +         1.115322 * sin(  3930.209696220*t + 1.422745069)
    +         0.794185 * sin( 11506.769769794*t + 2.322313077)
    +         0.600309 * sin(  1577.343542448*t + 2.678271909)
    +         0.496817 * sin(  6208.294251424*t + 5.696701824)
    +         0.486306 * sin(  5884.926846583*t + 0.520007179)
    +         0.468597 * sin(  6244.942814354*t + 5.866398759)
    +         0.447061 * sin(    26.298319800*t + 3.615796498)
    +         0.435206 * sin(  -398.149003408*t + 4.349338347)
    +         0.432392 * sin(    74.781598567*t + 2.435898309)
    +         0.375510 * sin(  5507.553238667*t + 4.103476804) ;

  t2  =          0.243085 * sin(  -775.522611324*t + 3.651837925)
    +         0.230685 * sin(  5856.477659115*t + 4.773852582)
    +         0.203747 * sin( 12036.460734888*t + 4.333987818)
    +         0.173435 * sin( 18849.227549974*t + 6.153743485)
    +         0.159080 * sin( 10977.078804699*t + 1.890075226)
    +         0.143935 * sin(  -796.298006816*t + 5.957517795)
    +         0.137927 * sin( 11790.629088659*t + 1.135934669)
    +         0.119979 * sin(    38.133035638*t + 4.551585768)
    +         0.118971 * sin(  5486.777843175*t + 1.914547226)
    +         0.116120 * sin(  1059.381930189*t + 0.873504123)
    +         0.101868 * sin( -5573.142801634*t + 5.984503847)
    +         0.098358 * sin(  2544.314419883*t + 0.092793886)
    +         0.080164 * sin(   206.185548437*t + 2.095377709)
    +         0.079645 * sin(  4694.002954708*t + 2.949233637)
    +         0.075019 * sin(  2942.463423292*t + 4.980931759)
    +         0.064397 * sin(  5746.271337896*t + 1.280308748)
    +         0.063814 * sin(  5760.498431898*t + 4.167901731)
    +         0.062617 * sin(    20.775395492*t + 2.654394814)
    +         0.058844 * sin(   426.598190876*t + 4.839650148)
    +         0.054139 * sin( 17260.154654690*t + 3.411091093) ;

  t3  =          0.048373 * sin(   155.420399434*t + 2.251573730)
    +         0.048042 * sin(  2146.165416475*t + 1.495846011)
    +         0.046551 * sin(    -0.980321068*t + 0.921573539)
    +         0.042732 * sin(   632.783739313*t + 5.720622217)
    +         0.042560 * sin(161000.685737473*t + 1.270837679)
    +         0.042411 * sin(  6275.962302991*t + 2.869567043)
    +         0.040759 * sin( 12352.852604545*t + 3.981496998)
    +         0.040480 * sin( 15720.838784878*t + 2.546610123)
    +         0.040184 * sin(    -7.113547001*t + 3.565975565)
    +         0.036955 * sin(  3154.687084896*t + 5.071801441)
    +         0.036564 * sin(  5088.628839767*t + 3.324679049)
    +         0.036507 * sin(   801.820931124*t + 6.248866009)
    +         0.034867 * sin(   522.577418094*t + 5.210064075)
    +         0.033529 * sin(  9437.762934887*t + 2.404714239)
    +         0.033477 * sin(  6062.663207553*t + 4.144987272)
    +         0.032438 * sin(  6076.890301554*t + 0.749317412)
    +         0.032423 * sin(  8827.390269875*t + 5.541473556)
    +         0.030215 * sin(  7084.896781115*t + 3.389610345)
    +         0.029862 * sin( 12139.553509107*t + 1.770181024)
    +         0.029247 * sin(-71430.695617928*t + 4.183178762) ;

  t4  =          0.028244 * sin( -6286.598968340*t + 5.069663519)
    +         0.027567 * sin(  6279.552731642*t + 5.040846034)
    +         0.025196 * sin(  1748.016413067*t + 2.901883301)
    +         0.024816 * sin( -1194.447010225*t + 1.087136918)
    +         0.022567 * sin(  6133.512652857*t + 3.307984806)
    +         0.022509 * sin( 10447.387839604*t + 1.460726241)
    +         0.021691 * sin( 14143.495242431*t + 5.952658009)
    +         0.020937 * sin(  8429.241266467*t + 0.652303414)
    +         0.020322 * sin(   419.484643875*t + 3.735430632)
    +         0.017673 * sin(  6812.766815086*t + 3.186129845)
    +         0.017806 * sin(    73.297125859*t + 3.475975097)
    +         0.016155 * sin( 10213.285546211*t + 1.331103168)
    +         0.015974 * sin( -2352.866153772*t + 6.145309371)
    +         0.015949 * sin(  -220.412642439*t + 4.005298270)
    +         0.015078 * sin( 19651.048481098*t + 3.969480770)
    +         0.014751 * sin(  1349.867409659*t + 4.308933301)
    +         0.014318 * sin( 16730.463689596*t + 3.016058075)
    +         0.014223 * sin( 17789.845619785*t + 2.104551349)
    +         0.013671 * sin(  -536.804512095*t + 5.971672571)
    +         0.012462 * sin(   103.092774219*t + 1.737438797) ;

  t5  =          0.012420 * sin(  4690.479836359*t + 4.734090399)
    +         0.011942 * sin(  8031.092263058*t + 2.053414715)
    +         0.011847 * sin(  5643.178563677*t + 5.489005403)
    +         0.011707 * sin( -4705.732307544*t + 2.654125618)
    +         0.011622 * sin(  5120.601145584*t + 4.863931876)
    +         0.010962 * sin(     3.590428652*t + 2.196567739)
    +         0.010825 * sin(   553.569402842*t + 0.842715011)
    +         0.010396 * sin(   951.718406251*t + 5.717799605)
    +         0.010453 * sin(  5863.591206116*t + 1.913704550)
    +         0.010099 * sin(   283.859318865*t + 1.942176992)
    +         0.009858 * sin(  6309.374169791*t + 1.061816410)
    +         0.009963 * sin(   149.563197135*t + 4.870690598)
    +         0.009370 * sin(149854.400135205*t + 0.673880395) ;

  t24 = t * (  102.156724 * sin(  6283.075849991*t + 4.249032005)
	       +         1.706807 * sin( 12566.151699983*t + 4.205904248)
	       +         0.269668 * sin(   213.299095438*t + 3.400290479)
	       +         0.265919 * sin(   529.690965095*t + 5.836047367)
	       +         0.210568 * sin(    -3.523118349*t + 6.262738348)
	       +         0.077996 * sin(  5223.693919802*t + 4.670344204) ) ;

  t25 = t * (    0.059146 * sin(    26.298319800*t + 1.083044735)
		 +         0.054764 * sin(  1577.343542448*t + 4.534800170)
		 +         0.034420 * sin(  -398.149003408*t + 5.980077351)
		 +         0.033595 * sin(  5507.553238667*t + 5.980162321)
		 +         0.032088 * sin( 18849.227549974*t + 4.162913471)
		 +         0.029198 * sin(  5856.477659115*t + 0.623811863)
		 +         0.027764 * sin(   155.420399434*t + 3.745318113)
		 +         0.025190 * sin(  5746.271337896*t + 2.980330535)
		 +         0.024976 * sin(  5760.498431898*t + 2.467913690)
		 +         0.022997 * sin(  -796.298006816*t + 1.174411803)
		 +         0.021774 * sin(   206.185548437*t + 3.854787540)
		 +         0.017925 * sin(  -775.522611324*t + 1.092065955)
		 +         0.013794 * sin(   426.598190876*t + 2.699831988)
		 +         0.013276 * sin(  6062.663207553*t + 5.845801920)
		 +         0.012869 * sin(  6076.890301554*t + 5.333425680)
		 +         0.012152 * sin(  1059.381930189*t + 6.222874454)
		 +         0.011774 * sin( 12036.460734888*t + 2.292832062)
		 +         0.011081 * sin(    -7.113547001*t + 5.154724984)
		 +         0.010143 * sin(  4694.002954708*t + 4.044013795)
		 +         0.010084 * sin(   522.577418094*t + 0.749320262)
		 +         0.009357 * sin(  5486.777843175*t + 3.416081409) ) ;

  t29 = tt * (   0.370115 * sin(                     4.712388980)
		 +         4.322990 * sin(  6283.075849991*t + 2.642893748)
		 +         0.122605 * sin( 12566.151699983*t + 2.438140634)
		 +         0.019476 * sin(   213.299095438*t + 1.642186981)
		 +         0.016916 * sin(   529.690965095*t + 4.510959344)
		 +         0.013374 * sin(    -3.523118349*t + 1.502210314) ) ;

  t30 = t * tt * 0.143388 * sin( 6283.075849991*t + 1.131453581) ;

  return (t1+t2+t3+t4+t5+t24+t25+t29+t30) * 1.0e-6 ;
}

