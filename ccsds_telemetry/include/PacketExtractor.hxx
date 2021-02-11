/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Declaration of the PacketExtractor class
*
*  @author Florian George ELSE
*
*  @version 5.1   2016-03-15 FGE  first released version
*
*/

#pragma once

#include <stdint.h>   // For explicit size integers (e.g. uint8_t, uint16_t)
#include <stdexcept>  // For standard base exceptions such as runtime_exception
#include <functional> // For std::function used for packet extraction callback
#include <vector>

#include "CcsdsFrameReader.hxx"
#include "CcsdsPacketReader.hxx"

/// Callback function for each packet extracted from the telemetry frames stream.
/// @param [in] packetReader   The CCSDS Packet Reader of the extracted packet.
/// @param [in] reconstructed  Indicates whether the packet was split across frames and had to be reconstructed.
///                            When true, the memory holding the packet is only valid during the callback and has
///                            to be copied to be used outside of the callback.
using PacketCallback = std::function<void(CcsdsPacketReader packetReader, bool reconstructed)>;

/** ****************************************************************************
 *  @ingroup ccsds_telemetry
 *  @author Florian George ELSE
 *
 *  @brief Provides extraction of the telemetry packets from the stream of
 *         telemetry frames.
 */
class PacketExtractor
{
private:
    /// The Virtual Channel number of this packet extractor.
    uint8_t m_virtualChannel;
    /// The callback function called for each packet that is extracted from the telemetry frames.
    PacketCallback m_callback;
    
    /// Expected Virtual Channel Frame Count value of next frame
    uint8_t m_expectedVcFrameCount = 0;
    
    /// Contains the bytes of the current partial packet being reconstructed
    std::vector<uint8_t> m_packetBytes;

    /// Counter of processed frames.
    uint32_t m_frameCount = 0;
    /// Counter of extracted packets.
    uint32_t m_packetExtractedCount = 0;
    /// Counter of missing frames.
    uint32_t m_missingFrameCount = 0;
    /// Counter of lost packets.
    uint32_t m_lostPacketCount = 0;
    
public:
    /** ****************************************************************************
     *  @brief Creates a Packet Extractor for the given Virtual Channel.
     *
     *  @param [in] virtualChannel  The Virtual Channel number of this packet extractor.
     *  @param [in] callback        The callback function called for each packet
     *                              that is extracted from the telemetry frames.
     *                              The packet is not validated, only of the correct length.
     */
    PacketExtractor(uint8_t virtualChannel, PacketCallback callback);
    
    /** ****************************************************************************
     *  @brief Adds the next frame of the telemetry stream and calls the callback
     *         for each packet that are fully extracted.
     *
     *  @throw std::runtime_error  The telemetry frame is not valid.
     */
    void Add(const CcsdsFrameReader& frame);
    
    /** ****************************************************************************
     *  @brief Indicates to the packet extractor that there is no more frames.
     *         Allows to count last packet as lost if it is being reconstructed.
     */
    void End();
    
    /// Gets the number of extracted packets.
    uint32_t GetPacketExtractedCount() const { return m_packetExtractedCount; }
    /// Gets the number of missing frames detected.
    uint32_t GetMissingFrameCount()    const { return m_missingFrameCount; }
    /// Gets the number of lost packets detected. It is a low estimation, can be greater due to lost frames.
    uint32_t GetLostPacketCount()      const { return m_lostPacketCount; }
    
};

