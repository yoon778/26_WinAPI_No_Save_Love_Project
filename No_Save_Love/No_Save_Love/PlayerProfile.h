#pragma once
#include <windows.h>
#include <array>
#include <string>
#include <filesystem>
#include <algorithm>
#include "GameOptions.h"

// 설정과 해금만 보관한다. 진행 중인 이야기의 저장/불러오기 기능은 아니다.
struct PlayerProfile
{
    unsigned int events = 0;
    unsigned int endings = 0;
    std::array<std::array<int, 6>, 3> bestScores{};
    std::wstring storagePath;
    bool saveFailed = false;

    void Load(const std::wstring& explicitPath = L"")
    {
        storagePath = explicitPath;
        if (storagePath.empty())
        {
            wchar_t path[32768]{};
            // 자동 검사에서는 별도 파일을 지정해 실제 플레이 기록을 보존한다.
            DWORD size = GetEnvironmentVariableW(L"NO_SAVE_LOVE_PROFILE", path, 32768);
            if (size > 0 && size < 32768) storagePath = path;
            else
            {
                size = GetEnvironmentVariableW(L"LOCALAPPDATA", path, 32768);
                if (size > 0 && size < 32768) storagePath = std::wstring(path) + L"\\NoSaveLove\\profile.ini";
            }
        }
        if (storagePath.empty()) { saveFailed = true; return; }
        const auto read = [&](const wchar_t* section, const wchar_t* key, int fallback, int max) {
            return std::clamp(static_cast<int>(GetPrivateProfileIntW(section, key, fallback, storagePath.c_str())), 0, max);
        };
        GameOptions::difficulty = read(L"settings", L"difficulty", 1, 2);
        GameOptions::musicVolume = read(L"settings", L"music", 100, 100);
        GameOptions::effectsVolume = read(L"settings", L"effects", 100, 100);
        GameOptions::textSpeed = read(L"settings", L"text", 1, 2);
        GameOptions::screenShake = read(L"settings", L"shake", 1, 1) != 0;
        GameOptions::fullscreen = read(L"settings", L"fullscreen", 1, 1) != 0;
        events = read(L"unlocks", L"events", 0, 32767);
        endings = read(L"unlocks", L"endings", 0, 127);
        for (int d = 0; d < 3; ++d)
            for (int g = 0; g < 6; ++g)
            {
                const std::wstring key = std::to_wstring(d) + L"_" + std::to_wstring(g);
                bestScores[d][g] = read(L"scores", key.c_str(), 0, 100);
            }
    }
    bool Save()
    {
        if (storagePath.empty()) { saveFailed = true; return false; }
        std::error_code error;
        auto parent = std::filesystem::path(storagePath).parent_path();
        if (!parent.empty()) std::filesystem::create_directories(parent, error);
        if (error) { saveFailed = true; return false; }
        bool ok = true;
        const auto write = [&](const wchar_t* section, const wchar_t* key, int value) {
            if (!WritePrivateProfileStringW(section, key, std::to_wstring(value).c_str(), storagePath.c_str())) ok = false;
        };
        write(L"settings", L"difficulty", GameOptions::difficulty);
        write(L"settings", L"music", GameOptions::musicVolume);
        write(L"settings", L"effects", GameOptions::effectsVolume);
        write(L"settings", L"text", GameOptions::textSpeed);
        write(L"settings", L"shake", GameOptions::screenShake);
        write(L"settings", L"fullscreen", GameOptions::fullscreen);
        write(L"unlocks", L"events", events);
        write(L"unlocks", L"endings", endings);
        for (int d = 0; d < 3; ++d)
            for (int g = 0; g < 6; ++g)
            {
                const std::wstring key = std::to_wstring(d) + L"_" + std::to_wstring(g);
                write(L"scores", key.c_str(), bestScores[d][g]);
            }
        saveFailed = !ok;
        return ok;
    }
    bool HasEvent(int round, int heroine) const
    {
        return round >= 0 && round < 5 && heroine >= 0 && heroine < 3
            && (events & (1u << (round * 3 + heroine))) != 0;
    }
};
