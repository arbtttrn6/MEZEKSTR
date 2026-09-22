#include "../include/mezekstr/utils.h"


namespace MEZEkstr
{

void SwapEndian16(BYTE* pbyData, DWORD dwSize)
{
	if (!pbyData || dwSize < 2)
	{
		return;
	}

	for (DWORD dwIdx = 0; dwIdx < dwSize - 1; dwIdx += 2)
	{
		BYTE byTemp = pbyData[dwIdx];
		pbyData[dwIdx] = pbyData[dwIdx + 1];
		pbyData[dwIdx + 1] = byTemp;
	}
}

int CompareSignature(const BYTE* pbyData, DWORD dwPos, const char* szSig)
{
	return (pbyData[dwPos]   == static_cast<BYTE>(szSig[0]) &&
			pbyData[dwPos+1] == static_cast<BYTE>(szSig[1]) &&
			pbyData[dwPos+2] == static_cast<BYTE>(szSig[2]) &&
			pbyData[dwPos+3] == static_cast<BYTE>(szSig[3]));
}

DWORD ReadDWORDLE(const BYTE* pbyData, DWORD dwPos)
{
	return static_cast<DWORD>(pbyData[dwPos])         |
			(static_cast<DWORD>(pbyData[dwPos+1]) << 8)  |
			(static_cast<DWORD>(pbyData[dwPos+2]) << 16) |
			(static_cast<DWORD>(pbyData[dwPos+3]) << 24);
}

WORD ReadWORDLE(const BYTE* pbyData, DWORD dwPos)
{
	return static_cast<WORD>(pbyData[dwPos]) | (static_cast<WORD>(pbyData[dwPos+1]) << 8);
}

const char* GetFormatName(MEZEkstr::EISACTCompressionFormat eFormat)
{
	switch (eFormat)
	{
		case MEZEkstr::ISACT_CF_PCM:       return "PCM LE";
		case MEZEkstr::ISACT_CF_IMA4ADPCM: return "IMA4 ADPCM";
		case MEZEkstr::ISACT_CF_OGGVORBIS: return "OGG Vorbis";
		case MEZEkstr::ISACT_CF_WMA:       return "WMA";
		case MEZEkstr::ISACT_CF_XMA:       return "XMA";
		case MEZEkstr::ISACT_CF_MSMP3:     return "MSF MP3 (PS3)";
		case MEZEkstr::ISACT_CF_MSADPCM:   return "MSF ADPCM (PS3)";
		case MEZEkstr::ISACT_CF_MSPCMBIG:  return "PCM BE (PS3)";
		default:                           return "Unknown";
	}
};

}
