/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the OBT class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 5.1 2016-03-24 RRO #10458: implementation of the OBT reset counter
 *  @version 4.3 2015-10-27 RRO #9302: new +, += and -= operators
 *  @version 4.3 2015-10-27 RRO #9541 synchronization bit changed from MSB to LSB
 *  @version 4.1 2015-07-26 RRO make OBT::getUtc() a const method
 *  @version 3.2 2015-04-03 RRO remove functions to read and write OBT into filenames
 *  @version 3.0 2015-01-26 RRO first released version
 */

// first include system header files

// third include the data model header files

// last include the header files of this module
#include "Obt.hxx"
#include "Utc.hxx"
#include "ObtUtcCorrelation.hxx"
#include "DeltaTime.hxx"
#include "FitsDalTable.hxx"


std::set<OBT::ResetRecord> OBT::m_resetRecords;

void OBT::SetResetCounter(const UTC & utc) {

   if (utc == UTC()  || m_resetRecords.size() == 0)
      return;

   std::set<ResetRecord>::reverse_iterator ir_resetRecord = m_resetRecords.rbegin();

   while (utc < ir_resetRecord->m_validityStart) {

      ir_resetRecord++;
      if(ir_resetRecord == m_resetRecords.rend()) {
         // should never happen, but you never know
         return;
      }
   }

   // we accept if the utc is inaccurate by 2 days. If the time of the reset was less
   // than 2 days before utc we consider that the obt may belong to the previous
   // reset.
   if (((utc - ir_resetRecord->m_validityStart).getSeconds() <  172800) &&
       (m_obt > (ir_resetRecord->m_firstObt -  ir_resetRecord->m_obtDiff - (172800LL << 16)))) {
      ir_resetRecord++;
      if(ir_resetRecord == m_resetRecords.rend()) {
         ir_resetRecord--;
      }
   }

   // we accept if the utc is inaccurate by 2 days. If the time of the reset was less
   // than 2 days after utc we consider that the obt may belong to the next
   // reset.
   if (ir_resetRecord != m_resetRecords.rbegin())
      {
      auto ir_nextResetRecord = ir_resetRecord;
      ir_nextResetRecord--;

      if (((ir_nextResetRecord->m_validityStart - utc).getSeconds() < 172800) &&
          (m_obt < (ir_nextResetRecord->m_firstObt + (172800LL << 16))))
         ir_resetRecord--;
      }

   m_obt = (m_obt & 0xFFFFFFFFFFFF) +
           (((int64_t)ir_resetRecord->m_resetCounter) << 48);

}

////////////////////////////////////////////////////////////////////////////////
int64_t OBT::UpdateResetCoounter(int64_t obt, int16_t resetCounter) const {

   if (m_resetRecords.size() == 0)
      return obt + ((int64_t)resetCounter << 48);

   std::set<ResetRecord>::const_iterator i_resetRecord =
         m_resetRecords.upper_bound(resetCounter);

   while (i_resetRecord != m_resetRecords.end() &&
          obt > i_resetRecord->m_firstObt - i_resetRecord->m_obtDiff) {

      obt +=  i_resetRecord->m_obtDiff;

      resetCounter = i_resetRecord->m_resetCounter;

      i_resetRecord++;
   }

   while (i_resetRecord != m_resetRecords.begin()) {
      i_resetRecord--;

      if (obt < i_resetRecord->m_firstObt) {
         obt -=  i_resetRecord->m_obtDiff;

         resetCounter = i_resetRecord->m_resetCounter -1;
      }
      else
         break;
   }

   if (obt < 0 ) obt = 0;

   return obt + ((int64_t)resetCounter << 48);

}

////////////////////////////////////////////////////////////////////////////////
DeltaTime OBT::operator - (const OBT & obt) const {

   if ((m_obt & 0xFFFF000000000000LL) == (obt.m_obt & 0xFFFF000000000000LL)) {
      // simple case: identical reset counter.
      return DeltaTime( ((m_obt | 0x01LL) - (obt.m_obt | 0x01LL))
                        / double(0x10000)  );
   }

   int64_t resetOffset = 0;
   std::set<ResetRecord>::iterator i_resetRecord =
         m_resetRecords.upper_bound(ResetRecord(obt.m_obt >> 48));
   while (i_resetRecord != m_resetRecords.end())  {
      // m_obtDiff is typically a negative number
      resetOffset -= i_resetRecord->m_obtDiff;

      if (i_resetRecord->m_resetCounter >= (m_obt >> 48))
         break;

      i_resetRecord++;
      }

   return DeltaTime( (((m_obt     | 0x01LL) & 0xFFFFFFFFFFFF) -
                      ((obt.m_obt | 0x01LL) & 0xFFFFFFFFFFFF) +
                      resetOffset)
                     / double(0x10000)  );

}

////////////////////////////////////////////////////////////////////////////////
OBT OBT::operator + (const DeltaTime & deltaTime) const {

   int16_t resetCounter = m_obt >> 48;
   int64_t newObtWithoutReset =  (m_obt & 0xFFFFFFFFFFFELL)
               + int64_t(deltaTime.getSeconds() * 0x10000 + 0.5);

   return OBT( UpdateResetCoounter(newObtWithoutReset, resetCounter), isSynchronized() );
}

////////////////////////////////////////////////////////////////////////////////
OBT OBT::operator - (const DeltaTime & deltaTime) const {

   int16_t resetCounter = m_obt >> 48;
   int64_t newObtWithoutReset = (m_obt & 0xFFFFFFFFFFFELL)
                     - int64_t(deltaTime.getSeconds() * 0x10000 + 0.5);


   return OBT( UpdateResetCoounter(newObtWithoutReset, resetCounter), isSynchronized() );
}

////////////////////////////////////////////////////////////////////////////////
OBT & OBT::operator += (const DeltaTime & deltaTime)  {

   bool synchronized = isSynchronized();

   int16_t resetCounter = m_obt >> 48;

   int64_t newObtWithoutReset = (m_obt & 0xFFFFFFFFFFFELL)
                     + int64_t(deltaTime.getSeconds() * 0x10000 + 0.5);

   m_obt = UpdateResetCoounter(newObtWithoutReset, resetCounter);

   if (synchronized)
      m_obt |= 0x01LL;
   else
      m_obt &= 0xFFFFFFFFFFFFFFFELL;

   return *this;
}

////////////////////////////////////////////////////////////////////////////////
OBT & OBT::operator -= (const DeltaTime & deltaTime)  {

   bool synchronized = isSynchronized();
   int16_t resetCounter = m_obt >> 48;

   int64_t newObtWithoutReset = (m_obt & 0xFFFFFFFFFFFELL)
                     - int64_t(deltaTime.getSeconds() * 0x10000 + 0.5);

   m_obt = UpdateResetCoounter(newObtWithoutReset, resetCounter);

   if (synchronized)
      m_obt |= 0x01LL;
   else
      m_obt &= 0xFFFFFFFFFFFFFFFELL;

   return *this;
}


////////////////////////////////////////////////////////////////////////////////
UTC OBT::getUtc() const {

   return OBTUTCCorrelation::getUtc(*this);

}

////////////////////////////////////////////////////////////////////////////////
void OBT::SetResetCounterFileNames(const std::list< std::string > & fileNames) {

   if (fileNames.size() == 0)
      return;

   // in case this function is called more than once, we delete the previous
   // input and use only the resets define in this new file.
   m_resetRecords.clear();

   // we expect only one file. Just in case we use the last in the input list
   FitsDalTable obtResetTable(fileNames.back());

   uint16_t    resetCounter;
   int64_t     obtDiff;
   std::string resetUtc;
   int64_t     firstObt = 3;

   obtResetTable.Assign("OBT_RESET_COUNTER", &resetCounter);
   obtResetTable.Assign("RESET_UTC",         &resetUtc, 26);
   obtResetTable.Assign("OBT_DIFF",          &obtDiff);
   obtResetTable.Assign("FIRST_OBT",         &firstObt);

   while (obtResetTable.ReadRow())   {
      m_resetRecords.insert(ResetRecord(resetCounter, UTC(resetUtc),
                                        obtDiff, firstObt));
       }

}
