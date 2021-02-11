/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief Implementations of the FitsDalTable class
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 9.3.1 2018-06-18 RRO #16505: new method: SetReadRow()
 *  @version 9.0.1 2018-01-31 RRO #15376: update of method IsUnsignedColumn()
 *  @version 9.0   2018-01-04 RRO #15057: new method: GetColUnit() and update of
 *                                        FitsColMetaDataIntern with the unit of
 *                                        the columns.
 *  @version 8.1.1 2017-10-30 RRO #14849: The header keyword TSCALx is not required
 *                                        any more to identify a column as unsigned
 *                                        short, int or long
 *  @version 6.4   2016-11-28 RRO #11599: Do not read rows after m_numWrittenRows in m_updaet mode
 *  @version 6.1   2016-08-10 RRO #11072: close a memory leak in ReadRow()
 *  @version 5.2   2016-05-05 RRO #10580: implement random access of rows
 *  @version 5.1   2016-03-20 RRO bug fix in loop at line 448
 *  @version 4.3.1 2015-11-02 RRO #9609: FitsDalTable::GetFitsColMetaData() shall return the list of metadata for the columns also when the FITS file was created.
 *  @version 4.3   2015-10-17 RRO #9378: New method: GetFitsColMetaData()
 *  @version 4.3   2015-10-13 RRO #9304: Initialize values for which we have no column any more in the FITS table
 *  @version 4.1   2015-07-28 RRO #8916: New method: ReAssign()
 *  @version 3.0   2014-12-17 RRO #7054: Support NULL values of columns
 *  @version 3.0   2015-01-05 RRO #7056: better error message if a new HDU
 *                                       cannot be created.
 *  @version 1.1   2014-07-24 RRO #6076: Call FitsDalHeader::VerifyChecksum()
 *  @version 1.0   2013-12-14 RRO first released version
 *
 */


#include <string>
#include <typeinfo>
#include <string.h>
#include <sys/stat.h>
#include <cmath>
#include <limits>

#include "FitsDalTable.hxx"

using namespace std;



static map<int, int> fits_data_type_to_index {
   {TLOGICAL,    0},
   {TSBYTE,      1},
   {TBYTE ,      2},
   {TSHORT ,     3},
   {TUSHORT ,    4},
   {TINT ,       5},
   {TUINT ,      6},
   {TLONGLONG ,  7},
   {TULONGLONG , 8},
   {TFLOAT ,     9},
   {TDOUBLE ,   10},
   {TSTRING ,   11}
};

template <typename T> char TypeId() {
   throw runtime_error(string("Requesting a not supported data type in TypeId :") +
                       typeid(T).name() ); }       ///< returns the FITS data type ID
template<> char TypeId<bool>()      {return 'L';}  ///< returns the FITS data type ID
template<> char TypeId<int8_t>()    {return 'S';}  ///< returns the FITS data type ID
template<> char TypeId<uint8_t>()   {return 'B';}  ///< returns the FITS data type ID
template<> char TypeId<int16_t>()   {return 'I';}  ///< returns the FITS data type ID
template<> char TypeId<uint16_t>()  {return 'U';}  ///< returns the FITS data type ID
template<> char TypeId<int32_t>()   {return 'J';}  ///< returns the FITS data type ID
template<> char TypeId<uint32_t>()  {return 'V';}  ///< returns the FITS data type ID
template<> char TypeId<int64_t>()   {return 'K';}  ///< returns the FITS data type ID
template<> char TypeId<uint64_t>()  {return 'W';}  ///< returns the FITS data type ID
template<> char TypeId<float>()     {return 'E';}  ///< returns the FITS data type ID
template<> char TypeId<double>()    {return 'D';}  ///< returns the FITS data type ID
template<> char TypeId<string>()    {return 'A';}  ///< returns the FITS data type ID

template <typename T> int DataTypeIndex() {
   throw runtime_error(string("Requesting a not supported data type in DataTypeIndex :") +
                       typeid(T).name() ); }            ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<bool>()     {return  0;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<int8_t>()   {return  1;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<uint8_t>()  {return  2;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<int16_t>()  {return  3;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<uint16_t>() {return  4;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<int32_t>()  {return  5;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<uint32_t>() {return  6;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<int64_t>()  {return  7;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<uint64_t>() {return  8;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<float>()    {return  9;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<double>()   {return 10;}  ///< returns the fits_dal - index of the data type
template <> int DataTypeIndex<string>()   {return 11;}  ///< returns the fits_dal - index of the data type


template <typename T> int FitsSize() {
   throw runtime_error(string("Requesting a not supported data type in FitsSize :") +
                       typeid(T).name() ); }       ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<bool> ()     {return 1;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<int8_t> ()   {return 1;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<uint8_t> ()  {return 1;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<int16_t> ()  {return 2;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<uint16_t> () {return 2;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<int32_t> ()  {return 4;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<uint32_t> () {return 4;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<int64_t> ()  {return 8;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<uint64_t> () {return 8;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<float> ()    {return 4;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<double> ()   {return 8;}  ///< returns the number of bytes of one variable of the give data type
template <> int FitsSize<string> ()   {return 1;}  ///< returns the number of bytes of one variable of the give data type

FitsDalTable::FitsDalTable(const string & filename, const char *mode )
{
   // open the table
   int status = 0;
   m_numWrittenRows = 0;
   m_nextWriteRow = 1;
   m_nextReadRow = 1;
   m_rowLength    = 0;

   m_update = false;


   if (strcmp(mode, "READONLY") == 0)
      {
      fits_open_table(&m_fitsFile, filename.c_str(), READONLY, &status);
      if (status != 0)
           throw runtime_error("Failed to open table " + filename +
                          ", cfitsio error: " + to_string(status));

      // get the offset and other attributes for every column
      // this is needed once the user assigns a column to a user variable
      // (function Assign)

      VerifyChecksum();

      ReadAllKeywords();

      // get number of columns
      int numCols = 0;
      fits_get_num_cols(m_fitsFile, &numCols, &status);
      for (int col = 1; col <= numCols; col++)
         {

         // get the information about this column from the FITS table
         int   dataType;
         long  repeat;
         long  width;
         char  colName[80];
         fits_get_coltype(m_fitsFile, col, &dataType, &repeat, &width, &status);
         if (dataType == TSTRING)
             width = 1;

          if (dataType == TLONG)   dataType = TINT;

          char strNum[20];
          sprintf(strNum, "%d", col);
          int dummy;
          fits_get_colname(m_fitsFile, CASEINSEN, strNum, colName, &dummy, &status);
          if (status != 0)
             throw runtime_error("Failed to get name and column attributes of "
                                 "column number " + to_string(col) + " in table " +
                                 filename + ", cfitsio error: " + to_string(status));

          // is a column a unsigned int or unsigned short column?
          if (dataType == TLONGLONG && IsUnsignedColumn(col, uint64_t(0x8000000000000000ULL)) )
             dataType = TULONGLONG;
          if (dataType == TINT && IsUnsignedColumn(col, uint32_t(2147483648)) )
             dataType = TUINT;
          if (dataType == TSHORT && IsUnsignedColumn(col, uint16_t(32768)) )
             dataType = TUSHORT;
          if (dataType == TBYTE && IsUnsignedColumn(col, int8_t(-128)) )
             dataType = TSBYTE;

          map<int, int>::iterator i_fits_data_type_to_index = fits_data_type_to_index.find(dataType);
          if (i_fits_data_type_to_index == fits_data_type_to_index.end())
             {
             // this column has a not supported data type. Write a warning and continue

             m_rowLength += repeat * width;
             continue;
             }

          // get the unit of the column
          char unit[80];
          unit[0] = 0;
          char fitsUnit[80];
          char keyname[10];
          sprintf(keyname, "TUNIT%d", col);
          if (fits_read_keyword(m_fitsFile, keyname, fitsUnit, NULL, &status) == 0) {
             // remove the ' ' of the unit string
             ffc2s(fitsUnit, unit, &status);
          }
          else {
             // the unit is not defined, this is OK
             status = 0;
          }

          // we can use this column
          FitsColMetaDataIntern fits_col_meta_data(col, unit,
                               i_fits_data_type_to_index->second, repeat, m_rowLength);
          m_fitsColMetaData.insert( pair<string, FitsColMetaDataIntern>(colName, fits_col_meta_data ) );


          m_rowLength += repeat * width;

         }

         //ReadAllKeywords();
      }

   else if (strcmp(mode, "CREATE") == 0 || strcmp(mode, "APPEND") == 0)
      {
      fits_create_file(&m_fitsFile, filename.c_str(), &status);
      if (status == 105)
      {
         struct stat buf;
         if (stat(filename.c_str(), &buf) == 0) {
            // the files exist already
            if (strcmp(mode, "CREATE") == 0)
               // the file must not exist already
               throw runtime_error("File " + filename + " exist already.");

         // in APPEND mode we try to open it to be able to create a new extension
         status = 0;
         fits_open_file(&m_fitsFile, filename.c_str(), READWRITE, &status);
         if (status != 0)
            // file exist, but we cannot open it
            throw runtime_error("File " + filename +
                                " cannot be opened to append a new HDU,"
                                " cfitsio error: " + to_string(status));
         }
         else
            throw runtime_error("Cannot create file " + filename +
                                ". Does its directory exist already?"
                                " cfitsio error: 105");

      }

      char * noName [0];
      fits_create_tbl(m_fitsFile, BINARY_TBL, 0, 0, noName, noName, NULL, NULL, &status);
      if (status != 0)
         throw runtime_error("Failed to create a table in " + filename +
                         ", cfitsio error: " + to_string(status));

      m_update = true;
      }
   else
        throw runtime_error("To open / create the file " + filename + " a not supported mode was defined: " +
                          string(mode) + ". Supported modes are READONLY, CREATE and APPEND" );

}

///////////////////////////////////////////////////////////////////////////////
FitsDalTable::~FitsDalTable()
{
   if (!m_update)
      // nothing to do
      return;

  // m_nextWriteRow - 1 is the number of rows the table should have. For performance
  // optimization some more rows may be inserted in the Write() function.
  // The real number of rows in the table is tableLength. Here, the additional
  // rows are deleted.
  int status = 0;
  long tableLength;
  fits_get_num_rows(m_fitsFile, &tableLength, &status);

  if (tableLength > m_numWrittenRows)
     {
     status = fits_delete_rows(m_fitsFile, m_numWrittenRows + 1, tableLength - m_numWrittenRows, &status);
     if (status != 0)
        throw runtime_error("Failed to delete surplus rows of table " + GetFileName() +
                           " before closing the table. cfitsio error: " + to_string(status));
     }
}


///////////////////////////////////////////////////////////////////////////////
template <typename T>
void FitsDalTable::Assign(const std::string & colName, T * varPointer, int binSize,
                          const std::string & comment, const std::string & unit,
                          T * nullValue)
{

   if (m_update)
      {
      // we will create the new column
      int status = 0;
      int colNum; // column number of the new column == number of existing columns + 1
      fits_get_num_cols(m_fitsFile, &colNum, &status);
      colNum += 1;
      char strColName[colName.length() + 1];
      strcpy(strColName, colName.c_str());

      char typeId = TypeId<T>();
      if (typeId == 'W')
         {
         // unsigned long is not implemented in cfitsio, we do it by
         // ourself
         // set the TZERO and TSCALE keyword
         char card[100];
         sprintf(card, "TZERO%d    ", colNum);
         strcpy(card+8, "=  9223372036854775808 / offset for unsigned integers                    ");
         fits_write_record(m_fitsFile, card, &status);

         sprintf(card, "TSCAL%d    ", colNum);
         strcpy(card+8, "=                    1 / data are not scaled                             ");
         fits_write_record(m_fitsFile, card, &status);

         // make cfitsio believe that we have a long (K) column
         typeId = 'K';

         }


      char sizeType[20]; // defines the data type and the bin size of the new column
      sprintf(sizeType, "%d%c", binSize, typeId);
      fits_insert_col(m_fitsFile, colNum, strColName, sizeType, &status);
      if (status != 0)
         throw runtime_error("Failed to insert column " + colName + " into table " +
                              GetFileName() + ", cfitsio error: " + to_string(status));

      // set the comment of the column
      if (comment.length() > 0)
         {
         char keyName[10];
         sprintf(keyName, "TTYPE%d", colNum);

         char strComment[comment.length() + 1];
         strcpy(strComment, comment.c_str());

         fits_modify_comment(m_fitsFile, keyName, strComment, &status);
         if (status != 0)
            throw runtime_error("Failed to set the comment value = '" + comment + "' (keyword " + keyName + ") into table " +
                                GetFileName().c_str() + ", cfitsio error: " + to_string(status));

         }

      // set the unit of the column
      if (unit.length() > 0)
         {
         char keyName [10];
         sprintf(keyName, "TUNIT%d", colNum);
         char comment [80];
         sprintf(comment, "Unit of column %s", colName.c_str());
         char strUnit[100];
         strcpy(strUnit, unit.c_str());

         fits_write_key(m_fitsFile, TSTRING, keyName, strUnit, comment, &status);
         if (status != 0)
               throw runtime_error("Failed to write the unit value =  '" + unit + "' (keyword " + keyName  + ") into table " +
                                    GetFileName().c_str() + ", cfitsio error: " + to_string(status));
         }

      // set the TNULL value if it is defined and if it is an integer column
      if (nullValue  && DataTypeIndex<T>() >= 1 && DataTypeIndex<T>() <= 8) {
         char keyName [10];
         sprintf(keyName, "TNULL%d", colNum);
         char comment [80];
         sprintf(comment, "NULL value of column %s", colName.c_str());

         if (DataTypeIndex<T>() == 1) {
            // signed byte
            uint8_t buffer;
            memcpy(&buffer, nullValue, 1);
            buffer ^= 0x80;
            SetAttr(keyName, buffer, comment );
         }
         else if (DataTypeIndex<T>() == 4) {
            // unsigned short
            int16_t buffer;
            memcpy(&buffer, nullValue, 2);
            buffer ^= 0x8000;
            SetAttr(keyName, buffer, comment );
         }
         else if (DataTypeIndex<T>() == 6) {
            // unsigned int
            int32_t buffer;
            memcpy(&buffer, nullValue, 4);
            buffer ^= 0x80000000;
            SetAttr(keyName, buffer, comment );
         }
         else if (DataTypeIndex<T>() == 8) {
            // unsigned long
            int64_t buffer;
            memcpy(&buffer, nullValue, 8);
            buffer ^= 0x8000000000000000;
            SetAttr(keyName, buffer, comment );
         }
         else {
            SetAttr(keyName, *nullValue, comment);
         }
      }

      // add the entry of this column in m_colCopy;

      // get the dataType - index in the RdCopy and WrCopy array.
      int dataTypeIndex = DataTypeIndex<T>();
      int fitsSize      = FitsSize<T>();

      ColCopy  colCopy(colName, RdCopy[dataTypeIndex][dataTypeIndex],
                       WrCopy[dataTypeIndex][dataTypeIndex],
                       m_rowLength, varPointer, binSize);
      m_colCopy.push_back(colCopy);

      // create an entry in m_fitsColMetaData. It will be used by the
      // GetFitsColMetaData() method.
      FitsColMetaDataIntern fits_col_meta_data(colNum, unit, dataTypeIndex, binSize, m_rowLength);
      m_fitsColMetaData.insert( pair<string, FitsColMetaDataIntern>(colName, fits_col_meta_data ) );

      m_rowLength += fitsSize * binSize;
      }
   else
      // assign to a already existing column
      {
      map<string, FitsColMetaDataIntern>::iterator i_fitsColMetaData =
            m_fitsColMetaData.find(colName);
      if (i_fitsColMetaData != m_fitsColMetaData.end())
         {
         int userDataTypeIndex = DataTypeIndex<T>();
         int fitsDataTypeIndex = i_fitsColMetaData->second.m_dataTypeIndex;

         if ( i_fitsColMetaData->second.m_arraySize < binSize )
            binSize = i_fitsColMetaData->second.m_arraySize;

         ColCopy colCopy(colName, RdCopy[fitsDataTypeIndex][userDataTypeIndex],
                         WrCopy[userDataTypeIndex][fitsDataTypeIndex],
                         i_fitsColMetaData->second.m_offset,
                         varPointer, binSize);
         m_colCopy.push_back(colCopy);

         // read the TNULL value if the use wants it and if it is an integer column
         if (nullValue  && DataTypeIndex<T>() >= 1 && DataTypeIndex<T>() <= 8)
            {
            char keyName [10];
            sprintf(keyName, "TNULL%hd", i_fitsColMetaData->second.m_colNum);

            try {
                if (DataTypeIndex<T>() == 1) {
                   // signed byte
                   uint8_t buffer = GetAttr<uint8_t>(keyName);
                   buffer ^= 0x80;
                   memcpy(nullValue, &buffer, 1);
                }
                else if (DataTypeIndex<T>() == 4) {
                   // unsigned short
                   int16_t buffer = GetAttr<int16_t>(keyName);
                   buffer ^= 0x8000;
                   memcpy(nullValue, &buffer, 2);
                }
                else if (DataTypeIndex<T>() == 6) {
                   // unsigned int
                   int32_t buffer = GetAttr<int32_t>(keyName);
                   buffer ^= 0x80000000;
                   memcpy(nullValue, &buffer, 4);
               }
                else if (DataTypeIndex<T>() == 8) {
                   // unsigned long
                   int64_t buffer = GetAttr<int64_t>(keyName);
                   buffer ^= 0x8000000000000000;
                   memcpy(nullValue, &buffer, 8);
               }
                else {
                   *nullValue = GetAttr<T>(keyName);
                }
            }
            catch (runtime_error) {
               // there is most probably no NULL value defined, we do not
               // update the nullVallue.
            }
            }

         }
      else {
         // this column does not exist in the FITS files
         // we set the variable to a default value
         if (DataTypeIndex<T>() == DataTypeIndex<string>()) {
            *varPointer = T();
         }
         else {
            for (int32_t loop = 0; loop < binSize; loop++) {
               varPointer[loop] = T();
            }
         }
      }

      }
}

///////////////////////////////////////////////////////////////////////////////
void FitsDalTable::ReAssign(const std::string & colName, void * varPointer) {

   std::list<ColCopy>::iterator i_colCopy = m_colCopy.begin();
   while (i_colCopy != m_colCopy.end()) {
      if (i_colCopy->m_colName == colName) {
         i_colCopy->m_variable = varPointer;
         break;
      }

      ++i_colCopy;
   }
}

///////////////////////////////////////////////////////////////////////////////
std::list<FitsColMetaData> FitsDalTable::GetFitsColMetaData() {

   std::list<FitsColMetaData> fitsColMetaData;

   std::map<std::string, FitsColMetaDataIntern>::iterator i_colMD =
         m_fitsColMetaData.begin();
   while (i_colMD != m_fitsColMetaData.end()) {
      fitsColMetaData.push_back(FitsColMetaData(i_colMD->first,
                                                static_cast<ColDataType>(i_colMD->second.m_dataTypeIndex),
                                                i_colMD->second.m_arraySize));

      i_colMD++;
   }

   return fitsColMetaData;

}

///////////////////////////////////////////////////////////////////////////////
std::string FitsDalTable::GetColUnit(const std::string & colName) {

   std::map<std::string, FitsColMetaDataIntern>::iterator i_colMD =
         m_fitsColMetaData.find(colName);

   if (i_colMD == m_fitsColMetaData.end())
      // the column does not exist, we return an empty string
      return string();

   return i_colMD->second.m_unit;

}

///////////////////////////////////////////////////////////////////////////////
void FitsDalTable::PrepareWriteRow(uint64_t row) {

   if (row == 0)
      row = 1;

   if (row > (uint64_t)m_numWrittenRows) {
      /// user anyhow wants to add a new row at the end of the table.
      /// nothing special to be done.
      m_nextWriteRow = m_numWrittenRows + 1;
      return;
   }

   // read the already available data from the FITS table back to the
   // memory.
   ReadRow(row);

   // next time we want to read from this row.
   m_nextWriteRow = row;

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Writes one new row to the table
///
/// Writes one new row of all previously assigned columns at the end of the
/// table or to a by PrepareWriteRow() specified row.
///
/// @todo Not assigned columns are filled with 0. This may be a problem
/// for string columns.
void FitsDalTable::WriteRow()
{
   int status = 0;

   long tableLength; // number of rows in current table. This may me
                     // more than already written to, as we add several
                     // rows in one go to improve the performance.
   fits_get_num_rows(m_fitsFile, &tableLength, &status);

   // add rows if we would write behind the end of the table
   if (m_nextWriteRow > tableLength)
      {
      long bestNumRows; // optimal number of rows to read and write in one step
      fits_get_rowsize(m_fitsFile, &bestNumRows, &status);
      // use only half of bestNumRows to let some buffers for other tables.
      // and add at most 5 times the current number of rows.
      if (m_nextWriteRow  * 5 < bestNumRows / 2 )
         bestNumRows = m_nextWriteRow * 5 * 2;

      fits_insert_rows(m_fitsFile, tableLength, bestNumRows / 2, &status);
      if (status != 0)
         throw runtime_error("Failed to add " + to_string(bestNumRows / 2) +
                             " rows to the end of table " +  GetFileName() +
                             ". cfitsio error: " + to_string(status) );
      }



   // fill the ioBuffer with the values to be written to the table
   unsigned char * ioBuffer = (unsigned char*) malloc(m_rowLength);
   memset(ioBuffer, 0, m_rowLength);

   std::list<ColCopy>::iterator i_colCopy = m_colCopy.begin();
   while (i_colCopy != m_colCopy.end())
      {
      i_colCopy->WriteFct(ioBuffer + i_colCopy->m_colOffset,
                          i_colCopy->m_variable,
                          i_colCopy->m_arraySize);
      ++i_colCopy;
      }

   // we are prepared and write all data to the table.
   fits_write_tblbytes(m_fitsFile, m_nextWriteRow, 1, m_rowLength,
                       ioBuffer, &status);

   free (ioBuffer);

   if (status != 0)
      throw runtime_error("Failed to write to row " + to_string(m_nextWriteRow) +
                          " in table " +  GetFileName() +
                          ". cfitsio error: " + to_string(status) );

   if (m_nextWriteRow > m_numWrittenRows)
      // a new row was written
      m_numWrittenRows = m_nextWriteRow;
   else if (m_nextWriteRow < m_numWrittenRows)
      // an already existing row was updated, but this was not the last row in the table.
      // read the next row before it is updated with new values
      ReadRow(m_nextWriteRow + 1);

   ++m_nextWriteRow;
}
///////////////////////////////////////////////////////////////////////////////
/// @brief Reads one row from all recognised columns.
///
/// @param [in] row  Reads data of row number @b row. First row in the table
///                  is @b row == 1. If @b row is set to 0 (the
///                  default value) data of the next row are read.
///
/// @return false if the "read pointer" points behind the end of the table and
///               no further row can be read. (Or in case of an error)\n
///         true  if a row could be read
bool FitsDalTable::ReadRow(uint64_t row)
{
   int status = 0;

   if (row != 0) {
      m_nextReadRow = row;
   }

   if (m_update) {
      // table was just created, nevertheless rows are read to be able
      // to update columns
      if (m_nextReadRow > m_numWrittenRows)
         // the row the user wants to read was never written
         return false;
   }
   else {
      long tableLength; // number of rows in current table.
      fits_get_num_rows(m_fitsFile, &tableLength, &status);

      if (m_nextReadRow > tableLength)
         // we look after the last row
         return false;
   }

   // read one row
   unsigned char * ioBuffer = (unsigned char*) malloc(m_rowLength);

   fits_read_tblbytes(m_fitsFile, m_nextReadRow, 1, m_rowLength,
                      ioBuffer, &status);

   if (status != 0)
      throw runtime_error("Failed to read from row " + to_string(m_nextReadRow) +
                          " in table " + GetFileName() +
                          ". cfitsio error: " + to_string(status) );

   // copy the data from the buffer into the variables
   std::list<ColCopy>::iterator i_colCopy = m_colCopy.begin();
   while (i_colCopy != m_colCopy.end())
       {
       i_colCopy->ReadFct(i_colCopy->m_variable,
                          ioBuffer + i_colCopy->m_colOffset,
                          i_colCopy->m_arraySize);
       ++i_colCopy;
       }

   free (ioBuffer);

   // read the next time the next row
   m_nextReadRow++;

   return true;
}

///////////////////////////////////////////////////////////////////////////////
bool FitsDalTable::SetReadRow(uint64_t row) {

   int status = 0;

   long tableLength; // number of rows in current table.
   fits_get_num_rows(m_fitsFile, &tableLength, &status);

   if (row == 0 ||
       row > static_cast<uint64_t>(std::numeric_limits<long>::max()) ||
       static_cast<long>(row) > tableLength)
      return false;

   m_nextReadRow = row;
   return true;

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns true if column with column number @b col is an unsigned
///        column.
///
/// An unsigned column must have a keyword TZEROnnn with value @b expectedValue
/// and a keyword TSCALnnn with value 1
template<class EV>
   bool FitsDalTable::IsUnsignedColumn(int col, EV expectedValue)
{
   char colname[80];
   sprintf(colname, "TZERO%d", col);

   EV fitsValue;
   try {
      fitsValue =  EV(round(GetAttr<double>(colname, NULL, NULL)));
   }
   catch (runtime_error & e) {
      return false;
   }

   if (fitsValue != expectedValue)
      return false;

   int32_t fits_tscal;
   sprintf(colname, "TSCAL%d", col);
   try {
      fits_tscal =  int32_t(round(GetAttr<double>(colname, NULL, NULL)));
   }
   catch (runtime_error & e) {
      // we accept if TSCAL is not defined
      return true;
   }

   if (fits_tscal != 1)
      return false;

   return true;
}


// explicit instantiation
template void FitsDalTable::Assign<string>   (const string & colName, string *   varPointer,  int binSize, const string & comment, const string & unit, string   * nullValue);
template void FitsDalTable::Assign<bool>     (const string & colName, bool *     varPointer,  int binSize, const string & comment, const string & unit, bool     * nullValue);
template void FitsDalTable::Assign<int8_t>   (const string & colName, int8_t *   varPointer,  int binSize, const string & comment, const string & unit, int8_t   * nullValue);
template void FitsDalTable::Assign<uint8_t>  (const string & colName, uint8_t *  varPointer,  int binSize, const string & comment, const string & unit, uint8_t  * nullValue);
template void FitsDalTable::Assign<int16_t>  (const string & colName, int16_t *  varPointer,  int binSize, const string & comment, const string & unit, int16_t  * nullValue);
template void FitsDalTable::Assign<uint16_t> (const string & colName, uint16_t * varPointer,  int binSize, const string & comment, const string & unit, uint16_t * nullValue);
template void FitsDalTable::Assign<int32_t>  (const string & colName, int32_t *  varPointer,  int binSize, const string & comment, const string & unit, int32_t  * nullValue);
template void FitsDalTable::Assign<uint32_t> (const string & colName, uint32_t * varPointer,  int binSize, const string & comment, const string & unit, uint32_t * nullValue);
template void FitsDalTable::Assign<int64_t>  (const string & colName, int64_t *  varPointer,  int binSize, const string & comment, const string & unit, int64_t  * nullValue);
template void FitsDalTable::Assign<uint64_t> (const string & colName, uint64_t * varPointer,  int binSize, const string & comment, const string & unit, uint64_t * nullValue);
template void FitsDalTable::Assign<float>    (const string & colName, float *    varPointer,  int binSize, const string & comment, const string & unit, float    * nullValue);
template void FitsDalTable::Assign<double>   (const string & colName, double *   varPointer,  int binSize, const string & comment, const string & unit, double   * nullValue);
