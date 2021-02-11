/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Implementation of the OBTUTCCorrelation class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *
 *  @version 10.3.1 2018-11-27  RRO throw an exception if the GRADIENT is NULL
 *  @version 10.0.1 2018-08-20  RRO #16725 follow the change of the iOBTUTC
 *                                       of the MOC-SOC ICD 3.3 in method
 *                                       ReadCorrelationRecords()
 *  @version 3.4 2015-05-22 RRO first version
 */

// first include system header files
#include <cmath>

// third include the data model header files
#include "FitsDalTable.hxx"

// last include the header files of this module
#include "DeltaTime.hxx"
#include "ObtUtcCorrelation.hxx"



using namespace std;

OBTUTCCorrelation::CorrelationRecord::SEARCH_TIME
      OBTUTCCorrelation::CorrelationRecord::s_searchTime =
              OBTUTCCorrelation::CorrelationRecord::S_OBT;

std::set<OBTUTCCorrelation::CorrelationRecord>
      OBTUTCCorrelation::m_correlationRecords;

std::list<std::string> OBTUTCCorrelation::m_correlationFileNames;

////////////////////////////////////////////////////////////////////////////////
void OBTUTCCorrelation::ReadCorrelationRecords() {

   m_correlationRecords.clear();

   std::list<std::string>::const_iterator i_fileName = m_correlationFileNames.begin();
   while (i_fileName != m_correlationFileNames.end()) {

      FitsDalTable hdu(*i_fileName);

      UTC timeStamp;
      UTC utc;
      OBT obt;
      double gradient;
      double offset;
      double tcOffset;

      hdu.Assign("UTC_TIMESTAMP", &timeStamp,   26);
      hdu.Assign("UTC",           &utc,         26);
      hdu.Assign("OBT",           &obt);
      hdu.Assign("GRADIENT",      &gradient);
      hdu.Assign("OFFSET",        &offset);
      hdu.Assign("TC_OFFSET",     &tcOffset);


      while (hdu.ReadRow())   {
         if (std::isnan(gradient))
            throw runtime_error("GRADIENT of a OBT-UTC Correlation is NULL");


         if (!std::isnan(tcOffset) && offset > 10000000.0)
            // new definition MOC-SOC ICD issue >= 3.3
            m_correlationRecords.insert(CorrelationRecord(timeStamp, utc, obt,
                                                          gradient, tcOffset));
         else
            // old definition MOC-SOC ICD issue <= 3.2
            m_correlationRecords.insert(CorrelationRecord(timeStamp, utc, obt,
                                                          gradient, offset));
         }

      i_fileName++;
   }
}

////////////////////////////////////////////////////////////////////////////////
OBT OBTUTCCorrelation::getObt(const UTC & utc) {

   CorrelationRecord::SetSearchTime(CorrelationRecord::S_UTC);

   if (m_correlationRecords.size() == 0)
      ReadCorrelationRecords();

   if (m_correlationRecords.size() == 0) {
      throw runtime_error("There is no OBT - UTC Correlation File defined.");
   }

   set<CorrelationRecord>::const_iterator i_corrRecord =
         m_correlationRecords.upper_bound(CorrelationRecord(utc));
   if (i_corrRecord != m_correlationRecords.begin())
      i_corrRecord--;

   return i_corrRecord->m_obt + (utc - i_corrRecord->m_utc - i_corrRecord->m_offset) / i_corrRecord->m_gradient;

}

////////////////////////////////////////////////////////////////////////////////
UTC OBTUTCCorrelation::getUtc(const OBT & obt) {

   CorrelationRecord::SetSearchTime(CorrelationRecord::S_OBT);

   if (m_correlationRecords.size() == 0)
      ReadCorrelationRecords();

   if (m_correlationRecords.size() == 0) {
      throw runtime_error("There is no OBT - UTC Correlation File defined.");
   }

   set<CorrelationRecord>::const_iterator i_corrRecord =
         m_correlationRecords.upper_bound(CorrelationRecord(obt));
   if (i_corrRecord != m_correlationRecords.begin())
      i_corrRecord--;

   return i_corrRecord->m_utc + ((obt - i_corrRecord->m_obt) * i_corrRecord->m_gradient + i_corrRecord->m_offset);

}
