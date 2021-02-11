/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Declaration of the NctrsFileReader class
*
*  @author Florian George ELSE
*
*  @version 6.0   2016-06-24 FGE  Initial implementation, can iterate the NCTRS Telemetry Data Units from a NCTRS file loading in memory.
*
*/

#pragma once

// include standard header files
#include <cstdint>   // For explicit size integers (e.g. uint8_t, uint16_t)

// include headers of this module
#include "CcsdsFieldReader.hxx"
#include "CcsdsFrameReader.hxx"


/** ****************************************************************************
*  @ingroup ccsds_telemetry
*  @author Florian George ELSE
*
*  @brief NCTRS Telemetry Data Unit.
*/
struct NctrsTmDu
{
    /// Packet Size field.
    uint32_t PacketSize;
    /// Spacecraft ID field.
    uint16_t SpacecraftId;
    /// Data Stream Type field.
    uint8_t DataStreamType;
    /// Virtual Channel ID field.
    uint8_t VirtualChannelId;
    /// Route ID field.
    uint16_t RouteId;
    /// Earth Reception Time field.
    uint64_t EarthReceptionTime;
    /// Sequence Flag field.
    uint8_t SequenceFlag;
    /// Quality Flag field.
    uint8_t QualityFlag;
    /// Pointer to the user data of this NCTRS Telemetry Data Unit.
    const uint8_t* UserData;
};


/** ****************************************************************************
*  @ingroup ccsds_telemetry
*  @author Florian George ELSE
*
*  @brief Iterator of the NctrsFileReader class. 
*
*  Iterates each NCTRS Telemetry Data Unit from a NCTRS file.
*/
class NctrsFileReaderIterator
{
private:
    /// Pointer to the current location of the iterator.
    const uint8_t* m_pointer;
    /// Pointer to end boundary of the file.
    const uint8_t* m_end;
    /// Current NCTRS Telemetry Data Unit.
    NctrsTmDu m_current;


    /// Allows NctrsFileReader class to use private constructor.
    friend class NctrsFileReader;

    /// Creates an iterator starting at the specified location.
    /// @param [in] start   Pointer to the first byte of the file.
    /// @param [in] end     Pointer to the first byte past the end of the file.
    NctrsFileReaderIterator(const uint8_t* start, const uint8_t* end);

    /// Reads the values of the current NCTRS header.
    void readCurrent();

public:
    /// Returns the current NCTRS header.
    /// @return The values of the current NCTRS header.
    const NctrsTmDu& operator*() const;

    /// Moves the iterator to the next NCTRS header.
    /// @return This iterator moved to the next NCTRS header.
    NctrsFileReaderIterator& operator++();

    /// Returns whether another iterator points to the same location.
    /// @param [in] other   The other iterator.
    bool operator==(const NctrsFileReaderIterator& other) const;

    /// Returns whether another iterator points to a different location.
    /// @param [in] other   The other iterator.
    bool operator!=(const NctrsFileReaderIterator& other) const;
};

/** ****************************************************************************
*  @ingroup ccsds_telemetry
*  @author Florian George ELSE
*
*  @brief Provides iteration of the NCTRS Telemetry Data Units in a NCTRS file.
*
*  No memory allocation or copy are made by this class, it takes a memory pointer 
*  and provides access NCTRS headers and Telemetry Transfer Frames.
*  Ownership of the memory block belong to the caller and must remain valid to
*  read data.
*/
class NctrsFileReader
{
private:
    /// Pointer to the first byte of the file.
    const uint8_t* m_begin;
    /// Pointer to the first byte past the end of the file.
    const uint8_t* m_end;

public:
    /// Length in octets of a NCTRS header.
    static const size_t NctrsHeaderLength = 20;

    /// @brief Creates a NCTRS File Reader for the file.
    /// @param [in] data    Pointer to the first byte of the memory buffer/mapped file.
    /// @param [in] length  The number of bytes in the memory buffer/mapped file.
    NctrsFileReader(const uint8_t* data, size_t length);

    /// The iterator type.
    typedef NctrsFileReaderIterator const_iterator;

    /// Gets an iterator that points to the first NCTRS header of the file.
    NctrsFileReaderIterator begin() { return NctrsFileReaderIterator(m_begin, m_end); }
    /// Gets an iterator that points past the last NCTRS header of the file.
    NctrsFileReaderIterator end() { return NctrsFileReaderIterator(m_end, m_end); }
};

