/** ****************************************************************************
 *  @file
 *
 *  @ingroup FitsDal
 *  @brief  all small functions to convert data types of FITS columns
 *
 *
 *   @author Reiner Rohlfs ISDC
 *
 *  @version 3.0   2014-12-18 RRO #7054: Support NULL values of columns
 *  @version 1.0   2018-07-23 RRO first version
 *
*/
#include <stdexcept>
#include <iostream>
using namespace std;


#include <float.h>
#include <string.h>

#include "fitsio2.h"

inline void Swap1(void * dest, void * src)
{
   *((char*)dest)     = *(((char*)src));
}

// swap unsigned data. i.e. flip the first bit
inline void Swap1U(void * dest, void * src)
{
   *((char*)dest)     = *(((char*)src));
   *((char*)dest) ^= 0x80;
}

//#ifdef WORDS_BIGENDIAN
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

// we do not need to distinguisch between bite switch for reading and          
// writing. We map the two funcitons names to one function name                
#define Swap2RU  Swap2U
#define Swap2WU  Swap2U

#define Swap4RU  Swap8U
#define Swap4WU  Swap8U

#define Swap8RU  Swap8U
#define Swap8WU  Swap8U

// Note: We have to make a memcpy instaead of a simple assign, because the     
// source or the destination may have uneven start address from the            
// byte position in the fits table.                                            
inline void Swap2(void * dest, void * src)
{
   memcpy(dest, src, 2);
}


inline void Swap2U(void * dest, void * src)
{
   memcpy(dest, src, 2);
   *((char*)dest) ^= 0x80;
}

inline void Swap4(void * dest, void * src)
{
   memcpy(dest, src, 4);
}

inline void Swap4U(void * dest, void * src)
{
   memcpy(dest, src, 4);
   *((char*)dest) ^= 0x80;
}

inline void Swap8(void * dest, void * src)
{
   memcpy(dest, src, 8);
}

inline void Swap8U(void * dest, void * src)
{
   memcpy(dest, src, 8);
   *((char*)dest) ^= 0x80;
}

#else        // of WORDS_BIGENDIAN

inline void Swap2(void * dest, void * src)
{
   *((char*)dest)     = *(((char*)src)+1);
   *(((char*)dest)+1) = *((char*)src);
}

/// Swap a two byte data and swith the first bit of src, which was read from   
/// a FITS unsigned data column                                                
inline void Swap2RU(void * dest, void * src)
{
   *((char*)src) ^= 0x80;
   *((char*)dest)     = *(((char*)src)+1);
   *(((char*)dest)+1) = *((char*)src);
}

/// Swap a two byte data and swith the first bit of sest, which will be        
/// written to a FITS unsigned data column                                     
inline void Swap2WU(void * dest, void * src)
{
   *((char*)dest)     = *(((char*)src)+1);
   *(((char*)dest)+1) = *((char*)src);
   *((char*)dest) ^= 0x80;
}

///////////////////////////////////////////////////////////////////////////////

inline void Swap4(void * dest, void * src)
{
   *((char*)dest)     = *(((char*)src)+3);
   *(((char*)dest)+1) = *(((char*)src)+2);
   *(((char*)dest)+2) = *(((char*)src)+1);
   *(((char*)dest)+3) = *((char*)src);
}

/// Swap a four byte data and swith the first bit of src, which was read from  
/// a FITS unsigned data column                                                
inline void Swap4RU(void * dest, void * src)
{
   *((char*)src) ^= 0x80;
   *((char*)dest)     = *(((char*)src)+3);
   *(((char*)dest)+1) = *(((char*)src)+2);
   *(((char*)dest)+2) = *(((char*)src)+1);
   *(((char*)dest)+3) = *((char*)src);
}

/// Swap a four byte data and swith the first bit of sest, which will be       
/// written to a FITS unsigned data column                                     
inline void Swap4WU(void * dest, void * src)
{
   *((char*)dest)     = *(((char*)src)+3);
   *(((char*)dest)+1) = *(((char*)src)+2);
   *(((char*)dest)+2) = *(((char*)src)+1);
   *(((char*)dest)+3) = *((char*)src);
   *((char*)dest) ^= 0x80;
}

///////////////////////////////////////////////////////////////////////////////

inline void Swap8(void * dest, void * src)
{
   *((char*)dest)    = *(((char*)src)+7);
   *(((char*)dest)+1) = *(((char*)src)+6);
   *(((char*)dest)+2) = *(((char*)src)+5);
   *(((char*)dest)+3) = *(((char*)src)+4);
   *(((char*)dest)+4) = *(((char*)src)+3);
   *(((char*)dest)+5) = *(((char*)src)+2);
   *(((char*)dest)+6) = *(((char*)src)+1);
   *(((char*)dest)+7) = *((char*)src);
}

/// Swap a eight byte data and swith the first bit of src, which was read from 
/// a FITS unsigned data column                                                
inline void Swap8RU(void * dest, void * src)
{
   *((char*)src) ^= 0x80;
   *((char*)dest)    = *(((char*)src)+7);
   *(((char*)dest)+1) = *(((char*)src)+6);
   *(((char*)dest)+2) = *(((char*)src)+5);
   *(((char*)dest)+3) = *(((char*)src)+4);
   *(((char*)dest)+4) = *(((char*)src)+3);
   *(((char*)dest)+5) = *(((char*)src)+2);
   *(((char*)dest)+6) = *(((char*)src)+1);
   *(((char*)dest)+7) = *((char*)src);
}

/// Swap a eight byte data and swith the first bit of sest, which will be      
/// written to a FITS unsigned data column                                     
inline void Swap8WU(void * dest, void * src)
{
   *((char*)dest)    = *(((char*)src)+7);
   *(((char*)dest)+1) = *(((char*)src)+6);
   *(((char*)dest)+2) = *(((char*)src)+5);
   *(((char*)dest)+3) = *(((char*)src)+4);
   *(((char*)dest)+4) = *(((char*)src)+3);
   *(((char*)dest)+5) = *(((char*)src)+2);
   *(((char*)dest)+6) = *(((char*)src)+1);
   *(((char*)dest)+7) = *((char*)src);
   *((char*)dest) ^= 0x80;
}

#endif   //  WORDS_BIGENDIAN

template <class T>
bool L_0(T value)    {return value < 0;}

template <class T>
bool L_I8(T value)   {return value < CHAR_MIN;}

template <class T>
bool L_I16(T value)  {return value < SHRT_MIN;}

template <class T>
bool L_I32(T value)  {return value < INT32_MIN;}

template <class T>
bool L_I64(T value)  {return value < LONGLONG_MIN;}

template <class T>
bool L_F(T value)    {return value < -FLT_MAX;}


template <class T>
bool H_I8(T value)    {return value > CHAR_MAX;}

template <class T>
bool H_U8(T value)    {return value > UCHAR_MAX;}

template <class T>
bool H_I16(T value)   {return value > SHRT_MAX;}

template <class T>
bool H_U16(T value)   {return value > USHRT_MAX;}

template <class T>
bool H_I32(T value)   {return value > INT32_MAX;}

template <class T>
bool H_U32(T value)   {return value > UINT32_MAX;}

template <class T>
bool H_I64(T value)   {return value > LONGLONG_MAX;}

template <class T>
bool H_U64(T value)   {return value > ULLONG_MAX;}

template <class T>
bool H_F(T value)     {return value > FLT_MAX;}


///////////////////////////////////////////////////////////////////////////////
/// Copies variables without data conversion                                   
void CpB2B(void * dest, void * src, int arraySize)
{
   memcpy(dest, src, arraySize);
}

///////////////////////////////////////////////////////////////////////////////
/// Reads string and removes blanks at the end of the string                   
void RdA2A(void * dest, void * src, int arraySize)
{
   char tmpDest[arraySize+1];
   memcpy(tmpDest, src, arraySize);

   char * end = (char*)tmpDest;
   end += arraySize;
   do {
      *end = 0;
      end--;
      }
   while(end != tmpDest && *end == ' ');
   *((std::string*)dest) = tmpDest;
}

///////////////////////////////////////////////////////////////////////////////
/// Writes string and includes spaces ' ' at the and after the terminating 0
void WrA2A(void * dest, void * src, int arraySize)
{
   const char * src0 = ((std::string*)src)->c_str();
   int   srcLength = ((std::string*)src)->length();

   memcpy(dest, src0, arraySize < srcLength ? arraySize : srcLength);
   if (srcLength < arraySize)
      memset((char*)dest + srcLength, ' ', arraySize - srcLength);

   if (srcLength == 0)  // this is the NULL value of a string
      memset(dest, 0, 1);
}

///////////////////////////////////////////////////////////////////////////////
/// Logical data in FITS files are not standard.                               
/// 0 or 'F' means false, 'T' should be true, everything else is also true     

template <class DEST>
void RdL2Y(void * dest, void * src, int arraySize)
{
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      if ( ((char*)src)[numVal] == 0 ||
           ((char*)src)[numVal] == 'F'   )
         ((DEST*)dest)[numVal] = 0;   
      else
         ((DEST*)dest)[numVal] = 1;
      }   
}

template <class SRC, void (*swap)(void *, void*)>
void RdX2L(void * dest, void * src, int arraySize)
{
   SRC tmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      swap(&tmp, ((SRC*)src)+numVal );
      if ( tmp == 0)
         ((bool*)dest)[numVal] = false;
      else
         ((bool*)dest)[numVal] = true;
      }   
}

template <class SRC>
void WrX2L(void * dest, void * src, int arraySize)
{
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      if ( ((SRC*)src)[numVal] == 0)
         ((char*)dest)[numVal] = 'F';   // means false in FITS table
      else
         ((char*)dest)[numVal] = 'T';   // means true in FITS table 
      }   
}

template <class DEST, void (*swap)(void *, void*)>
void WrL2Y(void * dest, void * src, int arraySize)
{
   DEST destTmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      if (((bool*)src)[numVal])
         destTmp = 1;
      else
         destTmp = 0;
      swap(((DEST*)dest)+numVal, &destTmp);
      }   
}

///////////////////////////////////////////////////////////////////////////////
// most general copy function for reading from FITS table without overflow     
// check                                                                       
template <class DEST, class SRC, void (*swap)(void *, void*)> 
void RdX2Y(void * dest, void * src, int arraySize)
{
   SRC tmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      swap(&tmp, ((SRC*)src)+numVal );
      *(((DEST*)dest)+numVal) = (DEST)tmp;
      }
}

template <class DEST, class SRC, bool (*lim)(SRC), void (*swap)(void *, void*)>
void RdX2Y_1Lim(void * dest, void * src, int arraySize)
{
   SRC tmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      swap(&tmp, ((SRC*)src)+numVal);
      if (lim(tmp))
         throw std::runtime_error("Overflow Error while reading a colum");
      else
         *(((DEST*)dest)+numVal) = (DEST)tmp;
      }
}

template <class DEST, class SRC, bool (*min)(SRC), bool (*max)(SRC),
          void (*swap)(void *, void*)> 
void RdX2Y_2Lim(void * dest, void * src, int arraySize)
{
   SRC tmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      swap(&tmp, ((SRC*)src)+numVal);
      if (min(tmp) || max(tmp))
         throw std::runtime_error("Overflow Error while reading a colum");
      else
         *(((DEST*)dest)+numVal) = (DEST)tmp;
      }
}

///////////////////////////////////////////////////////////////////////////////
// most general copy function for writing to FITS table without overflow check 
template <class DEST, class SRC, void (*swap)(void *, void*)> 
void WrX2Y(void * dest, void * src, int arraySize)
{
   DEST destTmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      destTmp = (DEST)*(((SRC*)src)+numVal);
      swap(((DEST*)dest)+numVal, &destTmp);
      }
}

template <class DEST, class SRC, bool (*lim)(SRC), void (*swap)(void *, void*)>
void WrX2Y_1Lim(void * dest, void * src, int arraySize)
{
   SRC srcTmp;
   DEST destTmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      srcTmp = *(((SRC*)src)+numVal);
      if (lim(srcTmp))
         throw std::runtime_error("Overflow Error while writing a colum");
      destTmp = (DEST)srcTmp;
      swap(((DEST*)dest)+numVal, &destTmp);
      }
}

template <class DEST, class SRC, bool (*min)(SRC), bool (*max)(SRC),
          void (*swap)(void *, void*)> 
void WrX2Y_2Lim(void * dest, void * src, int arraySize)
{
   SRC srcTmp;
   DEST destTmp;
   for (int numVal = 0; numVal < arraySize; numVal++)
      {
      srcTmp = *(((SRC*)src)+numVal);
      if (min(srcTmp) || max(srcTmp))
         throw std::runtime_error("Overflow Error while writing a colum");
      destTmp = (DEST)srcTmp;
      swap(((DEST*)dest)+numVal, &destTmp);
      }
}


#define RdL2L RdL2Y<bool                                               >
#define RdS2L RdX2L<char,                                         Swap1U>
#define RdB2L RdX2L<unsigned char,                                        Swap1>
#define RdI2L RdX2L<short,                                        Swap2>
#define RdU2L RdX2L<unsigned short,                                       Swap2RU>
#define RdJ2L RdX2L<int,                                          Swap4>
#define RdV2L RdX2L<unsigned int,                                         Swap4RU>
#define RdK2L RdX2L<long,                                       Swap8>
#define RdW2L RdX2L<unsigned long,                                      Swap8RU>
#define RdE2L RdX2L<float,                                        Swap4>
#define RdD2L RdX2L<double,                                       Swap8>

#define WrL2L WrX2L<bool>
#define WrS2L WrX2L<char>
#define WrB2L WrX2L<unsigned char>
#define WrI2L WrX2L<short>
#define WrU2L WrX2L<unsigned short>
#define WrJ2L WrX2L<int>
#define WrV2L WrX2L<unsigned int>
#define WrK2L WrX2L<long>
#define WrW2L WrX2L<unsigned long>
#define WrE2L WrX2L<float>
#define WrD2L WrX2L<double>

#define RdL2S  RdL2Y     <char>
#define RdS2S  RdX2Y     <char, char,                                      Swap1U>
#define RdB2S  RdX2Y_1Lim<char, unsigned char,                    H_I8<unsigned char>,   Swap1>
#define RdI2S  RdX2Y_2Lim<char, short,    L_I8<short>,  H_I8<short>,   Swap2>
#define RdU2S  RdX2Y_1Lim<char, unsigned short,                   H_I8<unsigned short>,  Swap2RU>
#define RdJ2S  RdX2Y_2Lim<char, int,      L_I8<int>,    H_I8<int>,     Swap4>
#define RdV2S  RdX2Y_1Lim<char, unsigned int,                     H_I8<unsigned int>,    Swap4RU>
#define RdK2S  RdX2Y_2Lim<char, long,   L_I8<long>, H_I8<long>,  Swap8>
#define RdW2S  RdX2Y_1Lim<char, unsigned long,                  H_I8<unsigned long>, Swap8RU>
#define RdE2S  RdX2Y_2Lim<char, float,    L_I8<float>,  H_I8<float>,   Swap4>
#define RdD2S  RdX2Y_2Lim<char, double,   L_I8<double>, H_I8<double>,  Swap8>
                                                                     
#define WrL2S  WrL2Y     <char,                                               Swap1U>
#define WrS2S  WrX2Y     <char, char,                                       Swap1U>
#define WrB2S  WrX2Y_1Lim<char, unsigned char,                    H_I8<unsigned char>,    Swap1U>
#define WrI2S  WrX2Y_2Lim<char, short,    L_I8<short>,  H_I8<short>,    Swap1U>
#define WrU2S  WrX2Y_1Lim<char, unsigned short,                   H_I8<unsigned short>,   Swap1U>
#define WrJ2S  WrX2Y_2Lim<char, int,      L_I8<int>,    H_I8<int>,      Swap1U>
#define WrV2S  WrX2Y_1Lim<char, unsigned int,                     H_I8<unsigned int>,     Swap1U>
#define WrK2S  WrX2Y_2Lim<char, long,   L_I8<long>, H_I8<long>,   Swap1U>
#define WrW2S  WrX2Y_1Lim<char, unsigned long,                  H_I8<unsigned long>,  Swap1U>
#define WrE2S  WrX2Y_2Lim<char, float,    L_I8<float>,  H_I8<float>,    Swap1U>
#define WrD2S  WrX2Y_2Lim<char, double,   L_I8<double>, H_I8<double>,   Swap1U>


#define RdL2B  RdL2Y     <unsigned char>
#define RdS2B  RdX2Y_1Lim<unsigned char,  char,     L_0<char>,                    Swap1U>
#define RdB2B  CpB2B
#define RdI2B  RdX2Y_2Lim<unsigned char,  short,    L_0<short>,  H_U8<short>,   Swap2>
#define RdU2B  RdX2Y_1Lim<unsigned char,  unsigned short,                  H_U8<unsigned short>,  Swap2RU>
#define RdJ2B  RdX2Y_2Lim<unsigned char,  int,      L_0<int>,    H_U8<int>,     Swap4>
#define RdV2B  RdX2Y_1Lim<unsigned char,  unsigned int,                    H_U8<unsigned int>,    Swap4RU>
#define RdK2B  RdX2Y_2Lim<unsigned char,  long,   L_0<long>, H_U8<long>,  Swap8>
#define RdW2B  RdX2Y_1Lim<unsigned char,  unsigned long,                 H_U8<unsigned long>, Swap8RU>
#define RdE2B  RdX2Y_2Lim<unsigned char,  float,    L_0<float>,  H_U8<float>,   Swap4>
#define RdD2B  RdX2Y_2Lim<unsigned char,  double,   L_0<double>, H_U8<double>,  Swap8>
                                    
#define WrL2B  WrL2Y     <unsigned char,                                              Swap1>
#define WrS2B  WrX2Y_1Lim<unsigned char,  char,     L_0<char>,                    Swap1>
#define WrB2B  CpB2B
#define WrI2B  WrX2Y_2Lim<unsigned char,  short,    L_0<short>,  H_U8<short>,   Swap1>
#define WrU2B  WrX2Y_1Lim<unsigned char,  unsigned short,                  H_U8<unsigned short>,  Swap1>
#define WrJ2B  WrX2Y_2Lim<unsigned char,  int,      L_0<int>,    H_U8<int>,     Swap1>
#define WrV2B  WrX2Y_1Lim<unsigned char,  unsigned int,                    H_U8<unsigned int>,    Swap1>
#define WrK2B  WrX2Y_2Lim<unsigned char,  long,   L_0<long>, H_U8<long>,  Swap1>
#define WrW2B  WrX2Y_1Lim<unsigned char,  unsigned long,                 H_U8<unsigned long>, Swap1>
#define WrE2B  WrX2Y_2Lim<unsigned char,  float,    L_0<float>,  H_U8<float>,   Swap1>
#define WrD2B  WrX2Y_2Lim<unsigned char,  double,   L_0<double>, H_U8<double>,  Swap1>


#define RdL2I  RdL2Y     <short>
#define RdS2I  RdX2Y     <short,  char,                                        Swap1U>
#define RdB2I  RdX2Y     <short,  unsigned char,                                       Swap1>
#define RdI2I  RdX2Y     <short,  short,                                       Swap2>
#define RdU2I  RdX2Y_1Lim<short,  unsigned short,                   H_I16<unsigned short>,   Swap2RU>
#define RdJ2I  RdX2Y_2Lim<short,  int,     L_I16<int>,    H_I16<int>,      Swap4>
#define RdV2I  RdX2Y_1Lim<short,  unsigned int,                     H_I16<unsigned int>,     Swap4RU>
#define RdK2I  RdX2Y_2Lim<short,  long,  L_I16<long>, H_I16<long>,   Swap8>
#define RdW2I  RdX2Y_1Lim<short,  unsigned long,                  H_I16<unsigned long>,  Swap8RU>
#define RdE2I  RdX2Y_2Lim<short,  float,   L_I16<float>,  H_I16<float>,    Swap4>
#define RdD2I  RdX2Y_2Lim<short,  double,  L_I16<double>, H_I16<double>,   Swap8>
                                                                    
#define WrL2I  WrL2Y     <short,                                                 Swap2>
#define WrS2I  WrX2Y     <short,  char,                                        Swap2>
#define WrB2I  WrX2Y     <short,  unsigned char,                                       Swap2>
#define WrI2I  WrX2Y     <short,  short,                                       Swap2>
#define WrU2I  WrX2Y_1Lim<short,  unsigned short,                   H_I16<unsigned short>,   Swap2>
#define WrJ2I  WrX2Y_2Lim<short,  int,     L_I16<int>,    H_I16<int>,      Swap2>
#define WrV2I  WrX2Y_1Lim<short,  unsigned int,                     H_I16<unsigned int>,     Swap2>
#define WrK2I  WrX2Y_2Lim<short,  long,  L_I16<long>, H_I16<long>,   Swap2>
#define WrW2I  WrX2Y_1Lim<short,  unsigned long,                  H_I16<unsigned long>,  Swap2>
#define WrE2I  WrX2Y_2Lim<short,  float,   L_I16<float>,  H_I16<float>,    Swap2>
#define WrD2I  WrX2Y_2Lim<short,  double,  L_I16<double>, H_I16<double>,   Swap2>


#define RdL2U  RdL2Y     <unsigned short>
#define RdS2U  RdX2Y_1Lim<unsigned short, char,     L_0<char>,                      Swap1U>
#define RdB2U  RdX2Y     <unsigned short, unsigned char,                                      Swap1>
#define RdI2U  RdX2Y_1Lim<unsigned short, short,    L_0<short>,                     Swap2>
#define RdU2U  RdX2Y     <unsigned short, unsigned short,                                     Swap2RU>
#define RdJ2U  RdX2Y_2Lim<unsigned short, int,      L_0<int>,    H_U16<int>,      Swap4>
#define RdV2U  RdX2Y_1Lim<unsigned short, unsigned int,                    H_U16<unsigned int>,     Swap4RU>
#define RdK2U  RdX2Y_2Lim<unsigned short, long,   L_0<long>, H_U16<long>,   Swap8>
#define RdW2U  RdX2Y_1Lim<unsigned short, unsigned long,                 H_U16<unsigned long>,  Swap8RU>
#define RdE2U  RdX2Y_2Lim<unsigned short, float,    L_0<float>,  H_U16<float>,    Swap4>
#define RdD2U  RdX2Y_2Lim<unsigned short, double,   L_0<double>, H_U16<double>,   Swap8>
                                                                     
#define WrL2U  WrL2Y     <unsigned short,                                               Swap2WU>
#define WrS2U  WrX2Y_1Lim<unsigned short, char,     L_0<char>,                      Swap2WU>
#define WrB2U  WrX2Y     <unsigned short, unsigned char,                                      Swap2WU>
#define WrI2U  WrX2Y_1Lim<unsigned short, short,    L_0<short>,                     Swap2WU>
#define WrU2U  WrX2Y     <unsigned short, unsigned short,                                     Swap2WU>
#define WrJ2U  WrX2Y_2Lim<unsigned short, int,      L_0<int>,    H_U16<int>,      Swap2WU>
#define WrV2U  WrX2Y_1Lim<unsigned short, unsigned int,                    H_U16<unsigned int>,     Swap2WU>
#define WrK2U  WrX2Y_2Lim<unsigned short, long,   L_0<long>, H_U16<long>,   Swap2WU>
#define WrW2U  WrX2Y_1Lim<unsigned short, unsigned long,                 H_U16<unsigned long>,  Swap2WU>
#define WrE2U  WrX2Y_2Lim<unsigned short, float,    L_0<float>,  H_U16<float>,    Swap2WU>
#define WrD2U  WrX2Y_2Lim<unsigned short, double,   L_0<double>, H_U16<double>,   Swap2WU>


#define RdL2J  RdL2Y     <int>
#define RdS2J  RdX2Y     <int,    char,                                       Swap1U>
#define RdB2J  RdX2Y     <int,    unsigned char,                                      Swap1>
#define RdI2J  RdX2Y     <int,    short,                                      Swap2>
#define RdU2J  RdX2Y     <int,    unsigned short,                                     Swap2RU>
#define RdJ2J  RdX2Y     <int,    int,                                        Swap4>
#define RdV2J  RdX2Y_1Lim<int,    unsigned int,                    H_I32<unsigned int>,     Swap4RU>
#define RdK2J  RdX2Y_2Lim<int,    long, L_I32<long>, H_I32<long>,   Swap8>
#define RdW2J  RdX2Y_1Lim<int,    unsigned long,                 H_I32<unsigned long>,  Swap8RU>
#define RdE2J  RdX2Y_2Lim<int,    float,  L_I32<float>,  H_I32<float>,    Swap4>
#define RdD2J  RdX2Y_2Lim<int,    double, L_I32<double>, H_I32<double>,   Swap8>
                                                                     
#define WrL2J  WrL2Y     <int,                                                  Swap4>
#define WrS2J  WrX2Y     <int,    char,                                       Swap4>
#define WrB2J  WrX2Y     <int,    unsigned char,                                      Swap4>
#define WrI2J  WrX2Y     <int,    short,                                      Swap4>
#define WrU2J  WrX2Y     <int,    unsigned short,                                     Swap4>
#define WrJ2J  WrX2Y     <int,    int,                                        Swap4>
#define WrV2J  WrX2Y_1Lim<int,    unsigned int,                    H_I32<unsigned int>,     Swap4>
#define WrK2J  WrX2Y_2Lim<int,    long, L_I32<long>, H_I32<long>,   Swap4>
#define WrW2J  WrX2Y_1Lim<int,    unsigned long,                 H_I32<unsigned long>,  Swap4>
#define WrE2J  WrX2Y_2Lim<int,    float,  L_I32<float>,  H_I32<float>,    Swap4>
#define WrD2J  WrX2Y_2Lim<int,    double, L_I32<double>, H_I32<double>,   Swap4>
                                                                     
                                                                     
#define RdL2V  RdL2Y     <unsigned int>
#define RdS2V  RdX2Y_1Lim<unsigned int,   char,   L_0<char>,                       Swap1U>
#define RdB2V  RdX2Y     <unsigned int,   unsigned char,                                     Swap1>
#define RdI2V  RdX2Y_1Lim<unsigned int,   short,  L_0<short>,                      Swap2>
#define RdU2V  RdX2Y     <unsigned int,   unsigned short,                                    Swap2RU>
#define RdJ2V  RdX2Y_1Lim<unsigned int,   int,    L_0<int>,                        Swap4>
#define RdV2V  RdX2Y     <unsigned int,   unsigned int,                                      Swap4RU>
#define RdK2V  RdX2Y_2Lim<unsigned int,   long, L_0<long>,   H_U32<long>,  Swap8>
#define RdW2V  RdX2Y_1Lim<unsigned int,   unsigned long,                 H_U32<unsigned long>, Swap8RU>
#define RdE2V  RdX2Y_2Lim<unsigned int,   float,  L_0<float>,    H_U32<float>,   Swap4>
#define RdD2V  RdX2Y_2Lim<unsigned int,   double, L_0<double>,   H_U32<double>,  Swap8>

#define WrL2V  WrL2Y     <unsigned int,                                                Swap4WU>
#define WrS2V  WrX2Y_1Lim<unsigned int,   char,   L_0<char>,                       Swap4WU>
#define WrB2V  WrX2Y     <unsigned int,   unsigned char,                                     Swap4WU>
#define WrI2V  WrX2Y_1Lim<unsigned int,   short,  L_0<short>,                      Swap4WU>
#define WrU2V  WrX2Y     <unsigned int,   unsigned short,                                    Swap4WU>
#define WrJ2V  WrX2Y_1Lim<unsigned int,   int,    L_0<int>,                        Swap4WU>
#define WrV2V  WrX2Y     <unsigned int,   unsigned int,                                      Swap4WU>
#define WrK2V  WrX2Y_2Lim<unsigned int,   long, L_0<long>,   H_U32<long>,  Swap4WU>
#define WrW2V  WrX2Y_1Lim<unsigned int,   unsigned long,                 H_U32<unsigned long>, Swap4WU>
#define WrE2V  WrX2Y_2Lim<unsigned int,   float,  L_0<float>,    H_U32<float>,   Swap4WU>
#define WrD2V  WrX2Y_2Lim<unsigned int,   double, L_0<double>,   H_U32<double>,  Swap4WU>


#define RdL2K  RdL2Y     <long>
#define RdS2K  RdX2Y     <long,    char,                                       Swap1U>
#define RdB2K  RdX2Y     <long,    unsigned char,                                      Swap1>
#define RdI2K  RdX2Y     <long,    short,                                      Swap2>
#define RdU2K  RdX2Y     <long,    unsigned short,                                     Swap2RU>
#define RdJ2K  RdX2Y     <long,    int,                                        Swap4>
#define RdV2K  RdX2Y     <long,    unsigned int,                                       Swap4RU>
#define RdK2K  RdX2Y     <long,    long,                                     Swap8>
#define RdW2K  RdX2Y_1Lim<long,    unsigned long,                  H_I64<unsigned long>, Swap8RU>
#define RdE2K  RdX2Y_2Lim<long,    float,   L_I64<float>,  H_I64<float>,   Swap4>
#define RdD2K  RdX2Y_2Lim<long,    double,  L_I64<double>, H_I64<double>,  Swap8>
                                                                     
#define WrL2K  WrL2Y     <long,                                                  Swap8>
#define WrS2K  WrX2Y     <long,    char,                                       Swap8>
#define WrB2K  WrX2Y     <long,    unsigned char,                                      Swap8>
#define WrI2K  WrX2Y     <long,    short,                                      Swap8>
#define WrU2K  WrX2Y     <long,    unsigned short,                                     Swap8>
#define WrJ2K  WrX2Y     <long,    int,                                        Swap8>
#define WrV2K  WrX2Y     <long,    unsigned int,                                       Swap8>
#define WrK2K  WrX2Y     <long,    long,                                     Swap8>
#define WrW2K  WrX2Y_1Lim<long,    unsigned long,                  H_I64<unsigned long>, Swap8>
#define WrE2K  WrX2Y_2Lim<long,    float,   L_I64<float>,  H_I64<float>,   Swap8>
#define WrD2K  WrX2Y_2Lim<long,    double,  L_I64<double>, H_I64<double>,  Swap8>
                                                                     
                                                                   
#define RdL2W  RdL2Y     <unsigned long>
#define RdS2W  RdX2Y_1Lim<unsigned long,   char,    L_0<char>,                       Swap1U>
#define RdB2W  RdX2Y     <unsigned long,   unsigned char,                                      Swap1>
#define RdI2W  RdX2Y_1Lim<unsigned long,   short,   L_0<short>,                      Swap2>
#define RdU2W  RdX2Y     <unsigned long,   unsigned short,                                     Swap2RU>
#define RdJ2W  RdX2Y_1Lim<unsigned long,   int,     L_0<int>,                        Swap4>
#define RdV2W  RdX2Y     <unsigned long,   unsigned int,                                       Swap4RU>
#define RdK2W  RdX2Y_1Lim<unsigned long,   long,  L_0<long>,                     Swap8>
#define RdW2W  RdX2Y     <unsigned long,   unsigned long,                                    Swap8RU>
#define RdE2W  RdX2Y_2Lim<unsigned long,   float,   L_0<float>,     H_U64<float>,  Swap4>
#define RdD2W  RdX2Y_2Lim<unsigned long,   double,  L_0<double>,    H_U64<double>, Swap8>

#define WrL2W  WrL2Y     <unsigned long,                                                 Swap8WU>
#define WrS2W  WrX2Y_1Lim<unsigned long,   char,    L_0<char>,                       Swap8WU>
#define WrB2W  WrX2Y     <unsigned long,   unsigned char,                                      Swap8WU>
#define WrI2W  WrX2Y_1Lim<unsigned long,   short,   L_0<short>,                      Swap8WU>
#define WrU2W  WrX2Y     <unsigned long,   unsigned short,                                     Swap8WU>
#define WrJ2W  WrX2Y_1Lim<unsigned long,   int,     L_0<int>,                        Swap8WU>
#define WrV2W  WrX2Y     <unsigned long,   unsigned int,                                       Swap8WU>
#define WrK2W  WrX2Y_1Lim<unsigned long,   long,  L_0<long>,                     Swap8WU>
#define WrW2W  WrX2Y     <unsigned long,   unsigned long,                                    Swap8WU>
#define WrE2W  WrX2Y_2Lim<unsigned long,   float,   L_0<float>,     H_U64<float>,  Swap8WU>
#define WrD2W  WrX2Y_2Lim<unsigned long,   double,  L_0<double>,    H_U64<double>, Swap8WU>


#define RdL2E  RdL2Y     <float>
#define RdS2E  RdX2Y     <float,  char,                                  Swap1U>
#define RdB2E  RdX2Y     <float,  unsigned char,                                 Swap1>
#define RdI2E  RdX2Y     <float,  short,                                 Swap2>
#define RdU2E  RdX2Y     <float,  unsigned short,                                Swap2RU>
#define RdJ2E  RdX2Y     <float,  int,                                   Swap4>
#define RdV2E  RdX2Y     <float,  unsigned int,                                  Swap4RU>
#define RdK2E  RdX2Y     <float,  long,                                Swap8>
#define RdW2E  RdX2Y     <float,  unsigned long,                               Swap8RU>
#define RdE2E  RdX2Y     <float,  float,                                 Swap4>
#define RdD2E  RdX2Y_2Lim<float,  double,  L_F<double>, H_F<double>, Swap8>

#define WrL2E  WrL2Y     <float,                                           Swap4>
#define WrS2E  WrX2Y     <float,  char,                                  Swap4>
#define WrB2E  WrX2Y     <float,  unsigned char,                                 Swap4>
#define WrI2E  WrX2Y     <float,  short,                                 Swap4>
#define WrU2E  WrX2Y     <float,  unsigned short,                                Swap4>
#define WrJ2E  WrX2Y     <float,  int,                                   Swap4>
#define WrV2E  WrX2Y     <float,  unsigned int,                                  Swap4>
#define WrK2E  WrX2Y     <float,  long,                                Swap4>
#define WrW2E  WrX2Y     <float,  unsigned long,                               Swap4>
#define WrE2E  WrX2Y     <float,  float,                                 Swap4>
#define WrD2E  WrX2Y_2Lim<float,  double,  L_F<double>, H_F<double>, Swap4>

#define RdL2D  RdL2Y     <double>
#define RdS2D  RdX2Y     <double, char,                          Swap1U>
#define RdB2D  RdX2Y     <double, unsigned char,                         Swap1>
#define RdI2D  RdX2Y     <double, short,                         Swap2>
#define RdU2D  RdX2Y     <double, unsigned short,                        Swap2RU>
#define RdJ2D  RdX2Y     <double, int,                           Swap4>
#define RdV2D  RdX2Y     <double, unsigned int,                          Swap4RU>
#define RdK2D  RdX2Y     <double, long,                        Swap8>
#define RdW2D  RdX2Y     <double, unsigned long,                       Swap8RU>
#define RdE2D  RdX2Y     <double, float,                         Swap4>
#define RdD2D  RdX2Y     <double, double,                        Swap8>

#define WrL2D  WrL2Y     <double,                                  Swap8>
#define WrS2D  WrX2Y     <double, char,                          Swap8>
#define WrB2D  WrX2Y     <double, unsigned char,                         Swap8>
#define WrI2D  WrX2Y     <double, short,                         Swap8>
#define WrU2D  WrX2Y     <double, unsigned short,                        Swap8>
#define WrJ2D  WrX2Y     <double, int,                           Swap8>
#define WrV2D  WrX2Y     <double, unsigned int,                          Swap8>
#define WrK2D  WrX2Y     <double, long,                        Swap8>
#define WrW2D  WrX2Y     <double, unsigned long,                       Swap8>
#define WrE2D  WrX2Y     <double, float,                         Swap8>
#define WrD2D  WrX2Y     <double, double,                        Swap8>


void (*RdCopy[12][12]) (void *, void *, int) = {
//  L    S        B      I     U      J      V      K      W      E      D       A         
{RdL2L, RdL2S, RdL2B, RdL2I, RdL2U, RdL2J, RdL2V, RdL2K, RdL2W, RdL2E, RdL2D,  NULL},  // L
{RdS2L, RdS2S, RdS2B, RdS2I, RdS2U, RdS2J, RdS2V, RdS2K, RdS2W, RdS2E, RdS2D,  NULL},  // S
{RdB2L, RdB2S, RdB2B, RdB2I, RdB2U, RdB2J, RdB2V, RdB2K, RdB2W, RdB2E, RdB2D,  NULL},  // B
{RdI2L, RdI2S, RdI2B, RdI2I, RdI2U, RdI2J, RdI2V, RdI2K, RdI2W, RdI2E, RdI2D,  NULL},  // I
{RdU2L, RdU2S, RdU2B, RdU2I, RdU2U, RdU2J, RdU2V, RdU2K, RdU2W, RdU2E, RdU2D,  NULL},  // U
{RdJ2L, RdJ2S, RdJ2B, RdJ2I, RdJ2U, RdJ2J, RdJ2V, RdJ2K, RdJ2W, RdJ2E, RdJ2D,  NULL},  // J
{RdV2L, RdV2S, RdV2B, RdV2I, RdV2U, RdV2J, RdV2V, RdV2K, RdV2W, RdV2E, RdV2D,  NULL},  // V
{RdK2L, RdK2S, RdK2B, RdK2I, RdK2U, RdK2J, RdK2V, RdK2K, RdK2W, RdK2E, RdK2D,  NULL},  // K
{RdW2L, RdW2S, RdW2B, RdW2I, RdW2U, RdW2J, RdW2V, RdW2K, RdW2W, RdW2E, RdW2D,  NULL},  // W
{RdE2L, RdE2S, RdE2B, RdE2I, RdE2U, RdE2J, RdE2V, RdE2K, RdE2W, RdE2E, RdE2D,  NULL},  // E
{RdD2L, RdD2S, RdD2B, RdD2I, RdD2U, RdD2J, RdD2V, RdD2K, RdD2W, RdD2E, RdD2D,  NULL},  // D
{ NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL, RdA2A}   // A
};

void (*WrCopy[12][12]) (void *, void *, int) = {
//  L    S        B      I     U      J      V      K      W      E      D       A         
{WrL2L, WrL2S, WrL2B, WrL2I, WrL2U, WrL2J, WrL2V, WrL2K, WrL2W, WrL2E, WrL2D,  NULL},  // L
{WrS2L, WrS2S, WrS2B, WrS2I, WrS2U, WrS2J, WrS2V, WrS2K, WrS2W, WrS2E, WrS2D,  NULL},  // S
{WrB2L, WrB2S, WrB2B, WrB2I, WrB2U, WrB2J, WrB2V, WrB2K, WrB2W, WrB2E, WrB2D,  NULL},  // B
{WrI2L, WrI2S, WrI2B, WrI2I, WrI2U, WrI2J, WrI2V, WrI2K, WrI2W, WrI2E, WrI2D,  NULL},  // I
{WrU2L, WrU2S, WrU2B, WrU2I, WrU2U, WrU2J, WrU2V, WrU2K, WrU2W, WrU2E, WrU2D,  NULL},  // U
{WrJ2L, WrJ2S, WrJ2B, WrJ2I, WrJ2U, WrJ2J, WrJ2V, WrJ2K, WrJ2W, WrJ2E, WrJ2D,  NULL},  // J
{WrV2L, WrV2S, WrV2B, WrV2I, WrV2U, WrV2J, WrV2V, WrV2K, WrV2W, WrV2E, WrV2D,  NULL},  // V
{WrK2L, WrK2S, WrK2B, WrK2I, WrK2U, WrK2J, WrK2V, WrK2K, WrK2W, WrK2E, WrK2D,  NULL},  // K
{WrW2L, WrW2S, WrW2B, WrW2I, WrW2U, WrW2J, WrW2V, WrW2K, WrW2W, WrW2E, WrW2D,  NULL},  // W
{WrE2L, WrE2S, WrE2B, WrE2I, WrE2U, WrE2J, WrE2V, WrE2K, WrE2W, WrE2E, WrE2D,  NULL},  // E
{WrD2L, WrD2S, WrD2B, WrD2I, WrD2U, WrD2J, WrD2V, WrD2K, WrD2W, WrD2E, WrD2D,  NULL},  // D
{ NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL, WrA2A}   // A
};


