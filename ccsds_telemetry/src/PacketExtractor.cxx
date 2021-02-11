/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Implementation of the PacketExtractor class
*
*  @author Florian George ELSE
*
*  @version 12.0.2 2020-02-19 FGE  Do not count a FHP!=0 in first frame as an error anymore, log info instead (#20932).
*  @version 11.4.1 2019-07-05 FGE  Ignore remaining bytes of last frame if only zeroes or start of idle packet (#19175). Log when incomplete packet at end of last frame.
*  @version 10.3   2018-10-29 FGE  Use common_sw's logger module for logging by default, use cerr when NO_COMMON_SW defined (#16198).
*  @version 5.1    2016-03-15 FGE  first released version
*
*/

#include "PacketExtractor.hxx"

#include <boost/format.hpp>
#include <iostream>

using namespace std;

// When not using common_sw, log into cerr instead of using logger module
#if defined(NO_COMMON_SW)
    ostream& logger = cerr;
    string error("Error: ");
    string warn("Warning: ");
    string info("Info: ");
#else
#include <Logger.hxx>
#endif


// Creates a Packet Extractor for the given memory Virtual Channel
PacketExtractor::PacketExtractor(uint8_t virtualChannel, PacketCallback callback)
    : m_virtualChannel(virtualChannel), m_callback(callback)
{
}

// Adds the next frame of the telemetry stream and calls the callback for each packet that are fully extracted
void PacketExtractor::Add(const CcsdsFrameReader& frame)
{
    // Ensure frame belong to our virtual channel
    if (frame.GetVcId() != m_virtualChannel)
        throw std::runtime_error((boost::format("Telemetry transfer frame belongs to virtual channel %1%, extractor is for %2%.") % (int)frame.GetVcId() % (int)m_virtualChannel).str());
    m_frameCount++;

    // Check for missing or repeated frame
    uint8_t vcFrameCount = frame.GetVcFrameCount();
    // If this is the first frame being processed, its counter is the initial value
    if (m_frameCount == 1)
        m_expectedVcFrameCount = vcFrameCount;
    bool missedFrame = false;
    if(vcFrameCount != m_expectedVcFrameCount)
    {
        // Gap in VC frame counter, we missed some frame(s)
        uint8_t missedCount = (uint8_t)(vcFrameCount - m_expectedVcFrameCount);
        m_missingFrameCount += missedCount;
        missedFrame = true;
        
        // Output missing frame details for debug
        logger << warn << "Missing " << (int)missedCount << " frame(s) between counter "
            << (int)(uint8_t)(m_expectedVcFrameCount-1) << " and " << (int)vcFrameCount << "." << endl;
        
        // If we have a partial packet being reconstructed, it is lost as well
        if(m_packetBytes.size() != 0)
        {
            m_packetBytes.clear();
            m_lostPacketCount++;
        }
    }
    // Compute next expected VC frame count
    m_expectedVcFrameCount = vcFrameCount + 1;

    
    // Process data of frame, get value first header pointer
    uint16_t fhp = frame.GetFirstHeaderPointer();
    
    // If frame contains only idle data, ignore it
    // even if packet is currently being reconstructed, continue with next frame
    if (fhp == CcsdsFrameReader::FhpIdleData)
        return;
    
    // Get data pointer and data size
    uint16_t bytesRemaining;
    const uint8_t* dataPtr = frame.GetDataPointer(bytesRemaining);


    // If we have a partial packet being reconstructed, but no bytes for, it is lost
    if(fhp == 0 && m_packetBytes.size() != 0)
    {
        // No frame was missed, should not be happening
        if (!missedFrame)
            logger << error << "CCSDS Frame protocol violation: expected remaining bytes of packet started in previous frame, but got FHP=0 (VCFC="<<(int)vcFrameCount<<")." << endl;

        m_packetBytes.clear();
        m_lostPacketCount++;
    }
    // End/next bytes of segmented packet, we don't have its start
    // Can only be due to protocol violation (missedFrame=false) or lost frame (missedFrame=true)
    // Packet loss already counted when detecting lost frame (can't know if same packet)
    else if(fhp != 0 && m_packetBytes.size() == 0)
    {
        // Protocol violation, FHP value is wrong, should be 0
        if (!missedFrame)
        {
            string message = (boost::format("CCSDS Frame protocol violation in frame #%1%: expected FHP=0 but got FHP=%2% (VCFC=%3%).") % m_frameCount % (int)fhp % (int)vcFrameCount).str();
            // Not an error if in first frame, see #20932
            if (m_frameCount == 1)
            {
                logger << info << message << endl;
            }
            // Otherwise an error and assume packet lost (bytes before FHP value)
            else
            {
                m_lostPacketCount++;
                logger << error << message << endl;
            }
        }

        // Move data pointer to location of next packet
        if(fhp == CcsdsFrameReader::FhpNoPacketStart)
        {
            // No packet start, move data pointer to end, no byte we can use in this frame
            dataPtr += bytesRemaining;
            bytesRemaining = 0;
        }
        else
        {
            // Move data pointer to FHP
            dataPtr += fhp;
            bytesRemaining -= fhp;
        }
    }
    else if(m_packetBytes.size() != 0)
    {
        // Bytes for current packets are up to FHP, or all if no packet start in frame
        uint16_t byteCount = fhp;
        if(fhp == CcsdsFrameReader::FhpNoPacketStart)
            byteCount = bytesRemaining;

        // Append bytes to packet be reconstructed
        m_packetBytes.insert(m_packetBytes.end(), dataPtr, dataPtr+byteCount);
        dataPtr += byteCount;
        bytesRemaining -= byteCount;

        // Possibly complete packet
        bool hasCompletePacket = false;
        uint16_t packetLength = 0;
        if(m_packetBytes.size() >= 7)
        {
            // Check if we have all the bytes of the packet
            CcsdsPacketReader reader(&m_packetBytes[0], m_packetBytes.size());
            packetLength = reader.GetPacketLength();
            if(m_packetBytes.size() >= packetLength)
            {
                // Packet is complete, can callback the extraction
                m_callback(reader, true);
                m_packetExtractedCount++;
                hasCompletePacket = true;

                // Reset packet buffer for next partial packet
                m_packetBytes.clear();
            }
        }
        
        // If packet was supposed to end but we don't have enough bytes, it is lost
        if(!hasCompletePacket && fhp != CcsdsFrameReader::FhpNoPacketStart)
        {
            if (packetLength > 0)
                logger << error << "CCSDS Frame protocol violation: missing " << (int64_t)(packetLength-m_packetBytes.size()) << " bytes to reconstruct packet of " << packetLength << " bytes (VCFC="<<(int)vcFrameCount<<")." << endl;
            else
                logger << error << "CCSDS Frame protocol violation: missing bytes to reconstruct packet, only had " << (uint64_t)(m_packetBytes.size()) << " bytes (VCFC="<<(int)vcFrameCount<<")." << endl;
            m_packetBytes.clear();
            m_lostPacketCount++;
        }
    }

    // Process packets that start in the frame
    while(bytesRemaining > 0)
    {
        bool foundFullPacket = false;

        // Check if enough bytes to get packet size
        if(bytesRemaining >= 7)
        {
            // Create a packet reader on the packet start to get its length
            CcsdsPacketReader reader(dataPtr, bytesRemaining);
            uint16_t packetLength = reader.GetPacketLength();

            // HACK: Ignore packets full of zero and treat it as idle data
            // TODO: Ensure using correct idle bytes value, currently assuming zeros
            if (packetLength == 7 && reader.GetPacketErrorControl() == 0x0000)
            {
                logger << info << "Found idle data in frame, ignoring remaining bytes (VCFC="<<(int)vcFrameCount<<")." << endl;
                bytesRemaining = 0;
                continue;
            }

            // Check if the full packet is in the frame
            if(packetLength <= bytesRemaining)
            {
                // Full packet in one part inside frame, can callback directly from frame data
                m_callback(reader, false);
                m_packetExtractedCount++;
                foundFullPacket = true;

                // Move data pointer to read next packet
                dataPtr += packetLength;
                bytesRemaining -= packetLength;
            }
        }

        if(!foundFullPacket)
        {
            // Packet starts in frame, but is not complete, copy its start into packet buffer
            m_packetBytes.clear();
            m_packetBytes.insert(m_packetBytes.end(), dataPtr, dataPtr+bytesRemaining);
            dataPtr += bytesRemaining;
            bytesRemaining = 0;
        }
    }
}

// Indicates to the packet extractor that there is no more frames
void PacketExtractor::End()
{
    // If a packet was being reconstructed, it is lost
    if (m_packetBytes.size() != 0)
    {
        // Check if only zero data (fill) that can be ignored
        bool isNonZero = false;
        for (auto& v : m_packetBytes)
            if (v != 0)
                isNonZero = true;

        // Check if data is a start of an idle packet (APID 0x7FFF, first two header bytes)
        bool isIdlePacket = (m_packetBytes[0] == 0x07) && (m_packetBytes.size() < 2 || m_packetBytes[1] == 0xFF);

        if (isNonZero && !isIdlePacket)
        {
            logger << warn << "Packet extraction ended with incomplete packet being reconstructed (got " << (uint64_t)m_packetBytes.size() << " bytes)." << endl;
            logger << warn << (int)m_packetBytes[0] << "-" << (int)m_packetBytes[1] << "-" << (int)m_packetBytes[2] << "-" << (int)m_packetBytes[3] << "-" << endl;
            m_lostPacketCount++;
        }
    }
}