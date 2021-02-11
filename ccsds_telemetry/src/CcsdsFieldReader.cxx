/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Implementation of the CcsdsFieldReader class
*
*  @author Florian George ELSE
*
*  @version 6.2   2016-10-14 FGE  Implemented GetCurrentOffset method.
*  @version 1.0   2015-04-23 FGE  First released version
*
*/

#include "CcsdsFieldReader.hxx"

// Creates a CCSDS Field Reader for the given memory location and length
CcsdsFieldReader::CcsdsFieldReader(const uint8_t *buffer, size_t length)
    : m_buffer(buffer), m_bufferLength(length)
{
    if (buffer == nullptr)
        throw std::invalid_argument("The buffer containing the packet cannot be null.");

    m_end = buffer + length - 1;
    m_position = buffer;
}

// Returns the current offset of the reader
size_t CcsdsFieldReader::GetCurrentOffset() const
{
    return m_position - m_buffer;
}
