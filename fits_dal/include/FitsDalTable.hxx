/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief Declaration of the FitsDalTable class and two internal used
 *         structures
 *
 *  @author Reiner Rohlfs UGE
 *
 *  @version 9.3.1 2018-06-18 RRO #16505: new method: SetReadRow()
 *  @version 9.0.1 2018-01-31 RRO #15376: update of method IsUnsignedColumn()
 *  @version 9.0   2018-01-04 RRO #15057: new method: GetColUnit and update of
 *                                        FitsColMetaDataIntern with the unit of
 *                                        the columns.
 *  @version 5.2   2016-05-05 RRO #10580: implement random access of rows
 *  @version 4.3   2015-10-17 RRO #9378: New method: GetFitsColMetaData()
 *  @version 4.1   2015-07-28 RRO #8916: New method: ReAssign()
 *  @version 3.0   2014-12-17 RRO #7054: Support NULL values of columns
 *  @version 1.0 first released version
 *
 */

#ifndef _FITS_DAL_TABLE_HXX_
#define _FITS_DAL_TABLE_HXX_

#include <list>
#include <map>
#include <string>

#include "FitsDalHeader.hxx"

#ifndef TULONGLONG
/** ****************************************************************************
 *  As long as cfitsio does not support unsigned long long we define
 *  TULONGTULONG by ourself.
 */
#define TULONGLONG 80
#endif


/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Structure to store information needed to copy data of one column
 *         from the read/write - butter into its variable and visa versa.
 */
struct ColCopy
{
   /** ****************************************************************************
    *  @brief The only constructor, initializing all member variables.
    */
   ColCopy(const std::string & colName, void (*readFct)(void *, void *, int),
               void (*writeFct)(void *, void *, int),
               int colOffset, void * variable, int arraySize)
      : m_colName(colName),  ReadFct(readFct), WriteFct(writeFct),
        m_colOffset(colOffset), m_variable(variable), m_arraySize(arraySize) { }

   std::string m_colName;                   ///< name of the column
   void  (*ReadFct) (void *, void *, int);  ///< The read function used for this column
   void  (*WriteFct) (void *, void *, int); ///< The write function used for this column
   int   m_colOffset;   ///< Offset in bytes in the read/write buffer
   void  * m_variable;  ///< Address of the associated variable
   int   m_arraySize;   ///< @brief number of bins in a vector column,
                        ///  1: normal column (no vector column)

};


/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Structure to store information of existing columns in the open
 *         FITS table.
 *
 *  This Structure is used only internally in FitsDalTable.
 *  This structure is used only in READONLY mode. It is filled while the FITS
 *  table is opened with meta data of all the columns of the table. It is used
 *  in the FitsDalTable::Assign function, if the user wants to set
 *  a variable to an already existing column.
 */
struct FitsColMetaDataIntern
{
   /** ****************************************************************************
    *  @brief The only constructor, initializing all member variables.
    */
   FitsColMetaDataIntern(int16_t colNum, const std::string unit,
                         int16_t dataTypeIndex, long arraySize, int offset)
      : m_colNum(colNum), m_unit(unit),  m_dataTypeIndex(dataTypeIndex) ,
        m_arraySize(arraySize),  m_offset(offset) { }

   int16_t     m_colNum;        ///< @brief column number in table, first column = 1
   std::string m_unit;          ///< @brief unit if the column or empty string.
   int16_t     m_dataTypeIndex; ///< @brief index of the data type in the WrCopy and
                                ///         RdCopy arrays.
   int32_t     m_arraySize;     ///< @brief number of bins in a vector column,
                                ///         1: normal column (no vector column)
   int32_t     m_offset;        ///< @brief offset in bytes of this column in the
                                ///         FITS table row.

};




/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Defines the data type of FITS columns.
 *
 *  It is used by the FitsColMetaData struct.
 */
enum ColDataType {
   col_bool      = 0,
   col_int8      = 1,
   col_uint8     = 2,
   col_int16     = 3,
   col_uint16    = 4,
   col_int32     = 5,
   col_uint32    = 6,
   col_int64     = 7,
   col_uint64    = 8,
   col_float     = 9,
   col_double    = 10,
   col_string    = 11
};

/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Structure to store information of existing columns in the open
 *         FITS table.
 *
 *  A list of this struct is return by the method GetFitsColMetaData();
 */
struct FitsColMetaData {

   /** *************************************************************************
    *  @brief The only constructor, initializing all member variables.
    */
   FitsColMetaData(const std::string & colName, ColDataType colDataType,
                   int32_t arraySize)
      : m_colName(colName), m_colDataType(colDataType),
        m_arraySize(arraySize) { }

   std::string    m_colName;     ///<  name of the column
   ColDataType    m_colDataType; ///<  data type of the column
   int32_t        m_arraySize;   ///<  @brief number of bins in a vector column,
                                 ///   or length of a string column
                                 ///   1: normal column (no vector column)
};

/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief Representing a FITS table with access functions to the data of its
 *         columns.
 */
class FitsDalTable : public FitsDalHeader
{

public:
   /** ****************************************************************************
    *  @brief Opens / creates a FITS table on disk
    */
	FitsDalTable(const std::string & fileName, const char *mode = "READONLY");


   /** ****************************************************************************
    *  @brief Closes the table
    */
	~FitsDalTable();

  /** **************************************************************************
   * @brief Returns the class name as a string.
   *
   * @return class name
   */
  static std::string getClassName() { return "FitsDalTable"; }

   /** ****************************************************************************
    *  @brief Assigns one application variable with a column in the FITS table
    *
    *  The value of the application variable @b varPointer will be written into
    *  the next row of the table by calling the WriteRow() method. The application variable
    *  @b varPointer will be filled with the value(s) of the cell in the next
    *  row in the FITS by calling ReadRow().
    *
    *  If the table was opened in READONLY mode, only already existing columns
    *  can be assigned to an application variable. If the column name defined
    *  by @b colName does not exist, the function will return silently and the
    *  variable @b varPointer is not used. It is implemented like this to be
    *  able to open an old or a new version of the FITS table that does not
    *  have the column any more or not yet.\n
    *  If the table was opened in
    *  CREATE or UPDATE mode a new column after the last columns is created in
    *  the FTIS table by this method.
    *
    *  @param [in] colName    name of the column in the FITS table to which the
    *                         @b varPointer is assigned to.
    *  @param [in] varPointer address of the application variable. It has be an
    *                         array if @b binSize > 1. The array has to be at
    *                         least of size @b  binSize.
    *                         If in READONLY mode binSize is less than the number
    *                         of bins in a FITS vector column only @b binSize
    *                         values are copied into this array.
    *  @param [in] binSize    size of the array of @b varPointer and size of
    *                         one bin in the new columns if this function is
    *                         used to create a new column.
    *  @param [in] comment    If the table was opened in UPDATE or CREATE mode
    *                         the text is written as value to the TTYPExxx
    *                         keyword. It has no affect if the table was
    *                         opened in READONLY mode.
    *  @param [in] unit       If the table was opened in UPDATE or CREATE mode
    *                         the text is written as value to the TUNITxxx
    *                         keyword. It has no affect if the table was
    *                         opened in READONLY mode.
    *  @param [in] nullValue  If it possible to set the pointer to nullptr
    *                         (the default value) to not use the NULL value.\n
    *                         In READONLY mode the value is updated with the
    *                         value of the TNULLxxx header keyword.
    *                         The value is not modified if the TNULLxxx
    *                         keyword does not exist.\n
    *                         In UPDATE or CREATE mode a TNULLxxx keyword,
    *                         is created and the value is set in this keyword.
    *
    */
	template <typename T>
	void Assign(const std::string & colName, T * varPointer, int binSize = 1,
	            const std::string & comment = std::string(),
	            const std::string & unit    = std::string(),
	            T * nullValue = nullptr);

	/** ****************************************************************************
	 *  @brief Re-assigns a column to an new variable
	 *
	 *  This method overwrites the @b varPointer of a previous call of Assign()
	 *  or ReAssign() method.\n
	 *  The data type of the variable has to be the same as the before assigned
	 *  variable to the column. Also the array size has to be the same as the
	 *  before assigned variable.\n
	 *  The method does nothing if the column was not assigned to a variable
	 *  before calling this method.
	 *
	 *  @param [in] colName     The column with name @b colName will be
	 *                          re-assigned to a new variable.
	 *  @param [in] varPointer  pointer to a variable to which the column will
	 *                          read and write its value at the next
	 */
	void ReAssign(const std::string & colName, void * varPointer);

   /** ****************************************************************************
    *  @brief Returns a list of MetaData of all columns in a  FITS table.
    *
    */
	std::list<FitsColMetaData> GetFitsColMetaData();

   /** ****************************************************************************
    *  @brief Returns the unit of a column.
    *
    *  Returns an empty string if the column of name @b colName does not exist,
    *  or if the unit is not defined.
    */
   std::string GetColUnit(const std::string & colName);


   /** ****************************************************************************
    *  @brief Prepares a Row that exist already to be updated at the next call of
    *         WriteRow()
    *
    *  This method has to be called in case some columns should be updated in
    *  a row that was already written. Important: This method has to be called
    *  before any value of the columns is updated, for example with the secCellX
    *  methods of a fits_data_model class. Once this method is called then the next
    *  calls of WriteRow() will not add a new row to the end of the table but will
    *  update the row, defined by the parameter @b row.
    *  Following calls of WriteRow() will write to the next row, independent if
    *  it exist already or not. That means this function has to be called only
    *  once with PrePareWriteRow(1) to update all rows with consecutive calls of
    *  WriteRow();
    *
    *  @param [in] row      At the next call of WriteRow() the @b row will be
    *                       updated instead of creating a new row. If @b row
    *                       is larger than the last row a new row will be added
    *                       at the next call of WriteRow(). Note: first row = 1!
    */
	void PrepareWriteRow(uint64_t row);

   /** ****************************************************************************
    *  @brief Creates a new row at the end of the table and fills it with the
    *         values of the assigned variables.
    *
    *  If PrepareWriteRow() was called before not a new row will be filled but
    *  the by PrepareWriteRow() specified row will be updated.
    */
	void WriteRow();

   /** ****************************************************************************
    *  @brief Reads the next row from the FITS table and copies the data into the
    *         assigned variables.
    */
	bool ReadRow(uint64_t row = 0);


   /** ****************************************************************************
    *  @brief Sets the row of the FITS table that will be read with the next
    *         call of ReadRow(0);
    *
    * @param [in] row  Row number of next row to be read by ReadRow(0).
    *                  First row in the table is @b row == 1.
    *
    *  @return false if the @b row is == 0 or if the number of rows in the
    *                table is less than @b row.\n
    *           true if the row number @b row can be read.
    *
    */
	bool SetReadRow(uint64_t row);

private:

      /** *************************************************************************
       *  @brief Returns true if column with column number @b col is an unsigned
       *         column.
       */
	   template<class EV>
	      bool IsUnsignedColumn(int col, EV expectedValue);

	   ///
	   long   m_numWrittenRows; ///< number of rows already written
	   long   m_nextWriteRow;   ///< this row will be filled at the next Write() call. First row = 1
	   long   m_nextReadRow;    ///< this row will be read at the next Read() call. First row = 1

	   int    m_rowLength;      ///< number of bytes of one row

	   /// copy information about all columns, used during the Read() and Write() functions
	   std::list<ColCopy> m_colCopy;

	   /// Meta data of all columns, found while the table was opened in READONLY mode
	   std::map<std::string, FitsColMetaDataIntern>  m_fitsColMetaData;



};

/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief A 2 dim array of functions, which can be used to copy the data
 *         of a provided columns variable into a FITS column
 *
 */
extern void (*WrCopy[12][12])   (void *, void *, int);

/** ****************************************************************************
 *  @ingroup FitsDal
 *  @author Reiner Rohlfs UGE
 *
 *  @brief A 2 dim array of functions, which can be used to copy the data
 *         from a FITS columns into a provided columns variable.
 *
 */
extern void (*RdCopy[12][12])   (void *, void *, int);

#endif /* _FITS_DAL_TABLE_HXX_ */
