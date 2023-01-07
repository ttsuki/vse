/// @file
/// @brief  Tsukikage::util::jcode
/// @author (C) 2022 ttsuki

/// Original: ttsuki-csmix
/// https://github.com/ttsuki/ttsuki-csmix/blob/master/Util/TextFile.cs
/// NYSL Version 0.9982: 2012 ttsuki

#pragma once

#include <Windows.h>
#include <cstdint>
#include <string_view>

namespace Tsukikage::util::jcode
{
    // Windows code page
    enum encoding : int
    {
        unknown = 0,
        unicode = 1200,
        unicode_be = 1201,
        utf8 = 65001,
        shift_jis = 932,
        euc_jp = 20932,
        iso_2022_jp = 50222,
    };

    static encoding guess_encoding(std::string_view data)
    {
        size_t len = data.size();

        // Detect UTF BOM
        {
            uint32_t b1 = len >= 1 ? data[0] : 0u;
            uint32_t b2 = len >= 2 ? data[1] | b1 << 8 : 0u;
            uint32_t b3 = len >= 3 ? data[2] | b2 << 8 : 0u;

            if (b2 == 0xFFFE) return encoding::unicode;
            if (b2 == 0xFEFF) return encoding::unicode_be;
            if (b3 == 0xEFBBBF) return encoding::utf8;
        }

        // Detect UTF-16 ASCII: such as 0x?? 0x00
        for (size_t i = 0; i < len - 1; i++)
            if (data[i] <= 0x7F && data[i + 1] == 0x00)
                return encoding::unicode;

        // Detect JIS-like escape sequence: such as [ESC]$B
        for (size_t i = 0; i < len - 3; i++)
        {
            uint32_t c = static_cast<uint8_t>(data[i]) << 24 | static_cast<uint8_t>(data[i + 1]) << 16 | static_cast<uint8_t>(data[i + 2]) << 8 | static_cast<uint8_t>(data[i + 3]);

            switch (c)
            {
            case 0x1B242840: // JIS C 6226 [ESC]$(@
            case 0x1B242842: // JIS X 0208 [ESC]$(B
            case 0x1B242844: // JIS X 0212 [ESC]$(D
            case 0x1B24284F: // JIS X 0213 [ESC]$(O
            case 0x1B242850: // JIS X 0213 [ESC]$(P
            case 0x1B242851: // JIS X 0213 [ESC]$(Q
                return encoding::iso_2022_jp;
            }

            switch (c >> 8)
            {
            case 0x1B2440: // JIS C 6226 [ESC]$@ 
            case 0x1B2442: // JIS X 0208 [ESC]$B
            case 0x1B2840: // JIS X 0201 [ESC](B // ASCII
            case 0x1B2849: // JIS X 0201 [ESC](I
            case 0x1B284A: // JIS X 0201 [ESC](J // Roman
                return encoding::iso_2022_jp;
            }
        }

        size_t sjis = 0;
        size_t euc = 0;
        size_t utf8 = 0;

        using uint32_t = uint32_t;

        // Count Shift_JIS-like byte sequence
        for (size_t i = 0; i < len - 1; i++)
        {
            uint32_t b1 = static_cast<uint8_t>(data[i]);
            uint32_t b2 = static_cast<uint8_t>(data[i + 1]);
            if ((b1 ^ 0x20) - 0xA1 < 0x3C && b2 - 0x40 < 0xBD && b2 != 0x7F)
            {
                sjis += 2;
                i++;
            }
        }

        // Count EUC_JP-like byte sequence
        for (size_t i = 0; i < len - 1; i++)
        {
            uint32_t b1 = static_cast<uint8_t>(data[i]);
            uint32_t b2 = static_cast<uint8_t>(data[i + 1]);
            uint32_t b3 = i < len - 2 ? static_cast<uint8_t>(data[i + 2]) : 0u;

            if (b1 == 0x8E)
            {
                if (b2 - 0xA1 < 0x3F)
                {
                    euc += 2;
                    i++;
                }
                else if (b2 - 0xA1 < 0x5E && b3 - 0xA1 < 0x5E)
                {
                    euc += 3;
                    i += 2;
                }
            }
            else if (b1 - 0xA1 < 0x5E && b2 - 0xA1 < 0x5E)
            {
                euc += 2;
                i++;
            }
        }

        // Count UTF8-like byte sequence
        for (size_t i = 0; i < len - 1; i++)
        {
            uint32_t b1 = static_cast<uint8_t>(data[i]);
            bool b2 = i < len - 1 && static_cast<uint8_t>(data[i + 1]) - 0x80 < 0x40;
            bool b3 = i < len - 2 && static_cast<uint8_t>(data[i + 2]) - 0x80 < 0x40;
            bool b4 = i < len - 3 && static_cast<uint8_t>(data[i + 3]) - 0x80 < 0x40;

            if (b1 - 0xC2 < 0x33)
            {
                switch (b1 >> 4)
                {
                case 0xC: if (b2)
                    {
                        utf8 += 2;
                        i++;
                    }
                    break;
                case 0xE: if (b2 && b3)
                    {
                        utf8 += 3;
                        i += 2;
                    }
                    break;
                case 0xF: if (b2 && b3 && b4)
                    {
                        utf8 += 4;
                        i += 3;
                    }
                    break;
                }
            }
        }

        if (euc >= utf8 && euc >= sjis) return encoding::euc_jp;
        if (utf8 >= sjis) return encoding::utf8;
        return encoding::shift_jis;
    }

    static inline std::wstring convert_to_wstring(std::string_view source_string, int source_code_page)
    {
        int size_needed = ::MultiByteToWideChar(source_code_page, 0, source_string.data(), static_cast<int>(source_string.size()), nullptr, 0);
        std::wstring result(size_needed, L'\0');
        ::MultiByteToWideChar(source_code_page, 0, source_string.data(), static_cast<int>(source_string.size()), result.data(), static_cast<int>(result.size()));
        return result;
    }
}
