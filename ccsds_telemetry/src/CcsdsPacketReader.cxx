/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Implementation of the CcsdsPacketReader class
*
*  @author Florian George ELSE
* 
*  @version 6.1  2016-10-10 FGE  Implemented GetPacketErrorControl method.
*  @version 4.0  2015-08-21 FGE  Improved validation of packets
*  @version 1.0  2015-04-17 FGE  First released version
*
*/

#include "CcsdsFieldReader.hxx"
#include "CcsdsPacketReader.hxx"
#include "CcsdsPacketCrc.hxx"

#include <boost/format.hpp>


CcsdsPacketReader::CcsdsPacketReader(const uint8_t *buffer, size_t length)
    : m_buffer(buffer), m_bufferLength(length)
{
    if (buffer == nullptr)
        throw std::invalid_argument("The buffer containing the packet cannot be null.");
    if (length < 7)
        throw std::invalid_argument("The buffer is too small to contain any valid packet.");
}

void CcsdsPacketReader::ValidatePacket() const
{
    // Compute packet length
    uint16_t packetLength = GetPacketLength();

    // Validate that buffer contains entire packet
    if (m_bufferLength < packetLength)
        throw std::runtime_error((boost::format("The packet is truncated or the buffer is too small, packet is %1% bytes long, got %2% bytes.") % packetLength % m_bufferLength).str());

    // Validate packet CRC
    // Get CRC value read from field in packet
    uint16_t readChecksum = GetPacketErrorControl();
    // Compute CRC of packet, all bytes except last 2 bytes of CRC field itself
    uint16_t computedChecksum = CcsdsPacketCrcCompute(m_buffer, packetLength - 2);
    // Ensure that values are identical
    if (readChecksum != computedChecksum)
        throw std::runtime_error((boost::format("The packet is corrupted, checksum validation failed. Computed 0x%04x, read 0x%04x.") % computedChecksum % readChecksum).str());

    // Ensure correct version of standard (Version Number == 0)
    if((m_buffer[0] & 0xE0) != 0)
        throw std::runtime_error("Only packets version number 0 are supported.");

    // Ensure it is a telemetry packet (Packet Type == 0)
    if((m_buffer[0] & 0x10) != 0)
        throw std::runtime_error("Only telemetry packets are supported.");

    // Ensure packet has a Data Field Header (Data Field Header Flag == 1)
    if ((m_buffer[0] & 0x08) == 0)
        throw std::runtime_error("Packets without a Data Field Header are not supported.");

    // Ensure packet is not segmented (Segmentation Flags == 0b11)
    if ((m_buffer[2] & 0xC0) != 0xC0)
        throw std::runtime_error("Segmented packets are not supported.");
}

uint16_t CcsdsPacketReader::GetApid() const
{
    // APID is 11 LSB of first 16-bit word (big-endian)
    return CcsdsFieldRead<uint16_t>(m_buffer) & 0x7FF;
}

uint16_t CcsdsPacketReader::GetSequenceCount() const
{
    // Sequence Count is 14 LSB of second 16-bit word (big-endian)
    return CcsdsFieldRead<uint16_t>(m_buffer + 2) & 0x3FFF;
}

uint16_t CcsdsPacketReader::GetPacketLength() const
{
    // Packet Length is third 16-bit word (big-endian) + 7 (field itself is minus-1 and doesn't account for packet header length)
    return CcsdsFieldRead<uint16_t>(m_buffer + 4) + 7;
}


uint8_t CcsdsPacketReader::GetServiceType() const
{
    // Service Type is always the second byte of Data Field Header
    return m_buffer[HeaderLength + 1];
}

uint8_t CcsdsPacketReader::GetServiceSubtype() const
{
    // Service Subtype is always the third byte of Data Field Header
    return m_buffer[HeaderLength + 2];
}

uint8_t CcsdsPacketReader::GetDestinationId() const
{
    // Destination ID is fourth byte (for CHEOPS) of Data Field Header
    return m_buffer[HeaderLength + 3];
}

uint64_t CcsdsPacketReader::GetTime() const
{
    // Time starts at the fifth byte (for CHEOPS) of Data Field Header, 6 bytes for CHEOPS
    // Read 8 bytes (64-bit) instead, starting 2 bytes early
    return CcsdsFieldRead<uint64_t>(m_buffer + HeaderLength + 4 - 2) & 0x0000FFFFFFFFFFFF;
}

/// Gets the value of the Packet Error Control field.
uint16_t CcsdsPacketReader::GetPacketErrorControl() const
{
    //Packet Error Control is last two bytes of packet
    return CcsdsFieldRead<uint16_t>(m_buffer + GetPacketLength() - 2);
}


const uint8_t * CcsdsPacketReader::GetDataPointer(uint16_t &dataLength) const
{
    // Data length is packet length minus all headers and PEC
    dataLength = GetPacketLength() - HeaderLength - DataHeaderLength - 2;
    return m_buffer + HeaderLength + DataHeaderLength;
}
