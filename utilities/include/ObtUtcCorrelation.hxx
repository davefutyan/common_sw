/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the OBTUTCCorrelation class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 10.0.1 2018-08-20  RRO #16725 follow the change of the iOBTUTC
 *                                       of the MOC-SOC ICD 3.3
 *                                       new member: CorrelationRecord:: m_obtTimeStamp
 *  @version 5.1 2016-03-22 RRO #10458 follow the changes of the iOBTUTC interface with MOC
 *  @version 4.1 2015-08-12 RRO doxygen documentation
 *  @version 3.4 2015-05-22 RRO first version
 */

#ifndef _OBTUTCCORRELATION_HXX_
#define _OBTUTCCORRELATION_HXX_

#include <string>
#include <set>
#include <list>

#include "Utc.hxx"
#include "Obt.hxx"
#include "DeltaTime.hxx"

/** ****************************************************************************
 *  @brief   A helper class to perform the OBT -> UTC and UTC -> OBT conversion
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  The class should be used only by the OBT and UTC classes.
 */
class OBTUTCCorrelation {


   /** *************************************************************************
    *  @brief   A container struct to store one correlation record
    *  @ingroup utilities
    *  @author  Reiner Rohlfs, UGE
    *
    *  According to the MOC-SOC IDC the OBT - UTC correldation is dafiend as:
    *     ( UTC – UTC_N ) = OFFSET + GRADIENT · ( OBT – OBT_N )
    *
    *  Records of this class can be sorted either by OBT or by UTC. The
    *  static variable s_searchTime defines which time should be used for the
    *  sorting. It is assumed that one program is always converting times
    *  either from UTC to OBT or from OBT to UTC. Therefore the OBT and UTC
    *  classes define the sorting by calling SetSearchTime().
    *
    */
   struct CorrelationRecord {

      /// @brief used to define the time (UTC or OBT) that is used to sort
      ///        records of this struct.
      enum SEARCH_TIME {
         S_OBT,    ///< OBT shall be used to sort the records
         S_UTC     ///< UTC shall be used to sort the records
      };

      static SEARCH_TIME s_searchTime;  ///< defines the time to be used for sorting these structs.

      UTC    m_utcTimeStamp;  ///< this record is valid from this time until next record
      OBT    m_obtTimeStamp;  ///< same time as m_utcTimeStamp in OBT
      UTC    m_utc;        ///< UTC time of one record
      OBT    m_obt;        ///< OBT time of one record
      double m_gradient;   ///< the slope to the correlation
      double m_offset;     ///< (constant + variable) offset of the correlation

      /** **********************************************************************
       *  @brief Standard constructor to initialize all member variables
       *
       *  @param [in] timeStamp this record is valid until this time
       *  @param [in] utc       UTC time of this record
       *  @param [in] obt       OBT time of this record
       *  @param [in] gradient  slope to the next record
       *  @param [in] offset    offset of the correlation
       */
      CorrelationRecord(const UTC & timeStamp,
                        const UTC & utc, const OBT & obt,
                        double gradient, double offset)
         : m_utcTimeStamp(timeStamp),
           m_obtTimeStamp(obt + (timeStamp - utc - offset) / gradient),
           m_utc(utc), m_obt(obt),
           m_gradient(gradient), m_offset(offset) {}

      /** **********************************************************************
       *  @brief Constructor, used to search for a record by UTC time
       */
      CorrelationRecord(const UTC & utc)
         : m_utcTimeStamp(utc) {}

     /** **********************************************************************
      *  @brief Constructor, used to search for a record by OBT time
      */
      CorrelationRecord(const OBT & obt)
         : m_obtTimeStamp(obt)  {}


      /** **********************************************************************
       *  @brief Sets the time that is used to sort these correlation records
       */
      static void SetSearchTime(SEARCH_TIME searchTime) {
         s_searchTime = searchTime;
      }

      /** **********************************************************************
       *  @brief Standard < operator.
       *
       *  It uses either the OBT or the UTC to define the relation. See also
       *  documentation of this struct.
       */
      bool operator < (const CorrelationRecord & corrRecord) const {
         if (s_searchTime == S_OBT)
            return m_obtTimeStamp < corrRecord.m_obtTimeStamp;
         else
            return m_utcTimeStamp < corrRecord.m_utcTimeStamp;
      }

   };

   /// set of all correlation records, sorted either by OBT or by UTC
   static std::set<CorrelationRecord> m_correlationRecords;

   /// names of the correlation file from where all the time correlation records are read
   static std::list<std::string> m_correlationFileNames;


   /** *************************************************************************
    *  @brief Reads all the correlatin record from file @b m_correlationFileName
    *         and stored them in @b m_correlationRecords
    */
   static void ReadCorrelationRecords();

   /** *************************************************************************
    *  @brief Converts one UTC time to OBT
    *
    *  @param [in] utc  the UTC to be converted
    *
    *  @return  the converted OBT time
    */
   static OBT getObt(const UTC & utc);

   /** *************************************************************************
    *  @brief Converts one OBT time to UTC
    *
    *  @param [in] obt  the OBT to be converted
    *
    *  @return  the converted UTC time
    */
   static UTC getUtc(const OBT & obt);

   friend class UTC;
   friend class OBT;


public:
   /** *************************************************************************
    *  @brief Sets the file name of the correlation records
    *
    */
   static void SetCorrelationFileNames(const std::list<std::string> & fileNames)
      {m_correlationFileNames = fileNames;}
};

#endif /* #define _OBTUTCCORRELATION_HXX_
 */
