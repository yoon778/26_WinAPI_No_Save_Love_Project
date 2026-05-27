#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <unordered_map>

#pragma comment(lib, "winmm.lib")

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    void Initialize();
    void Shutdown();

    void RegisterBgm(const std::wstring& key, const std::wstring& path);
    void RegisterSfx(const std::wstring& key, const std::wstring& path);

    void PlayBgm(const std::wstring& key, bool repeat = true);
    void PlayBgmFadeIn(const std::wstring& key, int startVolume, int targetVolume, DWORD durationMs, bool repeat = true);
    void StopBgm();

    void PlaySfx(const std::wstring& key);

    void Update();

    void SetBgmVolume(int volume);
    void SetSfxVolume(int volume);

    std::wstring GetCurrentBgmKey() const;

private:
    std::unordered_map<std::wstring, std::wstring> bgmPaths;
    std::unordered_map<std::wstring, std::wstring> sfxPaths;
    std::unordered_map<std::wstring, std::wstring> sfxAliases;

    std::wstring currentBgmKey;
    std::wstring currentBgmAlias;

    int bgmVolume;
    int sfxVolume;
    bool initialized;

    bool isBgmFadeInActive;
    DWORD bgmFadeInStartTime;
    DWORD bgmFadeInDuration;
    int bgmFadeInStartVolume;
    int bgmFadeInTargetVolume;

private:
    std::wstring MakeSfxAlias(const std::wstring& key) const;

    void CloseCurrentBgm();
    void CloseAllSfx();

    bool SendMciCommand(const std::wstring& command) const;
};
