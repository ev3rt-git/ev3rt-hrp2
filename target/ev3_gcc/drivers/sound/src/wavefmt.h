/**
 * \file    wavefmt.h
 * \brief	Header for Waveform Audio File Format (WAVE)
 * \details See 'Multimedia Programming Interface and Data Specifications 1.0'.
 * \author	ertl-liyixiao
 */

#pragma once

typedef uint16_t	  WORD;
typedef unsigned long DWORD;
typedef unsigned char BYTE;

typedef DWORD  FOURCC; // Four-character code
typedef FOURCC CKID;   // Four-character-code chunk identifier
typedef DWORD  CKSIZE; // 32-bit unsigned size value

#if 0
typedef struct { // Chunk structure
	CKID   ckID; 		   // Chunk type identifier
	CKSIZE ckSize; 		   // Chunk size field (size of ckData)
	BYTE   ckData[ckSize]; // Chunk data
} CK;
#endif

typedef struct { // Chunk header structure
#define CHUNK_ID_RIFF ('R' | ('I' << 8) | ('F' << 16) | ('F' << 24))
#define CHUNK_ID_FMT  ('f' | ('m' << 8) | ('t' << 16) | (' ' << 24))
#define CHUNK_ID_DATA ('d' | ('a' << 8) | ('t' << 16) | ('a' << 24))
	CKID   ckID; 		   // Chunk type identifier
	CKSIZE ckSize; 		   // Chunk size field (size of ckData)
} ChunkHeader;

typedef struct { // WAVE format chunk data structure
#define WAVE_FORMAT_PCM (0x0001)
	WORD  wFormatTag;			  // Format category
	WORD  wChannels; 			  // Number of channels
	DWORD dwSamplesPerSec; 	      // Sampling rate
	DWORD dwAvgBytesPerSec; 	  // For buffer estimation
	WORD  wBlockAlign; 			  // Data block size
	BYTE  formatSpecificFields[]; // Flexible array member for <format-specific-fields>
} FormatChunkData;

typedef struct { // PCM format specific fields
	WORD wBitsPerSample; // Sample size
} PCMFormatSpecificFields;

