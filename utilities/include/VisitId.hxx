/** ****************************************************************************
 *  @file
 *
 *  @ingroup utilities
 *  @brief Declaration of the VisitId class
 *
 *  @author Reiner Rohlfs, UGE
 *
 *  @version 9.3 2018-06-07 RRO #16416 Set ProgramType to 0 in default constructor
 *  @version 6.3 2016-10-21 RRO        remove deprecated method getObservationId()
 *  @version 6.1 2016-07-29 RRO #11327 Replace m_observatinoId by m_requestId.
 *                                     New method: getRequestId()
 *  @version 5.0 2016-01-14 RRO #9940 new programType 5 = data of no visit
 *  @version 4.3 2015-10-21 RRO #9407 operator==, operator!= and operator< implemented
 *  @version 4.1 2015-07-26 RRO update Doxygen documentation of VisitId::VistiID()
 *  @version 3.3 2015-06-24 RRO remove the getMpsVisit() method
 *  @version 3.0 2015-01-27 RRO first released version
 */

#ifndef _VISIT_ID_HXX_
#define _VISIT_ID_HXX_

#include <cstdint>
#include <string>


/** ****************************************************************************
 *  @brief   A class to store the fields of a VisitId
 *  @ingroup utilities
 *  @author  Reiner Rohlfs, UGE
 *
 *  A visit-Id is defined by four items:
 *  - a program type, only a few program types are defined: guaranteed time
 *      program (10-19), guest observer program (20-29), monitoring, characterization
 *      program (30-39), discretionary program (40-49) and data not belonging to any
 *      visit (50). Simulated data use as program type the number 90, 98 or 99.
 *  - a program request id, it is a unique number per program type.
 *  - a observation request id, a unique number per program request id.
 *  - visit counter, counts the visits of one observation request.
 *
 */
class VisitId
{
private:

   uint8_t   m_programType;   ///< @brief type of programme: 10-19 = guaranteed,
                              ///         20-29 = guest observer,
                              ///         30-39 = characterization,
                              ///         40-49 = discretionary,
                              ///         50 = introduced by Preprocessing (for example a slew)
   uint16_t  m_programId;     ///< Id of program request, counted per programme type
   uint16_t  m_requestId;     ///< Id of an observation request, unique id per programme
   uint8_t   m_visitCounter;  ///< visit counter of specific observation and programme
   bool      m_valid;         ///< set to true if this instance holds a valid visit Id

   /** *************************************************************************
    *  @brief Validates the data in this instance.
    *
    *  @throw runtime_error if m_programType is not between 10 and 50, or 90, 98 or 99
    */
    void Validate();


public:

   /** *************************************************************************
    *  @brief Default constructor of the VisitId class.
    *
    *  The visit id stored in this instance is not valid. Without assigning a
    *  valid VisitId to this class the function VisitId::isValid() will
    *  return false.
    */

   VisitId()
      : m_programType(0), m_programId(0),
        m_requestId(0), m_visitCounter(1), m_valid(false) {}

   /** *************************************************************************
    *  @brief Initializes the variables of the class
    *
    *  @param [in] programType    Valid values are: 10-19 (guaranteed program),
    *                             20-29 (guest observer program), 30-39 (monitoring and
    *                             characterization program), 40-49 (discretionary
    *                             program), 50 (data not belonging to any
    *                             visit) and 90, 98, 99 (simulated data)
    *  @param [in] programId      The program request Id of a program type
    *                             [0 .. 9999]
    *  @param [in] requestId      A unique observation request Id per
    *                             program Id [0 .. 9999]
    *  @param [in] visitCounter   A counter, counting the visits of the target
    *                             [1 .. 99]
    *
    */
   VisitId(uint8_t programType, uint16_t programId,
           uint16_t requestId, uint8_t visitCounter)
      : m_programType(programType), m_programId(programId),
        m_requestId(requestId), m_visitCounter(visitCounter) , m_valid(true)
     { Validate(); }

   /** *************************************************************************
     *  @brief Initializes the variables of the class from a sub-string of a
     *         file name
     *
     *  @param [in] fileNamePattern  the format has to be like a visit id is coded in
     *                               a CHEOPS file-name: PRppnnnn-TGoooonn.
     *                               pp is the program type, nnnn is the program
     *                               request Id, oooo is the observatin request
     *                               id and nn is the visit counter.
     *
     */
   VisitId(const std::string & fileNamePattern);

   /** *************************************************************************
     *  @brief Initializes the variables of the class from the header keyword
     *         of a HDU.
     *
     *  @param [in] hdu a pointer to an instance of a FITS Data Model class
     *
     *  @tparam HDU  If should be a FITS Data Model class. Following methods
     *               must be defined:
     *               - hdu->getKeyProgtype(),
     *               - hdu->getKeyProgId(),
     *               - hdu->getKeyReqId() and
     *               - hdu->getKeyVisitctr().\n
     *               These are defined if the corresponding FITS HDU has
     *               following keywords:
     *               - PROGTYPE,
     *               - PROG_ID,
     *               - REQ_ID  and
     *               - VISITCTR.
     *
     */
   template <class HDU>
   VisitId(HDU * hdu) : m_valid(true) {
      m_programType   = hdu->getKeyProgtype();
      m_programId     = hdu->getKeyProgId();
      m_requestId     = hdu->getKeyReqId();
      m_visitCounter  = hdu->getKeyVisitctr();

      Validate();
   }

   /** *************************************************************************
    *  @brief Returns true if this instance holds a valid Visit Id
    */
   bool   isValid() const  {return m_valid;}

   /** *************************************************************************
    *  @brief Less - operator.
    */
   bool operator < (const VisitId & visitId) const;

   /** *************************************************************************
    *  @brief not equal - operator.
    */
   bool operator != (const VisitId & visitId) const
        {return !operator==(visitId);}

   /** *************************************************************************
    *  @brief equal - operator.
    */
   bool operator == (const VisitId & visitId) const;


   /** @brief Returns the program type, 10 - 50, or 90, 98 or 99  */
   uint8_t     getProgramType()  const   { return m_programType;}

   /** @brief Returns the program request Id  */
   uint16_t    getProgramId() const      { return m_programId;}

   /** @brief Returns the observation request Id  */
   uint16_t    getRequestId() const      { return m_requestId; }

   /** @brief Returns the visit counter   */
   uint8_t     getVisitCounter() const   { return m_visitCounter; }


   /** *************************************************************************
    *  @brief Returns a string in the format as the Visit Id should be written
    *         in a CHEOPS file-name
    *
    *  The format of the returned string is PRppnnnn_TGttttnn
    */
   std::string getFileNamePattern() const;

};

#endif /* _VISIT_ID_HXX_ */
