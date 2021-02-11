/** @file
 *  @ingroup utilities
 *  @brief Implementation of the VisitId class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 9.3 2018-06-07 RRO #16416 accept programType of 0
 *  @version 7.4 2017-05-24 RRO #11934 programType can be between 1 and 50
 *  @version 6.1 2016-07-29 RRO #11327 replace m_observatinoId by m_requestId
 *  @version 5.0 2016-01-14 RRO #9940 new programType 5 = data of no visit
 *  @version 4.3 2015-10-21 RRO #9407 operator== and operator< implemented
 *  @version 4.1 2015-07-26 RRO method Validate validates all values
 *  @version 3.3 2015-06-24 RRO remove the getMpsVisit() method
 *  @version 3.0 2015-01-27 RRO first released version
 */

// first include system header files
#include <stdexcept>


// last include the header files of this module
#include "VisitId.hxx"


using namespace std;

////////////////////////////////////////////////////////////////////////////////
void VisitId::Validate() {

   if ((m_programType < 10 || m_programType > 50) &&
        m_programType != 0 && m_programType != 90 &&
        m_programType != 98 && m_programType != 99   )
      throw runtime_error("The program type of a VisitID has to be either "
                          "between 10 and 50, or 0, 90, 98 or 99 - constructor found "
                          + to_string(m_programType) );

   if (m_programId > 9999)
      throw runtime_error("The program Id of a VisitID has to be less than 10000"
                          " - constructor found " + to_string(m_programId) );

   if (m_requestId > 9999)
      throw runtime_error("The request Id of a VisitID has to be less than 10000"
                          " - constructor found " + to_string(m_requestId) );

   if (m_visitCounter > 99)
      throw runtime_error("The visit counter of a VisitID has to be in the range"
                          " 1 to 99 - constructor found " + to_string(m_visitCounter) );

   if (m_programType == 0)
      m_valid = false;
}

////////////////////////////////////////////////////////////////////////////////
VisitId::VisitId(const std::string & fileNamePattern) {
   m_programType   = stoi(fileNamePattern.substr(2,2));
   m_programId     = stoi(fileNamePattern.substr(4,4));
   m_requestId     = stoi(fileNamePattern.substr(11,4));
   m_visitCounter  = stoi(fileNamePattern.substr(15,2));
   m_valid         = true;

   Validate();
}


////////////////////////////////////////////////////////////////////////////////
bool VisitId::operator < (const VisitId & visitId) const {

   if (m_programType != visitId.m_programType)
      return m_programType < visitId.m_programType;

   if (m_programId != visitId.m_programId)
      return m_programId < visitId.m_programId;

   if (m_requestId != visitId.m_requestId)
      return m_requestId < visitId.m_requestId;

   return m_visitCounter < visitId.m_visitCounter;
}

////////////////////////////////////////////////////////////////////////////////
bool VisitId::operator == (const VisitId & visitId) const {

   return m_valid         == visitId.m_valid          &&
          m_programType   == visitId.m_programType    &&
          m_programId     == visitId.m_programId      &&
          m_requestId     == visitId.m_requestId  &&
          m_visitCounter  == visitId.m_visitCounter;
}

////////////////////////////////////////////////////////////////////////////////
string VisitId::getFileNamePattern() const {

   char hstr[100];
   sprintf(hstr, "PR%02hhu%04hu_TG%04hu%02hhu",
           m_programType, m_programId, m_requestId, m_visitCounter);

   return string(hstr);

}
