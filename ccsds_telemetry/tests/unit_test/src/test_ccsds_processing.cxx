#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <string>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdint.h>   // For explicit size integers (e.g. uint8_t, uint16_t)
#include <stdexcept>  // For standard base exceptions such as runtime_exception

#include <boost/iostreams/device/mapped_file.hpp>

#include "CcsdsPacketCrc.hxx"
#include "CcsdsFieldReader.hxx"
#include "CcsdsPacketReader.hxx"
#include "CcsdsFrameReader.hxx"
#include "PacketExtractor.hxx"
#include "NctrsFileReader.hxx"

using namespace std;
using namespace boost::unit_test;


const string testPacketFileName = "resources/TM_HK_def_report_0.bin";
const string testFrameFileName = "resources/Frame_Sample.bin";
const string testNctrsFilePath = "resources/NCTRS_Sample.bin";


#define P_EX_MESS(MESSAGE) [](const std::runtime_error& ex) { return string(ex.what()).find(MESSAGE)!=string::npos; }


// Wrapper of boost::iostreams::mapped_file_source to expose read-only uint8_t buffer and size easily
class MappedFile
{
private:
    boost::iostreams::mapped_file_source mappedFile;
public:
    MappedFile(const string& fileName) { mappedFile.open(fileName); }
    size_t size() { return mappedFile.size(); }
    const uint8_t* buffer() { return reinterpret_cast<const uint8_t*>(mappedFile.data()); }
};


// ********************************** Packets **********************************

// Test CCSDS CRC computation function
BOOST_AUTO_TEST_CASE ( testCrc )
{
    // Validate CRC function with test cases from ECSS-E-70-41A (30 January 2003) Annex A.1.5

    uint8_t data1[] = {0x00, 0x00};
    uint8_t data2[] = {0x00, 0x00, 0x00};
    uint8_t data3[] = {0xAB, 0xCD, 0xEF, 0x01};
    uint8_t data4[] = {0x14, 0x56, 0xF8, 0x9A, 0x00, 0x01};

    uint16_t result1 = 0x1D0F;
    uint16_t result2 = 0xCC9C;
    uint16_t result3 = 0x04A2;
    uint16_t result4 = 0x7FD5;

    BOOST_CHECK( CcsdsPacketCrcCompute(data1, sizeof(data1)) == result1 );
    BOOST_CHECK( CcsdsPacketCrcCompute(data2, sizeof(data2)) == result2 );
    BOOST_CHECK( CcsdsPacketCrcCompute(data3, sizeof(data3)) == result3 );
    BOOST_CHECK( CcsdsPacketCrcCompute(data4, sizeof(data4)) == result4 );
}

// Test CcsdsFieldReader exceptions
BOOST_AUTO_TEST_CASE ( testFieldReaderExceptions )
{
    // Null buffer
    BOOST_CHECK_THROW(
        CcsdsFieldReader reader(nullptr, 0),
        std::invalid_argument
    );
}

// Test reading of all headers of an HK packet
BOOST_AUTO_TEST_CASE ( testPacketReading1 )
{
    // Map packet file into memory
    MappedFile mappedFile(testPacketFileName);

    // Construct CCSDS packet reader and validate that buffer contains valid packet
    CcsdsPacketReader packetReader(mappedFile.buffer(), mappedFile.size());
    packetReader.ValidatePacket();

    // Print packets fields to console for debug
    std::cout << "Packet Length: "   << packetReader.GetPacketLength() << std::endl;
    std::cout << "APID: 0x" << hex   << packetReader.GetApid() << dec << std::endl;
    std::cout << "Sequence Count: "  << static_cast<uint16_t>(packetReader.GetSequenceCount()) << std::endl;
    std::cout << "Service Type: "    << static_cast<uint16_t>(packetReader.GetServiceType()) << std::endl;
    std::cout << "Service Subtype: " << static_cast<uint16_t>(packetReader.GetServiceSubtype()) << std::endl;
    std::cout << "Destination ID: "  << static_cast<uint16_t>(packetReader.GetDestinationId()) << std::endl;
    std::cout << "Time: 0x" << hex   << packetReader.GetTime() << dec << std::endl;

    // Assert correct values
    BOOST_CHECK( packetReader.GetPacketLength()   == 50 );
    BOOST_CHECK( packetReader.GetApid()           == 0x3c1 );
    BOOST_CHECK( packetReader.GetSequenceCount()  == 0 );
    BOOST_CHECK( packetReader.GetServiceType()    == 3 );
    BOOST_CHECK( packetReader.GetServiceSubtype() == 25 );
    BOOST_CHECK( packetReader.GetDestinationId()  == 0 );
    BOOST_CHECK( packetReader.GetPacketLength()   == 50 );
    BOOST_CHECK( packetReader.GetTime()           == 0x123456789ab );
}


// Test reading housekeeping
BOOST_AUTO_TEST_CASE ( testHkReading1 )
{
   // Map packet file into memory
    MappedFile mappedFile(testPacketFileName);

    // Construct CCSDS packet reader and validate that buffer contains valid packet
    CcsdsPacketReader packetReader(mappedFile.buffer(), mappedFile.size());
    packetReader.ValidatePacket();

    // Get data pointer and construct a field reader
    uint16_t dataLength;
    auto dataPointer = packetReader.GetDataPointer(dataLength);
    CcsdsFieldReader reader(dataPointer, dataLength);

    // Assert correct values
    BOOST_CHECK( reader.GetCurrentOffset()  == 0 );
    BOOST_CHECK( reader.Read<uint16_t>() == 0x0001 );
    BOOST_CHECK( reader.Read<uint32_t>() == 0x00050005 );
    BOOST_CHECK( reader.Read<uint8_t>()  == 0x80 );
    // Read 7 bytes, offset should reflect that
    BOOST_CHECK( reader.GetCurrentOffset()  == 7 );
}


// Test CcsdsPacketReader exceptions
BOOST_AUTO_TEST_CASE ( testPacketReaderNullBufer )
{
    MappedFile mappedFile(testPacketFileName);

    // Null buffer
    BOOST_CHECK_THROW(
        CcsdsPacketReader packetReader(nullptr, mappedFile.size()),
        std::invalid_argument
    );
}

BOOST_AUTO_TEST_CASE ( testPacketReaderZeroSizeBuffer )
{
    MappedFile mappedFile(testPacketFileName);

    // Size too small for any packet
    BOOST_CHECK_THROW(
        CcsdsPacketReader packetReader(mappedFile.buffer(), 0),
        std::invalid_argument
    );
}

BOOST_AUTO_TEST_CASE ( testPacketReaderShorterBuffer )
{
    MappedFile mappedFile(testPacketFileName);

    // Size too small for this packet
    BOOST_CHECK_THROW(
        CcsdsPacketReader packetReader(mappedFile.buffer(), mappedFile.size() - 1);
        packetReader.ValidatePacket(),
        std::runtime_error
    );
}        


// Test fixture that contains a writable packet for corruption/invalid tests
struct WritablePacketFixture
{
    uint8_t* buffer;
    size_t size;
    WritablePacketFixture()
    {
        // Map packet file into memory
         MappedFile mappedFile(testPacketFileName);
         size = mappedFile.size();
         // Copy packet into buffer
         buffer = new uint8_t[size];
         memcpy(buffer, mappedFile.buffer(), size);
    }
    ~WritablePacketFixture()
    {
        delete[] buffer;
    }
    void RecomputeCrc()
    {
        uint16_t crc = CcsdsPacketCrcCompute(buffer, size-2);
        buffer[size-2] = (uint8_t)(crc >> 8);
        buffer[size-1] = (uint8_t)(crc);
    }
};

BOOST_FIXTURE_TEST_CASE ( testCorruptedCrc, WritablePacketFixture )
{
    buffer[size-1]++; // corrupt CRC field
    
    CcsdsPacketReader packetReader(buffer, size);
    BOOST_CHECK_THROW(
        packetReader.ValidatePacket(),
        std::runtime_error
    );
}

BOOST_FIXTURE_TEST_CASE ( testInvalidVersion, WritablePacketFixture )
{
    buffer[0] |= 0xE0; // version number != 0
    RecomputeCrc();
    
    CcsdsPacketReader packetReader(buffer, size);
    BOOST_CHECK_THROW(
        packetReader.ValidatePacket(),
        std::runtime_error
    );
}

BOOST_FIXTURE_TEST_CASE ( testNotTelemetry, WritablePacketFixture )
{
    buffer[0] |= 0x10; // packet type != 0
    RecomputeCrc();
    
    CcsdsPacketReader packetReader(buffer, size);
    BOOST_CHECK_THROW(
        packetReader.ValidatePacket(),
        std::runtime_error
    );
}

BOOST_FIXTURE_TEST_CASE ( testNoDataHeader, WritablePacketFixture )
{
    buffer[0] &= ~0x08; // data header == 0
    RecomputeCrc();
    
    CcsdsPacketReader packetReader(buffer, size);
    BOOST_CHECK_THROW(
        packetReader.ValidatePacket(),
        std::runtime_error
    );
}

BOOST_FIXTURE_TEST_CASE ( testSegmented, WritablePacketFixture )
{
    buffer[2] &= ~0xC0; // Segmentation Flags != 0b11
    RecomputeCrc();
    
    CcsdsPacketReader packetReader(buffer, size);
    BOOST_CHECK_THROW(
        packetReader.ValidatePacket(),
        std::runtime_error
    );
}



// ********************************** Frames ***********************************

// Test fixture that contains a writable frame for CcsdsFrameReader tests
struct WritableFrameFixture
{
    uint8_t* buffer;
    size_t size;
    WritableFrameFixture()
    {
        // Map packet file into memory
         MappedFile mappedFile(testFrameFileName);
         size = mappedFile.size();
         // Copy packet into buffer
         buffer = new uint8_t[size];
         memcpy(buffer, mappedFile.buffer(), size);
    }
    ~WritableFrameFixture()
    {
        delete[] buffer;
    }
};

// Test reading of a valid CCSDS Frame using CcsdsFrameReader
BOOST_FIXTURE_TEST_CASE ( testFrameReading, WritableFrameFixture )
{
    // Read and validate frame
    CcsdsFrameReader frameReader(buffer, size);
    frameReader.ValidateFrame();

    // Check value of headers
    BOOST_CHECK( frameReader.GetScId() == 210 );
    BOOST_CHECK( (int)frameReader.GetVcId() == 1 );
    BOOST_CHECK( (int)frameReader.GetMcFrameCount() == 0xAA );
    BOOST_CHECK( (int)frameReader.GetVcFrameCount() == 0xBB );
    BOOST_CHECK( frameReader.GetFirstHeaderPointer() == 0x555 );
}

// Test constructor of CcsdsFrameReader
BOOST_AUTO_TEST_CASE ( testCcsdsFrameReaderCtor )
{
    uint8_t buffer[100];

    // Null buffer
    BOOST_CHECK_THROW(CcsdsFrameReader(nullptr, 100), std::invalid_argument);
    
    // Buffer too small
    BOOST_CHECK_THROW(CcsdsFrameReader(buffer, 5), std::invalid_argument);
    
    // OK construction
    BOOST_CHECK_NO_THROW(CcsdsFrameReader(buffer, 100));
}

// Test CcsdsFrameReader::ValidateFrame
BOOST_AUTO_TEST_CASE ( testCcsdsFrameReaderValidate )
{
    uint8_t buffer[50] = { 0x00, 0x01, 0xAB, 0xCD, 0x9D, 0x55, 0xAA, 0xAA, 0xAA, 0xAA };
    
    CcsdsFrameReader frameReader(buffer, 50);
    
    // Frame as-is is valid
    frameReader.ValidateFrame();
    
    // Invalid Transfer Frame Version Number
    buffer[0] |= 0x80;
    BOOST_CHECK_EXCEPTION(frameReader.ValidateFrame(), std::runtime_error, P_EX_MESS("Frame Version Number"));
    buffer[0] = 0x00;
    frameReader.ValidateFrame();
    
    // Invalid Transfer Frame Secondary Header Flag
    buffer[4] &= ~0x80;
    BOOST_CHECK_EXCEPTION(frameReader.ValidateFrame(), std::runtime_error, P_EX_MESS("Frame Secondary Header Flag"));
    buffer[4] = 0x9D;
    frameReader.ValidateFrame();
    
    // Invalid Synchronization Flag
    buffer[4] |= 0x40;
    BOOST_CHECK_EXCEPTION(frameReader.ValidateFrame(), std::runtime_error, P_EX_MESS("Synchronization Flag"));
    buffer[4] = 0x9D;
    frameReader.ValidateFrame();
    
    // Invalid Packet Order Flag
    buffer[4] |= 0x20;
    BOOST_CHECK_EXCEPTION(frameReader.ValidateFrame(), std::runtime_error, P_EX_MESS("Packet Order Flag"));
    buffer[4] = 0x9D;
    frameReader.ValidateFrame();
    
    // Invalid Segment Length Identifier
    buffer[4] &= ~0x10;
    BOOST_CHECK_EXCEPTION(frameReader.ValidateFrame(), std::runtime_error, P_EX_MESS("Segment Length Identifier"));
    buffer[4] = 0x9D;
    frameReader.ValidateFrame();
}


// ********************************** NCTRS ************************************

// Test reading of a NCTRS telemetry file
BOOST_AUTO_TEST_CASE ( testNctrsReading )
{
    // Create packet extractor, passing packets to readPacket method with the file statistics object as well
    PacketExtractor packetExtractor(VirtualChannelHousekeeping, [&](CcsdsPacketReader packetReader, bool reconstructed)
    {
        // Ensure packet is valid
        packetReader.ValidatePacket();
    });

    // Map packet file into memory
    MappedFile mappedFile(testNctrsFilePath);

    // Create a NCTRS File Reader and read the frames
    NctrsFileReader nctrsFileReader(mappedFile.buffer(), mappedFile.size());
    int frameCount = 0;
    for (const auto& nctrsTmDu : nctrsFileReader)
    {
        // Sample file contains ASM and Reed-Solomon symbols, skip them
        auto start = nctrsTmDu.UserData + 4;
        auto length = nctrsTmDu.PacketSize - NctrsFileReader::NctrsHeaderLength - 4 - 160;

        // Create frame reader and ensure frame is valid
        CcsdsFrameReader frameReader(start, (uint16_t)length);
        frameReader.ValidateFrame();
        frameCount++;

        // Pass frame to packet extractor, failure here will stop conversion
        packetExtractor.Add(frameReader);
    }
    packetExtractor.End();
    
    // NCTRS sample file contains 4 frames
    BOOST_CHECK( frameCount == 4 );
    // No frame is missing
    BOOST_CHECK( packetExtractor.GetMissingFrameCount() == 0 );
    // One packet is lost, the one started in the last frame of the sample
    BOOST_CHECK( packetExtractor.GetLostPacketCount() == 1 );
}

// Test the NctrsFileReaderIterator class
BOOST_AUTO_TEST_CASE ( testNctrsFileReaderIterator )
{
    // Map packet file into memory
    MappedFile mappedFile(testNctrsFilePath);
    
    // Create a NCTRS File Reader
    NctrsFileReader nctrsFileReader(mappedFile.buffer(), mappedFile.size());
    
    // Construct two iterators
    auto iterator1 = nctrsFileReader.begin();
    auto iterator2 = nctrsFileReader.begin();
    
    // Test inequality by moving only one
    ++iterator1;
    BOOST_CHECK ( iterator1 != iterator2 );
    
    // Test equality by moving the other one
    ++iterator2;
    BOOST_CHECK ( iterator1 == iterator2 );
    
    // Comparison of begin and end
    BOOST_CHECK ( nctrsFileReader.begin() == nctrsFileReader.begin() );
    BOOST_CHECK ( nctrsFileReader.begin() != nctrsFileReader.end() );
    BOOST_CHECK ( nctrsFileReader.end() == nctrsFileReader.end() );
}


// ***************************** PacketExtractor *******************************
BOOST_AUTO_TEST_CASE ( testPacketExtractorWrongVc )
{
    // Packet Extractor for VC2
    PacketExtractor packetExtractor(2, [](CcsdsPacketReader, bool) { });
    
    // Simple frame for VC1
    uint8_t buffer[1115] = { 0x00, 0x03, 0xAB, 0xCD, 0x98, 0x00, 0x03 };
    CcsdsFrameReader frameReader(buffer, sizeof(buffer));
    
    BOOST_CHECK_EXCEPTION(packetExtractor.Add(frameReader), std::runtime_error, P_EX_MESS("virtual channel"));
}


// Test fixture that contains a packet extractor and packet counter
struct PacketExtractorFixture
{
    uint32_t packetsCount;
    PacketExtractor packetExtractor;
    
    PacketExtractorFixture()
        : packetExtractor(1, std::bind(&PacketExtractorFixture::packetCallback, this, std::placeholders::_1, std::placeholders::_2))
    {
        packetsCount = 0;
    }
    
    void packetCallback(CcsdsPacketReader packetReader, bool reconstructed)
    {
        // Ensure packet is valid
        packetReader.ValidatePacket();
        packetsCount++;
    }
};


// Test missing frame scenarios
BOOST_FIXTURE_TEST_CASE ( testPacketExtractorMissingFrame, PacketExtractorFixture )
{
    cout << "-- testPacketExtractorMissingFrame --" << endl;

    // Empty frame, only idle data
    uint8_t buffer[1115] = { 0x00, 0x03, 0x01, 0x01, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //                       |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|
    CcsdsFrameReader frameReader(buffer, sizeof(buffer));
    frameReader.ValidateFrame();

    
    // Read two frames, no missing, no packet
    packetExtractor.Add(frameReader);
    // Increment counters
    buffer[2] = buffer[3] = 2;
    packetExtractor.Add(frameReader);
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    // Miss one frame
    buffer[2] = buffer[3] = 4;
    packetExtractor.Add(frameReader);
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    
    // Miss one frame but with packet started
    // Frame with start of very big packet that doesn't fit in frame
    uint8_t buffer2[1115] = { 0x00, 0x03, 0x05, 0x05, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0xC0, 0x00, 0x04, 0x48, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //                        |-- ID --|   MC    VC         Flags+FHP   |- Secondary Header -|  |-- TM --|  |- Seq  -|  | Length |  DFH    ST   SST   DID   |--             Time           --|  ...
    packetExtractor.Add(CcsdsFrameReader(buffer2, sizeof(buffer2)));
    // No change, only started a packet
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    // Increment counters for 2 missed frames, packet lost as end was in missing frame
    buffer[2] = buffer[3] = 8;
    packetExtractor.Add(frameReader);
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 3 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    
    // No change in values after call to End
    packetExtractor.End();
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 3 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );

    cout << "------" << endl;
}

// Test protocol violation of having next frame not containing end of started packet
BOOST_FIXTURE_TEST_CASE ( testPacketExtractorInvalidFhpZero, PacketExtractorFixture )
{
    cout << "-- testPacketExtractorInvalidFhpZero --" << endl;
    
    // Frame with start of very big packet that doesn't fit in frame
    uint8_t buffer1[1115] = { 0x00, 0x03, 0x01, 0x01, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0xC0, 0x00, 0x04, 0x48, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|  |-- TM --|  |- Seq  -|  | Length |  DFH    ST   SST   DID   |--             Time           --|  ...
    CcsdsFrameReader frameReader1(buffer1, sizeof(buffer1));
    frameReader1.ValidateFrame();
    packetExtractor.Add(frameReader1);

    // Nothing yet, only started packet
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );

    
    // Empty frame, only idle data
    uint8_t buffer2[1115] = { 0x00, 0x03, 0x02, 0x02, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|
    CcsdsFrameReader frameReader2(buffer2, sizeof(buffer2));
    frameReader2.ValidateFrame();
    packetExtractor.Add(frameReader2);
    
    // Invalid FHP, not end of packet, lost packet
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );

    cout << "------" << endl;
}

// Test protocol violation of having next frame containing end of not-started packet
BOOST_FIXTURE_TEST_CASE ( testPacketExtractorInvalidFhpNonZero, PacketExtractorFixture )
{
    cout << "-- testPacketExtractorInvalidFhpNonZero --" << endl;
    
    // Frame with single packet, then idle data
    uint8_t buffer[1115] = { 0x00, 0x03, 0xAB, 0xCD, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0xC0, 0x00, 0x00, 0x0B, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE5, 0x5C };
    //                       |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|  |-- TM --|  |- Seq  -|  | Length |  DFH    ST   SST   DID   |--             Time           --|  |- CRC  -|
    CcsdsFrameReader frameReader(buffer, sizeof(buffer));
    frameReader.ValidateFrame();

    
    // Read frame with packet
    packetExtractor.Add(frameReader);
    BOOST_CHECK ( packetsCount == 1 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    // Set FHP inside frame and increment counters
    buffer[5] = 0xFF;
    buffer[2]++;
    buffer[3]++;
    packetExtractor.Add(frameReader);
    // Counter don't change, protocol violation but no packet was being reconstructed, however packet before FHP is lost
    BOOST_CHECK ( packetsCount == 1 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    // Set FHP to NoPacketStart and increment counters
    buffer[4] = 0x9F;
    buffer[5] = 0xFF;
    buffer[2]++;
    buffer[3]++;
    packetExtractor.Add(frameReader);
    // Counter don't change, protocol violation but no packet was being reconstructed, however packet before FHP is lost
    BOOST_CHECK ( packetsCount == 1 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 2 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    cout << "------" << endl;
}

// Test with large packet spanning on 3 frames
BOOST_FIXTURE_TEST_CASE ( testPacketExtractorLargePacket, PacketExtractorFixture )
{
    cout << "-- testPacketExtractorLargePacket --" << endl;

    // Frame with start of very big packet that doesn't fit in frame
    uint8_t buffer1[1115] = { 0x00, 0x03, 0x01, 0x01, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0xC0, 0x00, 0x08, 0x95, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|  |-- TM --|  |- Seq  -|  | Length |  DFH    ST   SST   DID   |--             Time           --|  ...

    // Frame with only data of current packet, no start
    uint8_t buffer2[1115] = { 0x00, 0x03, 0x02, 0x02, 0x9F, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00  };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|
    
    // Frame with just CRC of large packet end
    uint8_t buffer3[1115] = { 0x00, 0x03, 0x03, 0x03, 0x98, 0x02, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00  };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|
    
    
    // Add 3 frames with our single packet
    packetExtractor.Add(CcsdsFrameReader(buffer1, sizeof(buffer1)));
    packetExtractor.Add(CcsdsFrameReader(buffer2, sizeof(buffer2)));
    packetExtractor.Add(CcsdsFrameReader(buffer3, sizeof(buffer3)));
    packetExtractor.End();
    
    // 1 packet, no loss
    BOOST_CHECK ( packetsCount == 1 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );
    
    cout << "------" << endl;
}

// Test packet missing its end, but no missing frame
BOOST_FIXTURE_TEST_CASE ( testPacketExtractorMissingPacketEnd, PacketExtractorFixture )
{
    cout << "-- testPacketExtractorMissingPacketEnd --" << endl;

    // Frame with start of very big packet that doesn't fit in frame
    uint8_t buffer1[1115] = { 0x00, 0x03, 0x01, 0x01, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0xC0, 0x00, 0x08, 0x95, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|  |-- TM --|  |- Seq  -|  | Length |  DFH    ST   SST   DID   |--             Time           --|  ...
    // Frame with just 2 bytes of current packet, not enough
    uint8_t buffer2[1115] = { 0x00, 0x03, 0x02, 0x02, 0x98, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00  };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|

    // Add 2 frames with our incomplete packet
    packetExtractor.Add(CcsdsFrameReader(buffer1, sizeof(buffer1)));
    packetExtractor.Add(CcsdsFrameReader(buffer2, sizeof(buffer2)));
    packetExtractor.End();
    
    // 0 packet, 1 loss
    BOOST_CHECK ( packetsCount == 0 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );

    cout << "------" << endl;
}

// Test packet missing its end, but no missing frame and not enough data to even known its size
BOOST_FIXTURE_TEST_CASE ( testPacketExtractorMissingPacketEndSmall, PacketExtractorFixture )
{
    cout << "-- testPacketExtractorMissingPacketEndSmall --" << endl;

    // Frame with big packet that makes next packet start with only 2 bytes in this frame
    uint8_t buffer1[1115] = { 0x00, 0x03, 0x01, 0x01, 0x98, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0xC0, 0x00, 0x04, 0x44, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    buffer1[1107] = 0xD0; // CRC
    buffer1[1108] = 0xA6;
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|  |-- TM --|  |- Seq  -|  | Length |  DFH    ST   SST   DID   |--             Time           --|  ...
    // Frame with just 2 bytes of current packet, not enough
    uint8_t buffer2[1115] = { 0x00, 0x03, 0x02, 0x02, 0x98, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00  };
    //                        |-- ID --|   MC    VC   Flags+FHP   |- Secondary Header -|

    // Add 2 frames with our incomplete packet
    packetExtractor.Add(CcsdsFrameReader(buffer1, sizeof(buffer1)));
    packetExtractor.Add(CcsdsFrameReader(buffer2, sizeof(buffer2)));
    packetExtractor.End();
    
    // 1 packet, 1 loss
    BOOST_CHECK ( packetsCount == 1 );
    BOOST_CHECK ( packetExtractor.GetLostPacketCount() == 1 );
    BOOST_CHECK ( packetExtractor.GetMissingFrameCount() == 0 );
    BOOST_CHECK ( packetExtractor.GetPacketExtractedCount() == packetsCount );

    cout << "------" << endl;
}