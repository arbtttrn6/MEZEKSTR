#include "../include/mezekstr/typedef.h"
#include "../include/mezekstr/wavdef.h"
#include "../include/mezekstr/utils.h"

#include <cstdio>


namespace MEZEkstr
{

BYTE* ReadPcmFromFile(const char* szInputPath, DWORD dwOffset, DWORD dwSize)
{
	std::FILE* pFile = NULL;
	BYTE* pbyBuffer = NULL;

	if (!szInputPath || dwSize == 0)
	{
		return NULL;
	}

	pFile = std::fopen(szInputPath, "rb");
	if (!pFile)
	{
		return NULL;
	}

	if (std::fseek(pFile, static_cast<long>(dwOffset), SEEK_SET) != 0)
	{
		std::fclose(pFile);
		return NULL;
	}

	pbyBuffer = new BYTE[dwSize];
	if (!pbyBuffer)
	{
		std::fclose(pFile);
		return NULL;
	}

	if (std::fread(pbyBuffer, 1, dwSize, pFile) != dwSize)
	{
		delete[] pbyBuffer;
		std::fclose(pFile);
		return NULL;
	}

	std::fclose(pFile);
	return pbyBuffer;
}

int SaveIsactPcmToWav(const char* szOutputPath,
					  const BYTE* pbyPcmData,
					  DWORD dwPcmSize,
					  WORD wNumChannels,
					  DWORD dwSampleRate,
					  WORD wBitsPerSample,
					  EISACTCompressionFormat eFormat)
{
	SWavHeader sHeader;
	std::FILE* pFile = NULL;
	BYTE* pbyAllocatedBuffer = NULL;
	const BYTE* pbyFinalPcmPointer = pbyPcmData;

	if (!szOutputPath || !pbyPcmData || dwPcmSize == 0)
	{
		return 0;
	}

	if (eFormat != ISACT_CF_PCM &&
		eFormat != ISACT_CF_MSPCMBIG &&
		eFormat != ISACT_CF_OGGVORBIS)
	{
		// External
		return 0;
	}

	// set RIFF header
	sHeader.szChunkId[0] = 'R';
	sHeader.szChunkId[1] = 'I';
	sHeader.szChunkId[2] = 'F';
	sHeader.szChunkId[3] = 'F';

	sHeader.dwChunkSize = 36 + dwPcmSize;

	sHeader.szFormat[0] = 'W';
	sHeader.szFormat[1] = 'A';
	sHeader.szFormat[2] = 'V';
	sHeader.szFormat[3] = 'E';

	// set fmt header
	sHeader.szSubchunk1Id[0] = 'f';
	sHeader.szSubchunk1Id[1] = 'm';
	sHeader.szSubchunk1Id[2] = 't';
	sHeader.szSubchunk1Id[3] = ' ';

	sHeader.dwSubchunk1Size = 16;
	sHeader.wAudioFormat = 1; // 1 = linear PCM
	sHeader.wNumChannels = wNumChannels;
	sHeader.dwSampleRate = dwSampleRate;

	// offsets and byterate
	sHeader.wBitsPerSample = wBitsPerSample;
	sHeader.wBlockAlign = (wNumChannels * wBitsPerSample) / 8;
	sHeader.dwByteRate = dwSampleRate * sHeader.wBlockAlign;

	// set data header
	sHeader.szSubchunk2Id[0] = 'd';
	sHeader.szSubchunk2Id[1] = 'a';
	sHeader.szSubchunk2Id[2] = 't';
	sHeader.szSubchunk2Id[3] = 'a';

	sHeader.dwSubchunk2Size = dwPcmSize;

	// if sound is Big Endian PCM (PS3) -- swap it
	if (eFormat == ISACT_CF_MSPCMBIG && wBitsPerSample == 16)
	{
		pbyAllocatedBuffer = new BYTE[dwPcmSize];
		if (!pbyAllocatedBuffer)
		{
			return 0;
		}

		for (DWORD dwIdx = 0; dwIdx < dwPcmSize; ++dwIdx)
		{
			pbyAllocatedBuffer[dwIdx] = pbyPcmData[dwIdx];
		}

		// swapping for get Little Endian
		SwapEndian16(pbyAllocatedBuffer, dwPcmSize);
		pbyFinalPcmPointer = pbyAllocatedBuffer;
	}

	// write file
	pFile = std::fopen(szOutputPath, "wb");
	if (!pFile)
	{
		if (pbyAllocatedBuffer)
		{
			delete[] pbyAllocatedBuffer;
		}
		return 0;
	}

	// write 44-byte header
	if (std::fwrite(&sHeader, sizeof(SWavHeader), 1, pFile) != 1)
	{
		std::fclose(pFile);
		if (pbyAllocatedBuffer)
		{
			delete[] pbyAllocatedBuffer;
		}
		return 0;
	}

	// write pcm data
	if (std::fwrite(pbyFinalPcmPointer, 1, dwPcmSize, pFile) != dwPcmSize)
	{
		std::fclose(pFile);
		if (pbyAllocatedBuffer)
		{
			delete[] pbyAllocatedBuffer;
		}
		return 0;
	}

	std::fclose(pFile);
	if (pbyAllocatedBuffer)
	{
		delete[] pbyAllocatedBuffer;
	}
	return 1;
}

int ConvertIsactFileToWav(const char* szSourcePath,
						  DWORD dwSourceOffset,
						  DWORD dwSourceSize,
						  const char* szOutputPath,
						  WORD wNumChannels,
						  DWORD dwSampleRate,
						  WORD wBitsPerSample,
						  EISACTCompressionFormat eFormat)
{
	int bResult = 0;

	BYTE* pbyPcmBuffer = ReadPcmFromFile(szSourcePath, dwSourceOffset, dwSourceSize);
	if (!pbyPcmBuffer)
	{
		return 0; // error
	}

	bResult = SaveIsactPcmToWav(szOutputPath,
								pbyPcmBuffer,
								dwSourceSize,
								wNumChannels,
								dwSampleRate,
								wBitsPerSample,
								eFormat);

	delete[] pbyPcmBuffer;

	return bResult;
}

void ParseChunkRange(const BYTE* pbyData, DWORD dwRangeStart, DWORD dwRangeEnd,
					 DWORD dwTotalSize, SIsactTrackInfo* pOutTracks,
					 DWORD dwMaxTracks, DWORD& dwTrackCount)
{

	if (dwRangeStart >= dwRangeEnd || dwRangeStart >= dwTotalSize)
	{
		return;
	}

	DWORD dwPos = dwRangeStart;

	while (dwPos + 8 <= dwRangeEnd && dwPos + 8 <= dwTotalSize && dwTrackCount < dwMaxTracks)
	{
		const char* szChunkName = reinterpret_cast<const char*>(pbyData + dwPos);
		DWORD dwChunkNamePos = dwPos;

		if (pbyData[dwChunkNamePos] < 32 ||
			pbyData[dwChunkNamePos] > 126)
		{
			break;
		}

		dwPos += 4;

		DWORD dwChunkLen = 0;
		if (CompareSignature(pbyData, dwChunkNamePos, "snde"))
		{
			dwChunkLen = 0;
		}
		else
		{
			dwChunkLen = ReadDWORDLE(pbyData, dwPos);
			dwPos += 4;
		}

		if (dwPos + dwChunkLen > dwRangeEnd || dwPos + dwChunkLen > dwTotalSize)
		{
			break;
		}

		if (CompareSignature(pbyData, dwChunkNamePos, "LIST"))
		{
			if (dwChunkLen >= 4)
			{
				const char* szObjectType = reinterpret_cast<const char*>(pbyData + dwPos);

				DWORD dwObjectTypePos = dwPos;
				DWORD dwListContentStart = dwPos + 4;
				DWORD dwListContentEnd   = dwPos + dwChunkLen;

				if (CompareSignature(pbyData, dwObjectTypePos, "samp"))
				{
					SIsactTrackInfo& sTrack = pOutTracks[dwTrackCount];
					sTrack.szName[0] = '\0';
					sTrack.dwSampleRate = 44100;
					sTrack.dwByteLength = 0;
					sTrack.dwPhysicalSize = 0;
					sTrack.wChannels = 1;
					sTrack.wBitsPerSample = 16;
					sTrack.dwSampleOffset = 0;
					sTrack.dwDataOffset = 0;
					sTrack.eFormat = ISACT_CF_PCM;
					sTrack.bHasOffset = 0;
					sTrack.bIsEmbedded = 0;

					int bHasSinf = 0;
					DWORD dwSubPos = dwListContentStart;

					while (dwSubPos + 8 <= dwListContentEnd)
					{
						const char* szSubName = reinterpret_cast<const char*>(pbyData + dwSubPos);
						DWORD dwSubNamePos = dwSubPos;
						dwSubPos += 4;
						DWORD dwSubLen = ReadDWORDLE(pbyData, dwSubPos);
						dwSubPos += 4;

						if (dwSubPos + dwSubLen > dwListContentEnd)
						{
							break;
						}

						if (CompareSignature(pbyData, dwSubNamePos, "titl"))
						{
							DWORD dwCharCount = (dwSubLen > 2) ? (dwSubLen - 2) / 2 : 0;
							if (dwCharCount > 127)
							{
								dwCharCount = 127;
							}
							for (DWORD dwC = 0; dwC < dwCharCount; ++dwC)
							{
								sTrack.szName[dwC] = static_cast<char>(pbyData[dwSubPos + (dwC * 2)]);
							}
							sTrack.szName[dwCharCount] = '\0';
						}
						else if (CompareSignature(pbyData, dwSubNamePos, "sinf") &&
								 dwSubLen >= 20)
						{
							const SSampleInfo* pSinf = reinterpret_cast<const SSampleInfo*>(pbyData + dwSubPos);
							sTrack.dwSampleRate = pSinf->dwSamplesPerSecond;
							sTrack.dwByteLength = pSinf->dwByteLength;
							if (sTrack.dwPhysicalSize == 0)
							{
								sTrack.dwPhysicalSize = pSinf->dwByteLength;
							}
							sTrack.wBitsPerSample = pSinf->wBitsPerSample;
							bHasSinf = 1;
						}
						else if (CompareSignature(pbyData, dwSubNamePos, "chnk") &&
								 dwSubLen >= 4)
						{
							sTrack.wChannels = static_cast<WORD>(*reinterpret_cast<const int*>(pbyData + dwSubPos));
						}
						else if (CompareSignature(pbyData, dwSubNamePos, "cmpi") &&
								 dwSubLen >= 24)
						{
							const SCompressionInfo* pCmpi = reinterpret_cast<const SCompressionInfo*>(pbyData + dwSubPos);
							sTrack.eFormat = static_cast<EISACTCompressionFormat>(pCmpi->dwCurrentFormat);

							if (pCmpi->dwCurrentFormat != ISACT_CF_PCM &&
								pCmpi->dwCurrentFormat != ISACT_CF_MSPCMBIG)
							{
								sTrack.dwPhysicalSize = pCmpi->dwTotalSize;
							}
						}
						else if (CompareSignature(pbyData, dwSubNamePos, "soff") &&
								 dwSubLen >= 4)
						{
							sTrack.dwSampleOffset = ReadDWORDLE(pbyData, dwSubPos);
							sTrack.bHasOffset = 1;
						}
						else if (CompareSignature(pbyData, dwSubNamePos, "data"))
						{
							sTrack.dwDataOffset = dwSubPos;
							sTrack.bIsEmbedded = 1;
						}
						dwSubPos += dwSubLen;
						if (dwSubLen % 2 == 1)
						{
							dwSubPos++;
						}
					}
					if (bHasSinf)
					{
						// if track doesnt have name - set index name
						if (sTrack.szName[0] == '\0')
						{
							sprintf(sTrack.szName, "track_%03u", dwTrackCount);
						}
						dwTrackCount++;
					}
				}
				ParseChunkRange(pbyData, dwListContentStart, dwListContentEnd,
								dwTotalSize, pOutTracks, dwMaxTracks, dwTrackCount);
			}
		}
		dwPos += dwChunkLen;
		if (dwChunkLen % 2 == 1 &&
			!CompareSignature(pbyData, dwChunkNamePos, "snde"))
		{
			dwPos++;
		};
	}
}

DWORD EnumerateTracks(const BYTE* pbyData, DWORD dwDataSize,
					  SIsactTrackInfo* pOutTracks, DWORD dwMaxTracks)
{
	if (!pbyData || dwDataSize < 12 || !pOutTracks || dwMaxTracks == 0)
	{
		return 0;
	}

	DWORD dwRiffPos = 0;
	int bFoundRiff = 0;
	while (dwRiffPos + 12 <= dwDataSize)
	{
		if (CompareSignature(pbyData, dwRiffPos, "RIFF"))
		{
			const char* szType = reinterpret_cast<const char*>(pbyData + dwRiffPos + 8);
			if (CompareSignature(pbyData, dwRiffPos + 8, "icbf") ||
				CompareSignature(pbyData, dwRiffPos + 8, "isbf"))
			{
				bFoundRiff = 1;
				break;
			}
		}
		dwRiffPos++;
	}

	if (!bFoundRiff)
	{
		return 0;
	}

	DWORD dwTrackCount = 0;
	ParseChunkRange(pbyData, dwRiffPos + 12, dwDataSize, dwDataSize, pOutTracks,
					dwMaxTracks, dwTrackCount);

	return dwTrackCount;
}

int SaveIsactOggToFile(const char* szOutputPath,
					   const BYTE* pbyOggData,
					   DWORD dwOggSize)
{
	std::FILE* pFile = NULL;

	if (!szOutputPath || !pbyOggData || dwOggSize == 0)
	{
		return 0;
	}

	pFile = std::fopen(szOutputPath, "wb");
	if (!pFile)
	{
		return 0;
	}

	if (std::fwrite(pbyOggData, 1, dwOggSize, pFile) != dwOggSize)
	{
		std::fclose(pFile);
		return 0;
	}

	std::fclose(pFile);
	return 1;
}


}

