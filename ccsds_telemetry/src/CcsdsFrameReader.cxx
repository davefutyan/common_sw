/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Implementation of the CcsdsFrameReader class
*
*  @author Florian George ELSE
*
*  @version 6.0   2016-06-24 FGE  Frames include a 4-byte Secondary Header.
*  @version 5.1   2016-03-15 FGE  Fixed field readings to conform to correct version of standard.
*                                 Added validation of fixed Data Field Status flags.
*  @version 1.0   2015-06-18 FGE  first released version
*
*/

#include "CcsdsFrameReader.hxx"
#include "CcsdsFieldReader.hxx"

CcsdsFrameReader::CcsdsFrameReader(const uint8_t *buffer, uint16_t length)
    : m_buffer(buffer), m_frameLength(length)
{
    if (buffer == nullptr)
        throw std::invalid_argument("The buffer containing the frame cannot be null.");
    if (length < 7)
        throw std::invalid_argument("The buffer is too small to contain any valid frame.");
}

// Validates that the buffer contains a complete and valid CHEOPS CCSDS telemetry frame.
void CcsdsFrameReader::ValidateFrame() const
{
    // Verify Transfer Frame Version Number == 0b00 (first 2 bits of frame)
    if((m_buffer[0] & 0xC0) != 0x00)
        throw std::runtime_error("Invalid Transfer Frame Version Number, only version 0 is supported.");

    // Ensure spacecraft is CHEOPS
    // TODO: Enable when DFS puts correct SCID in frames
    //uint16_t scId = GetScId();
    //if (scId != CheopsSpacecraftId)
    //    throw std::runtime_error(std::string("Invalid Spacecraft ID " + std::to_string(scId) + ", expected " + std::to_string(CheopsSpacecraftId)));
    
    // Validate fixed Data Field Status flags
    uint8_t flags = m_buffer[4];
    // Transfer Frame Secondary Header Flag must be 0
    if ((flags & 0x80) == 0)
        throw std::runtime_error("Transfer Frame Secondary Header Flag must be 1, secondary header is always present.");
    // Synchronization Flag must be 1
    if ((flags & 0x40) != 0)
        throw std::runtime_error("Synchronization Flag must be 0, any other value is not supported.");
    // Packet Order Flag Flag must be 0
    if ((flags & 0x20) != 0)
        throw std::runtime_error("Packet Order Flag Flag must be 0, any other value is not supported.");
    // Segment Length Identifier must be 0b11
    if ((flags & 0x18) != 0x18)
        throw std::runtime_error("Segment Length Identifier must be 0b11, any other value is not supported.");
}


// Gets the value of the Spacecraft ID (SCID) header field.
uint16_t CcsdsFrameReader::GetScId() const
{
    // SCID is 10 bits 2-11 of first 16-bit word (big-endian)
    return (CcsdsFieldRead<uint16_t>(m_buffer) >> 4) & 0x03FF;
}

// Gets the value of the Virtual Channel ID (VCID) header field.
uint8_t CcsdsFrameReader::GetVcId() const
{
    // VCID is 3 bits 12-14 of first 16-bit word (big-endian)
    //   i.e. bits 4-6 of second byte
    return (m_buffer[1] >> 1) & 0x07;
}

// Gets the value of the Master Channel Frame Count header field.
uint8_t CcsdsFrameReader::GetMcFrameCount() const
{
    // Virtual Channel Frame Count is third byte
    return m_buffer[2];
}

// Gets the value of the Virtual Channel Frame Count header field.
uint8_t CcsdsFrameReader::GetVcFrameCount() const
{
    // Virtual Channel Frame Count is fourth byte
    return m_buffer[3];
}


// Gets whether the frame has an Operational Control Field.
bool CcsdsFrameReader::HasOperationalControlField() const
{
    // Operational Control Field Flag is last bit of first word.
    //   i.e. last bit of second byte
    return (m_buffer[1] & 0x01) != 0;
}

    
// Gets the value of the First Header Pointer field.
uint16_t CcsdsFrameReader::GetFirstHeaderPointer() const
{
    // First Header Pointer is 11-bit integer in LSB of last word of the Transfer Frame Primary Header (big-endian)
    return CcsdsFieldRead<uint16_t>(m_buffer + PrimaryHeaderLength - 2) & 0x7FF;
}


// Gets a pointer to the start of the data in the Packet Zone field and its length.
const uint8_t * CcsdsFrameReader::GetDataPointer(uint16_t &dataLength) const
{
    uint16_t trailerLength = HasOperationalControlField() ? 4 : 0;
    
    // Data length is packet length minus all headers and trailer
    dataLength = m_frameLength - PrimaryHeaderLength - SecondaryHeaderLength - trailerLength;
    return m_buffer + PrimaryHeaderLength + SecondaryHeaderLength;
}
