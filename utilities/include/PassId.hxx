/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the PassId class
 *
 *  @author  Reiner Rohlfs, UGE
 *
 *  @version 6.1 2016-07-14 RRO       change  operator std::string (); to
 *                                            operator std::string () const;
 *  @version 6.0 2016-07-01 ABE #11081 return default year, month, day and hour
 *                                     if PassId is invalid
 *  @version 4.3 2015-10-21 RRO #9407 operator==, operator!= and operator< implemented
 *  @version 3.3 2015-05-05 RRO #8130 update to be able to read / write a
 *                                    PassId to a FITS header
 *  @version 3.0 2015-01-26 RRO first released version
 */

#ifndef _PASS_ID_HXX_
#define _PASS_ID_HXX_

#include <cstdint>
#include <string>


/** ****************************************************************************
 *  @brief   A class to store a full passId
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGe
 *
 *  A pass-id is defined by the time (year, month, day and hour) when the
 *  manifest file for the TM data of this pass arrived at SOC.
 *
 */
class PassId
{
private:

   uint8_t  m_year;     ///< two digits of the year [10-30]
   uint8_t  m_month;    ///< month of the year [1-12]
   uint8_t  m_day;      ///< day of the month [1-31]
   uint8_t  m_hour;     ///< hour of the day [0..23]
   bool     m_valid;    ///< set to true if this instance holds a valid pass Id

   /** *************************************************************************
    *  @brief Validates the data in this instance.
    *
    *  @throw runtime_error if one of the 4 values is outside its accepted
    *         range.
    */
    void Validate();

public:

   /** *************************************************************************
    *  @brief Default constructor of the PassId class.
    *
    *  The pass stored in this instance is not valid. Without assigning a
    *  valid PassId to this class the function PassId::isValid() will return false.
    */
   PassId()  { m_valid = false; m_year = 10; m_month = 1; m_day = 1; m_hour = 0;}

   /** *************************************************************************
    *  @brief Initializes the variables of the class
    *
    *  @param [in] year   the year of the passId, allowed range: [10 - 30] and
    *                     [2010 - 2030]
    *  @param [in] month  the month of the passId, allowed range: [1 - 12]
    *  @param [in] day    the day of the passId, allowed range: [1-31]
    *  @param [in] hour   the hour of the passId, allowed range: [0-23]
    *
    */
   PassId(uint16_t year, uint8_t month, uint8_t day, uint8_t hour);


   /** *************************************************************************
    *  @brief Initializes the variables of the class from a sub-string of a
    *         file name
    *
    *  param [in] passId  The format has to be either yymmddhh or PSyymmddhh.
    *                     The second option is a sub-string of a filename.
    *
    */
   PassId(const std::string & passId);

   /** *************************************************************************
    *  @brief Cast operator, for example required by the fits_data_model
    *
    *  @return the PassId value of data type std::string
    */
   operator std::string () const;

   /** *************************************************************************
    *  @brief Returns true if this instance holds a valid Pass Id
    */
   bool   isValid() const  {return m_valid;}

   /** *************************************************************************
    *  @brief Less - operator.
    */
   bool operator < (const PassId & passId) const;

   /** *************************************************************************
    *  @brief not equal - operator.
    */
   bool operator != (const PassId & passId) const
        {return !operator==(passId);}

   /** *************************************************************************
    *  @brief equal - operator.
    */
   bool operator == (const PassId & passId) const;

   /** *************************************************************************
    *  @brief Returns the year of the PassId in the range from 10 - 30
    */
   uint8_t getYear()  const { return m_valid ? m_year : 10; }

   /** *************************************************************************
    *  @brief Returns the month of the PassId in the range from 1 - 12
    */
   uint8_t getMonth() const { return m_valid ? m_month : 1; }

   /** *************************************************************************
    *  @brief Returns the day of the PassId in the range from 1 - 31
    */
   uint8_t getDay()   const { return m_valid ? m_day : 1; }

   /** *************************************************************************
    *  @brief Returns the hour of the PassId in the range from 0 - 23
    */
   uint8_t getHour()  const { return m_valid ? m_hour : 0; }

   /** *************************************************************************
    *  @brief Returns a string in the format as the PassId should be written
    *         in a CHEOPS file-name
    *
    *  The format of the returned string is PSyymmddhh
    */
   std::string getFileNamePattern() const;

};




#endif /* _TEMPLATE_CLASS_HXX_ */
