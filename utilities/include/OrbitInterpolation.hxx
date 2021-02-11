/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the OrbitInterpolation class
 *
 *  @author  David Futyan, UGE
 *
 *  @version 3.2 2017-01-13 DFU first released version
 */

#ifndef ORBIT_INTERPOLATION_HXX_
#define ORBIT_INTERPOLATION_HXX_

// first include system header files
#include <string>
#include <vector>

#include "FitsDalTable.hxx"

// last include the header files of this module
#include "Utc.hxx"

/** ****************************************************************************
 *  @brief   A class to perform interpolation in time for the data stored in
 *  		 AUX_RES_Orbit and MPS_PRE_VisitConstraints
 *  @ingroup utilities
 *  @author  David Futyan, UGE
 *
 *  An instance of this class can be used to perform interpolation of
 *  		 the data stored in AUX_RES_Orbit and MPS_PRE_VisitConstraints to
 *  		 a user specified time
 *
 */
class OrbitInterpolation
{
public:

	/// @brief Structure to contain the interpolated data
	struct OrbitData {
		/** *************************************************************************
		 *  @brief Constructor
		 *
		 *  @param [in] sunAngle		Angle between the pointing direction and the Sun
		 *  @param [in] moonAngle		Angle between the pointing direction and the Moon
		 *  @param [in] earthLimbAngle	Angle between the pointing direction and the Earth limb
		 *  @param [in] SAAFlag			Flag to indicate whether or not the satellite is
		 *  							over the SAA
		 */
		OrbitData(double sunAngle, double moonAngle, double earthLimbAngle, bool SAAFlag) :
			m_sunAngle(sunAngle), m_moonAngle(moonAngle), m_earthLimbAngle(earthLimbAngle),
			m_earthOccultationFlag(earthLimbAngle<0.), m_SAAFlag(SAAFlag) {};

		double m_sunAngle; ///<Angle between the pointing direction and the Sun
		double m_moonAngle; ///<Angle between the pointing direction and the Moon
		double m_earthLimbAngle; ///<Angle between the pointing direction and the Earth limb
		bool m_earthOccultationFlag; ///<Flag to indicate whether or not the target is occulted by the Earth
		bool m_SAAFlag; ///<Flag to indicate whether or not the satellite is over the SAA
	};

	/** *************************************************************************
	 *  @brief Constructor
	 *
	 *  @param [in] orbitFilename	Filename containing the input FITS table to be interpolated.
	 *  							The FITS file must either contain AUX_RES_VisitConstraints as the first extension,
	 *  							or MPS_PRE_Visits as the first extension and MPS_PRE_VisitConstraints as the second extension
	 *  @param [in] saaMapFilename	Filename for the SAA map, provided as a FITS file
	 *  							with the EXT_APP_SAAMap data structure as the first extension
	 */
	OrbitInterpolation(std::string orbitFilename, std::string saaMapFilename);

	virtual ~OrbitInterpolation() {delete m_orbitFile;};

	/** *************************************************************************
	 *  @brief Method to interpolate the data to the user specified UTC time
	 *
	 *  @param [in] utc_time	UTC time to which the input data should be interpolated
	 */
	OrbitData interpolate(UTC utc_time);

private:

	/** *************************************************************************
	 *  @brief Method to read the SAA map
	 *
	 *  @param [in] saaMapFilename	Filename for the SAA map, provided as a FITS file
	 *  							with the EXT_APP_SAAMap data structure as the first extension
	 */
	void readSAAMap(std::string saaMapFilename);

	FitsDalTable* m_orbitFile; ///< Pointer to the input FITS table to be interpolated, corresponding to either an AUX_RES_Orbit or a MPS_PRE_VisitConstraints data structure
	int m_SAAMap_lat[90]; ///< Latitude values for SAA map
	int m_SAAMap_long[121]; ///< Longitude values for SAA map
	bool m_SAAMap_value[90][121]; ///< SAA flag values for each lat/long bin
	bool m_firstCall; ///Flag to indicate whether or not this is the first time the interpolate has been called
};


#endif /* ORBIT_INTERPOLATION_HXX_ */
