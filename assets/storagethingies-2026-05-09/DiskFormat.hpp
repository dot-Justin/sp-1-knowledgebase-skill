#pragma once

#include <cstdint>

namespace storage {
namespace format {

/**
 * The Stem Player uses a custom eMMC layout rather than a standard FAT32/exFAT filesystem.
 * The disk starts with a Master Table of Contents (TOC) that points to the albums.
 */

#pragma pack(push, 1)

// Represents a single track/song within an album
struct TrackEntry {
    uint32_t start_sector;       // Logical block address where the audio starts
    uint32_t length_sectors;     // Length of the track in 512-byte sectors
    uint32_t sample_rate;        // Usually 48000
    uint8_t  num_channels;       // Usually 8 (4 stereo stems)
    uint8_t  bit_depth;          // Usually 24
    uint16_t bpm;                // Beats per minute for sync/effects
    char     title[64];          // Null-terminated track title
};

// Represents an album (a collection of tracks)
struct AlbumEntry {
    uint32_t magic;              // e.g., 0x5354454D ("STEM")
    uint32_t num_tracks;
    char     album_name[64];
    char     artist_name[64];
    TrackEntry tracks[32];       // Max 32 tracks per album
};

// The Master Table of Contents at Sector 0
struct MasterTOC {
    uint32_t magic;              // e.g., 0x4B414E4F ("KANO")
    uint32_t version;
    uint32_t num_albums;
    uint32_t active_album_idx;
    uint32_t album_sector_offsets[16]; // Pointers to AlbumEntry structs
};

#pragma pack(pop)

} // namespace format
} // namespace storage
