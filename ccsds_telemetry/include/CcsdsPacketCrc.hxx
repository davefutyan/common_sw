/** ****************************************************************************
*  @file
*
*  @ingroup ccsds_telemetry
*  @brief Declaration of the CCSDS packet CRC computation function
*
*  @author Florian George ELSE
*
*  @version 1.0   2015-04-17 FGE first released version
*
*  CRC computation code taken and adapted from annex A.1 of the ECSS-E-70-41A
*  (30 january 2003 issue) standard for CCSDS packets.
*/

#pragma once

#include <stdint.h>   // For explicit size integer types (e.g. uint8_t, uint16_t)
#include <cstddef>    // For size_t type


/** ****************************************************************************
*  @ingroup ccsds_telemetry
*  @author Florian George ELSE
*
*  @brief Computes the CCSDS packet CRC of a specified memory buffer.
*
*  @param [in] buffer   Pointer to the memory buffer.
*  @param [in] length   Length of the memory buffer.
*/
extern uint16_t CcsdsPacketCrcCompute(const uint8_t *buffer, size_t length);

