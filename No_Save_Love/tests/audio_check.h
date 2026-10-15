#pragma once

// 실제 배포 폴더의 긴 경로를 사용한다. 파일 존재 여부만 검사하면 waveaudio 오류를 놓친다.
void CheckEffectSounds(HWND window)
{
    const auto source = std::filesystem::current_path();
    const auto package = source.parent_path() / L"No_Save_Love_Distribution";
    for (const auto& root : {source, package})
    {
        std::filesystem::current_path(root);
        GameManager manager;
        manager.Initialize(window);
        // 실제로 등록된 공통 효과음 18개와 리듬게임 효과음 4개를 같은 재생 경로로 검사한다.
        const wchar_t* common[] = {L"click", L"minigame1_success", L"minigame1_fail", L"minigame1_water",
            L"minigame1_noodle_soup", L"minigame1_topping", L"minigame3_jump", L"minigame3_slide",
            L"minigame3_hit", L"minigame3_jelly_collect", L"minigame4_lolpopup", L"minigame4_malpa_rock",
            L"minigame4_malpa_ult", L"minigame4_rider", L"minigame4_reels", L"minigame4_food", L"defense_shot", L"defense_support"};
        const auto playing = [](const std::wstring& alias)
        {
            wchar_t mode[64] = {};
            assert(mciSendStringW((L"status " + alias + L" mode").c_str(), mode, 64, nullptr) == 0);
            assert(std::wstring(mode) == L"playing");
        };
        audioManager.PlayBgm(L"story1");
        for (const auto key : common)
        {
            audioManager.SetSfxVolume(key, 0);
            for (int i = 0; i < 3; ++i) { audioManager.PlaySfx(key); playing(L"sfx_" + std::wstring(key)); }
            audioManager.StopSfx(key);
        }
        playing(L"currentBgm"); // 효과음 반복 재생이 BGM을 끊지 않는다.
        manager.minigame2.LoadEffectSounds();
        for (const auto alias : {L"hitSound", L"missSound", L"windowSound", L"impactSound"})
        {
            assert(mciSendStringW((L"setaudio " + std::wstring(alias) + L" volume to 0").c_str(), nullptr, 0, nullptr) == 0);
            for (int i = 0; i < 3; ++i) { manager.minigame2.PlayEffectSound(alias); playing(alias); }
        }
        manager.minigame2.CloseEffectSounds();
        manager.Shutdown();
    }
    std::filesystem::current_path(source);
    std::cout << "PASS: source/package 22 SFX open/play/restart, rhythm effects, simultaneous BGM\n";
}
