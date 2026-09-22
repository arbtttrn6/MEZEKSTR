#include "./core/include/mezekstr/pcmtowav.h"
#include "core/include/mezekstr/utils.h"

#include <cstdio>
#include <cstring>

void CombinePath(char* szOut, const char* szFolder,
				 const char* szFileName, const char* szExt)
{
	std::strcpy(szOut, szFolder);
	size_t nLen = std::strlen(szOut);
	if (nLen > 0 && szOut[nLen - 1] != '/' && szOut[nLen - 1] != '\\')
	{
#ifdef _WIN32
		std::strcat(szOut, "\\");
#else
		std::strcat(szOut, "/");
#endif
	}
	std::strcat(szOut, szFileName);

	std::strcat(szOut, szExt);
}

const char* get_program_version() {
	static char szFullVersion[17] = "1.0.0.00000000";

	if (szFullVersion[6] == '0' && szFullVersion[7] == '0')
	{
		const char* pszD = __DATE__;

		szFullVersion[6] = (pszD[4] == ' ') ? '0' : pszD[4];
		szFullVersion[7] = pszD[5];

		const char* pszM = "JanFebMarAprMayJunJulAugSepOctNovDec";
		int nMonth = 1;
		for (int nI = 0; nI < 12; ++nI)
		{
			if (pszD[0] == pszM[nI * 3] && pszD[1] == pszM[nI * 3 + 1])
			{
				nMonth = nI + 1;
				break;
			}
		}
		szFullVersion[8] = (char)('0' + nMonth / 10);
		szFullVersion[9] = (char)('0' + nMonth % 10);

		szFullVersion[10] = pszD[7];
		szFullVersion[11] = pszD[8];
		szFullVersion[12] = pszD[9];
		szFullVersion[13] = pszD[10];
	}

	return szFullVersion;
}

void PrintInfo(const char* pszProgramName)
{
	const char* const pszProgramVersion = get_program_version();
	printf("author: Artur Ajvazjan (arbtttrn6)\n");
	printf("version: %s\n", pszProgramVersion);
	printf("usage:\n");
	printf("  %s --list <file.icb>                           - View all audiofiles\n",
		   pszProgramName);
	printf("  %s --extract-all <file.icb> <output_folder>    - Ekstrakt all files in folder\n",
		   pszProgramName);
	printf("  %s --extract <file.icb> <name> <output_folder> - Ekstrakt file by name\n",
		   pszProgramName);
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		PrintInfo(argv[0]);
		return 1;
	}

	const char* szCommand = argv[1];
	const char* szInputPath = argv[2];

	char szPairPath[512];
	std::strncpy(szPairPath, szInputPath, sizeof(szPairPath) - 5);
	size_t nInLen = std::strlen(szPairPath);
	if (nInLen > 4)
	{
		if (std::strcmp(&szPairPath[nInLen - 4], ".isb") == 0 ||
			std::strcmp(&szPairPath[nInLen - 4], ".ISB") == 0)
		{
			std::strcpy(&szPairPath[nInLen - 4], ".icb");
		}
		else
		{
			std::strcpy(&szPairPath[nInLen - 4], ".isb");
		}
	}

	std::FILE* pInputFile = std::fopen(szInputPath, "rb");
	if (!pInputFile)
	{
		fprintf(stderr, "Error: Fail while open file: %s\n", szInputPath);
		return 1;
	}
	std::fseek(pInputFile, 0, SEEK_END);
	MEZEkstr::DWORD dwDataSize = static_cast<MEZEkstr::DWORD>(std::ftell(pInputFile));
	std::fseek(pInputFile, 0, SEEK_SET);

	MEZEkstr::BYTE* pbyDataBuffer = new MEZEkstr::BYTE[dwDataSize];
	std::fread(pbyDataBuffer, 1, dwDataSize, pInputFile);
	std::fclose(pInputFile);

	const MEZEkstr::DWORD dwMaxTracks = 2048;
	MEZEkstr::SIsactTrackInfo* pTracks = new MEZEkstr::SIsactTrackInfo[dwMaxTracks];

	MEZEkstr::DWORD dwTotalTracks = MEZEkstr::EnumerateTracks(pbyDataBuffer,
															  dwDataSize,
															  pTracks,
															  dwMaxTracks);

	if (dwTotalTracks == 0)
	{
		fputs("Audios not found or format of file is invalid.\n", stderr);
		delete[] pbyDataBuffer;
		delete[] pTracks;
		return 1;
	}

	if (std::strcmp(szCommand, "--list") == 0)
	{
		printf("Audios found in file: %u\n\n", dwTotalTracks);

		size_t MaxNameLength = 12;

		for (MEZEkstr::DWORD i = 0; i < dwTotalTracks; ++i)
		{
			const char* str = pTracks[i].szName;
			size_t CurrentLength = 0;

			while (str[CurrentLength] != '\0')
			{
				CurrentLength++;
			}

			if (CurrentLength > MaxNameLength)
			{
				MaxNameLength = CurrentLength;
			}
		}

		int NameColumnWidth = static_cast<int>(MaxNameLength);

		int TotalWidth = 1 + 6 + 1 + NameColumnWidth + 1 + 12 + 1 + 10 + 1 + 5 + 1 + 13 + 1 + 14 + 1;

		for (int i = 0; i < TotalWidth + 7; ++i) fputc('-', stdout);
		fputc('\n', stdout);

		printf("| %-6s| %-*s | %-11s| %-10s| %-5s| %-13s| %-14s|\n",
				"Sound", NameColumnWidth, "Name (file)", "Size (PCM)",
			   "Rate (Hz)", "Ch", "Is embedded", "Format");

		for (int i = 0; i < TotalWidth + 7; ++i) fputc('-', stdout);
		fputc('\n', stdout);

		for (MEZEkstr::DWORD i = 0; i < dwTotalTracks; ++i)
		{
			printf("| %-6u| %-*s | %-11u| %-10u| %-5u| %-13s| %-14s|\n",
				   i + 1,
				   NameColumnWidth, pTracks[i].szName,
				   pTracks[i].dwByteLength,
				   pTracks[i].dwSampleRate,
				   pTracks[i].wChannels,
				   (pTracks[i].bIsEmbedded ? "Embedded" : "External (Link)"),
				   MEZEkstr::GetFormatName(pTracks[i].eFormat));

			for (int j = 0; j < TotalWidth + 7; ++j) fputc('-', stdout);
			fputc('\n', stdout);
		}
		fputc('\n', stdout);
	}
	else if (std::strcmp(szCommand, "--extract-all") == 0)
	{
		if (argc < 4) { PrintInfo(argv[0]); return 1; }
		const char* szOutFolder = argv[3];

		printf("Ekstrakt all audios in folder: %s\n", szOutFolder);
		MEZEkstr::DWORD dwExtracted = 0;

		for (MEZEkstr::DWORD i = 0; i < dwTotalTracks; ++i)
		{
			MEZEkstr::BYTE* pbyData = NULL;

			if (pTracks[i].bIsEmbedded)
			{
				pbyData = new MEZEkstr::BYTE[pTracks[i].dwPhysicalSize]; //dwByteLength
				std::memcpy(pbyData, pbyDataBuffer + pTracks[i].dwDataOffset,
							pTracks[i].dwPhysicalSize);
			}
			else if (pTracks[i].bHasOffset)
			{
				pbyData = MEZEkstr::ReadPcmFromFile(szPairPath,
													pTracks[i].dwSampleOffset,
													pTracks[i].dwPhysicalSize);
			}

			if (!pbyData)
			{
				fprintf(stderr, "Error: Fail while reading audios for %s\n",
						pTracks[i].szName);
				continue;
			}

			char szFullOutPath[512];

			switch (pTracks[i].eFormat)
			{
				case MEZEkstr::ISACT_CF_PCM:
				case MEZEkstr::ISACT_CF_MSPCMBIG:
				{
					char szFullOutPath[512];
					CombinePath(szFullOutPath, szOutFolder, pTracks[i].szName,
								".wav");

					if (MEZEkstr::SaveIsactPcmToWav(szFullOutPath, pbyData,
													pTracks[i].dwByteLength,
													pTracks[i].wChannels,
													pTracks[i].dwSampleRate,
													pTracks[i].wBitsPerSample,
													pTracks[i].eFormat))
					{
						printf("Ekstrakted: %s.wav\n", pTracks[i].szName);
						dwExtracted++;
					}
					break;
				}
				case MEZEkstr::ISACT_CF_OGGVORBIS:
				{
					CombinePath(szFullOutPath, szOutFolder, pTracks[i].szName,
								".ogg");

					if (MEZEkstr::SaveIsactOggToFile(szFullOutPath, pbyData,
													 pTracks[i].dwPhysicalSize)) //dwByteLength
					{
						printf("Ekstrakted: %s.ogg\n", pTracks[i].szName);
						dwExtracted++;
					}
					break;
				}
				default:
					printf("Pass [%s]: needed external other decoder (%s).\n",
						   pTracks[i].szName, MEZEkstr::GetFormatName(pTracks[i].eFormat));
					break;
			}

			delete[] pbyData;
		}
		printf("Done. Success ekstrakted: %u from %u\n", dwExtracted, dwTotalTracks);
	}
	else if (std::strcmp(szCommand, "--extract") == 0)
	{
		if (argc < 5) { PrintInfo(argv[0]); return 1; }
		const char* szTargetName = argv[3];
		const char* szOutFolder = argv[4];

		int bFound = 0;
		for (MEZEkstr::DWORD i = 0; i < dwTotalTracks; ++i)
		{
			if (std::strcmp(pTracks[i].szName, szTargetName) == 0)
			{
				bFound = 1;

				MEZEkstr::BYTE* pbyData = NULL;

				if (pTracks[i].bIsEmbedded)
				{
					pbyData = new MEZEkstr::BYTE[pTracks[i].dwByteLength];
					std::memcpy(pbyData, pbyDataBuffer + pTracks[i].dwDataOffset,
								pTracks[i].dwByteLength);
				}
				else if (pTracks[i].bHasOffset)
				{
					pbyData = MEZEkstr::ReadPcmFromFile(szPairPath,
														pTracks[i].dwSampleOffset,
														pTracks[i].dwByteLength);
				}

				if (!pbyData)
				{
					fprintf(stderr, "Error: Fail while read audio-data for %s\n",
							szTargetName);
					break;
				}

				char szFullOutPath[512];

				switch (pTracks[i].eFormat)
				{
					case MEZEkstr::ISACT_CF_PCM:
					case MEZEkstr::ISACT_CF_MSPCMBIG:
						CombinePath(szFullOutPath, szOutFolder, pTracks[i].szName,
									".wav");
						if (MEZEkstr::SaveIsactPcmToWav(szFullOutPath, pbyData,
														pTracks[i].dwByteLength,
														pTracks[i].wChannels,
														pTracks[i].dwSampleRate,
														pTracks[i].wBitsPerSample,
														pTracks[i].eFormat))
						{
							printf("File [%s] saved in folder %s as wav!\n",
								   szTargetName, szOutFolder);
						}
						break;

					case MEZEkstr::ISACT_CF_OGGVORBIS:
						CombinePath(szFullOutPath, szOutFolder, pTracks[i].szName,
									".ogg");
						if (MEZEkstr::SaveIsactOggToFile(szFullOutPath, pbyData,
														 pTracks[i].dwByteLength))
						{
							printf("File [%s] saved in folder %s as ogg!\n",
								   szTargetName, szOutFolder);
						}
						break;

					default:
						fprintf(stderr, "Error: file is found, but uses unsupported codec (%s).\n",
								MEZEkstr::GetFormatName(pTracks[i].eFormat));
						break;
				}

				delete[] pbyData;
				break;
			}
		}
		if (!bFound)
		{
			std::fprintf(stderr, "Error: Audiofile with name '%s' not found in bank.\n",
						 szTargetName);
		}
	}
	else
	{
		PrintInfo(argv[0]);
	}

	delete[] pbyDataBuffer;
	delete[] pTracks;
	return 0;
}
