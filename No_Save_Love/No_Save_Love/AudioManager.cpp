#include "AudioManager.h"

AudioManager::AudioManager()
{
    bgmVolume = 250;
    sfxVolume = 700;
    initialized = false;
    currentBgmAlias = L"currentBgm";
    isBgmFadeInActive = false;
    bgmFadeInStartTime = 0;
    bgmFadeInDuration = 0;
    bgmFadeInStartVolume = 0;
    bgmFadeInTargetVolume = 250;
    isBgmFadeOutActive = false;
    bgmFadeOutStartTime = 0;
    bgmFadeOutDuration = 0;
    bgmFadeOutStartVolume = 0;
}

AudioManager::~AudioManager()
{
    Shutdown();
}

void AudioManager::Initialize()
{
    if (initialized)
    {
        return;
    }

    initialized = true;
}

void AudioManager::Shutdown()
{
    if (!initialized)
    {
        return;
    }

    StopBgm();
    CloseAllSfx();

    bgmPaths.clear();
    sfxPaths.clear();
    sfxAliases.clear();
    isBgmFadeInActive = false;
    isBgmFadeOutActive = false;

    initialized = false;
}

void AudioManager::RegisterBgm(const std::wstring& key, const std::wstring& path)
{
    bgmPaths[key] = path;
}

void AudioManager::RegisterSfx(const std::wstring& key, const std::wstring& path)
{
    sfxPaths[key] = path;

    std::wstring alias = MakeSfxAlias(key);
    sfxAliases[key] = alias;

    SendMciCommand(L"close " + alias);
    SendMciCommand(L"open \"" + path + L"\" type waveaudio alias " + alias);
    SendMciCommand(L"setaudio " + alias + L" volume to " + std::to_wstring(sfxVolume));
}

void AudioManager::PlayBgm(const std::wstring& key, bool repeat)
{
    if (key.empty())
    {
        return;
    }

    if (key == L"stop")
    {
        StopBgm();
        return;
    }

    if (currentBgmKey == key)
    {
        return;
    }

    auto it = bgmPaths.find(key);
    if (it == bgmPaths.end())
    {
        return;
    }

    isBgmFadeInActive = false;
    isBgmFadeOutActive = false;

    CloseCurrentBgm();

    const std::wstring& path = it->second;

    SendMciCommand(L"open \"" + path + L"\" type mpegvideo alias " + currentBgmAlias);
    SendMciCommand(L"setaudio " + currentBgmAlias + L" volume to " + std::to_wstring(bgmVolume));

    if (repeat)
    {
        SendMciCommand(L"play " + currentBgmAlias + L" repeat");
    }
    else
    {
        SendMciCommand(L"play " + currentBgmAlias);
    }

    currentBgmKey = key;
}

void AudioManager::PlayBgmFadeIn(const std::wstring& key, int startVolume, int targetVolume, DWORD durationMs, bool repeat)
{
    if (key.empty())
    {
        return;
    }

    if (key == L"stop")
    {
        StopBgm();
        return;
    }

    if (currentBgmKey == key)
    {
        return;
    }

    if (bgmPaths.find(key) == bgmPaths.end())
    {
        return;
    }

    bgmFadeInStartVolume = startVolume;
    bgmFadeInTargetVolume = targetVolume;
    bgmFadeInDuration = durationMs;
    bgmFadeInStartTime = GetTickCount();
    isBgmFadeInActive = durationMs > 0;
    isBgmFadeOutActive = false;

    SetBgmVolume(startVolume);
    PlayBgm(key, repeat);

    if (durationMs == 0)
    {
        SetBgmVolume(targetVolume);
    }
}

void AudioManager::FadeOutBgm(DWORD durationMs)
{
    if (currentBgmKey.empty())
    {
        return;
    }

    isBgmFadeInActive = false;

    if (durationMs == 0)
    {
        StopBgm();
        return;
    }

    isBgmFadeOutActive = true;
    bgmFadeOutStartTime = GetTickCount();
    bgmFadeOutDuration = durationMs;
    bgmFadeOutStartVolume = bgmVolume;
}

void AudioManager::StopBgm()
{
    isBgmFadeInActive = false;
    isBgmFadeOutActive = false;
    CloseCurrentBgm();
    currentBgmKey.clear();
}

void AudioManager::PlaySfx(const std::wstring& key)
{
    auto it = sfxAliases.find(key);
    if (it == sfxAliases.end())
    {
        return;
    }

    const std::wstring& alias = it->second;

    SendMciCommand(L"stop " + alias);
    SendMciCommand(L"seek " + alias + L" to start");
    SendMciCommand(L"play " + alias);
}

void AudioManager::Update()
{
    if (isBgmFadeOutActive && !currentBgmKey.empty())
    {
        DWORD currentTime = GetTickCount();
        DWORD elapsedTime = currentTime - bgmFadeOutStartTime;

        if (elapsedTime >= bgmFadeOutDuration)
        {
            StopBgm();
            return;
        }

        int nextVolume = bgmFadeOutStartVolume -
            static_cast<int>(bgmFadeOutStartVolume * elapsedTime / bgmFadeOutDuration);

        SetBgmVolume(nextVolume);
        return;
    }

    if (!isBgmFadeInActive || currentBgmKey.empty())
    {
        return;
    }

    DWORD currentTime = GetTickCount();
    DWORD elapsedTime = currentTime - bgmFadeInStartTime;

    if (elapsedTime >= bgmFadeInDuration)
    {
        SetBgmVolume(bgmFadeInTargetVolume);
        isBgmFadeInActive = false;
        return;
    }

    int volumeRange = bgmFadeInTargetVolume - bgmFadeInStartVolume;
    int nextVolume = bgmFadeInStartVolume +
        static_cast<int>(volumeRange * elapsedTime / bgmFadeInDuration);

    SetBgmVolume(nextVolume);
}

void AudioManager::SetBgmVolume(int volume)
{
    bgmVolume = volume;

    if (!currentBgmKey.empty())
    {
        SendMciCommand(L"setaudio " + currentBgmAlias + L" volume to " + std::to_wstring(bgmVolume));
    }
}

void AudioManager::SetSfxVolume(int volume)
{
    sfxVolume = volume;

    for (const auto& pair : sfxAliases)
    {
        SendMciCommand(L"setaudio " + pair.second + L" volume to " + std::to_wstring(sfxVolume));
    }
}

std::wstring AudioManager::GetCurrentBgmKey() const
{
    return currentBgmKey;
}

std::wstring AudioManager::MakeSfxAlias(const std::wstring& key) const
{
    return L"sfx_" + key;
}

void AudioManager::CloseCurrentBgm()
{
    if (currentBgmAlias.empty())
    {
        return;
    }

    SendMciCommand(L"stop " + currentBgmAlias);
    SendMciCommand(L"close " + currentBgmAlias);
}

void AudioManager::CloseAllSfx()
{
    for (const auto& pair : sfxAliases)
    {
        SendMciCommand(L"close " + pair.second);
    }
}

bool AudioManager::SendMciCommand(const std::wstring& command) const
{
    MCIERROR result = mciSendString(command.c_str(), nullptr, 0, nullptr);
    return result == 0;
}
