/** ****************************************************************************
 *  @file
 *  @ingroup utilities
 *  @brief Implementation of class ValidRefFile.
 *
 *  @author Anja Bekkelien UGE
 *  
 *  @version 7.4 2017-06-01 ABE #14081 Order files by archive revision and
 *                                     processing number instead of by creation
 *                                     time.
 *  @version 3.2 2015-04-14 ABE        First version.
 */


#include "ValidRefFile.hxx"

////////////////////////////////////////////////////////////////////////////////
//
//  Class ValidityPeriod
//
////////////////////////////////////////////////////////////////////////////////

ValidityPeriod::ValidityPeriod(UTC validityStart, UTC validityStop, int32_t archRev, int32_t procNume,
                               std::string fileName, std::string baseFileName) :
    m_validityStart(validityStart),
    m_validityStop(validityStop),
    m_archRev(archRev),
    m_procNum(procNume),
    m_fileName(fileName),
    m_baseFileName(baseFileName) {}

ValidityPeriod::ValidityPeriod(std::string validityStart, std::string validityStop, int32_t archRev, int32_t procNum,
      std::string fileName, std::string baseFileName) :
    m_validityStart( UTC(validityStart) ),
    m_validityStop( UTC(validityStop) ),
    m_archRev(archRev),
    m_procNum(procNum),
    m_fileName(fileName),
    m_baseFileName(baseFileName) {}

bool ValidityPeriod::operator < (const ValidityPeriod & validityPeriod) const {
  if (m_archRev < validityPeriod.m_archRev) {
    return true;
  }
  if (m_archRev == validityPeriod.m_archRev &&
      m_procNum < validityPeriod.m_procNum) {
    return true;
  }
  return false;
}

bool ValidityPeriod::operator == (const ValidityPeriod & validityPeriod) const {
  return m_archRev == validityPeriod.m_archRev && m_procNum == validityPeriod.m_procNum;
}

bool ValidityPeriod::operator != (const ValidityPeriod & validityPeriod) const {
  return m_archRev != validityPeriod.m_archRev || m_procNum != validityPeriod.m_procNum;
}

bool ValidityPeriod::contains(const UTC utc) const {
  return (m_validityStart == utc || m_validityStart < utc) &&
         (utc == m_validityStop || utc < m_validityStop);
}
