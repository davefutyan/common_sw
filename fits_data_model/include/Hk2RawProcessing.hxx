/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDataModel
 *  @brief Declaration of the HK 2 RAW Processing classes
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 13.0  2020-08-28 MBE      #22243 CHEOPS and Gaia magnitudes in the output data
 *  @version 7.3 2017-05-18 RRO        #12838 New header keywords: MAG_V, MAG_VERR, MAG_CHPS, MAG_CERR
 *  @version 6.3.1 2016-11-07 RRO      #11946 New header keyword: OBS_CAT, PRP_VST1 and PRP_VSTN
 *  @version 6.3 2016-10-21 RRO        New header keyword: PI_UID
 *  @version 5.2 2016-06-03 RRO        Header keywords CHEOPSID, TG_TYPE and OBS_MODE
 *                                     does not exist any more.
 *                                     New header keyword: SPECTYPE
 *  @version 4.1 2015-07-22 RRO        first released version
 *
 */

#ifndef _HK_2_RAW_PROCESSING_HXX_
#define _HK_2_RAW_PROCESSING_HXX_

#include <string>
#include <map>

#include <FitsDalTable.hxx>
#include <VisitId.hxx>
#include <PassId.hxx>
#include <Obt.hxx>
#include <Utc.hxx>
#include <Mjd.hxx>
#include <Bjd.hxx>

#include "HkProcessing.hxx"



/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Class to fill one RAW_HK FITS tables with data
 *
 *  It is a classed used by Hk2RawProcessing. Applications cannot use this
 *  class.
 */
class HkRawFitsTable
{
private:
   FitsDalTable * m_fitsDalTable;  ///< the FITS table associated to this class
   std::string    m_hkStructName;  ///< name of the HK FITS table, used in error messages

protected:

   UTC            m_firstUtc;      ///< UTC of first data in table
   UTC            m_currentUtc;    ///< UTC of current row in table

protected:

   /** *************************************************************************
    *  @brief Constructor, Can be called as base class of HkRawFitsTableChild
    *
    *  Beside the initialization of the member variables, the FITS columns
    *  are re-assigned to the "ColInfo::m_value" variable.
    */
   HkRawFitsTable(FitsDalTable * fitsDalTable,
                  const std::string hkStructName,
                  std::map<std::string, ColMetaData*> & colData);

public:
   /** *************************************************************************
    *  @brief Deletes m_fitsDalTable.
    *
    */
   virtual ~HkRawFitsTable() {delete m_fitsDalTable;}

   /** *************************************************************************
    *  @brief Writes one row to the associated RAW table
    *
    *  Columns that are not set by the user are updated with a random offset
    *  value. The time columns are set
    */
   void writeRow(const UTC & utc, std::map<std::string, ColMetaData*> & colData);

   /// Set value of FITS header attribute PROC_CHN
   virtual void setKeyProcChn   (std::string  keyProcChn)  = 0;
   /// Set value of FITS header attribute TARGNAME
   virtual void setKeyTargname  (std::string  keyTargname) = 0;
   /// Set value of FITS header attribute MAG_V
   virtual void setKeyMagG      (double       keyMagG)     = 0;
   /// Set value of FITS header attribute MAG_VERR
   virtual void setKeyMagGerr   (double       keyMagGerr)  = 0;
   /// Set value of FITS header attribute MAG_CHPS
   virtual void setKeyMagChps   (double       keyMagChps)  = 0;
   /// Set value of FITS header attribute MAG_CERR
   virtual void setKeyMagCerr   (double       keyMagCerr)  = 0;
   /// Set value of FITS header attribute SPECTYPE
   virtual void setKeySpectype  (std::string  keySpectype) = 0;
   /// Set value of FITS header attribute PI_NAME
   virtual void setKeyPiName    (std::string  keyPiName)   = 0;
   /// Set value of FITS header attribute PI_NAME
   virtual void setKeyPiUid     (uint32_t     keyPiUid)    = 0;
   /// Set value of FITS header attribute OBSID
   virtual void setKeyObsid     (uint32_t     keyObsid)    = 0;
   /// Set value of FITS header attribute RA_TARG
   virtual void setKeyRaTarg    (double       keyRaTarg)   = 0;
   /// Set value of FITS header attribute DEC_TARG
   virtual void setKeyDecTarg   (double       keyDecTarg)  = 0;
   /// Set value of FITS header attribute OBS_CAT
   virtual void setKeyObsCat    (std::string  keyObsCat)   = 0;
   /// Set value of FITS header attribute PRP_VST1
   virtual void setKeyPrpVst1   (uint32_t     keyPrpVst1)  = 0;
   /// Set value of FITS header attribute PRP_VSTN
   virtual void setKeyPrpVstn   (uint32_t     keyPrpVstn)  = 0;

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Class to fill one RAW_HK FITS tables with data
 *
 *  It is a classed used by Hk2RawProcessing. Applications cannot use this
 *  class.
 *  The only purpose of this template class is to keep the fits_data_model
 *  data type of the table, to be able to fill some keywords. See destructor.
 *
 *  @tparam HK_RAW_TABLE  has to be a SCI_RAW_HK* class
 */
template <class HK_RAW_DAL_TABLE>
class HkRawFitsTableChild : public HkRawFitsTable {

private:
   HK_RAW_DAL_TABLE * m_hkRawFitsDalTable;   ///< fits_data_model class of the FITS table

   /** *************************************************************************
    *  @brief The only constructor
    *
    */
   HkRawFitsTableChild(HK_RAW_DAL_TABLE * hkRawFitsDalTable,
                       std::map<std::string, ColMetaData*> & colData)
        :  HkRawFitsTable(hkRawFitsDalTable, HK_RAW_DAL_TABLE::getExtName(),
                          colData),
           m_hkRawFitsDalTable(hkRawFitsDalTable) {}


   friend class Hk2RawProcessing;

public:

   /// Set value of FITS header attribute PROC_CHN
   void setKeyProcChn   (std::string  keyProcChn)     {m_hkRawFitsDalTable->setKeyProcChn(keyProcChn);}
   /// Set value of FITS header attribute TARGNAME
   void setKeyTargname  (std::string  keyTargname)    {m_hkRawFitsDalTable->setKeyTargname(keyTargname);}
   /// Set value of FITS header attribute SPECTYPE
   void setKeySpectype  (std::string  keySpectype)    {m_hkRawFitsDalTable->setKeySpectype(keySpectype);}
   /// Set value of FITS header attribute MAG_V
   void setKeyMagG      (double       keyMagG)        {m_hkRawFitsDalTable->setKeyMagG(keyMagG);}
   /// Set value of FITS header attribute MAG_VERR
   void setKeyMagGerr   (double       keyMagGerr)     {m_hkRawFitsDalTable->setKeyMagGerr(keyMagGerr);}
   /// Set value of FITS header attribute MAG_CHPS
   void setKeyMagChps   (double       keyMagChps)     {m_hkRawFitsDalTable->setKeyMagChps(keyMagChps);}
   /// Set value of FITS header attribute MAG_CERR
   void setKeyMagCerr   (double       keyMagCerr)     {m_hkRawFitsDalTable->setKeyMagCerr(keyMagCerr);}
   /// Set value of FITS header attribute PI_NAME
   void setKeyPiName    (std::string  keyPiName)      {m_hkRawFitsDalTable->setKeyPiName(keyPiName);}
   /// Set value of FITS header attribute PI_NAME
   void setKeyPiUid     (uint32_t     keyPiUid)       {m_hkRawFitsDalTable->setKeyPiUid(keyPiUid);}
   /// Set value of FITS header attribute OBSID
   void setKeyObsid     (uint32_t     keyObsid)       {m_hkRawFitsDalTable->setKeyObsid(keyObsid);}
   /// Set value of FITS header attribute RA_TARG
   void setKeyRaTarg    (double       keyRaTarg)      {m_hkRawFitsDalTable->setKeyRaTarg(keyRaTarg);}
   /// Set value of FITS header attribute DEC_TARG
   void setKeyDecTarg   (double       keyDecTarg)     {m_hkRawFitsDalTable->setKeyDecTarg(keyDecTarg);}
   /// Set value of FITS header attribute OBS_CAT
   void setKeyObsCat    (std::string  keyObsCat)      {m_hkRawFitsDalTable->setKeyObsCat(keyObsCat);}
   /// Set value of FITS header attribute PRP_VST1
   void setKeyPrpVst1   (uint32_t     keyPrpVst1)     {m_hkRawFitsDalTable->setKeyPrpVst1(keyPrpVst1);}
   /// Set value of FITS header attribute PRP_VSTN
   void setKeyPrpVstn   (uint32_t     keyPrpVstn)     {m_hkRawFitsDalTable->setKeyPrpVstn(keyPrpVstn);}

  /** *************************************************************************
   *  @brief  Writes the validity start and end times in the FITS header keywords.
   */
   virtual  ~HkRawFitsTableChild() {
      m_hkRawFitsDalTable->setKeyVStrtU(m_firstUtc);
      m_hkRawFitsDalTable->setKeyVStopU(m_currentUtc);
      m_hkRawFitsDalTable->setKeyVStrtM(m_firstUtc.getMjd());
      m_hkRawFitsDalTable->setKeyVStopM(m_currentUtc.getMjd());

   }

};


/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Root class to create HK data in RAW FITS tables
 *
 */
class Hk2RawProcessing : public HkProcessing
{

   std::string m_outDir;          ///< directory where all the files will be created
   VisitId     m_visitId;         ///< Visit Id the output files belong to

   std::string m_keyProcChn;      ///< Processing chain creating this data structure
   int32_t     m_keyCheopsid = 0; ///< CHEOPS ID of target
   std::string m_keyTargname;     ///< Name of the target as provided by the proposal
   std::string m_keySpectype;     ///< Spectral Type of the target
   double      m_keyMagG = 0;     ///< [mag] Brightness of the target in V band
   double      m_keyMagGerr = 0;  ///< [mag] Error of brightness of the target in V band
   double      m_keyMagChps = 0;  ///< [mag] Brightness of the target in CHEOPS band
   double      m_keyMagCerr = 0;  ///< [mag] Error of brightness of the target in CHEOPS band
   std::string m_keyPiName;       ///< Name of the PI of the observing program
   uint32_t    m_keyPiUid = 0;    ///< UID of the PI at UGE
   uint32_t    m_keyObsid = 0;    ///< Unique identified of a visit, defined by MPS
   double      m_keyRaTarg = 0;   ///< Right ascension of the target (J2000)
   double      m_keyDecTarg = 0;  ///< Declination of the target (J2000)
   std::string m_keyObsCat;       ///< Observation Category
   uint32_t    m_keyPrpVst1 = 0;  ///< [days] Proprietary period, depending on first visit
   uint32_t    m_keyPrpVstn = 0;  ///< [days] Proprietary period, depending on last visit




   /// map<structName, HkRawFitsTable * > map of open FTIS tables
   std::map<std::string, HkRawFitsTable *>  m_hkRawFitsTables;

public:

   /** *************************************************************************
    *  @brief  Root class to create RAW HK Tables with either random values
    *          or application defined values.
    *
    *  There shall be only one instance per program of this class.
    *
    *  @param [in] hkConversionFilename  filename of the HK conversion REF file
    *  @param [in] outDir                RAW HK files will be create in this
    *                                    directory.
    *  @param [in] visitId               This visit ID will be part of the
    *                                    filename and header keywords will be
    *                                    set with this visitID value.
    */
   Hk2RawProcessing(const std::string & hkConversionFilename,
                    const std::string & outDir,
                    const VisitId & visitId);

   /** *************************************************************************
    *  @brief Updates the header keyword and closes all the FITS files.
    */
   virtual ~Hk2RawProcessing();


   /** *************************************************************************
    *  @brief Updates the value of one column.
    *
    *  At the next calls of writeRow this @b value will be written to the column
    *  until it is updated again by this function.
    *  The column will not be updated randomly any more after the value is set
    *  by this function at least once.
    *
    *  @param [in] colName  name of the column for which the @b value will be set
    *  @param [in] value    this value will be set to the @b column.
    *
    *  @throw runtime_error  if @b colName does not exist in any RAW HK table.
    */
   template < class T >
   void updateParameter(const std::string & colName, const T & value) {

      // find the table in which the column is present.
      std::map<std::string, std::map<std::string, ColMetaData *>>::iterator i_colData =
            m_colData.begin();
      while (i_colData != m_colData.end()) {
         std::map<std::string, ColMetaData *>::iterator ii_colData = i_colData->second.find(colName);
         if (ii_colData != i_colData->second.end()) {
            // this is the table, we set the value
            ii_colData->second->setValue(value);
            return;
         }

         i_colData++;
      }

      // the colName is unknow. This should not happen, we throw an exception
      throw std::runtime_error ("Cannot update value for parameter " + colName +
                           ". Unknown parameter.");
   }

   /** *************************************************************************
    *  @brief Writes one row to the table of @b structName
    *
    *  The FITS file with the named table will be created if this function is
    *  called for the first time for a given @b structName.
    *
    *
    *  @param [in] utc         This time will be written to the OBT, UTC and
    *                          MJD time columns.
    *  @param [in] structName  A new row to the table of this name will
    *                          added. Currently supported sturctNames are:
    *                          SCI_RAW_HkDefault, SCI_RAW_HkExtended,
    *                          SCI_RAW_HkIfsw, SCI_RAW_HkIbswPar, SCI_RAW_HkIbswDg,
    *                          SCI_RAW_HkIaswPar and SCI_RAW_HkIaswDg
    *
    */
   void writeRow(const UTC & utc, const std::string & structName);

   /// Set value of FITS header attribute PROC_CHN
    void setKeyProcChn       (std::string  keyProcChn)     {m_keyProcChn = keyProcChn;}

   /// Set value of FITS header attribute CHEOPSID
    void setKeyCheopsid      (int32_t      keyCheopsid)    {m_keyCheopsid = keyCheopsid;}

    /// Set value of FITS header attribute TARGNAME
    void setKeyTargname      (std::string  keyTargname)    {m_keyTargname = keyTargname;}

    /// Set value of FITS header attribute SPECTYPE
    void setKeySpectype      (std::string  keySpectype)    {m_keySpectype = keySpectype;}

    /// Set value of FITS header attribute MAG_G
    void setKeyMagG          (double       keyMagG)        {m_keyMagG = keyMagG;}

    /// Set value of FITS header attribute MAG_GERR
    void setKeyMagGerr       (double       keyMagGerr)     {m_keyMagGerr = keyMagGerr;}

    /// Set value of FITS header attribute MAG_CHPS
    void setKeyMagChps       (double       keyMagChps)     {m_keyMagChps = keyMagChps;}

    /// Set value of FITS header attribute MAG_CERR
    void setKeyMagCerr       (double       keyMagCerr)     {m_keyMagCerr = keyMagCerr;}

    /// Set value of FITS header attribute PI_NAME
    void setKeyPiName        (std::string  keyPiName)      {m_keyPiName = keyPiName;}

    /// Set value of FITS header attribute PI_NAME
    void setKeyPiUid         (uint32_t     keyPiUid)       {m_keyPiUid = keyPiUid;}

    /// Set value of FITS header attribute OBSID
    void setKeyObsid         (uint32_t     keyObsid)       {m_keyObsid = keyObsid;}

    /// Set value of FITS header attribute RA_TARG
    void setKeyRaTarg        (double       keyRaTarg)      {m_keyRaTarg = keyRaTarg;}

    /// Set value of FITS header attribute DEC_TARG
    void setKeyDecTarg       (double       keyDecTarg)     {m_keyDecTarg = keyDecTarg;}

    /// Set value of FITS header attribute OBS_CAT
    void setKeyObsCat        (std::string  keyObsCat)      {m_keyObsCat = keyObsCat;}

    /// Set value of FITS header attribute PRP_VST1
    void setKeyPrpVst1       (uint32_t     keyPrpVst1)     {m_keyPrpVst1 = keyPrpVst1;}

    /// Set value of FITS header attribute PRP_VSTN
    void setKeyPrpVstn       (uint32_t     keyPrpVstn)     {m_keyPrpVstn = keyPrpVstn;}

};


#endif /* _HK_2_RAW_PROCESSING_HXX_ */

