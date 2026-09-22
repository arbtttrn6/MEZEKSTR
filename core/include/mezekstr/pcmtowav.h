#ifndef PCMTOWAV_H
#define PCMTOWAV_H

#include "./typedef.h"
#include "./wavdef.h"


namespace MEZEkstr
{

BYTE* ReadPcmFromFile(const char* szInputPath, DWORD dwOffset, DWORD dwSize);

int SaveIsactPcmToWav(const char* szOutputPath,
					  const BYTE* pbyPcmData,
					  DWORD dwPcmSize,
					  WORD wNumChannels,
					  DWORD dwSampleRate,
					  WORD wBitsPerSample,
					  EISACTCompressionFormat eFormat);

int ConvertIsactFileToWav(const char* szSourcePath,
						  DWORD dwSourceOffset,
						  DWORD dwSourceSize,
						  const char* szOutputPath,
						  WORD wNumChannels,
						  DWORD dwSampleRate,
						  WORD wBitsPerSample,
						  EISACTCompressionFormat eFormat);

int ParseIsactBankMetadata(const BYTE* pbyData,
						   DWORD dwDataSize,
						   SIsactAudioMetadata& sOutMeta);

DWORD EnumerateTracks(const BYTE* pbyData,
					  DWORD dwDataSize,
					  SIsactTrackInfo* pOutTracks,
					  DWORD dwMaxTracks);

int SaveIsactOggToFile(const char* szOutputPath,
					   const BYTE* pbyOggData,
					   DWORD dwOggSize);

}

#endif // PCMTOWAV_H
