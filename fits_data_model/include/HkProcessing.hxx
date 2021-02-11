/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDataModel
 *  @brief Declaration of the HK Processing class, the TMPacketId class and
 *         the Col classes.
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 12.1.4 2020-04-22 ABE #21297 Provide length of TM packet to
 *                                        HkPrwFitsTable::addTmPacket()
 *  @version 12.0.2 2020-02-14 RRO #20881 bug fix in ColReducedIntData::setFromTm
 *                                        for negative number of signed integer values.
 *                                        Affects columns OFFSET_X and OFFSET_Y in
 *                                        SCI_PRW_HkCentroid table.
 *  @version 12.0  2019-10-23 RRO #19772 Implement the decoding of TC(196,1)
 *  @version 11.0  2019-01-07 RRO #17849 New exception class: obt_error
 *  @version 9.1.4 2018-04-11 RRO #15832 new class: ColOBTData
 *  @version 6.3   2016-09-23 RRO        enum values are not continuous values any more
 *  @version 4.3.1 2015-11-02 RRO #9608  endian was wrong. Data have to be swapped.
 *  @version 4.1   2015-07-22 RRO        first released version
 */

#ifndef _HK_PROCESSING_HXX_
#define _HK_PROCESSING_HXX_

#include <string>
#include <map>
#include <stdexcept>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include <FitsDalTable.hxx>

#include <cstring>

template <typename T>
T swap_endian(T u)
{
    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
};


/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  An exception class, specifying that an OBT value is not correct
 */
class obt_error : public std::runtime_error {

public:
   obt_error(const std::string & what_arg) :
     runtime_error(what_arg) {}
};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Defines a HK TM packet with appId, and sid. Type/subtype is always 3/25
 */
class TmPacketId
{
private:
   uint16_t    m_appId;  ///< AppId of the TM packet
   uint16_t    m_sid;    ///< SID of the HK - TM packet

public:
   /** *************************************************************************
    *  @brief Default constructor. Sets sid and appId to 0
    */
   TmPacketId() : m_appId(0), m_sid(0)           {}

   /** *************************************************************************
    *  @brief Constructor, does nothing else than initializing the sid and appId.
    */
   TmPacketId(uint16_t appId, uint16_t sid) :
      m_appId(appId), m_sid(sid)                 {}


   /** *************************************************************************
    *  @brief Used to sort the TmPacketId, first key = appId, second key = SID
    */
   bool operator < (const TmPacketId & tmPacketId) const {
      if (m_appId == tmPacketId.m_appId)
         return m_sid < tmPacketId.m_sid;
      return m_appId < tmPacketId.m_appId;
   }

   /** *************************************************************************
    *  @brief Returns the App Id
    */
   uint16_t  getAppId() const  { return m_appId;}

   /** *************************************************************************
    *  @brief Returns the SID
    */
   uint16_t  getSid() const    { return m_sid; }

};



/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Stores meta-data of one column and defines interfaces (methods)
 *          to modify the value of the column.
 */
class ColMetaData {

protected:
   std::string m_colName;       ///< FITS column name

   static boost::random::mt19937 m_gen;  ///< common used random generator

public:
   /** *************************************************************************
    *  @brief Constructor, set the name of the column
    *
    *  @param [in] colName   name of the column this class is assigned to.
    */
   ColMetaData(std::string & colName) :
        m_colName(colName) {}

   /** *************************************************************************
    *  @brief Destructore, does nothing.
    */
   virtual ~ColMetaData() {}

   /** *************************************************************************
    *  @brief Re-assignes this class with the table.
    *
    *  Next time data are written to @b fitsDalTable, the values stored in
    *  the child classes of this class are written to the corresponding column.
    */
   virtual void reAssign(FitsDalTable * fitsDalTable) = 0;


   /** *************************************************************************
    *  @brief Set the HK conversion map
    *
    *  It is implemented in the ColStrData class
    */
   virtual void setHkConversion(std::map<std::string, std::vector<std::string>> & hkConversion) {}

   /** *************************************************************************
    *  @brief Set the value of the Column class from the TM packet
    *
    *  It is used only for PRW tables.
    */
   virtual void setFromTm(const int8_t * tmPacketData, uint16_t maxLength) {};

   /** *************************************************************************
    *  @brief Change the value of the column by a random value.
    *
    *  It is used only for PRW tables. The function does nothing if the user
    *  as set at least once the value by one of the setValue functions.
    */
   virtual void setRandomDiff()                   {}


   virtual void setValue(bool              value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int8_t            value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint8_t           value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int16_t           value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint16_t          value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int32_t           value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint32_t          value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int64_t           value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint64_t          value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(OBT               value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(float             value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(double            value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(const std::string value) {} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
           void setValue(const char * value) {setValue(std::string(value)); } ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Stores beside the meta-data the integer value of one column
 *
 *  Class can be used for any integer data types from int8_t to uint64_t
 */
template <class T>
class ColIntData : public ColMetaData {

   uint16_t    m_tmOffset;      ///< offset in bytes of location in TM packet

   T           m_value;         ///< value of the current row in the column
   T           m_minValue;      ///< minimum value that is possible.
   T           m_maxValue;      ///< maximum value that is possible.

   bool        m_randomValue = true;  ///< true if m_value is set by this class to a random value.

   /// support a uniform distribution of integer values.
   boost::random::uniform_int_distribution<int64_t> m_distribution;

public:

   /** *************************************************************************
    *  @brief  The only available constructor
    *
    *  The initial value of the column is set to (minValue + maxValue) / 2
    *
    *  @param colName   [in] name of the column this class is assigned to
    *  @param tmOffset  [in] the offset in bytes on the HK TM of the column of
    *                        this class.
    *  @param minValue  [in] minimum value the value of the column can have.
    *                        It is defined by instrument team.
    *  @param maxValue  [in] maximum value the value of the column can have.
    *                        It is defined by instrument team.
    */
   ColIntData(std::string & colName, uint16_t tmOffset,
                T minValue, T maxValue) :
           ColMetaData(colName), m_tmOffset(tmOffset) ,
           m_minValue(minValue), m_maxValue(maxValue)         {

   m_value = ( m_maxValue + m_minValue) / 2;

   int64_t diff = (int64_t)(m_maxValue / 50) - (int64_t)(m_minValue / 50) + 1;
   m_distribution  = boost::random::uniform_int_distribution<int64_t>(-diff, diff);

   }

   /** *************************************************************************
    *  @brief  re-assign this ColBaseData to the FITS column. That means the
    *          value of this class will be written to the column.
    *
    *  @param [in] fitsDalTable the column has to be in this table. It is
    *              identified by the column name
    */
   void reAssign(FitsDalTable * fitsDalTable)  {
      fitsDalTable->ReAssign(m_colName, &m_value);
   }

   /** *************************************************************************
    *  @brief  Sets the value from the TM packet
    *
    *  The location (offset) in the @b tmPacketData is defined by the tmOffset
    *  parameter of the constructor
    *
    *  @param [in] tmPacketData  the byte stream of the data of one HK TM
    *                            packet.
    *  @param [in] maxLength     the maximum length of the parameter in bytes.
    */
   void setFromTm(const int8_t * tmPacketData, uint16_t maxLength) {
     if (m_tmOffset + sizeof(T) <= maxLength) {
        m_value = swap_endian(*((T*)(tmPacketData + m_tmOffset)));
     }
     else {
        m_value = 0;
     }
   }

   /** *************************************************************************
    *  @brief Change the value of the column by a random value.
    *
    *  It is used only for PRW tables. The function does nothing if the user
    *  as set at least once the value by one of the setValue functions.
    */
   virtual void setRandomDiff();

   virtual void setValue(bool        value)  {m_value = value; m_randomValue = false;}  ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int8_t      value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint8_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int16_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint16_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int32_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint32_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int64_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint64_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(float       value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(double      value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Stores beside the meta-data the integer value of one column
 *
 *  Class can be used for any integer data types from int32_t to int64_t,
 *  but not uint64_t,
 *  where the data are stored in less bytes than sizeof(T) in the TM packet
 */
template <class T>
class ColReducedIntData : public ColMetaData {

   uint16_t    m_tmOffset;      ///< offset in bytes of location in TM packet
   uint16_t    m_tmBytes;       ///< Number of bytes in the TM Packet
   bool        m_signedInt;     ///< this is a signed integer

   T           m_value;         ///< value of the current row in the column
   T           m_minValue;      ///< minimum value that is possible.
   T           m_maxValue;      ///< maximum value that is possible.

   bool        m_randomValue = true;  ///< true if m_value is set by this class to a random value.

   /// support a uniform distribution of integer values.
   boost::random::uniform_int_distribution<int64_t> m_distribution;

public:

   /** *************************************************************************
    *  @brief  The only available constructor
    *
    *  The initial value of the column is set to (minValue + maxValue) / 2
    *
    *  @param colName   [in] name of the column this class is assigned to
    *  @param tmOffset  [in] the offset in bytes on the HK TM of the column of
    *                        this class.
    *  @param tmBytes   [in] number of bytes in the TM packet for this column.
    *                        Has to be equal or less than sizeof(T).
    *  @param minValue  [in] minimum value the value of the column can have.
    *                        It is defined by instrument team.
    *  @param maxValue  [in] maximum value the value of the column can have.
    *                        It is defined by instrument team.
    */
   ColReducedIntData(std::string & colName, uint16_t tmOffset, uint16_t tmBytes,
                     bool signedInt,
                     T minValue, T maxValue) :
           ColMetaData(colName), m_tmOffset(tmOffset) , m_tmBytes(tmBytes),
           m_signedInt(signedInt),
           m_minValue(minValue), m_maxValue(maxValue)         {

   m_value = ( m_maxValue + m_minValue) / 2;

   int64_t diff = (int64_t)(m_maxValue / 50) - (int64_t)(m_minValue / 50) + 1;
   m_distribution  = boost::random::uniform_int_distribution<int64_t>(-diff, diff);

   }

   /** *************************************************************************
    *  @brief  re-assign this ColBaseData to the FITS column. That means the
    *          value of this class will be written to the column.
    *
    *  @param [in] fitsDalTable the column has to be in this table. It is
    *              identified by the column name
    */
   void reAssign(FitsDalTable * fitsDalTable)  {
      fitsDalTable->ReAssign(m_colName, &m_value);
   }

   /** *************************************************************************
    *  @brief  Sets the value from the TM packet
    *
    *  The location (offset) in the @b tmPacketData is defined by the tmOffset
    *  parameter of the constructor
    *
    *  @param [in] tmPacketData  the byte stream of the data of one HK TM
    *                            packet.
    *  @param [in] maxLength     the maximum length of the parameter in bytes.
    *
    */
   void setFromTm(const int8_t * tmPacketData, uint16_t maxLength) {
     if (m_tmOffset + m_tmBytes <= maxLength) {
        int64_t localInt = 0;
        memcpy ( ((int8_t*)&localInt) + 8 - m_tmBytes, tmPacketData + m_tmOffset, m_tmBytes);
        localInt = swap_endian(localInt);
        m_value = localInt;

        if (m_signedInt && (m_value & (1 << (8 * m_tmBytes -1))) ) {
           // this is a negative signed value. We have to set the leading
           // bits to ff that are part of m_value but not part of the integer
           // in the TM packet.
           for (uint16_t byteNr = m_tmBytes; byteNr < sizeof(T); byteNr++)
              m_value |= (0xff << byteNr * 8);
        }

     }
     else {
        m_value = 0;
     }
   }


   virtual void setValue(bool        value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int8_t      value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint8_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int16_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint16_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int32_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint32_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int64_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint64_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(float       value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(double      value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.

};


/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Stores beside the meta-data the value of one column
 *
 *  Class can be used for columns of data type  bool, float and double.
 */
template <class T>
class ColRealData : public ColMetaData {

   uint16_t    m_tmOffset;      ///< offset in bytes of location in TM packet

   T           m_value;         ///< value of the current row in the column
   T           m_minValue;      ///< minimum value that is possible.
   T           m_maxValue;      ///< maximum value that is possible.

   bool        m_randomValue = true;  ///< true if m_value is set by this class to a random value.

   /// support a uniform distribution of integer values.
   boost::random::uniform_real_distribution<double> m_distribution;

public:
   /** *************************************************************************
    *  @brief  The only available constructor
    *
    *  The initial value of the column is set to (minValue + maxValue) / 2
    *
    *  @param colName   [in] name of the column this class is assigned to
    *  @param tmOffset  [in] the offset in bytes on the HK TM of the column of
    *                        this class.
    *  @param minValue  [in] minimum value the value of the column can have.
    *                        It is defined by instrument team.
    *  @param maxValue  [in] maximum value the value of the column can have.
    *                        It is defined by instrument team.
    */
   ColRealData(std::string & colName, uint16_t tmOffset,
                T minValue, T maxValue) :
           ColMetaData(colName), m_tmOffset(tmOffset) ,
           m_minValue(minValue), m_maxValue(maxValue)         {
   m_value = ( m_maxValue + m_minValue) / 2;

   m_distribution = boost::random::uniform_real_distribution<double>(-10000, 10000);
   }

   /** *************************************************************************
    *  @brief  re-assign this ColBaseData to the FITS column. That means the
    *          value of this class will be written to the column.
    *
    *  @param [in] fitsDalTable the column has to be in this table. It is
    *              identified by the column name
    */
   void reAssign(FitsDalTable * fitsDalTable)  {
      fitsDalTable->ReAssign(m_colName, &m_value);
   }

   /** *************************************************************************
    *  @brief  Sets the value from the TM packet
    *
    *  The location (offset) in the @b tmPacketData is defined by the tmOffset
    *  parameter of the constructor
    *
    *  @param [in] tmPacketData  the byte stream of the data of one HK TM
    *                            packet.
    *  @param [in] maxLength     the maximum length of the parameter in bytes.
    *
    */
   void setFromTm(const int8_t * tmPacketData, uint16_t maxLength) {
     if (m_tmOffset + sizeof(T) <= maxLength) {
        m_value = swap_endian(*((T*)(tmPacketData + m_tmOffset)));
     }
     else {
        m_value = std::numeric_limits<T>::quiet_NaN();
     }
   }

   /** *************************************************************************
    *  @brief Change the value of the column by a random value.
    *
    *  It is used only for PRW tables. The function does nothing if the user
    *  as set at least once the value by one of the setValue functions.
    */
   virtual void setRandomDiff();

   virtual void setValue(bool        value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int8_t      value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint8_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int16_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint16_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int32_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint32_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(int64_t     value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(uint64_t    value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(float       value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.
   virtual void setValue(double      value)  {m_value = value; m_randomValue = false;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Stores beside the meta-data the value of one string column
 *
 */
class ColStrData : public ColMetaData {

   std::string    m_calCurveName;        ///< name of the calibration curve, that has to be used for this column
   std::string    m_value;               ///< value of the current row in the column
   bool           m_randomValue = true;  ///< true if m_value is set by this class to a random value.
   std::vector<std::string> m_hkConversion;  ///< possible values for this column
   uint8_t        m_enumIndex = 0;       ///< current index in m_hkConversion value

   /// support a uniform distribution of int16_t values.
   boost::random::uniform_int_distribution<int16_t> m_distribution;

public:
   /** *************************************************************************
     *  @brief  The only available constructor
     *
     *  @param colName       [in] name of the column this class is assigned to
     *  @param calCurveName  [in] name of the conversion curve for this column
     *
     */
   ColStrData(std::string & colName, std::string & calCurveName) :
      ColMetaData(colName), m_calCurveName(calCurveName)  {

   m_distribution  = boost::random::uniform_int_distribution<int16_t>(0, 10);
   }

   /** *************************************************************************
    *  @brief  re-assign this ColInfoStrData to the FITS column. That means the
    *          value of this class will be written to the column.
    *
    *  @param [in] fitsDalTable the column has to be in this table. It is
    *              identified by the column name
    */
  void reAssign(FitsDalTable * fitsDalTable)  {
      fitsDalTable->ReAssign(m_colName, &m_value);
   }

  /** *************************************************************************
   *  @brief Set the HK conversion map
   *
   *  The function selects the valid conversion curve for this column and copies
   *  it to m_hkConversion.
   *
   *
   */
 virtual void setHkConversion(std::map<std::string, std::vector<std::string>> & hkConversion)
     { if (hkConversion.find(m_calCurveName) == hkConversion.end())
        throw std::runtime_error("unknown conversion curve: " + m_calCurveName +
                                 " requested for column " + m_colName);

       m_hkConversion = hkConversion[m_calCurveName];
          m_value = m_hkConversion[m_enumIndex];}

 /** *************************************************************************
  *  @brief Change the value of the column by a random value.
  *
  *  It is used only for PRW tables. The function does nothing if the user
  *  as set at least once the value by one of the setValue functions.
  */
   virtual void setRandomDiff();

   virtual void setValue(const std::string value);  ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.

};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Stores beside the meta-data the value of one time column, a OBT,
 *          a UTC, a MJD or a BJD column
 *
 */
template <class T>
class ColTimeData : public ColMetaData {

   T         m_value;      ///< current time of the column

public:

   /** *************************************************************************
    *  @brief Constructor, set the name of the column
    *
    *  @param [in] colName   name of the column this class is assigned to.
    */
   ColTimeData(std::string & colName) :
      ColMetaData(colName) {}

   /** *************************************************************************
    *  @brief  re-assign this ColInfoTimeData to the FITS column. That means the
    *          value of this class will be written to the column.
    *
    *  @param [in] fitsDalTable the column has to be in this table. It is
    *              identified by the column name
    */
   void reAssign(FitsDalTable * fitsDalTable)  {
      fitsDalTable->ReAssign(m_colName, &m_value);
   }

   /** *************************************************************************
     *  @brief  Set a new time to this column
     *
     *  @param [in] time  new time of the column.
     */
   void SetTime(const T & time) {m_value = time;}
};

/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief  Stores beside the meta-data the value of one time column, a OBT
 *          column
 *
 */

class ColOBTData : public ColMetaData {

   static const int32_t OBT_LEN; ///< the length of an OBT in the TM stream, in
                                 ///< bytes
   OBT       m_value;      ///< current time of the column
   uint16_t  m_tmOffset;   ///< offset in bytes of location in TM packet

public:

   /** *************************************************************************
    *  @brief Constructor, set the name of the column
    *
    *  @param [in] colName   name of the column this class is assigned to.
    */
   ColOBTData(std::string & colName, uint16_t tmOffset) :
      ColMetaData(colName), m_tmOffset(tmOffset)  {}

   /** *************************************************************************
    *  @brief  re-assign this ColInfoTimeData to the FITS column. That means the
    *          value of this class will be written to the column.
    *
    *  @param [in] fitsDalTable the column has to be in this table. It is
    *              identified by the column name
    */
   void reAssign(FitsDalTable * fitsDalTable)  {
      fitsDalTable->ReAssign(m_colName, &m_value);
   }

   /** *************************************************************************
    *  @brief  Sets the value from the TM packet
    *
    *  The location (offset) in the @b tmPacketData is defined by the tmOffset
    *  parameter of the constructor
    *
    *  @param [in] tmPacketData  the byte stream of the data of one HK TM
    *                            packet.
    *  @param [in] maxLength     the maximum length of the parameter in bytes.
    *
    */
   void setFromTm(const int8_t * tmPacketData, uint16_t maxLength) {
     if (m_tmOffset + OBT_LEN <= maxLength) {
        int64_t localObt = 0;
        memcpy (((int8_t*)&localObt) + 2, tmPacketData + m_tmOffset, OBT_LEN);
        localObt = swap_endian(localObt);
        m_value = localObt;
     }
     else {
        m_value = 0;
     }
   }

   /** *************************************************************************
     *  @brief  Set a new time to this column
     *
     *  @param [in] time  new time of the column.
     */
   void SetTime(const OBT & time) {m_value = time;}

   void setValue(OBT               value) {m_value = value;} ///< set the value of the column. Once it is set it will never be updated by the setRandomDiff function any more.

};




/** ****************************************************************************
 *  @ingroup FitsDataModel
 *  @author Reiner Rohlfs UGE
 *
 *  @brief base class of the housekeeping processing system
 *
 *  The class is responsible to read the HK fsd files and to store the
 *  information of the HK TM packets and the HK parameters in internal data
 *  structures.
 */
class HkProcessing
{
protected:

   /// multimap<packetId, extensionName>, only used for PRW HK tables
   std::multimap<TmPacketId, std::string>  m_prwTableInfo;

   /// map<extensionName, location of OBT in TM data>, only used for PRW HK tables
   /// if OBT location in TM < 0 then the OBT shall be used from the TM header
   std::map<std::string, int16_t>    m_obtTmOffset;

   /// map<extensionName, map<columName, ColMetaData *>>
   std::map<std::string, std::map<std::string, ColMetaData *>>  m_colData;

   /** *************************************************************************
    *  @brief Reads one SCW_PRW_HK*.fsd file
    */
   void ReadPrwFsdFile(const std::string & fileName);

   /** *************************************************************************
    *  @brief Reads one SCW_RAW_HK*.fsd file
    */
   void ReadRawFsdFile(const std::string & fileName);

protected:
   /** *************************************************************************
    *  @brief Default constructor
    *
    *  Reads all SCI_PRW_HK* and SCI_RAW_HK* fsd files and stores the
    *  information of the HK TM packets and the HK parameters in
    *  m_prwTableInfo, m_obtTmOffset, and m_colInfo
    *
    *  @param prw [in]  reads SCI_PRW_HK* fsd files if @b prw is set to true.
    *  @param raw [in]  reads SCI_RAW_HK* fsd files if @b raw is set to true.
    */
   HkProcessing(bool prw, bool raw);


   /** *************************************************************************
    *  @brief Destructor
    *
    *  Deletes the ColMetaData in m_colData.
    *
    */  virtual ~HkProcessing();
};


#endif /* _HK_PROCESSING_HXX_ */

