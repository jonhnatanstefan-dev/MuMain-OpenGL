// MultiLanguage.cpp: implementation of the CMultiLanguage class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <cstring>

namespace
{
    // The original MU Portuguese/Spanish BMD packs are not consistent: some
    // strings are UTF-8 while older tables are Windows-1252. Detect valid
    // UTF-8 first and only fall back to CP1252 when the byte sequence cannot
    // represent valid UTF-8. This keeps modern UTF-8 resources intact while
    // correctly decoding legacy accents such as ã, ç, í and ó.
    bool IsValidUtf8(const unsigned char* data, int length)
    {
        int i = 0;
        while (i < length)
        {
            const unsigned char c = data[i];
            if (c <= 0x7F)
            {
                ++i;
                continue;
            }

            int continuationCount = 0;
            unsigned int codePoint = 0;

            if ((c & 0xE0) == 0xC0)
            {
                continuationCount = 1;
                codePoint = c & 0x1F;
                if (codePoint == 0) // Reject overlong 2-byte sequences.
                    return false;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                continuationCount = 2;
                codePoint = c & 0x0F;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                continuationCount = 3;
                codePoint = c & 0x07;
            }
            else
            {
                return false;
            }

            if (i + continuationCount >= length)
                return false;

            for (int j = 1; j <= continuationCount; ++j)
            {
                const unsigned char cc = data[i + j];
                if ((cc & 0xC0) != 0x80)
                    return false;
                codePoint = (codePoint << 6) | (cc & 0x3F);
            }

            if ((continuationCount == 1 && codePoint < 0x80)
                || (continuationCount == 2 && codePoint < 0x800)
                || (continuationCount == 3 && codePoint < 0x10000)
                || codePoint > 0x10FFFF
                || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
            {
                return false;
            }

            i += continuationCount + 1;
        }

        return true;
    }
}

CMultiLanguage* CMultiLanguage::ms_Singleton = NULL;

CMultiLanguage::CMultiLanguage(std::wstring strSelectedML)
{
    ms_Singleton = this;

    if (wcsicmp(strSelectedML.c_str(), L"ENG") == 0)
    {
        byLanguage = 0;
    }
    else if (wcsicmp(strSelectedML.c_str(), L"POR") == 0)
    {
        byLanguage = 1;
    }
    else if (wcsicmp(strSelectedML.c_str(), L"SPN") == 0)
    {
        byLanguage = 2;
    }
    else
    {
        byLanguage = 0;
    }
}

BYTE CMultiLanguage::GetLanguage()
{
    return byLanguage;
}

/**
 * Converts a UTF-8 byte buffer to UTF-16.
 *
 * Reads up to @p maxSourceLength bytes from @p source (which may or may not
 * be null-terminated) and writes the converted UTF-16 characters to @p target.
 * The required number of UTF-16 characters is determined first using
 * MultiByteToWideChar().
 *
 * If a null terminator exists within the processed byte range, it is copied
 * by the conversion. Otherwise, the function appends one when possible.
 *
 * @param target Destination buffer for the UTF-16 output.
 * @param source UTF-8 encoded byte buffer.
 * @param maxSourceLength Maximum number of bytes to read from @p source.
 *
 * @return Number of UTF-16 characters written (excluding the null terminator
 *         when present). Returns 0 on failure.
 *
 * @note The caller must ensure @p target is large enough to hold the converted
 *       UTF-16 string plus a terminating null character.
 */
int32_t CMultiLanguage::ConvertFromUtf8(wchar_t* target, const char* source, int maxSourceLength)
{
    if (target == nullptr || source == nullptr)
    {
        return 0;
    }

    int sourceLength = 0;
    if (maxSourceLength < 0)
    {
        sourceLength = static_cast<int>(std::strlen(source));
    }
    else
    {
        while (sourceLength < maxSourceLength && source[sourceLength] != '\0')
            ++sourceLength;
    }

    if (sourceLength == 0)
    {
        target[0] = L'\0';
        return 0;
    }

    const bool validUtf8 = IsValidUtf8(
        reinterpret_cast<const unsigned char*>(source), sourceLength);

    // Portuguese/Spanish legacy BMD files shipped by MU frequently use
    // Windows-1252. Newer resources use UTF-8. Select the decoder from the
    // actual byte sequence instead of assuming every BMD is UTF-8.
    const UINT codePage = validUtf8 ? CP_UTF8 : 1252;

    const int requiredChars = MultiByteToWideChar(
        codePage, 0, source, sourceLength, nullptr, 0);
    if (requiredChars <= 0)
    {
        target[0] = L'\0';
        return 0;
    }

    const int written = MultiByteToWideChar(
        codePage, 0, source, sourceLength, target, requiredChars);
    if (written <= 0)
    {
        target[0] = L'\0';
        return 0;
    }

    target[written] = L'\0';
    return written;
}

int32_t CMultiLanguage::ConvertToUtf8(char* target, const wchar_t* source, int maxSourceLength)
{
    if (target == nullptr || source == nullptr)
    {
        return 0;
    }

    // In this codebase, maxSourceLength is effectively used as the destination buffer capacity.
    const int requiredBytesWithNull = WideCharToMultiByte(CP_UTF8, 0, source, -1, nullptr, 0, nullptr, nullptr);
    if (requiredBytesWithNull <= 0)
    {
        target[0] = '\0';
        return 0;
    }

    if (maxSourceLength > 0)
    {
        std::string tmp;
        tmp.resize(requiredBytesWithNull);
        const int writtenWithNull = WideCharToMultiByte(CP_UTF8, 0, source, -1, tmp.data(), requiredBytesWithNull, nullptr, nullptr);
        if (writtenWithNull <= 0)
        {
            target[0] = '\0';
            return 0;
        }

        const int available = std::max<int>(0, maxSourceLength - 1);
        const int srcLen = std::max<int>(0, writtenWithNull - 1);
        const int copyLen = std::min<int>(srcLen, available);

        if (copyLen > 0)
        {
            std::memcpy(target, tmp.data(), static_cast<size_t>(copyLen));
        }
        target[copyLen] = '\0';
        return copyLen;
    }

    const int requiredBytes = requiredBytesWithNull;
    if (requiredBytes <= 0)
    {
        target[0] = '\0';
        return 0;
    }

    const int written = WideCharToMultiByte(CP_UTF8, 0, source, -1, target, requiredBytes, nullptr, nullptr);
    if (written <= 0)
    {
        target[0] = '\0';
        return 0;
    }

    // When source length is -1, WinAPI includes the null terminator in 'written'.
    return written > 0 ? (written - 1) : 0;
}


WPARAM CMultiLanguage::ConvertFulltoHalfWidthChar(DWORD wParam)
{
    auto Char = (wchar_t)(wParam);

    if (Char >= 0xFF01 && Char <= 0xFF5A)
        wParam -= 0xFEE0;
    else if (Char == 0x3000)
        wParam = 0x0020;

    return wParam;
}