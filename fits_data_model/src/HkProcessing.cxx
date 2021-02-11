/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDataModel
 *  @brief Implementation of the HkProcessing class
 *
 *  @author Reiner Rohlfs UGE
 *
 *
 *  @version 12.1.4 2020-04-22 ABE #21297 Provide length of TM packet to
 *                                        HkPrwFitsTable::addTmPacket()
 *  @version 12.0.1 2020-03-16 RRO #20991 Implement the decoding of TC, appId=961
 *  @version 12.0  2019-10-23 RRO #19772  Implement the decoding of TC(196,1)
 *  @version 9.1.4 2018-04-11 RRO #15832  OBT can be a column filled with data
 *                                        from the TM data field
 *  @version 6.3 2016-09-23 RRO        enum values are not continuous values any more
 *  @version 4.1 2015-07-22 RRO        first released version
 */
#include <cstdlib>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include <Logger.hxx>
#include <Obt.hxx>
#include <Utc.hxx>
#include <Mjd.hxx>
#include <Bjd.hxx>

#include "fits_data_model_schema.hxx"
#include "HkProcessing.hxx"

using namespace std;
using namespace boost::filesystem;


boost::random::mt19937 ColMetaData::m_gen;


static map<column_data_type, int16_t> DATA_SIZE = {
      {column_data_type::A,       0},  // not used in PRW data structures
      {column_data_type::string,  0},
      {column_data_type::L,       1},
      {column_data_type::bool_,   1},
      {column_data_type::S,       1},
      {column_data_type::int8,    1},
      {column_data_type::B,       1},
      {column_data_type::uint8,   1},
      {column_data_type::I,       2},
      {column_data_type::int16,   2},
      {column_data_type::U,       2},
      {column_data_type::uint16,  2},
      {column_data_type::J,       4},
      {column_data_type::int24,   3},
      {column_data_type::int32,   4},
      {column_data_type::V,       4},
      {column_data_type::uint24,  3},
      {column_data_type::uint32,  4},
      {column_data_type::K,       8},
      {column_data_type::int64,   8},
      {column_data_type::W,       8},
      {column_data_type::uint64,  8},
      {column_data_type::E,       4},
      {column_data_type::float_,  4},
      {column_data_type::D,       8},
      {column_data_type::double_, 8},
      {column_data_type::OBT,     6},
      {column_data_type::CUC,     6},
      {column_data_type::UTC,     0},  // not used in TM data
      {column_data_type::MJD,     0},  // not used in TM data
      {column_data_type::BJD,     0}   // not used in TM data
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static ColMetaData * NewColData(table_type::column_iterator & i_col,
                                uint16_t tmOffset) {

   ColMetaData * colData;

   switch (i_col->data_type()) {
      case column_data_type::A:
      case column_data_type::string:
         if (!i_col->cal_curve().present()) {
            throw runtime_error("Missing calibration curve for column " + i_col->name());
         }
         colData = new ColStrData(i_col->name(), i_col->cal_curve().get());
         break;

      case column_data_type::L:
      case column_data_type::bool_:
         colData = new ColRealData<bool>(i_col->name(),  tmOffset, 0, 1);
         break;

      case column_data_type::S:
      case column_data_type::int8:
         colData = new ColIntData<int8_t>(i_col->name(),  tmOffset,
                         i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<int8_t>::min(),
                         i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<int8_t>::max() );
         break;

      case column_data_type::B:
      case column_data_type::uint8:
         colData = new ColIntData<uint8_t>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<uint8_t>::min(),
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<uint8_t>::max() );
         break;

      case column_data_type::I:
      case column_data_type::int16:
         colData = new ColIntData<int16_t>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<int16_t>::min(),
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<int16_t>::max() );
         break;

      case column_data_type::U:
      case column_data_type::uint16:
         colData = new ColIntData<uint16_t>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<uint16_t>::min(),
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<uint16_t>::max() );
         break;

      case column_data_type::J:
      case column_data_type::int32:
         colData = new ColIntData<int32_t>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<int32_t>::min() / 2,
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<int32_t>::max() / 2 );
         break;

      case column_data_type::int24:
         colData = new ColReducedIntData<int32_t>(i_col->name(), tmOffset, 3, true,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<int16_t>::min() / 2,
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<int16_t>::max() / 2 );
         break;

      case column_data_type::V:
      case column_data_type::uint32:
         colData = new ColIntData<uint32_t>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<uint32_t>::min() / 2,
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<uint32_t>::max() / 2);
         break;

      case column_data_type::uint24:
         colData = new ColReducedIntData<uint32_t>(i_col->name(), tmOffset, 3, false,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<uint16_t>::min() / 2,
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<uint16_t>::max() / 2 );
         break;

      case column_data_type::K:
      case column_data_type::int64:
         colData = new ColIntData<int64_t>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<int64_t>::min(),
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<int64_t>::max() );
         break;

      case column_data_type::W:
      case column_data_type::uint64:
         colData = new ColIntData<uint64_t>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : std::numeric_limits<uint64_t>::min(),
               i_col->max_value().present() ? i_col->max_value().get() : std::numeric_limits<uint64_t>::max() );
         break;

      case column_data_type::E:
      case column_data_type::float_:
         colData = new ColRealData<float>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : -1000000,
               i_col->max_value().present() ? i_col->max_value().get() : 1000000 );
         break;

      case column_data_type::D:
      case column_data_type::double_:
         colData = new ColRealData<double>(i_col->name(), tmOffset,
               i_col->min_value().present() ? i_col->min_value().get() : -1000000,
               i_col->max_value().present() ? i_col->max_value().get() : 1000000 );
         break;

      case column_data_type::OBT:
      case column_data_type::CUC:
         colData = new ColOBTData(i_col->name(), tmOffset);
         break;

      case column_data_type::UTC:
         colData = new ColTimeData<UTC>(i_col->name());
         break;

      case column_data_type::MJD:
         colData = new ColTimeData<MJD>(i_col->name());
         break;

      case column_data_type::BJD:
         colData = new ColTimeData<BJD>(i_col->name());
         break;
   }

   return colData;
}

////////////////////////////////////////////////////////////////////////////////
void HkProcessing::ReadPrwFsdFile(const std::string & fileName) {


   int16_t obtCounter = 0;

   auto_ptr<Fits_schema_type> fsd (FITS_schema(fileName));

   if (fsd->HDU().table().present() ) {
      table_type  table = fsd->HDU().table().get();

      if (table.appId().present() &&
          table.SID().present()       ) {
        m_prwTableInfo.insert( pair<TmPacketId, string>(
                  TmPacketId(table.appId().get(), table.SID().get() ) ,
                  fsd->HDU().extname()                  ) );

        if (table.obtOffset().present())
           m_obtTmOffset[fsd->HDU().extname()] = table.obtOffset().get();
        else
           // < 0  identifies this packat to use the OBT from the TM header
           //      and not from a specific offset in the data.
           m_obtTmOffset[fsd->HDU().extname()] = -1;

        map<string,ColMetaData*> & map_colData = m_colData[fsd->HDU().extname()];
        uint16_t tmOffset = 0;
        // loop over all columns
        table_type::column_iterator i_col = table.column().begin();
        table_type::column_iterator i_colEnd = table.column().end();
        while (i_col != i_colEnd)
        {
           if (i_col->tm_offset().present()) {
              tmOffset = i_col->tm_offset().get();
           }
           ColMetaData * colData = NewColData(i_col, tmOffset);
           map_colData.insert(pair<string, ColMetaData *> (i_col->name(), colData));

           // the first OBT in the fits table is not read from the data but
           // set from the TM - header
           if (i_col->data_type() == column_data_type::OBT)
              obtCounter++;

           if (i_col->data_type() != column_data_type::OBT ||
               obtCounter > 1 )
              tmOffset += DATA_SIZE[i_col->data_type()];

           ++i_col;
        }


      }
   }

}


////////////////////////////////////////////////////////////////////////////////
void HkProcessing::ReadRawFsdFile(const std::string & fileName) {


   auto_ptr<Fits_schema_type> fsd (FITS_schema(fileName));

   if (fsd->HDU().table().present() ) {
      table_type  table = fsd->HDU().table().get();

      map<string,ColMetaData*> & map_colData = m_colData[fsd->HDU().extname()];
      // loop over all columns
      table_type::column_iterator i_col = table.column().begin();
      table_type::column_iterator i_colEnd = table.column().end();
      while (i_col != i_colEnd)
      {
         ColMetaData * colData = NewColData(i_col, 0);
         map_colData.insert(pair<string, ColMetaData *> (i_col->name(), colData));

         ++i_col;
       }

   }

}

////////////////////////////////////////////////////////////////////////////////
HkProcessing::HkProcessing(bool prw, bool raw) {

   // find the HK = fsd files which are located in $CHEOPS_SW/resources

   const char * cheops_sw = getenv("CHEOPS_SW");
   if (cheops_sw == nullptr)
      throw runtime_error("Environment variable CHEOPS_SW is not defined. "
            "Cannot read the *.fsd files.");

   // get all SCI_RAW_Hk*.fsd and SCI_PRW_HK*.fsd files.
   string resourceDir = string(cheops_sw) + "/resources";
   directory_iterator i_dir = directory_iterator(resourceDir);
   while (i_dir != directory_iterator()) {
      string filePath = i_dir->path().string();
      if (raw && filePath.find("SCI_RAW_Hk") != string::npos  &&
                 // ignore SCI_RAW_HkOperationParameter #20991
                 filePath.find("SCI_RAW_HkOperationParameter") == string::npos &&
          filePath.find(".fsd")       != string::npos     ) {
         ReadRawFsdFile(filePath);
      }

      else if (prw && filePath.find("SCI_PRW_Hk") != string::npos  &&
               filePath.find(".fsd")       != string::npos     ) {
         ReadPrwFsdFile(filePath);
      }

      ++i_dir;
   }
}

////////////////////////////////////////////////////////////////////////////////
HkProcessing::~HkProcessing() {

   map<string, map<string, ColMetaData *>>::iterator i_colData = m_colData.begin();
   while (i_colData != m_colData.end()) {
      map<string, ColMetaData *>::iterator ii_colData = i_colData->second.begin();
      while (ii_colData != i_colData->second.end()) {
            delete ii_colData->second;

            ii_colData++;
      }

      i_colData++;
   }

}

////////////////////////////////////////////////////////////////////////////////
template <> void ColRealData<bool>::setRandomDiff() {

   if (m_randomValue) {
      if ( m_distribution(m_gen) > 5000)
         m_value = !m_value;
   }

}

////////////////////////////////////////////////////////////////////////////////
template <typename T> void ColRealData<T>::setRandomDiff() {
   if (m_randomValue) {
      double random = m_distribution(m_gen);
      if (random < 0 && m_value < m_minValue - random * 2)
         random = -random;
      else if (random > 0 && m_value > m_maxValue - random * 2)
         random = -random;

      m_value += random;
      }

}

////////////////////////////////////////////////////////////////////////////////
template <typename T> void ColIntData<T>::setRandomDiff() {
   if (m_randomValue) {
      int64_t random = m_distribution(m_gen);
      if (random < 0 && m_value < m_minValue - random * 2)
         random = -random;
      else if (random > 0 && m_value > m_maxValue - random * 2)
         random = -random;

      m_value += random;
      }
}

////////////////////////////////////////////////////////////////////////////////
void ColStrData::setRandomDiff() {

   if (m_hkConversion.size() == 0)
      throw runtime_error("There is no conversion defined for string parameter " +
            m_colName);

   if (m_randomValue) {
      int16_t random = m_distribution(m_gen);
      if (random == 0 && m_enumIndex != 0) {
         m_enumIndex--;
         m_value = m_hkConversion[m_enumIndex];
      }
      else if (random == 1 && m_enumIndex != m_hkConversion.size() - 1 ) {
         m_enumIndex++;
         m_value = m_hkConversion[m_enumIndex];
      }
   }

}

////////////////////////////////////////////////////////////////////////////////
void ColStrData::setValue(const std::string value)  {

   // is value a text which is valid for this column?
   vector<string>::iterator i_hkConversion = m_hkConversion.begin();
   while (i_hkConversion != m_hkConversion.end()) {
      if (value == *i_hkConversion) {
         // this is OK
         m_value = value;
         m_randomValue = false;
         return;
      }
      i_hkConversion++;
   }

   // the value is not one of the possible strings for this column
   throw runtime_error("Cannot set the text \"" + value + "\" in column " +
                        m_colName + ". It isn't one of the possilbe strings.");

}

////////////////////////////////////////////////////////////////////////////////
const int32_t ColOBTData::OBT_LEN = 6;

// explicit instantiation
template void ColIntData<bool>::setRandomDiff ();
template void ColIntData<int8_t>::setRandomDiff ();
template void ColIntData<uint8_t>::setRandomDiff ();
template void ColIntData<int16_t>::setRandomDiff ();
template void ColIntData<uint16_t>::setRandomDiff ();
template void ColIntData<int32_t>::setRandomDiff ();
template void ColIntData<uint32_t>::setRandomDiff ();
template void ColIntData<int64_t>::setRandomDiff ();
template void ColIntData<uint64_t>::setRandomDiff ();
template void ColRealData<float>::setRandomDiff ();
template void ColRealData<double>::setRandomDiff ();

