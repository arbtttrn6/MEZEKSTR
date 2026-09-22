#ifndef UTILS_H
#define UTILS_H

#include "./typedef.h"
#include "./wavdef.h"


namespace MEZEkstr
{

void SwapEndian16(BYTE* pbyData, DWORD dwSize);

int CompareSignature(const BYTE* pbyData, DWORD dwPos, const char* szSig);

DWORD ReadDWORDLE(const BYTE* pbyData, DWORD dwPos);

WORD ReadWORDLE(const BYTE* pbyData, DWORD dwPos);

const char* GetFormatName(MEZEkstr::EISACTCompressionFormat eFormat);

}

#endif // UTILS_H
