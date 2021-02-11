/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Implementation of the CCSDS packet CRC computation function
*
*  @author Florian George ELSE
*
*  @version 1.0   2015-04-17 FGE first released version
*
*  CRC computation code taken and adapted from annex A.1 of the ECSS-E-70-41A
*  (30 january 2003 issue) standard for CCSDS packets.
*/

#include "CcsdsPacketCrc.hxx"

// CRC look-up table
static uint16_t s_lookUpTable[256] = { 0 };

// Initializes the look-up table used by CcsdsPacketComputeCrc.
static void initCcsdsPacketCrcTable(void)
{
    uint16_t i, tmp;

    for (i = 0; i<256; i++)
    {
        tmp = 0;
        if ((i & 1) != 0)
            tmp = tmp ^ 0x1021;
        if ((i & 2) != 0)
            tmp = tmp ^ 0x2042;
        if ((i & 4) != 0)
            tmp = tmp ^ 0x4084;
        if ((i & 8) != 0)
            tmp = tmp ^ 0x8108;
        if ((i & 16) != 0)
            tmp = tmp ^ 0x1231;
        if ((i & 32) != 0)
            tmp = tmp ^ 0x2462;
        if ((i & 64) != 0)
            tmp = tmp ^ 0x48C4;
        if ((i & 128) != 0)
            tmp = tmp ^ 0x9188;
        s_lookUpTable[i] = tmp;
    }
}

// Computes the CCSDS packet CRC of a single byte with a specified syndrome.
static uint16_t ccsdsPacketCrcComputeByte(uint8_t data, uint16_t syndrome)
{
    return (((syndrome << 8) & 0xFF00) ^ s_lookUpTable[(((syndrome >> 8) ^ data) & 0x00FF)]);
}


// Computes the CCSDS packet CRC of a specified memory buffer.
uint16_t CcsdsPacketCrcCompute(const uint8_t *buffer, size_t length)
{
    // Lazy initialize CRC look-up table if needed
    if (s_lookUpTable[0] == 0)
        initCcsdsPacketCrcTable();

    // Compute checksum for data buffer
    uint16_t syndrome = 0xFFFF; // Init syndrome
    for (size_t i = 0; i < length; i++)
        syndrome = ccsdsPacketCrcComputeByte(buffer[i], syndrome);
    return syndrome;
}

