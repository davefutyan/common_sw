/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the MJD class
 *
 *  @author  Reiner Rohlfs, UGE
 *
 *  @version 5.0 2016-01-25 RRO #9933: new >, >= and <= operators
 *  @version 4.3 2015-10-21 RRO #9302 operator == and operator != implemented
 *  @version 3.3 2015-05-07 RRO #8158 const in MJD to double casting
 *  @version 3.2 2015-03-13 RRO first released version
 */


#ifndef MJD_HXX_
#define MJD_HXX_

#include <limits>

class UTC;
class BJD;
class BarycentricOffset;


/** ****************************************************************************
 *  @brief   A class to store a Modified Julian Day (MJD)
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  As time scale it is TT used, i.e. there are no leap seconds.
 *
 */
class MJD
{
private:
   double   m_mjd;      ///< the MJD time

public:


   /** *************************************************************************
    *  @brief Default constructor of the MJD class.
    *
    *  The time is set to 0.0.
    */
   MJD() : m_mjd(std::numeric_limits<double>::quiet_NaN()) {}

   /** *************************************************************************
    *  @brief Copy constructor of the MJD class.
    *
    */
   MJD(const MJD & mjd)
      : m_mjd(mjd.m_mjd)
      {  }

   /** *************************************************************************
    *  @brief Initializes the variables of the class
    *
    *  @param [in] mjd   the MJD time
    */
   MJD(double mjd)
       : m_mjd(mjd)
      { }

   /** *************************************************************************
    *  @brief Address operator, required by the fits_data_model
    *
    *  @return the MJD value of data type double
    */
   double * operator &()  {return &m_mjd;}

   /** *************************************************************************
    *  @brief Cast operator, for example required by the fits_data_model
    *
    *  @return the MJD value of data type double
    */
   operator double () const  { return m_mjd;}


   /** *************************************************************************
    *  @brief Less - operator.
    */
   bool operator < (const MJD & mjd) const
      {return m_mjd < mjd.m_mjd;}

   /** *************************************************************************
    *  @brief Less or equal - operator.
    */
   bool operator <= (const MJD & mjd) const
      {return m_mjd <= mjd.m_mjd;}

   /** *************************************************************************
    *  @brief Greater - operator.
    */
   bool operator > (const MJD & mjd) const
      {return m_mjd > mjd.m_mjd;}

   /** *************************************************************************
    *  @brief Greater of equal - operator.
    */
   bool operator >= (const MJD & mjd) const
      {return m_mjd >= mjd.m_mjd;}


   /** *************************************************************************
    *  @brief Comparison - operator, required by the BOOST_CHECK_EQUAL macro
    */
   bool operator == (double mjd) const
       {return m_mjd == mjd;}

   /** *************************************************************************
    *  @brief not equal - operator.
    *
    *  This operator has to be use with care, as it compares two double values.
    */
   bool operator != (const MJD & mjd) const
        {return m_mjd != mjd.m_mjd;}

   /** *************************************************************************
    *  @brief equal - operator.
    *
    *  This operator has to be use with care, as it compares two double values.
    */
   bool operator == (const MJD & mjd) const
         {return m_mjd == mjd.m_mjd;}


   /** *************************************************************************
    *  @brief Returns the MJD time as a double value.
    *
    */
   double getMjd() const   {return m_mjd;}

   /** *************************************************************************
    *  @brief Returns this MJD time as UTC time.
    *
    */
   UTC getUtc() const;

   /** *************************************************************************
    *  @brief Add to this MJD time the barycentric offset time and returns
    *         the BJD time.
    *
    */
   BJD operator + (BarycentricOffset & barycentricOffset ) const;

};




#endif /* MJD_HXX_ */
