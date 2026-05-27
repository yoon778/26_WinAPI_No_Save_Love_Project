#include "AudioManager.h"

AudioManager::AudioManager()
{
    bgmVolume = 250;
    sfxVolume = 700;
    initialized = false;
    currentBgmAlias = L"currentBgm";
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

void AudioManager::StopBgm()
{
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