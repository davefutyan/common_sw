/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the OBT class
 *
 *  @author  Reiner Rohlfs, UGE
 *
 *  @version 5.1 2016-03-24 RRO #10458: implementation of the OBT reset counter
 *  @version 5.0 2016-01-25 RRO #9933: new >, >= and <= operators
 *  @version 4.4 2015-12-04 RRO #9902: new constructor: OBT::OBT(int64_t obt);
 *  @version 4.3 2015-10-27 RRO #9302: new +, += and -= operators
 *  @version 4.3 2015-10-27 RRO #9541 synchronization bit changed from MSB to LSB
 *  @version 4.1 2015-07-26 RRO make OBT::getUtc() a const method
 *  @version 3.2 2015-04-03 RRO the obt is now a int_64 data type.
 *  @version 3.0 2015-01-26 RRO first released version
 */

#ifndef _OBT_HXX_
#define _OBT_HXX_

#include <cstdint>
#include <iostream>

#include <set>
#include <list>

#include <Utc.hxx>
class DeltaTime;

/** ****************************************************************************
 *  @brief   A class to store an On-Board-Time (OBT)
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  A OBT is defined as seconds + fraction of seconds since an arbitrary start.
 *  The length of a OBT second may be slightly different than a UTC second.
 *  The least significant bit is used to define whether the OBT of the instrument
 *  was synchronized (1) with the spacecraft OBT or not (0). \n
 *  As soon as the OBT is used in the CHEOPS system an OBT reset counter is
 *  assigned to the time. This reset counter is stored in the two most significant
 *  bytes.
 */
class OBT
{
private:

   /** *************************************************************************
    *  @brief The OBT time, as found in a TM packet, including the
    *         synchronized bit (bit 0) and the reset counter (byte 6 and 7)
    *
    *  The synchronization bit (bit 0) is set when the OBT is synchronized
    *  with the clock of the spacecraft.
    */
   int64_t  m_obt;


   /** *************************************************************************
    *  @brief   A container struct to store one OBT reset record
    *  @ingroup utilities
    *  @author  Reiner Rohlfs, UGE
    *
    */
   struct ResetRecord {
      int16_t     m_resetCounter;    ///< reset counter, first = 0.
      UTC         m_validityStart;   ///< @brief this reset counter is approximately
                                     ///  valid since this UTC
      int64_t     m_obtDiff;         ///< @brief (OBT after reset) - (OBT before the reset);
                                     ///  without reset counter. Typically less then 0.
      int64_t     m_firstObt;        ///< @brief first OBT value after the reset
                                     ///  without reset counter.

      ResetRecord(int16_t resetCounter, UTC validittyStart, int64_t obtDiff, int64_t firstObt)
         : m_resetCounter(resetCounter), m_validityStart(validittyStart),
           m_obtDiff(obtDiff), m_firstObt(firstObt)  {
      }

      /** *************************************************************************
       *  @brief   Constructor used to find records by their reset counter in
       *           a set of ResetRecords.
       *
       */
    ResetRecord(int16_t resetCounter) :  m_resetCounter(resetCounter) {

      }

      /** **********************************************************************
       *  @brief Standard < operator.
       *
       *  It uses the reset counter to define the order, i.e. to sort the records.
       */
     bool operator < (const ResetRecord & resetRecord) const {
         return m_resetCounter < resetRecord.m_resetCounter;
      }
   };

   /// set of all OBT reset records, sorted by reset counter.
   static std::set<ResetRecord> m_resetRecords;

   /** *************************************************************************
    *  @brief Sets the reset counter in this OBT, depending on @b utc..
    *
    *  The function does nothing if utc == UTC() or if no reset-counter
    *  REF file was read. The @b utc has to be accurate compared to this obt with
    *  at most an inaccuracy of 2 days.
    */
  void SetResetCounter(const UTC & utc);

  /** *************************************************************************
   *  @brief Updates the reset counter and the obt if the obt is outside the
   *         time range of the @b resetCounter
   *
   *  The function does nothing if no reset-counter
   *
   *  @return a OBT including the reset counter in the bytes 6 and 7.
   */
  int64_t UpdateResetCoounter(int64_t obt, int16_t resetCounter) const;

public:

   /** *************************************************************************
    *  @brief Default constructor of the OBT class.
    *
    *  The OBT is set to 0, but synchronized is not set, i.e. time is not
    *  synchronized. OBT reset counter is set to 0.
    */
   OBT() : m_obt(0)
      { }

   /** *************************************************************************
    *  @brief Copy constructor of the OBT class.
    *
    */
   OBT(const OBT & obt) : m_obt(obt.m_obt)
      { }

   /** *************************************************************************
    *  @brief Initializes the on-board time of the class.
    *
    *  The @b obt is defined as the SC Time in the TM packets:
    *  1 time tick is 2 ** -32 seconds.\n
    *
    *  @param [in] obt           the OBT time, including synchronization bit,
    *                            but without reset counter
    *  @param [in] utc           The approximately time of the @b obt as UTC.
    *                            The UTC time does not need to be very precise.
    *                            But should be in the range of +/- 2 days.
    *                            It is used to identify the correct OBT reset
    *                            counter. If it is not define (used the default
    *                            UTC time) or if no ObtReset file was provided
    *                            then the OBT reset counter is not updated.
    */
   OBT(int64_t obt, UTC utc = UTC()) : m_obt(obt) {
      SetResetCounter(utc);
   }


   /** *************************************************************************
    *  @brief Initializes the on-board time of the class.
    *
    *  The @b obt is defined as the SC Time in the TM packets:
    *  1 time tick is 2 ** -32 seconds.\n
    *  The synchronized bit can also be set as bit 0 of the @b obt.
    *
    *  @param [in] obt           the OBT time, without synchronization bit and
    *                            without reset counter.
    *  @param [in] synchronized  true: the @b obt is synchronized with the clock
    *                            of the spacecraft.\n
    *                            false: the @b obt is not synchronized.
    *  @param [in] utc           The approximately time of the @b obt as UTC.
    *                            The UTC time does not need to be very precise.
    *                            But should be in the range of +/- 2 days.
    *                            It is used to identify the correct OBT reset
    *                            counter. If it is not define (used the default
    *                            UTC time) or if no ObtReset file was provided
    *                            then the OBT reset counter is not updated.
    */
   OBT(int64_t obt, bool synchronized, UTC utc = UTC()) : m_obt(obt)
      { if (synchronized) m_obt |= 0x01LL;
        else m_obt &= 0xFFFFFFFFFFFFFFFELL;
      SetResetCounter(utc);
      }

   /** *************************************************************************
    *  @brief Address operator, required by the fits_data_model
    *
    *  @return the address of the OBT value of data type int64_t including the
    *          synchronized bit and the reset counter.
    */
   int64_t * operator &()  {return &m_obt;}

   /** *************************************************************************
    *  @brief Cast operator, for example required by the fits_data_model
    *
    *  @return the OBT value of data type int64_t, including the
    *          synchronization bit and the reset counter.
    */
   operator int64_t () const    {return m_obt;}


   /** *************************************************************************
    *  @brief Less - operator. The synchronized bit is ignored, but the reset
    *         counter is applied.
    */
   bool operator < (const OBT & obt) const
      {return (m_obt | 0x01LL) < (obt.m_obt | 0x01LL);}

   /** *************************************************************************
    *  @brief Less or equal - operator. The synchronized bit is ignored but the reset
    *         counter is applied.
    */
   bool operator <= (const OBT & obt) const
      {return (m_obt | 0x01LL) <= (obt.m_obt | 0x01LL);}

   /** *************************************************************************
    *  @brief Greater - operator. The synchronized bit is ignored but the reset
    *         counter is applied.
    */
   bool operator > (const OBT & obt) const
      {return (m_obt | 0x01LL) > (obt.m_obt | 0x01LL);}

   /** *************************************************************************
    *  @brief Greater or equal - operator. The synchronized bit is ignored but the reset
    *         counter is applied.
    */
   bool operator >= (const OBT & obt) const
      {return (m_obt | 0x01LL) >= (obt.m_obt | 0x01LL);}


   /** *************************************************************************
    *  @brief Comparison - operator, required by the BOOST_CHECK_EQUAL macro
    *
    *  The comparison depends also on the synchronized bit (bit 0).
    */
   bool operator == (int64_t obt) const
       {return m_obt == obt;}

   /** *************************************************************************
    *  @brief Calculates the difference of two OBT times
    *
    *  The calculation ignores the synchronized bit (bit 0), but any reset of
    *  the OBT between the two OBTs are recognized and correctly applied.
    *
    *  @param [in] obt the difference between this OBT and the @b obt is calculated.
    *
    *  @return The time difference as DeltaTime. Note: the DeltaTime is now
    *          the difference between two OBT times and not between two UTC
    *          times.
    *
    */
   DeltaTime operator - (const OBT & obt) const;

   /** *************************************************************************
    *  @brief Adds @b deltaTime to this OBT and return the result
    *
    *  The synchronization bit is not modified by this operation. A reset of
    *  the OBT which could happen during this @b deltaTime are correctly
    *  applied.
    *
    *  @param deltaTime It should be a time difference of two OBT times.
    *  If it is the time difference between two UTC times it first has to be
    *  "converted" into a deltaTime of two OBT times.\n
    *
    *  @return a OBT time = this OBT + deltaTime.
    *
    */
   OBT operator + (const DeltaTime & deltaTime) const;

   /** *************************************************************************
    *  @brief Subtracts @b deltaTime to this OBT and returns the result
    *
    *  The synchronization bit is not modified by this operation. A reset of
    *  the OBT which could happen during this @b deltaTime are correctly
    *  applied.
    *
    *  @param deltaTime It should be a time difference of two OBT times.
    *  If it is the time difference between two UTC times it first has to be
    *  "converted" into a deltaTime of two OBT times.\n
    *
    *  @return a OBT time = this OBT - deltaTime.
    *
    */
   OBT operator - (const DeltaTime & deltaTime) const;

   /** *************************************************************************
    *  @brief Adds @b deltaTime to this OBT
    *
    *  The synchronization bit is not modified by this operation A reset of
    *  the OBT which could happen during this @b deltaTime are correctly
    *  applied.
    *
    *  @param deltaTime It should be a time difference of two OBT times.
    *  If it is the time difference between two UTC times it first has to be
    *  "converted" into a deltaTime of two OBT times.\n
    *
    *  @return *this = this OBT += deltaTime.
    *
    */
   OBT & operator += (const DeltaTime & deltaTime);

   /** *************************************************************************
    *  @brief Subtracts @b deltaTime to this OBT
    *
    *  The synchronization bit is not modified by this operation. A reset of
    *  the OBT which could happen during this @b deltaTime are correctly
    *  applied.
    *
    *  @param deltaTime It should be a time difference of two OBT times.
    *  If it is the time difference between two UTC times it first has to be
    *  "converted" into a deltaTime of two OBT times.\n
    *
    *  @return *this = this OBT -= deltaTime.
    *
    */
   OBT & operator -= (const DeltaTime & deltaTime);

   /** *************************************************************************
    *  @brief Returns the OBT time as found in a TM packet, i.e. including the
    *         synchronized bit and the reset counter
    *
    */
   int64_t getObt() const   {return m_obt;}

   /** *************************************************************************
     *  @brief Return true if this OBT is synchronized with the spacecraft
     *         clock.
     *
     */
   bool isSynchronized() const   {return (m_obt & 0x01);}

   /** *************************************************************************
     *  @brief Return the reset counter of this OBT
     *
     */
   int16_t getResetCounter() const  {return m_obt >> 48;}

   /** *************************************************************************
    *  @brief Returns the time of this OBT converted into UTC
    *
    */
   UTC getUtc() const;

   /** *************************************************************************
     *  @brief Sets the file name of the REF_APP_ObtReset file.
     *
     *  The static struct m_resetRecords is filled.
     *
     */
   static void SetResetCounterFileNames(const std::list< std::string > & fileNames);
};


#endif /* _OBT_HXX_ */
