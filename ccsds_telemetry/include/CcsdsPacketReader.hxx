/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Declaration of the CcsdsPacketReader class
*
*  @author Florian George ELSE
*
*  @version 6.1  2016-10-10 FGE  Added GetPacketErrorControl method.
*  @version 4.0  2015-08-21 FGE  Added copy constructor and copy assignment operator
*  @version 1.0  2015-04-17 FGE  first released version
*
*/

#pragma once

#include <stdint.h>   // For explicit size integers (e.g. uint8_t, uint16_t)
#include <stdexcept>  // For standard base exceptions such as runtime_exception

/** ****************************************************************************
 *  @ingroup ccsds_telemetry
 *  @author Florian George ELSE
 *
 *  @brief Provides reading of fields' value of a CHEOPS CCSDS packet.
 *
 *  No memory allocation or copy are made by this class, it only provides access
 *  to the value of the fields for a given CCSDS packet in a memory buffer.
 *  Ownership of the memory block belong to the caller and must remain valid to
 *  read values.
 */
class CcsdsPacketReader
{
private:
    /// Pointer to memory buffer passed in constructor.
    const uint8_t* m_buffer;
    /// Length of the memory buffer passed in constructor.
    size_t m_bufferLength;

public:
    /// Constant of the length of the Packet Header
    static const int HeaderLength = 6;
    /// Constant of the length of the Data Field Header (for CHEOPS)
    static const int DataHeaderLength = 10;

    /** ****************************************************************************
     *  @brief Creates a CCSDS Packet Reader for the given memory location and length
     *
     *  @param [in] buffer   Pointer to the first byte of the CCSDS packet to read
     *  @param [in] length   Length of the memory buffer, can be greater than size
     *                       of packet. If too small, validation will fail.
     *  @throw std::invalid_argument   Buffer is null or to small to contain a packet.
     */
    CcsdsPacketReader(const uint8_t* buffer, size_t length);

    /// Copy constructor
    CcsdsPacketReader(const CcsdsPacketReader& other) = default;

    /// Copy assignment operator
    CcsdsPacketReader& operator=(const CcsdsPacketReader& other) = default;

    
    /** ****************************************************************************
    *  @brief Validates that the buffer contains a complete and valid CHEOPS CCSDS
    *         telemetry packet. Throws std::runtime_exception for validation failures.
    *
    *  @throw std::runtime_error   The packet is not valid or contains non-supported
    *                              features. Reason in exception message.
    */
    void ValidatePacket() const;

    /// Returns the pointer to the memory buffer containing the packet.
    const uint8_t* GetBuffer() const { return m_buffer; }

    /// Gets the total length of the packet; including all headers, data and PEC.
    uint16_t GetPacketLength() const;
    /// Gets the value of the Application Process ID (APID) header field.
    uint16_t GetApid() const;
    /// Gets the value of the Sequence Count header field.
    uint16_t GetSequenceCount() const;

    /// Gets the value of the Service Type data header field.
    uint8_t GetServiceType() const;
    /// Gets the value of the Service Subtype data header field.
    uint8_t GetServiceSubtype() const;
    /// Gets the value of the Destination ID data header field.
    uint8_t GetDestinationId() const;
    /// Gets the value of the Time data header field.
    uint64_t GetTime() const;

    /// Gets the value of the Packet Error Control field.
    uint16_t GetPacketErrorControl() const;


    /// @brief Gets a pointer to the start of the Source Data field and its length.
    /// @param [out] dataLength   The length of the Source Data field.
    const uint8_t * GetDataPointer(uint16_t& dataLength) const;
};

