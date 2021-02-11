/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the DeltaTime class
 *
 *  @author  Reiner Rohlfs, UGE
 *
 *  @version 5.1 2016-03-22 RRO #10458 new operators: operator+ and operator-
 *  @version 3.3 2015-06-10 RRO first released version
 */

#ifndef _DELTATIME_HXX_
#define _DELTATIME_HXX_


/** ****************************************************************************
 *  @brief   A class to store a delta time between two UTC times or two OBT times
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  This class can be used to store a delta time between two times. It is the
 *  result of the "operator -" of the UTC class and of the "operator -" of the
 *  OBT class.\n
 *  The "operator *" and the "operator /" can be used to modify the time difference
 *  and associate the time difference from a UTC time difference to a OBT time
 *  difference and vice versa.
 *
 *  usage: to calculate the UTC1 for a specific OBT1 while the correlation
 *         between UTC2 and OBT2 and the correlation slope is known:\n
 *         UTC1 = UTC2 + (OBT1 - OBT2) * slope
 *
 */
class DeltaTime {

   double  m_seconds;  ///< either OBT seconds or UTC seconds

public:

  /** *************************************************************************
    *  @brief Initializes the variable of the class with a time difference
    *
    *  @param [in] seconds  the time difference either in UTC seconds or in
    *                       OBT seconds.
    */
   DeltaTime(double seconds)
      : m_seconds(seconds)  {}

   /** *************************************************************************
     *  @brief adds seconds to this DeltaTime
     *
     *    @param [in] seconds  the returned DeltaTime is larger by this number
     *                         of seconds than this DeltaTime
     */
   DeltaTime operator + (double seconds) const {
      return DeltaTime(m_seconds + seconds);
   }

   /** *************************************************************************
     *  @brief subtracts seconds to this DeltaTime
     *
     *    @param [in] seconds  the returned DeltaTime is smaller by this number
     *                         of seconds than this DeltaTime
     */
   DeltaTime operator - (double seconds) const {
      return DeltaTime(m_seconds - seconds);
   }

   /** *************************************************************************
     *  @brief multiplies the time difference by a factor
     *
     *  This function can be used to change the time difference from a UTC
     *  delta time to a OBT delta time and vice versa.
     *
     *  @param [in] factor  this delta time is multiplied by @b factor
     */
   DeltaTime operator * (double factor) const {
      return DeltaTime(m_seconds * factor);
   }

   /** *************************************************************************
     *  @brief divides the time difference by a factor
     *
     *  This function can be used to change the time difference from a UTC
     *  delta time to a OBT delta time and vice versa.
     *
     *  @param [in] factor  this delta time is divided by @b factor
     */
   DeltaTime operator / (double factor) const {
      return DeltaTime(m_seconds / factor);
   }

   /** *************************************************************************
     *  @brief Returns the number of delta seconds
     *
     *  @return number of delta seconds
     */
   double getSeconds() const    {return m_seconds;}

};

#endif /* #define _DELTATIME_HXX_  */
