/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Declaration of the CcsdsFieldReader class and the CcsdsFieldRead function.
*
*  @author Florian George ELSE
*
*  @version 6.2   2016-10-14 FGE  Added GetCurrentOffset method.
*  @version 1.0   2015-04-23 FGE  First released version
*
*/

#pragma once

#include <stdint.h>   // For explicit size integers (e.g. uint8_t, uint16_t)
#include <stdexcept>  // For standard base exceptions such as runtime_exception


/// @brief Reads a value of the specified type at the specified location in memory.
/// @param [in] pointer   Pointer to the location in memory where read the value.
template <typename T>
T CcsdsFieldRead(const uint8_t *pointer)
{
    // Allocate value, but use memory byte per byte
    T value;
    uint8_t* valPtr = reinterpret_cast<uint8_t*>(&value);

    // Read bytes in order, but write them in reverse (big-endian -> little-endian)
    // TODO: If current architecture is already big-endian, do not write them reversed
    for(int i=sizeof(T)-1 ; i>=0 ; i--)
        valPtr[i] = *(pointer++);

    return value;
}


/** ****************************************************************************
 *  @ingroup ccsds_telemetry
 *  @author Florian George ELSE
 *
 *  @brief Provides reading of a sequence of CCSDS formatted values in a memory buffer.
 *
 *  No memory allocation or copy are made by this class, it only provides access
 *  to CCSDS formatted values in a given memory buffer.
 *  Ownership of the memory block belong to the caller and must remain valid to
 *  read values.
 */
class CcsdsFieldReader
{
private:
    /// Pointer to memory buffer passed in constructor.
    const uint8_t *m_buffer;
    /// Length of the memory buffer passed in constructor.
    const size_t m_bufferLength;
    /// Pointer to the last byte of the memory buffer
    const uint8_t *m_end;
    /// Current position of the read pointer
    const uint8_t *m_position;

public:
    /** ****************************************************************************
     *  @brief Creates a CCSDS Field Reader for the given memory location and length
     *
     *  @param [in] buffer   Pointer to the first byte of the memory buffer to read.
     *  @param [in] length   Length of the memory buffer.
     *  @throw std::invalid_argument   Buffer is null.
     */
    CcsdsFieldReader(const uint8_t *buffer, size_t length);

    /// Reads a value of the specified type and advance read position.
    template <typename T>
    T Read()
    {
        T value = CcsdsFieldRead<T>(m_position); // Read value using static Read
        m_position += sizeof(T);                 // Move read pointer
        return value;
    }

    /// @brief Returns the current offset of the reader.
    /// @return The current offset of the reader in bytes from the start of the memory buffer.
    size_t GetCurrentOffset() const;
};

