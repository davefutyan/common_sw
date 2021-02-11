/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the UTC class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 5.0  2016-01-25 RRO #9933: new >, >= and <= operators
 *  @version 4.3  2015-10-27 RRO #9302: new +, += and -= operators
 *  @version 3.3  2015-05-07 RRO #8158 const in UTC to string casting
 *  @version 3.2  2015-03-10 RRO Task #7620: Implement the new filename
 *                                           convention
 *  @version 3.0 2015-01-24 RRO first version
 */

#ifndef _UTC_HXX_
#define _UTC_HXX_


#include <cstdint>
#include <iostream>
#include <time.h>

class MJD;
class OBT;
class DeltaTime;

/** ****************************************************************************
 *  @brief   To store a UTC time
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  This class can store a UTC time, including fractions of seconds.
 */
class UTC
{
private:
   std::string   m_utc;  ///< UTC time in the format of yyyy-mm-ddThh:mm:ss.ffffff


   /** *************************************************************************
    *  @brief Initializes the internal utc as a string
    *
    *  @param [in] year         year of the UTC time, range: 1970 - 2037
    *  @param [in] month        month of the UTC time, range: 1 - 12
    *  @param [in] day          day of the UTC time, range: 1 - 31
    *  @param [in] hour         hour of the UTC time, range: 0 - 23
    *  @param [in] min          minutes of the UTC time, range: 0 - 59
    *  @param [in] sec          seconds of the UTC time, range: 0 - 60
    *                           (up to 60 to be able to specify leap seconds)
    *  @param [in] secFraction  fraction of seconds of the UTC time, range: < 1.0
    */
   void init(uint16_t year, uint8_t month, uint8_t day,
             uint8_t hour, uint8_t min, uint8_t sec, double secFraction);



public:

   /** *************************************************************************
    *  @brief Default constructor of the UTC class.
    *
    *  The time is set to 1970-01-01T00:00:00.000000
    */
   UTC() : m_utc("1970-01-01T00:00:00.000000")
      { }

   /** *************************************************************************
    *  @brief Initializes the class
    *
    *  param [in] utc  The UTC time of this instance. The format has to be
    *                  either yyyy-mm-ddThh:mm:ss.ffffff or
    *                  TUyyyy-mm-ddThh-mm-ss. The second option is a sub-string
    *                  of a filename. The fraction of the seconds,
    *                  i.e. ffffff is optional. The dot (.) between ss and
    *                  ffffff must not appear if ffffff is not defined.
    *
    *  @throw runtime_error if the @b utc string does not follow the required
    *                       format.
    */
   UTC(const std::string & utc);


   /** *************************************************************************
    *  @brief Initializes the class
    *
    *  @param [in] year         year of the UTC time, range: 1970 - 2037
    *  @param [in] month        month of the UTC time, range: 1 - 12
    *  @param [in] day          day of the UTC time, range: 1 - 31
    *  @param [in] hour         hour of the UTC time, range: 0 - 23
    *  @param [in] min          minutes of the UTC time, range: 0 - 59
    *  @param [in] sec          seconds of the UTC time, range: 0 - 60
    *                           (up to 60 to be able to specify leap seconds)
    *  @param [in] secFraction  fraction of seconds of the UTC time, range: < 1.0
    */
   UTC(uint16_t year, uint8_t month, uint8_t day,
       uint8_t hour, uint8_t min, uint8_t sec, double secFraction = 0.0);

   /** *************************************************************************
    *  @brief Initializes the class
    *
    *
    *  @param [in] seconds      number of seconds since 1970-01-01T00:00:00
    *  @param [in] secFraction  additional fraction of a second
    *
    */
   UTC(time_t seconds, double secFraction);

   /** *************************************************************************
    *  @brief Address operator, required by the fits_data_model
    *
    *  @return a pointer to the internal string of the MJD time
    */
   std::string * operator &()  {return &m_utc;}

   /** *************************************************************************
    *  @brief Cast operator, for example required by the fits_data_model
    *
    *  @return the MJD value of data type std::string
    */
   operator std::string () const { return m_utc;}

   /** *************************************************************************
    *  @brief Less - operator.
    */
   bool operator < (const UTC & utc) const
        {return m_utc < utc.m_utc;}

   /** *************************************************************************
    *  @brief Less or equal - operator.
    */
   bool operator <= (const UTC & utc) const
        {return m_utc <= utc.m_utc;}

   /** *************************************************************************
    *  @brief Greater - operator.
    */
   bool operator > (const UTC & utc) const
        {return m_utc > utc.m_utc;}

   /** *************************************************************************
    *  @brief Greater or equal - operator.
    */
   bool operator >= (const UTC & utc) const
        {return m_utc >= utc.m_utc;}

   /** *************************************************************************
    *  @brief not equal - operator.
    */
   bool operator != (const UTC & utc) const
        {return m_utc != utc.m_utc;}

   /** *************************************************************************
    *  @brief equal - operator.
    */
   bool operator == (const UTC & utc) const
         {return m_utc == utc.m_utc;}

   /** *************************************************************************
    *  @brief Comparison - operator, required by the BOOST_CHECK_EQUAL macro
    */
   bool operator == (const std::string & utc) const
         {return m_utc == utc;}

   /** *************************************************************************
     *  @brief Calculates the difference of two UTC times
     *
     *
     *  @param [in] utc the difference between this UTC and the @b utc is calculated.
     *
     *  @return The time difference as DeltaTime. Note: the DeltaTime is now
     *          the difference between two UTC times and not between two OBT
     *          times.
     *
     */
    DeltaTime operator - (const UTC & utc) const;

    /** *************************************************************************
     *  @brief Adds @b deltaTime to this UTC and returns the result
     *
     *  @param deltaTime It should be a time difference of two UTC times.
     *  If it is the time difference between two OBT times it first has to be
     *  "converted" into a deltaTime of two UTC times.\n
     *
     *  @return a UTC time = this UTC + deltaTime.
     */
    UTC operator + (const DeltaTime & deltaTime) const;

    /** *************************************************************************
     *  @brief Subtracts @b deltaTime to this UTC and returns the result
     *
     *  @param deltaTime It should be a time difference of two UTC times.
     *  If it is the time difference between two OBT times it first has to be
     *  "converted" into a deltaTime of two UTC times.\n
     *
     *  @return a UTC time = this UTC - deltaTime.
     */
    UTC operator - (const DeltaTime & deltaTime) const;

    /** *************************************************************************
     *  @brief Adds @b deltaTime to this UTC
     *
     *  @param deltaTime It should be a time difference of two UTC times.
     *  If it is the time difference between two OBT times it first has to be
     *  "converted" into a deltaTime of two UTC times.\n
     *
     *  @return *this  = this UTC + deltaTime.
     */
    UTC & operator += (const DeltaTime & deltaTime);

    /** *************************************************************************
     *  @brief Subtracts @b deltaTime to this UTC
     *
     *  @param deltaTime It should be a time difference of two UTC times.
     *  If it is the time difference between two OBT times it first has to be
     *  "converted" into a deltaTime of two UTC times.\n
     *
     *  @return *this = this UTC - deltaTime.
     */
    UTC & operator -= (const DeltaTime & deltaTime);



   /** *************************************************************************
    *  @brief Set time to "1970-01-01T00:00:00.000000" to define a NULL value
    */
   void clear ()  {m_utc = "1970-01-01T00:00:00.000000";}

   /** *************************************************************************
    *  @brief Returns true if the UTC time is a NULL value, i.e. is
    *         "1970-01-01T00:00:00.000000"
    */
   bool empty() const {return m_utc == "1970-01-01T00:00:00.000000";}

   /** *************************************************************************
    *  @brief Returns the UTC as formatted string.
    *
    *  @param [in] withoutSecFraction  the fraction of seconds (.ffffff) will
    *                                  not be part of the returns string if
    *                                  @b withoutSecFraction is set to true.
    *
    *  @return the format of the returned string is yyyy-mm-ddThh:mm:ss.fffff
    *          ".ffffff" is only available if @b withoutSecFraction is false
    */
   std::string getUtc(bool withoutSecFraction = false) const;

   /** *************************************************************************
    *  @brief Returns a string in the format as the UTC should be written
    *         in a CHEOPS file-name
    *
    *  The format of the returned string is TUyyyy-mm-ddThh-mm-ss
    */
    std::string getFileNamePattern() const;

   /** *************************************************************************
    *  @brief Returns the year of the UTC time in the range 1970 - 2037
    */
   uint16_t getYear() const        {return uint16_t(std::stoi(m_utc));}

   /** *************************************************************************
    *  @brief Returns the month of the UTC time in the range 1 - 12
    */
   uint8_t  getMonth() const       {return uint8_t(std::stoi(m_utc.substr(5)));}

   /** *************************************************************************
    *  @brief Returns the day of the UTC time in the range 1 - 31
    */
   uint8_t  getDay() const         {return uint8_t(std::stoi(m_utc.substr(8)));}

   /** *************************************************************************
    *  @brief Returns the hour of the UTC time in the range 0 - 23
    */
   uint8_t  getHour() const        {return uint8_t(std::stoi(m_utc.substr(11)));}

   /** *************************************************************************
    *  @brief Returns the minute of the UTC time in the range 0 - 59
    */
   uint8_t  getMinute() const      {return uint8_t(std::stoi(m_utc.substr(14)));}

   /** *************************************************************************
    *  @brief Returns the second of the UTC time in the range 0 - 60
    */
   uint8_t  getSecond() const      {return uint8_t(std::stoi(m_utc.substr(17)));}

   /** *************************************************************************
    *  @brief Returns the fraction of seconds of the UTC, always < 1.0
    */
   double   getSecFraction() const {return std::stod(m_utc.substr(19));}

   /** *************************************************************************
    *  @brief Returns the time of this UTC converted into MJD
    *
    */
   MJD getMjd() const;

   /** *************************************************************************
    *  @brief Returns the time of this UTC converted into OBT
    *
    */
   OBT getObt() const;


};

#endif /* _UTC_HXX_ */
