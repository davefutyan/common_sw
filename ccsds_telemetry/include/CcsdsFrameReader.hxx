/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Declaration of the CcsdsFrameReader class
*
*  @author Florian George ELSE
*
*  @version 6.0   2016-06-24 FGE  Frames include a 4-byte Secondary Header.
*  @version 5.1   2016-03-15 FGE  Fixed field readings for correct version of standard.
*  @version 1.0   2015-06-18 FGE  First released version
*
*/

#pragma once

#include <stdint.h>   // For explicit size integers (e.g. uint8_t, uint16_t)
#include <stdexcept>  // For standard base exceptions such as runtime_exception


//! Spacecraft ID of CHEOPS (0x2CB as per assignment http://sanaregistry.org/r/spacecraftid/spacecraftid.html)
const uint16_t CheopsSpacecraftId = 0x2CB;

// Definition of the Virtual Channel IDs used at SOC (see: https://redmine.isdc.unige.ch/projects/cheops/wiki/Virtual_Channels)
/// VC1: Playback TM - Contains the Service 3 Housekeeping packets.
const uint8_t VirtualChannelHousekeeping = 1;
/// VC3: Instrument Science TM - Contains the Service 13 LDTS packets.
const uint8_t VirtualChannelScience = 3;
/// VC5: Event TM - Contains the Service 5 Event packets.
const uint8_t VirtualChannelEvents = 5;


/** ****************************************************************************
 *  @ingroup ccsds_telemetry
 *  @author Florian George ELSE
 *
 *  @brief Provides reading of fields' value of a CHEOPS CCSDS telemetry frame.
 *
 *  CHEOPS telemetry frames have a 4 bytes Secondary Header and no Frame Error Control Field.
 * 
 *  No memory allocation or copy are made by this class, it only provides access
 *  to the value of the fields for a given CCSDS frame in a memory buffer.
 *  Ownership of the memory block belong to the caller and must remain valid to
 *  read values.
 */
class CcsdsFrameReader
{
private:
    /// Pointer to memory buffer passed in constructor.
    const uint8_t *m_buffer;
    /// Length of the frame passed in constructor.
    const uint16_t m_frameLength;

public:
    /// Constant of the length of the Transfer Frame Primary Header
    static const uint8_t PrimaryHeaderLength = 6;
    /// Constant of the length of the Transfer Frame Secondary Header
    static const uint8_t SecondaryHeaderLength = 4;
    
    /// Special value of the First Header Pointer when no packet starts in the frame data.
    static const uint16_t FhpNoPacketStart = 0x07FF;
    /// Special value of the First Header Pointer when frame contains only idle data.
    static const uint16_t FhpIdleData      = 0x07FE;

    /** ****************************************************************************
     *  @brief Creates a CCSDS Frame Reader for the given memory location and frame length
     *
     *  @param [in] buffer   Pointer to the first byte of the CCSDS frame to read
     *  @param [in] length   Total length of the CCSDS frame.
     *
     *  @throw std::invalid_argument   Buffer is null or to small to contain a valid frame.
     */
    CcsdsFrameReader(const uint8_t *buffer, uint16_t length);

    /// Copy constructor
    CcsdsFrameReader(const CcsdsFrameReader& other) = default;

    /// Copy assignment operator
    CcsdsFrameReader& operator=(const CcsdsFrameReader& other) = default;

    /** ****************************************************************************
    *  @brief Validates that the buffer contains a complete and valid CHEOPS CCSDS
    *         telemetry frame. Throws std::runtime_exception for validation failures.
    *
    *  @throw std::runtime_error   The frame is not valid or contains non-supported
    *                              features. Reason in exception message.
    */
    void ValidateFrame() const;


    /// Gets the total length of the frame; including all headers, data and trailer.
    uint16_t GetFrameLength() const { return m_frameLength; }
    /// Gets the value of the Spacecraft ID (SCID) header field.
    uint16_t GetScId() const;
    /// Gets the value of the Virtual Channel ID (VCID) header field.
    uint8_t GetVcId() const;
    /// Gets the value of the Master Channel Frame Count header field.
    uint8_t GetMcFrameCount() const;
    /// Gets the value of the Virtual Channel Frame Count header field.
    uint8_t GetVcFrameCount() const;
    
    /// Gets whether the frame has an Operational Control Field.
    bool HasOperationalControlField() const;

    /// Gets the value of the First Header Pointer field.
    uint16_t GetFirstHeaderPointer() const;

    /// @brief Gets a pointer to the start of the data in the Packet Zone field and its length.
    /// @param [out] dataLength   The length of the data.
    const uint8_t * GetDataPointer(uint16_t &dataLength) const;
};
