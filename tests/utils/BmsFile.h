/// @file
/// @brief  TTsukiGameSDK::bms::BmsFile
/// @author (C) 2022 ttsuki

/// Copied from TTsukiGameSDK
/// MIT License: (C) 2022 ttsuki

#pragma once

#include <map>
#include <optional>
#include <random>
#include <charconv>

#include <array>
#include <string>
#include <ostream>
#include <sstream>
#include <iomanip>

namespace TTsukiGameSdk::bms
{
    using MeasureNumber = int;
    using Bpm = double;

    struct BmsFileInfo
    {
        std::string Title = {};
        std::string SubTitle = {};
        std::string Artist = {};
        std::string SubArtist = {};
        std::string Genre = {};
        std::string SubGenre = {};
        std::string Comment = {};

        std::string BannerBmpPath = {};
        std::string StageBmpPath = {};
        std::string BackBmpPath = {};

        int Player = 1;
        int Rank = 3;
        int Difficulty = 1;
        int PlayLevel = 2;
        int WavePlaybackVolume = 100;
        int Total = 1000;
        Bpm InitialBpm = 130.0;

        std::string Url = {};
        std::string Mail = {};

        bool BgaDetected = false;
        bool LongNotesDetected = false;
    };

    struct BmsFile
    {
        enum struct BmsEventCode : int
        {
            PlayBackChorus = 0x01,
            ChangeBarLength = 0x02,
            ChangeBPM = 0x03,
            ChangeBMP = 0x04,
            ExtChr = 0x05,
            ChangeBMPPoor = 0x06,
            ChangeBMPLayer = 0x07,
            ChangeBPMFromTable = 0x08,
            StopSequenceFromTable = 0x09,
            ChangeBMPLayer2 = 0x0A,

            NormalNote00 = 0x10,
            NormalNote01 = 0x11,
            NormalNote02 = 0x12,
            NormalNote03 = 0x13,
            NormalNote04 = 0x14,
            NormalNote05 = 0x15,
            NormalNote06 = 0x16,
            NormalNote07 = 0x17,
            NormalNote08 = 0x18,
            NormalNote09 = 0x19,
            NormalNote0A = 0x1A,
            NormalNote0B = 0x1B,
            NormalNote0C = 0x1C,
            NormalNote0D = 0x1D,
            NormalNote0E = 0x1E,
            NormalNote0F = 0x1F,
            NormalNote10 = 0x20,
            NormalNote11 = 0x21,
            NormalNote12 = 0x22,
            NormalNote13 = 0x23,
            NormalNote14 = 0x24,
            NormalNote15 = 0x25,
            NormalNote16 = 0x26,
            NormalNote17 = 0x27,
            NormalNote18 = 0x28,
            NormalNote19 = 0x29,
            NormalNote1A = 0x2A,
            NormalNote1B = 0x2B,
            NormalNote1C = 0x2C,
            NormalNote1D = 0x2D,
            NormalNote1E = 0x2E,
            NormalNote1F = 0x2F,
            HiddenNote00 = 0x30,
            HiddenNote01 = 0x31,
            HiddenNote02 = 0x32,
            HiddenNote03 = 0x33,
            HiddenNote04 = 0x34,
            HiddenNote05 = 0x35,
            HiddenNote06 = 0x36,
            HiddenNote07 = 0x37,
            HiddenNote08 = 0x38,
            HiddenNote09 = 0x39,
            HiddenNote0A = 0x3A,
            HiddenNote0B = 0x3B,
            HiddenNote0C = 0x3C,
            HiddenNote0D = 0x3D,
            HiddenNote0E = 0x3E,
            HiddenNote0F = 0x3F,
            HiddenNote10 = 0x40,
            HiddenNote11 = 0x41,
            HiddenNote12 = 0x42,
            HiddenNote13 = 0x43,
            HiddenNote14 = 0x44,
            HiddenNote15 = 0x45,
            HiddenNote16 = 0x46,
            HiddenNote17 = 0x47,
            HiddenNote18 = 0x48,
            HiddenNote19 = 0x49,
            HiddenNote1A = 0x4A,
            HiddenNote1B = 0x4B,
            HiddenNote1C = 0x4C,
            HiddenNote1D = 0x4D,
            HiddenNote1E = 0x4E,
            HiddenNote1F = 0x4F,
            LongNote00 = 0x50,
            LongNote01 = 0x51,
            LongNote02 = 0x52,
            LongNote03 = 0x53,
            LongNote04 = 0x54,
            LongNote05 = 0x55,
            LongNote06 = 0x56,
            LongNote07 = 0x57,
            LongNote08 = 0x58,
            LongNote09 = 0x59,
            LongNote0A = 0x5A,
            LongNote0B = 0x5B,
            LongNote0C = 0x5C,
            LongNote0D = 0x5D,
            LongNote0E = 0x5E,
            LongNote0F = 0x5F,
            LongNote10 = 0x60,
            LongNote11 = 0x61,
            LongNote12 = 0x62,
            LongNote13 = 0x63,
            LongNote14 = 0x64,
            LongNote15 = 0x65,
            LongNote16 = 0x66,
            LongNote17 = 0x67,
            LongNote18 = 0x68,
            LongNote19 = 0x69,
            LongNote1A = 0x6A,
            LongNote1B = 0x6B,
            LongNote1C = 0x6C,
            LongNote1D = 0x6D,
            LongNote1E = 0x6E,
            LongNote1F = 0x6F,
            ChangeTextFromList = 0x99,
        };

        std::vector<std::string> Source{};
        BmsFileInfo Information{};
        std::map<MeasureNumber, double> MeasureLength{}; // MeasureNumber -> ratio of length to 4/4
        std::array<std::string, 1296> WaveTable{};
        std::array<std::string, 1296> BmpTable{};
        std::array<std::string, 1296> TextTable{};
        std::array<Bpm, 1296> BpmTable{};
        std::array<int, 1296> StopTable{};

        struct EventVector
        {
            MeasureNumber Measure;
            BmsEventCode Channel;
            std::vector<int> Events;
        };

        std::vector<EventVector> RawEvents{};

        int LNOBJ{};

        static BmsFile Parse(std::string source, int random_seed, std::ostream& cerr, std::ostream& cinfo, std::ostream& cdebug)
        {
#pragma warning(push)
#pragma warning(disable: 4456) //  declaration of '...' hides previous local declaration

            class ParserContext
            {
                std::ostream& cerr_;
                std::ostream& cinfo_;
                std::ostream& cdebug_;
                int current_line_number_{};

            public:
                ParserContext(std::ostream& c_error, std::ostream& c_info, std::ostream& c_debug) : cerr_(c_error), cinfo_(c_info), cdebug_(c_debug) {}

                BmsFile Parse(const std::string& source_script, int random_seed)
                {
                    using namespace std::string_view_literals;
                    std::istringstream input(source_script);
                    BmsFile output{};

                    std::string line{};
                    int line_num{};

                    // random context
                    std::default_random_engine random_engine_{static_cast<std::default_random_engine::result_type>(random_seed)};
                    int current_random_value_{};
                    bool ignoring_for_random_if{};

                    while (std::getline(input, line))
                    {
                        output.Source.push_back(line);
                        this->current_line_number_ = ++line_num;
                        if (line.empty()) continue;
                        if (line[0] != '#') continue;
                        std::string_view line_view = line;
                        if (!line_view.empty() && line_view.back() == '\r') line_view.remove_suffix(1);

                        const auto get_data_if_cmd_is = [&, line_upper = to_upper(line, 16)](std::string_view key) -> std::optional<std::string_view>
                        {
                            if (key.size() <= line_upper.size() && std::equal(key.begin(), key.end(), line_upper.begin()))
                                return std::optional<std::string_view>(std::in_place, line_view.substr(key.size()));
                            else
                                return std::nullopt;
                        };

                        // process #RANDOM and #IF

                        if (auto data = get_data_if_cmd_is("#ENDIF"sv))
                        {
                            cinfo_ << "#ENDIF at line " << line_num << std::endl;
                            ignoring_for_random_if = false;
                        }
                        else if (ignoring_for_random_if) {}

                        else if (auto data = get_data_if_cmd_is("#RANDOM "sv))
                        {
                            int rnd_max = Parse10(*data);
                            current_random_value_ = std::uniform_int_distribution(0, rnd_max)(random_engine_);
                            cinfo_ << "#RANDOM generated: " << line_num << " at line " << line_num << std::endl;
                        }

                        else if (auto data = get_data_if_cmd_is("#IF "sv))
                        {
                            ignoring_for_random_if = current_random_value_ != Parse10(*data);
                            cinfo_ << "#IF " << Parse10(*data) << " at line " << line_num << (ignoring_for_random_if ? " -> IGNORE" : " -> PICK") << std::endl;
                        }

                        // end process #RANDOM and #IF

                        else if (auto data = get_data_if_cmd_is("#TITLE "sv)) { cinfo_ << "#TITLE " << ((output.Information.Title = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#SUBTITLE "sv)) { cinfo_ << "#SUBTITLE " << ((output.Information.SubTitle = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#ARTIST "sv)) { cinfo_ << "#ARTIST " << ((output.Information.Artist = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#SUBARTIST "sv)) { cinfo_ << "#SUBARTIST " << ((output.Information.SubArtist = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#GENRE "sv)) { cinfo_ << "#GENRE " << ((output.Information.Genre = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#SUBGENRE "sv)) { cinfo_ << "#SUBGENRE " << ((output.Information.SubGenre = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#COMMENT "sv)) { cinfo_ << "#COMMENT " << ((output.Information.Comment = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#BANNER "sv)) { cinfo_ << "#BANNER " << ((output.Information.BannerBmpPath = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#STAGEFILE "sv)) { cinfo_ << "#STAGEFILE " << ((output.Information.StageBmpPath = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#BACKBMP "sv)) { cinfo_ << "#BACKBMP " << ((output.Information.BackBmpPath = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#PLAYER "sv)) { cinfo_ << "#PLAYER " << ((output.Information.Player = Parse10(*data))) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#RANK "sv)) { cinfo_ << "#STAGEFILE " << ((output.Information.Rank = Parse10(*data))) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#PLAYLEVEL "sv)) { cinfo_ << "#PLAYLEVEL " << ((output.Information.PlayLevel = Parse10(*data))) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#VOLWAV "sv)) { cinfo_ << "#VOLWAV " << ((output.Information.WavePlaybackVolume = Parse10(*data))) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#TOTAL "sv)) { cinfo_ << "#TOTAL " << ((output.Information.Total = Parse10(*data))) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#DIFFICULTY "sv)) { cinfo_ << "#DIFFICULTY " << ((output.Information.Difficulty = Parse10(*data))) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#BPM "sv)) { cinfo_ << "#BPM " << ((output.Information.InitialBpm = ParseFloat(*data).value_or(130.0))) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("%URL "sv)) { cinfo_ << "%URL " << ((output.Information.Url = *data)) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("%MAIL "sv)) { cinfo_ << "%MAIL " << ((output.Information.Mail = *data)) << std::endl; }

                        else if (auto data = get_data_if_cmd_is("#LNOBJ "sv)) { cdebug_ << "#LNOBJ " << ((output.LNOBJ = Parse36(*data))) << " / LongNoteEnabled=" << (output.Information.LongNotesDetected = true) << std::endl; }
                        else if (auto data = get_data_if_cmd_is("#LNTYPE "sv)) { cdebug_ << "#LNTYPE " << Parse36(*data) << " / LongNoteEnabled=" << (output.Information.LongNotesDetected = true) << std::endl; }

                        else if (auto data = get_data_if_cmd_is("#WAV"sv); data && data->size() > 3)
                        {
                            auto index = Parse36(data->substr(0, 2));
                            auto value = std::string(chomp(data->substr(2)));
                            cdebug_ << "#WAV[" << index << "] = " << ((output.WaveTable[index] = value)) << std::endl;
                        }

                        else if (auto data = get_data_if_cmd_is("#OGG"sv); data && data->size() > 3)
                        {
                            auto index = Parse36(data->substr(0, 2));
                            auto value = chomp(data->substr(3));
                            cdebug_ << "#OGG[" << index << "] = " << ((output.WaveTable[index] = value)) << std::endl;
                        }

                        else if (auto data = get_data_if_cmd_is("#BMP"sv); data && data->size() > 3)
                        {
                            auto index = Parse36(data->substr(0, 2));
                            auto value = chomp(data->substr(3));
                            cdebug_ << "#BMP[" << index << "] = " << ((output.BmpTable[index] = value)) << std::endl;
                        }

                        else if (auto data = get_data_if_cmd_is("#TEXT"sv); data && data->size() > 3)
                        {
                            auto index = Parse36(data->substr(0, 2));
                            auto value = chomp(data->substr(3));
                            cdebug_ << "#TEXT[" << index << "] = " << ((output.TextTable[index] = value)) << std::endl;
                        }

                        else if (auto data = get_data_if_cmd_is("#BPM"sv); data && data->size() > 3)
                        {
                            auto index = Parse36(data->substr(0, 2));
                            auto value = ParseFloat(chomp(data->substr(3)));
                            cdebug_ << "#BPM[" << index << "] = " << ((output.BpmTable[index] = value.value_or(Bpm{}))) << std::endl;
                        }

                        else if (auto data = get_data_if_cmd_is("#STOP"sv); data && data->size() > 2)
                        {
                            auto index = Parse36(data->substr(0, 2));
                            auto value = Parse10(chomp(data->substr(2)));
                            cdebug_ << "#STOP[" << index << "] = " << ((output.StopTable[index] = value)) << std::endl;
                        }

                        else if (auto data = [&]() -> std::optional<std::tuple<MeasureNumber, BmsEventCode, std::string_view>>
                        {
                            // Try parse "#xxxyy:"
                            if (line_view.size() < "#xxxyy:"sv.size()) return std::nullopt;
                            if (line_view[6] != ':') return std::nullopt;
                            auto measure = ParseInteger(line_view.substr(1, 3), 10);
                            auto channel = ParseInteger(line_view.substr(4, 2), 16);
                            if (!measure || !channel) return std::nullopt;

                            return std::tuple(
                                static_cast<MeasureNumber>(measure.value()),
                                static_cast<BmsEventCode>(channel.value()),
                                chomp(line_view.substr(7)));
                        }())
                        {
                            auto [measure, channel, data_body] = *data;

                            switch (channel)
                            {
                            case BmsEventCode::PlayBackChorus:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                break;
                            case BmsEventCode::ChangeBarLength:
                                output.MeasureLength[measure] = ParseFloat(data_body).value_or(1.0f);
                                break;
                            case BmsEventCode::ChangeBPM:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse16array(data_body)});
                                break;
                            case BmsEventCode::ChangeBPMFromTable:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                break;


                            case BmsEventCode::ChangeBMP:
                            case BmsEventCode::ChangeBMPPoor:
                            case BmsEventCode::ChangeBMPLayer:
                            case BmsEventCode::ChangeBMPLayer2:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                output.Information.BgaDetected = true;
                                break;

                            case BmsEventCode::StopSequenceFromTable:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                break;

                            case BmsEventCode::NormalNote00:
                            case BmsEventCode::NormalNote01:
                            case BmsEventCode::NormalNote02:
                            case BmsEventCode::NormalNote03:
                            case BmsEventCode::NormalNote04:
                            case BmsEventCode::NormalNote05:
                            case BmsEventCode::NormalNote06:
                            case BmsEventCode::NormalNote07:
                            case BmsEventCode::NormalNote08:
                            case BmsEventCode::NormalNote09:
                            case BmsEventCode::NormalNote0A:
                            case BmsEventCode::NormalNote0B:
                            case BmsEventCode::NormalNote0C:
                            case BmsEventCode::NormalNote0D:
                            case BmsEventCode::NormalNote0E:
                            case BmsEventCode::NormalNote0F:
                            case BmsEventCode::NormalNote10:
                            case BmsEventCode::NormalNote11:
                            case BmsEventCode::NormalNote12:
                            case BmsEventCode::NormalNote13:
                            case BmsEventCode::NormalNote14:
                            case BmsEventCode::NormalNote15:
                            case BmsEventCode::NormalNote16:
                            case BmsEventCode::NormalNote17:
                            case BmsEventCode::NormalNote18:
                            case BmsEventCode::NormalNote19:
                            case BmsEventCode::NormalNote1A:
                            case BmsEventCode::NormalNote1B:
                            case BmsEventCode::NormalNote1C:
                            case BmsEventCode::NormalNote1D:
                            case BmsEventCode::NormalNote1E:
                            case BmsEventCode::NormalNote1F:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                break;

                            case BmsEventCode::HiddenNote00:
                            case BmsEventCode::HiddenNote01:
                            case BmsEventCode::HiddenNote02:
                            case BmsEventCode::HiddenNote03:
                            case BmsEventCode::HiddenNote04:
                            case BmsEventCode::HiddenNote05:
                            case BmsEventCode::HiddenNote06:
                            case BmsEventCode::HiddenNote07:
                            case BmsEventCode::HiddenNote08:
                            case BmsEventCode::HiddenNote09:
                            case BmsEventCode::HiddenNote0A:
                            case BmsEventCode::HiddenNote0B:
                            case BmsEventCode::HiddenNote0C:
                            case BmsEventCode::HiddenNote0D:
                            case BmsEventCode::HiddenNote0E:
                            case BmsEventCode::HiddenNote0F:
                            case BmsEventCode::HiddenNote10:
                            case BmsEventCode::HiddenNote11:
                            case BmsEventCode::HiddenNote12:
                            case BmsEventCode::HiddenNote13:
                            case BmsEventCode::HiddenNote14:
                            case BmsEventCode::HiddenNote15:
                            case BmsEventCode::HiddenNote16:
                            case BmsEventCode::HiddenNote17:
                            case BmsEventCode::HiddenNote18:
                            case BmsEventCode::HiddenNote19:
                            case BmsEventCode::HiddenNote1A:
                            case BmsEventCode::HiddenNote1B:
                            case BmsEventCode::HiddenNote1C:
                            case BmsEventCode::HiddenNote1D:
                            case BmsEventCode::HiddenNote1E:
                            case BmsEventCode::HiddenNote1F:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                break;

                            case BmsEventCode::LongNote00:
                            case BmsEventCode::LongNote01:
                            case BmsEventCode::LongNote02:
                            case BmsEventCode::LongNote03:
                            case BmsEventCode::LongNote04:
                            case BmsEventCode::LongNote05:
                            case BmsEventCode::LongNote06:
                            case BmsEventCode::LongNote07:
                            case BmsEventCode::LongNote08:
                            case BmsEventCode::LongNote09:
                            case BmsEventCode::LongNote0A:
                            case BmsEventCode::LongNote0B:
                            case BmsEventCode::LongNote0C:
                            case BmsEventCode::LongNote0D:
                            case BmsEventCode::LongNote0E:
                            case BmsEventCode::LongNote0F:
                            case BmsEventCode::LongNote10:
                            case BmsEventCode::LongNote11:
                            case BmsEventCode::LongNote12:
                            case BmsEventCode::LongNote13:
                            case BmsEventCode::LongNote14:
                            case BmsEventCode::LongNote15:
                            case BmsEventCode::LongNote16:
                            case BmsEventCode::LongNote17:
                            case BmsEventCode::LongNote18:
                            case BmsEventCode::LongNote19:
                            case BmsEventCode::LongNote1A:
                            case BmsEventCode::LongNote1B:
                            case BmsEventCode::LongNote1C:
                            case BmsEventCode::LongNote1D:
                            case BmsEventCode::LongNote1E:
                            case BmsEventCode::LongNote1F:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                break;

                            case BmsEventCode::ChangeTextFromList:
                                output.RawEvents.emplace_back(EventVector{measure, channel, Parse36array(data_body)});
                                break;

                            case BmsEventCode::ExtChr:
                                // not supported
                                cerr_ << "Warning: Ignoring ExtChr channel data at line " << line_num << ": " << line_view << std::endl;
                                break;

                            default:
                                cerr_ << "Warning: Ignoring unknown channel data at line " << line_num << ": " << line_view << std::endl;
                                break;
                            }
                        }
                        else
                        {
                            cerr_ << "Warning: Ignoring unknown command data at line " << line_num << ": " << line_view << std::endl;
                        }
                    }

                    return output;
                }

            private:
                // Chomp whitespace characters
                static std::string_view chomp(std::string_view v)
                {
                    while (!v.empty() && v.front() <= ' ') v.remove_prefix(1);
                    while (!v.empty() && v.back() <= ' ') v.remove_suffix(1);
                    return v;
                }

                static std::string to_upper(const std::string& s, std::string::size_type max_size = std::string::npos)
                {
                    std::string upper = s.substr(0, max_size == std::string::npos ? std::string::npos : std::min(s.size(), max_size));
                    for (char& c : upper) c = static_cast<char>(std::toupper(c));
                    return upper;
                }

                // Parse base 10 integer
                std::optional<int> ParseInteger(std::string_view v, int base)
                {
                    v = chomp(v);
                    int value = 0;
                    auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), value, base);
                    if (ec != std::errc() || ptr != v.data() + v.size())
                    {
                        cerr_ << "Parse error: integer(base" << base << ") at line " << current_line_number_ << std::endl;
                        return std::nullopt;
                    }
                    return std::optional<int>{std::in_place, value};
                }

                std::optional<double> ParseFloat(std::string_view v)
                {
                    v = chomp(v);
                    double value{};
                    auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), value);
                    if (ec != std::errc() || ptr != v.data() + v.size())
                    {
                        cerr_ << "Parse error: floating(base10) at line " << current_line_number_ << std::endl;
                        return std::nullopt;
                    }
                    return std::optional<double>{std::in_place, value};
                }

                std::optional<std::vector<int>> ParseIntegerArray(std::string_view v, int base)
                {
                    v = chomp(v);
                    if (v.size() % 2 != 0)
                    {
                        cerr_ << "Parse error: length of integer array is not correct at line " << current_line_number_ << std::endl;
                        return std::nullopt;
                    }

                    std::vector<int> result;
                    for (size_t i = 0; i < v.size() / 2; i++)
                        result.push_back(ParseInteger(v.substr(i * 2, 2), base).value_or(0));

                    return std::optional<std::vector<int>>{std::in_place, std::move(result)};
                }


                int Parse10(std::string_view v) { return ParseInteger(v, 10).value_or(0); }
                int Parse16(std::string_view v) { return ParseInteger(v, 16).value_or(0); }
                int Parse36(std::string_view v) { return ParseInteger(v, 36).value_or(0); }
                std::vector<int> Parse16array(std::string_view v) { return ParseIntegerArray(v, 16).value_or(std::vector<int>()); }
                std::vector<int> Parse36array(std::string_view v) { return ParseIntegerArray(v, 36).value_or(std::vector<int>()); }
            };

            return ParserContext(cerr, cinfo, cdebug).Parse(source, random_seed);
        }
    };
}
