/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the OrbitInterpolation() class
 *
 *  @author David Futyan, UGE
 *
 *  @version 6.5  2017-01-05 DFU first version
 */

// include the header files of this module
#include "DeltaTime.hxx"
#include "OrbitInterpolation.hxx"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
OrbitInterpolation::OrbitInterpolation(string orbitFilename, string saaMapFilename) {

	m_firstCall = true;

	//Read in the input data to be interpolated
	FitsDalTable * orbitFile = new FitsDalTable(orbitFilename,"READONLY");
	if (orbitFile->GetFileName().find("MPS_PRE_Visits") != std::string::npos) {
		FitsDalTable * mpsVisitconstraints = new FitsDalTable(orbitFile->GetFileName() + "[MPS_PRE_VisitConstraints]");
		m_orbitFile = mpsVisitconstraints;
	} else if (orbitFile->GetFileName().find("AUX_RES_VisitConstraints") != std::string::npos) {
		m_orbitFile = orbitFile;
	} else {
		throw runtime_error("Error in OrbitInterpolation constructor: input files must have either MPS_PRE_Visits or AUX_RES_VisitConstraints as the first extension");
	}

	//Read in the SAA map
	readSAAMap(saaMapFilename);

}

////////////////////////////////////////////////////////////////////////////////
void OrbitInterpolation::readSAAMap(string saaMapFilename) {

	double latitude,longitude;
	bool saaFlag;

	//Open the SAA map input fits file
	FitsDalTable * SAAMap = new FitsDalTable(saaMapFilename,"READONLY");
	SAAMap->Assign("LATITUDE", &latitude);
	SAAMap->Assign("LONGITUDE", &longitude);
	SAAMap->Assign("SAA_FLAG", &saaFlag);

	//Read in the data
	int oldlat = -999;
	int ilat = -1;
	int ilong = 0;
	while (SAAMap->ReadRow()) {
		if (latitude != oldlat) {
			ilat++;
			m_SAAMap_lat[ilat] = latitude;
			ilong = 0;
			oldlat = latitude;
		}
		if (ilat>89 || ilong>120) throw runtime_error("Error in OrbitInterpolation::readSAAMap: SAA map dimension exceeds 90(lat)x121(long)");
		m_SAAMap_value[ilat][ilong] = saaFlag;
		if (ilat == 0) m_SAAMap_long[ilong] = longitude;
		ilong++;
	}
	delete SAAMap;

}

////////////////////////////////////////////////////////////////////////////////
OrbitInterpolation::OrbitData OrbitInterpolation::interpolate(UTC utc_time) {

	double sunAngle, moonAngle, earthLimbAngle, latitude, longitude;
	double sunAngle_prev, moonAngle_prev, earthLimbAngle_prev, latitude_prev, longitude_prev;
	double sunAngle_next, moonAngle_next, earthLimbAngle_next, latitude_next, longitude_next;
	UTC utc, utc_prev, utc_next;
	bool first=true;
	bool found = false;

	//Loop over the input files

	if (m_firstCall) {
		m_orbitFile->Assign("LOS_TO_SUN_ANGLE", &sunAngle);
		m_orbitFile->Assign("LOS_TO_MOON_ANGLE", &moonAngle);
		m_orbitFile->Assign("LOS_TO_EARTH_ANGLE", &earthLimbAngle);
		m_orbitFile->Assign("LATITUDE", &latitude);
		m_orbitFile->Assign("LONGITUDE", &longitude);
		m_orbitFile->Assign("UTC_TIME", &utc, 26);
		m_firstCall = false;
	} else {
		m_orbitFile->ReAssign("LOS_TO_SUN_ANGLE", &sunAngle);
		m_orbitFile->ReAssign("LOS_TO_MOON_ANGLE", &moonAngle);
		m_orbitFile->ReAssign("LOS_TO_EARTH_ANGLE", &earthLimbAngle);
		m_orbitFile->ReAssign("LATITUDE", &latitude);
		m_orbitFile->ReAssign("LONGITUDE", &longitude);
		m_orbitFile->ReAssign("UTC_TIME", &utc);
	}

	//Read the first row
	m_orbitFile->ReadRow(1);

	//Throw an error if the time of the first row of the file is later than the target time
	if (utc>utc_time) throw runtime_error("Error in OrbitInterpolation::interpolate: requested UTC time "+utc_time.getUtc()+" not found in VisitConstraints file(s) provided to OrbitInterpolation");

	//Read in the data until the points in time before and after the target time are obtained
	do {
		if (!first) {
			utc_prev = utc_next;
			sunAngle_prev = sunAngle_next;
			moonAngle_prev = moonAngle_next;
			earthLimbAngle_prev = earthLimbAngle_next;
			latitude_prev = latitude_next;
			longitude_prev = longitude_next;
		}
		utc_next = utc;
		sunAngle_next = sunAngle;
		moonAngle_next = moonAngle;
		earthLimbAngle_next = earthLimbAngle;
		latitude_next = latitude;
		longitude_next = longitude;
		if (utc_next>utc_time) found = true;
		first = false;
	} while (m_orbitFile->ReadRow() && !found);

	if (!found) throw runtime_error("Error in OrbitInterpolation::interpolate: requested UTC time "+utc_time.getUtc()+" not found in VisitConstraints file(s) provided to OrbitInterpolation");

	//Perform linear interpolation of the data
	double mu = (utc_time-utc_prev).getSeconds()/(utc_next-utc_prev).getSeconds();
	sunAngle = sunAngle_prev*(1.-mu)+sunAngle_next*mu;
	moonAngle = moonAngle_prev*(1.-mu)+moonAngle_next*mu;
	earthLimbAngle = earthLimbAngle_prev*(1.-mu)+earthLimbAngle_next*mu;
	latitude = latitude_prev*(1.-mu)+latitude_next*mu;
	longitude = longitude_prev*(1.-mu)+longitude_next*mu;

	//Determine the latitude and longitude indices of the closest point in the SAA map
	int ilat,ilong;
	for (ilat=0; ilat<90; ilat++) {
		if (m_SAAMap_lat[ilat]>latitude) {
			if (ilat>0 && m_SAAMap_lat[ilat]-latitude > latitude-m_SAAMap_lat[ilat-1]) ilat-=1;
			break;
		}
	}
	for (ilong=0; ilong<121; ilong++) {
		if (m_SAAMap_long[ilong]>longitude) {
			if (ilong>0 && m_SAAMap_long[ilong]-longitude > longitude-m_SAAMap_long[ilong-1]) ilong-=1;
			break;
		}
	}

	//Return the interpolated data
	return OrbitInterpolation::OrbitData(sunAngle, moonAngle, earthLimbAngle, m_SAAMap_value[ilat][ilong]);

}
