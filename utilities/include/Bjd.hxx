/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the BJD class
 *
 *  @author  Reiner Rohlfs, UGE
 *
 *  @version 5.0 2016-01-25 RRO #9933: new >, >= and <= operators
 *  @version 4.3 2015-10-21 RRO #9302 operator == and operator != implemented
 *  @version 3.4 2015-05-10 RRO #8131 conversion to MJD
 *  @version 3.3 2015-05-07 RRO #8158 const in BJD to double casting
 *  @version 3.2 2015-03-13 RRO first released version
 */


#ifndef BJD_HXX_
#define BJD_HXX_

#include <limits>

#include "Mjd.hxx"


/** ****************************************************************************
 *  @brief   A class to store a Barycentric Julian Day (BJD)
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  As time scale it is TT assumed, i.e. there are no leap seconds.
 *
 */
class BJD
{
private:
   double   m_bjd;      ///< the BJD time

public:


   /** *************************************************************************
    *  @brief Default constructor of the BJD class.
    *
    *  The time is set to NaN, which will be a NULL value in a FITS table.
    */
   BJD() : m_bjd(std::numeric_limits<double>::quiet_NaN()) {}

   /** *************************************************************************
    *  @brief Copy constructor of the BJD class.
    *
    */
   BJD(const BJD & bjd)
      : m_bjd(bjd.m_bjd)
      {  }

   /** *************************************************************************
    *  @brief Initializes the variables of the class
    *
    *  @param [in] bjd   the BJD time
    */
   BJD(double bjd)
       : m_bjd(bjd)
      { }

   /** *************************************************************************
    *  @brief Address operator, required by the fits_data_model
    *
    *  @return the BJD value of data type double
    */
   double * operator &()  {return &m_bjd;}

   /** *************************************************************************
    *  @brief Cast operator, for example required by the fits_data_model
    *
    *  @return the BJD value of data type double
    */
   operator double () const { return m_bjd;}


   /** *************************************************************************
    *  @brief Less - operator.
    */
   bool operator < (const BJD & bjd) const
      {return m_bjd < bjd.m_bjd;}

   /** *************************************************************************
    *  @brief Less or equal - operator.
    */
   bool operator <= (const BJD & bjd) const
      {return m_bjd <= bjd.m_bjd;}

   /** *************************************************************************
    *  @brief Greater - operator.
    */
   bool operator > (const BJD & bjd) const
      {return m_bjd > bjd.m_bjd;}

   /** *************************************************************************
    *  @brief Greater or equal - operator.
    */
   bool operator >= (const BJD & bjd) const
      {return m_bjd >= bjd.m_bjd;}

   /** *************************************************************************
    *  @brief Comparison - operator, required by the BOOST_CHECK_EQUAL macro
    */
   bool operator == (double bjd) const
       {return m_bjd == bjd;}

   /** *************************************************************************
     *  @brief not equal - operator.
     *
     *  This operator has to be use with care, as it compares two double values.
     */
    bool operator != (const BJD & bjd) const
         {return m_bjd != bjd.m_bjd;}

    /** *************************************************************************
     *  @brief equal - operator.
     *
     *  This operator has to be use with care, as it compares two double values.
     */
    bool operator == (const BJD & bjd) const
          {return m_bjd == bjd.m_bjd;}

   /** *************************************************************************
    *  @brief Returns the BJD time as a double value.
    *
    */
   double getBjd() const   {return m_bjd;}

   /** *************************************************************************
    *  @brief Subtracts to this BJD time the barycentric offset time and returns
    *         the MJD time.
    *
    */
   MJD operator - (BarycentricOffset & barycentricOffset ) const;

};




#endif /* BJD_HXX_ */
