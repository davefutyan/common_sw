/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDataModel
 *  @brief Declaration of the HK TM 2 PRW Processing classes
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 12.1.4 2020-04-22 ABE #21297 Provide length of TM packet to
 *                                        HkPrwFitsTable::addTmPacket()
 *  @version 9.1.4 2018-04-11 RRO #15832 change the data type of m_obtTmOffset from
 *                                       uint16_t to int16_t to use a negative value
 *                                       as indicator that this offset should not be used.
 *  @version 6.4 2016-12-14 RRO #12249 new Methods of class HkPrwFitsTable:
 *                                     setKeyObsCat(), setKeyPrpVst1() and setKeyPrpVstn().
 *  @version 6.2 2016-08-23 RRO #11468 new Method: HkPrwFitsTable::setKeyObsid()
 *  @version 5.0 2016-02-11 RRO #10117 Avoid raising an exception in the destructor:
 *                                     Remove m_firstObt.getUtc() in ~HkPrwFitsTableChild()
 *  @version 4.1 2015-08-03 RRO        first released version
 */

#ifndef _HK_TM_2_PRW_PROCESSING_HXX_
#define _HK_TM_2_PRW_PROCESSING_HXX_

#include <string>
#include <map>

#include <FitsDalTable.hxx>
#include <VisitId.hxx>
#include <PassId.hxx>
#include <Obt.hxx>

#include "HkProcessing.hxx"



/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Class to fill one PRW_HK FITS tables with data of one TM packet
 *  or some parameters of one TM packet.
 *
 *  The only way to create such a class is to call
 *  HkTm2PrwProcessing::getHkPrwFitsTable() for a specific HK TM packet.
 *
 */
class HkPrwFitsTable
{
private:
   FitsDalTable *      m_fitsDalTable;  ///< the FITS table associated to this class
   std::string         m_hkStructName;  ///< name of the HK FITS table, used in error messages
   std::map<std::string, ColMetaData *> &  m_colData;
                                        ///< Information of all columns of the FITS table.
   int16_t             m_obtTmOffset;   ///< @brief if >= 0 it defines the position of the OBT
                                        ///  in the data field of the TM packet for this table.

   HkPrwFitsTable  *   m_hkPrwFitsTableChain; ///< @brief builds up a chain of HkPrwFitsTable if data
                                              ///  of one TM Packet shall be store in several FITS tables


protected:

   bool               m_firstTmPacket = true; ///<  first TM packet for this table
   OBT                m_currentObt = 0; ///< OBT of current TM packet
   UTC                m_firstUtc;       ///< UTC of first TM packet
   UTC                m_currentUtc;     ///< UTC of current TM packet
   std::string        m_keyPiName;      ///< Name of PI, will be written to the FITS header
   uint32_t           m_keyPiUid = 0;   ///< UID of the PI at UGE
   uint32_t           m_keyObsid = 0;   ///< OBSID as defined by MPS, will be written to the FITS header
   std::string        m_keyObsCat;      ///< Observation Category
   uint32_t           m_keyPrpVst1 = 0; ///< [days] Proprietary period, depending on first visit
   uint32_t           m_keyPrpVstn = 0; ///< [days] Proprietary period, depending on last visit



protected:

   /** *************************************************************************
    *  @brief Constructor, Can be called as base class of HkPrwFitsTable
    *
    *  Beside the initialization of the member variables, the FITS columns
    *  are re-assigned to the "ColInfo::m_value" variable.
    */
   HkPrwFitsTable(FitsDalTable * fitsDalTable,
                  const std::string hkStructName,
                  std::map<std::string, ColMetaData *> & colData,
                  int16_t obtTmOffset,
                  HkPrwFitsTable * hkPrwFitsTableChain);

public:
   /** *************************************************************************
    *  @brief Deletes m_fitsDalTable and m_hkFitsTableChain.
    *
    */
   virtual ~HkPrwFitsTable();

   /// Set value of FITS header attribute PI_NAME
   void setKeyPiName   (const std::string & keyPiName);

   /// Set value of FITS header attribute PI_UID
   void setKeyPiUid    (uint32_t keyPiUid);

   /// Set value of FITS header attribute OBSID
   void setKeyObsid    (uint32_t keyObsid);

   /// Set value of FITS header attribute OBS_CAT
   void setKeyObsCat   (std::string  keyObsCat);

   /// Set value of FITS header attribute PRP_VST1
   void setKeyPrpVst1  (uint32_t     keyPrpVst1);

   /// Set value of FITS header attribute PRP_VSTN
   void setKeyPrpVstn  (uint32_t     keyPrpVstn);

   /** *************************************************************************
    *  @brief Writes the data of one TM packet to the next row in the
    *  associated FITS tables.
    *
    *  All FITS tables are updated if data of one TM packet shall be written
    *  in more than one FITS table.
    *
    *  @param [in] obt  The OBT of the data, as defined in the TM header.
    *                   The @b obt has to be greater then the obt of the previous
    *                   call of this method.
    *
    *  @param [in] tmPacketData  The data of a HK TM packet. The index of a
    *                            parameter in this @b tmPacketData is defined
    *                            by the tmOffset attribute of that column in the
    *                            SCI_PRW_HK* fsd file. The length of the buffer
    *                            has to be long enough to hold the complete data
    *                            of one TM packet. The length depends on the
    *                            TmPacket. There is no check inside if the length
    *                            is too short!
    *  @param [in] maxLength     The maximum length of the parameter in bytes.
    *  @throw  runtime_error if the @b obt is equal of less than the obt of the
    *                        previous call.
    *
    */
   void addTmPacket(const OBT & obt, const int8_t * tmPacketData, uint16_t maxLength = UINT16_MAX);

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Class to fill one or more PRW_HK FITS tables from one TM packet
 *
 *  The only purpose of this template class is to keep the fits_data_model
 *  data type of the table, to be able to fill some keywords. See destructor.
 *
 *  @tparam HK_PRW_DAL_TABLE  has to be a SCI_PRW_HK* class
 */
template <class HK_PRW_DAL_TABLE>
class HkPrwFitsTableChild : public HkPrwFitsTable {

private:
   HK_PRW_DAL_TABLE * m_hkPrwFitsDalTable; ///< fits_data_model class of the FITS table

   PassId             m_passId;            ///< the pass ID, will be set in the FITS header.


   /** *************************************************************************
    *  @brief Initialize the variable.
    *
    *  It will be called by the method HkTm2PrwProcessing::getHkFitsTable
    */
   HkPrwFitsTableChild(HK_PRW_DAL_TABLE * hkPrwFitsDalTable,
                       PassId & passId,
                       std::map<std::string, ColMetaData *> & colData,
                       uint16_t obtTmOffset,
                       HkPrwFitsTable * hkPrwFitsTableChain) :
              HkPrwFitsTable(hkPrwFitsDalTable, HK_PRW_DAL_TABLE::getExtName(), colData,
                             obtTmOffset, hkPrwFitsTableChain),
              m_hkPrwFitsDalTable(hkPrwFitsDalTable), m_passId(passId)  {}


   friend class HkTm2PrwProcessing;

public:

  /** *************************************************************************
   *  @brief  Writes several FITS header keywords.
   */
   virtual  ~HkPrwFitsTableChild() {
      m_hkPrwFitsDalTable->setKeyVStrtU(m_firstUtc);
      m_hkPrwFitsDalTable->setKeyVStopU(m_currentUtc);
      m_hkPrwFitsDalTable->setKeyPassId(m_passId);
      m_hkPrwFitsDalTable->setKeyObsid(m_keyObsid);
      m_hkPrwFitsDalTable->setKeyPiName(m_keyPiName);
      m_hkPrwFitsDalTable->setKeyPiUid(m_keyPiUid);
      m_hkPrwFitsDalTable->setKeyObsCat(m_keyObsCat);
      m_hkPrwFitsDalTable->setKeyPrpVst1(m_keyPrpVst1);
      m_hkPrwFitsDalTable->setKeyPrpVstn(m_keyPrpVstn);
   }

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Root class to process HK data from TM packets to PRW FITS files.
 *
 *  Only one instance of this class shall be created in a program.
 */
class HkTm2PrwProcessing : public HkProcessing
{
private:
   std::string       m_outDir;   ///< directory where all the files will be created
   VisitId           m_visitId;  ///< Visit Id the output files belong to
   PassId            m_passId;   ///< Pass Id the output files belong to


public:
   /** *************************************************************************
    *  @brief The only constructor of this class
    *
    *  @param [in] outDir     all HK PRW files will be created in this directory
    *  @param [in] visitId    all files will belong to this Visit Id. The Visit
    *                         Id will be set in the header keywords and will
    *                         be part of the filename.
    *  @param [in] passId     all files will belong to this Pass Id. The Pass
    *                         Id will be set in the header keywords and will
    *                         be part of the filename.
    */
   HkTm2PrwProcessing(const std::string & outDir,
                      const VisitId & visitId,
                      const PassId & passId);

   /** *************************************************************************
    *  @brief Test whether a FITS PRW table is available that should be filled
    *         with data of TM packet of @b tmPacktId
    *
    *  @param [in] tmPacketId  The test is done for this tmPacketId
    *
    *  @return true if a FITS PRW table is available for data of the TM
    *          packet @b tmPacketId.
    */
   bool           tmPacketIsAvailable(const TmPacketId & tmPacketId);

   /** *************************************************************************
    *  @brief Returns one HkPrwFitsTable, which can be used to add TM packet
    *
    *  This method shall be called only once per type of TM HK packet!
    *  The returned HkPrwFitsTable can be used to add data of corresponding
    *  TM HK packets to the associated HK FITS tables.\n
    *  Even if data of one HK TM packet should be stored in more than one FITS
    *  table this function shall be called only once per TM packet. The
    *  HkPrwFitsTable is capable to distribute the data of one TM packet to
    *  several FITS tables.\n
    *  The returned HkPrwFitsTable has to be deleted by the calling application!
    *
    *  @param [in] tmPacketId  The method returns a HkPrwFitsTable that can
    *                          store data of this TM packet.
    *  @param [in] obt         The @b obt is converted to UTC, which will be part
    *                          of the filenames. It shall be the OBT of the
    *                          first TM packet that will be stored in the FITS
    *                          file.
    *
    *  @throw  runtime_error if the tmPacketId is not known by this class. The
    *          method tmPacketIsAvailable() can be used to test this beforehand.
    */
   HkPrwFitsTable *  getHkPrwFitsTable(const TmPacketId & tmPacketId, const OBT & obt);
};


#endif /* _HK_TM_2_PRW_PROCESSING_HXX_ */

