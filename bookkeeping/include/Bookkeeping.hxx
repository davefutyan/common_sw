/** ****************************************************************************
 *  @file
 *
 *  @ingroup bookkeeping
 *  @brief Declaration of the Bookkeeping class, the BKNumber struct and the BKItem struct
 *
 *  @author  Reiner Rohlfs, UGE
 *
 *  @version 12.0.1 2019-11-14 ABE #20111 Handle 32 bit unsigned integers
 *  @version  9.0   2018-02-01 RRO        UTC can be used as bookkeeping number
 *  @version  7.0   2017-01-20 RRO        first released version
 */

#include <string>
#include <list>
#include <vector>
#include <set>

#include "Utc.hxx"


/** ****************************************************************************
 *  @ingroup bookkeeping
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Defines the possible data types a number of a bookkeeping item can
 *         have.
 */
enum BK_DATA_TYPE {
   bk_uint8,    ///< unsigned integer 1 byte
   bk_uint16,   ///< unsigned integer 2 bytes
   bk_UTC,       ///< a UTC time
   bk_uint32   ///< unsigned integer 4 bytes
   // bk_int16,    ///< signed integer 2 bytes    currently not implemented
   // bk_int32,    ///< signed integer 4 bytes    currently not implemented
};


/** ****************************************************************************
 *  @ingroup bookkeeping
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Defines one number of a bookkeeping item.
 */
struct BKNumber {
   std::string   m_name;           ///< name of the number
   bool          m_key;            ///< used by the find - queries if set to true
   BK_DATA_TYPE  m_bkDataType;     ///< defines the data type of the number

   uint8_t       m_uint8Number;     ///< valid if m_bkDataType == bk_uint8
   uint16_t      m_uint16Number;    ///< valid if m_bkDataType == bk_uint16
   UTC           m_utc;             ///< valid if m_bkDataType == bk_UTC
   uint32_t      m_uint32Number;    ///< valid if m_bkDataType == bk_uint32
   // int16_t    m_int16Number;     ///< valid if m_bkDataType == int16   currently not implemented
   // int32_t    m_int32Number;     ///< valid if m_bkDataType == int32   currently not implemented

   /** ************************************************************************
    *  @brief compares only m_name, m_key and m_bkDataType, but not the
    *          values of the number.
    */
   bool operator == (const BKNumber & bkNunber) const  {
      return m_key == bkNunber.m_key && m_bkDataType == bkNunber.m_bkDataType &&
             m_name == bkNunber.m_name; }

};

/** ****************************************************************************
 *  @ingroup bookkeeping
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Defines one item for which the status is / can be booked.
 *
 *  A BKItem is for example one visit.\n
 *  The status of a BKItem can be either "exist" or "does not exist"\n
 *  The list of kbNumbers has to be identical as defined for the bookkeeping
 *  group this item belongs to. The order of the BKNumbers as well as the
 *  BKNumber itself must fit (see also compare operator of BKNumber)
 */
struct BKItem {
   UTC                    m_start;      ///< start time of the item ( the visit )
   UTC                    m_stop;       ///< stop time of the item ( the visit )
   std::vector<BKNumber>  m_bkNumbers;  ///< list of BKNumbers of this item


   /** ************************************************************************
    *  @brief compares the start time of the BKItem.
    */
   bool operator < (const BKItem & bkItem) const {
      return m_start < bkItem.m_start;}

};

/** ****************************************************************************
 *  @ingroup bookkeeping
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Main class of the Bookkeeping module.
 *
 *
 */
class Bookkeeping {

   std::vector<BKNumber>   m_master;    ///< @brief the list of BKNumbers of
                                        ///  this bookkeeping group
   std::string             m_groupDir;  ///< @brief complete path of the directory
                                        ///  of this bookkeeping group

public:
   /** ****************************************************************************
    *  @brief Creates or opens an existing bookkeeping group.
    *
    *  @param [in] path   base directory of all bookkeeping groups
    *  @param [in] name   name of the bookkeeping group. All items will be stored
    *                     in a sub-directory of @b path. The sub-directory is
    *                     called @b name.
    *  @param [in] masterList  defines the BKNumbers of this bookkeeping group.
    *                          Only the m_name, m_key and m_bkDataType of each
    *                          BKNumber has to be specified. The value of the
    *                          numbers are ignored.
    *
    *  @throw runtime_error   if the bookkeeping group exists already but the
    *                         @b masterlist is not identical to the one when
    *                         the bookkeeping group was created.
    */
   Bookkeeping(const std::string & path, const std::string & name,
               std::vector<BKNumber> & masterList);

   /** ****************************************************************************
    *  @brief Inserts a new bookkeeping item into this group
    *
    *  Values for all numbers and the start and stop times must be provided.\n
    *  The time range defined by bkItem.m_start and bkItem.m_stop must not
    *  yet used by an other bookkeeping item. Except if the @b bkItem is
    *  identical to an already existing item. i.e. all values of all numbers are
    *  identical and the start and stop times are identical. In this case the
    *  method does nothing and returns without creating a new item.
    *
    *  @throw runtime_error if the key Numbers of @b bkItem are not identical as
    *         the masterList provided to the constructor of this class. Identical
    *         means the order in the vector has to be identical and the m_name, the
    *         m_key and the m_bkDataType have to be identical for each BKNumber.
    *
    *  @throw runtime_error if the time range of the @b bkItem, defined by
    *         @b bkItem.m_start and @b kbItem.m_stop is already used by an
    *         other bookkeeping item.
    */
  void Insert(const BKItem & bkItem);

   /** ****************************************************************************
    *  @brief Returns a list of BKItems that currently exist and have the
    *         same values as @b bkItem for the key - numbers, i.e. for which
    *         m_key is set to true.
    *
    *  The m_start and m_stop of @b bkItem are not used in the search.
    *
    *  @throw runtime_error if the key Numbers of @b bkItem are not identical as
    *         the masterList provided to the constructor or this class. Identical
    *         means the order in the vector has to be identical and the m_name, the
    *         m_key and the m_bkDataType have to be identical for each BKNumber.
    */
   std::list<BKItem>   Find(const BKItem & bkItem);



};
