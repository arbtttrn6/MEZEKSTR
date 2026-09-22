#ifndef WAVDEF_H
#define WAVDEF_H

#include "./typedef.h"


namespace MEZEkstr
{

#pragma pack(push, 1)

struct SWavHeader
{
	// RIFF Chunk
	char  szChunkId[4];	   // "RIFF"
	DWORD dwChunkSize;		// filesize - 8 byte
	char  szFormat[4];		// "WAVE"

	// fmt Sub-chunk
	char  szSubchunk1Id[4];   // "fmt "
	DWORD dwSubchunk1Size;	// 16 for PCM
	WORD  wAudioFormat;	   // 1 for PCM (uncompressed)
	WORD  wNumChannels;	   // 1 = mono, 2 = stereo
	DWORD dwSampleRate;	   // SampleRate (ex. 44100)
	DWORD dwByteRate;		 // dwSampleRate * wBlockAlign
	WORD  wBlockAlign;		// (wNumChannels * wBitsPerSample) / 8
	WORD  wBitsPerSample;	 // Bits per Sample (8, 16, 24, 32)

	// data Sub-chunk
	char  szSubchunk2Id[4];   // "data"
	DWORD dwSubchunk2Size;	// size of PCM in bytes
};

#pragma pack(pop)

enum EISACTCompressionFormat
{
	ISACT_CF_PCM		  = 0,
	ISACT_CF_IMA4ADPCM	  = 1,
	ISACT_CF_OGGVORBIS	  = 2,
	ISACT_CF_WMA		  = 3,
	ISACT_CF_XMA		  = 4,
	ISACT_CF_MSMP3		  = 5,  // PS3 only - MSF MP3
	ISACT_CF_MSADPCM	  = 6,  // PS3 only - MSF ADPCM
	ISACT_CF_MSPCMBIG	  = 7   // PS3 only - MSF PCM Big Endian
};


#pragma pack(push, 1)

// "cmpi" 24 bytes
struct SCompressionInfo
{
	DWORD dwCurrentFormat;     // EISACTCompressionFormat
	DWORD dwTargetFormat;      // EISACTCompressionFormat
	DWORD dwTotalSize;         // Size of compressed data
	DWORD dwPacketSize;        // Size of packet
	float fCompressionRatio;   // Ratio of compression
	float fCompressionQuality; // Quality of compression
};

// "sinf" 20 bytes
struct SSampleInfo
{
	DWORD dwBufferOffset;      // Offset of buffer
	DWORD dwTimeLength;        // Length of time
	DWORD dwSamplesPerSecond;  // Samples per second (Sample Rate)
	DWORD dwByteLength;        // Length of PCM-data in bytes
	WORD  wBitsPerSample;      // Bits per sample (8, 16, 24, 32 bytes)
	WORD  wPadding;            // Padding to 4 bytes (20 bytes)
};

#pragma pack(pop)

// isact
struct SIsactAudioMetadata
{
	DWORD dwSampleRate;      // from 'sinf'
	DWORD dwByteLength;      // from 'sinf'
	WORD  wChannels;         // from 'chnk' or default 1
	WORD  wBitsPerSample;    // from 'sinf'
	DWORD dwSampleOffset;    // from 'soff' external isbs offset
	EISACTCompressionFormat eFormat; // frmo 'cmpi'
	int   bHasOffset;
};

struct SIsactTrackInfo
{
	char  szName[128];
	DWORD dwSampleRate;
	DWORD dwByteLength;
	DWORD dwPhysicalSize; // PhysSize
	WORD  wChannels;
	WORD  wBitsPerSample;
	DWORD dwDataOffset;
	DWORD dwSampleOffset;
	EISACTCompressionFormat eFormat;
	int   bHasOffset;
	int   bIsEmbedded;
};

}

#endif // WAVDEF_H
