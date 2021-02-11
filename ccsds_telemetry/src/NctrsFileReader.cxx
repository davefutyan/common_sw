/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Implementation of the NctrsFileReader class
*
*  @author Florian George ELSE
*
*  @version 6.0   2016-06-24 FGE  Initial implementation, can iterate the NCTRS Telemetry Data Units from a NCTRS file loading in memory.
*
*/

#include "NctrsFileReader.hxx"

using namespace std;


//==============================
// NctrsFileReader class

NctrsFileReader::NctrsFileReader(const uint8_t* data, size_t length)
    : m_begin(data), m_end(data + length)
{
}


//==============================
// NctrsFileReaderIterator class

NctrsFileReaderIterator::NctrsFileReaderIterator(const uint8_t* start, const uint8_t* end)
    : m_pointer(start), m_end(end)
{
    readCurrent();
}

// Reads the values of the current NCTRS header.
void NctrsFileReaderIterator::readCurrent()
{
    CcsdsFieldReader reader(m_pointer, NctrsFileReader::NctrsHeaderLength);
    m_current.PacketSize = reader.Read<uint32_t>();
    m_current.SpacecraftId = reader.Read<uint16_t>();
    m_current.DataStreamType = reader.Read<uint8_t>();
    m_current.VirtualChannelId = reader.Read<uint8_t>();
    m_current.RouteId = reader.Read<uint16_t>();
    m_current.EarthReceptionTime = reader.Read<uint64_t>();
    m_current.SequenceFlag = reader.Read<uint8_t>();
    m_current.QualityFlag = reader.Read<uint8_t>();
    m_current.UserData = m_pointer + NctrsFileReader::NctrsHeaderLength;
}

// Returns the current NCTRS header.
const NctrsTmDu& NctrsFileReaderIterator::operator*() const
{
    return m_current;
}

// Moves the iterator to the next NCTRS header.
NctrsFileReaderIterator& NctrsFileReaderIterator::operator++()
{
    m_pointer += m_current.PacketSize;
    if (m_pointer >= m_end)
        m_pointer = m_end;
    else
        readCurrent();
    return *this;
}

// Returns whether another iterator points to the same location.
bool NctrsFileReaderIterator::operator==(const NctrsFileReaderIterator& other) const
{
    return (m_pointer == other.m_pointer);
}

// Returns whether another iterator points to a different location.
bool NctrsFileReaderIterator::operator!=(const NctrsFileReaderIterator& other) const
{
    return (m_pointer != other.m_pointer);
}
