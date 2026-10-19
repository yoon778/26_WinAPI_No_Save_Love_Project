#pragma once
#include "../No_Save_Love/DisplayGeometry.h"

void SaveQualityFrame(GameManager& manager, const std::filesystem::path& path, bool scaledOutput = false)
{
    CImage image; image.Create(1920, 1080, 24);
    HDC dc = image.GetDC(); manager.Render(dc); GdiFlush();
    if (manager.m_panel == GameManager::Panel::Notebook && manager.m_completedChoices[manager.m_notebookPage] >= 0)
    {
        const COLORREF firstPixel = GetPixel(dc, 320, 365);
        bool hasPicture = false;
        for (int y = 365; y < 680; y += 50)
            for (int x = 320; x < 890; x += 50)
                hasPicture |= GetPixel(dc, x, y) != firstPixel;
        assert(hasPicture); // 프로시저 배경의 사건도 빈 사진 칸으로 남으면 안 된다.
    }
    if (scaledOutput)
    {
        CImage scaled; scaled.Create(1366, 768, 24);
        HDC target = scaled.GetDC();
        const RECT client{0, 0, 1366, 768}, viewport = DisplayGeometry::Fit(1366, 768);
        FillRect(target, &client, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        SetStretchBltMode(target, HALFTONE); SetBrushOrgEx(target, 0, 0, nullptr);
        assert(StretchBlt(target, viewport.left, viewport.top, viewport.right - viewport.left,
            viewport.bottom - viewport.top, dc, 0, 0, 1920, 1080, SRCCOPY));
        scaled.ReleaseDC();
        assert(SUCCEEDED(scaled.Save(path.c_str())));
    }
    image.ReleaseDC();
    if (!scaledOutput) assert(SUCCEEDED(image.Save(path.c_str())));
}

void CheckQualityFeatures(HWND window, const std::filesystem::path& output)
{
    // 환경 변수는 이 프로세스에서만 바꾸며 실제 사용자의 해금/설정 파일을 건드리지 않는다.
    struct RestoreSettings
    {
        int difficulty = GameOptions::difficulty, music = GameOptions::musicVolume;
        int effects = GameOptions::effectsVolume, text = GameOptions::textSpeed;
        bool shake = GameOptions::screenShake, fullscreen = GameOptions::fullscreen;
        std::wstring profile;
        RestoreSettings()
        {
            wchar_t path[32768]{};
            GetEnvironmentVariableW(L"NO_SAVE_LOVE_PROFILE", path, 32768); profile = path;
        }
        ~RestoreSettings()
        {
            SetEnvironmentVariableW(L"NO_SAVE_LOVE_PROFILE", profile.empty() ? nullptr : profile.c_str());
            GameOptions::difficulty = difficulty; GameOptions::musicVolume = music;
            GameOptions::effectsVolume = effects; GameOptions::textSpeed = text;
            GameOptions::screenShake = shake; GameOptions::fullscreen = fullscreen;
        }
    } restore;
    const auto profilePath = std::filesystem::absolute(output / (L"quality_profile_" + std::to_wstring(GetTickCount64()) + L".ini"));
    SetEnvironmentVariableW(L"NO_SAVE_LOVE_PROFILE", profilePath.c_str());
    GameOptions::difficulty = GameOptions::textSpeed = 1;
    GameOptions::musicVolume = GameOptions::effectsVolume = 100;
    GameOptions::screenShake = GameOptions::fullscreen = true;
    PlayerProfile seed; seed.storagePath = profilePath; assert(seed.Save());

    GameManager manager; manager.Initialize(window); CompleteTransition(manager);
    SaveQualityFrame(manager, output / L"quality_title.png");
    manager.OnMouseClick(600, 850);
    assert(manager.m_panel == GameManager::Panel::Settings);
    manager.OnMouseClick(1060, 280); // 스토리 난이도
    manager.OnMouseClick(1080, 425); // 음악 90%
    manager.OnMouseClick(1080, 505); // 효과음 90%
    manager.OnMouseClick(1580, 590); // 글자 빠르게
    manager.OnMouseClick(1580, 680); // 흔들림 끄기
    manager.OnMouseClick(1580, 770); // 창 모드
    assert(GameOptions::difficulty == 0 && GameOptions::musicVolume == 90 && GameOptions::effectsVolume == 90);
    assert(GameOptions::textSpeed == 2 && !GameOptions::screenShake && !GameOptions::fullscreen);
    SaveQualityFrame(manager, output / L"quality_settings.png");
    SaveQualityFrame(manager, output / L"quality_settings_1366.png", true);
    PlayerProfile loaded; loaded.Load(profilePath);
    assert(GameOptions::difficulty == 0 && GameOptions::musicVolume == 90 && !GameOptions::fullscreen);
    assert(loaded.events == 0 && loaded.endings == 0);
    manager.OnMouseClick(1600, 145);
    assert(!manager.m_exitConfirmOpen && manager.m_panel == GameManager::Panel::None);

    const auto samePlayer = [](const Player_state& a, const Player_state& b) {
        return a.money == b.money && a.speech == b.speech && a.charm == b.charm && a.appearance == b.appearance;
    };
    const auto gained = [](Player_state state, int game, int score) {
        const auto bonus = StatBalance::CalculateGain(game, score);
        state.money += bonus.money; state.speech += bonus.speech;
        state.charm += bonus.charm; state.appearance += bonus.appearance; return state;
    };
    const auto finishDefense = [&]() {
        manager.promiseScene.BeginBattle(); manager.promiseScene.time = 160;
        manager.promiseScene.baseHp = 100; manager.promiseScene.kills = 42;
        manager.promiseScene.spawnIndex = 30;
        manager.promiseScene.victory = manager.promiseScene.finished = true;
        manager.FinishCurrentMiniGameIfNeeded(); CompleteTransition(manager);
    };
    GameOptions::difficulty = 1; GameOptions::musicVolume = GameOptions::effectsVolume = 0; manager.ApplyOptions();
    manager.DebugEnterMiniGameByIndex(4); CompleteTransition(manager);
    const Player_state before{20, 20, 20, 20}; manager.player = before;
    finishDefense();
    assert(samePlayer(manager.player, gained(before, 4, 100)));
    assert(manager.resultScene.m_difficulty == 1 && manager.resultScene.m_details[0].find(L"100%") != std::wstring::npos);
    SaveQualityFrame(manager, output / L"quality_result.png");
    manager.EnterResult(4, 100); // 같은 결과의 중복 반영 방지
    assert(samePlayer(manager.player, gained(before, 4, 100)));
    manager.OnMouseClick(440, 980); CompleteTransition(manager);
    assert(manager.now_game_mode == GameManager::MiniGame5 && samePlayer(manager.player, before));
    finishDefense();
    assert(samePlayer(manager.player, gained(before, 4, 100)));
    manager.ReturnToTitle(); CompleteTransition(manager);

    manager.OnMouseClick(1260, 850);
    assert(manager.m_panel == GameManager::Panel::Practice);
    manager.OnMouseClick(1000, 330); // 연습 동행 이린
    SaveQualityFrame(manager, output / L"quality_practice.png");
    manager.OnMouseClick(500, 760); CompleteTransition(manager);
    assert(manager.m_practice && manager.now_game_mode == GameManager::MiniGameTutor5);
    manager.OpenPanel(GameManager::Panel::Settings);
    manager.OnMouseClick(1060, 280); manager.OnMouseClick(1600, 145);
    manager.OnMouseClick(960, 838); CompleteTransition(manager);
    assert(manager.now_game_mode == GameManager::MiniGame5 && manager.promiseScene.difficulty == 0);
    assert(manager.m_attemptDifficulty == 0 && manager.promiseScene.heroine == 2);
    const Player_state practicePlayer = manager.player;
    finishDefense();
    assert(samePlayer(manager.player, practicePlayer) && manager.resultScene.m_practice);
    assert(manager.m_profile.bestScores[0][4] == 100);
    manager.OnMouseClick(440, 980); CompleteTransition(manager);
    assert(manager.m_practice && samePlayer(manager.player, practicePlayer));
    finishDefense();
    assert(samePlayer(manager.player, practicePlayer));
    manager.OnMouseClick(960, 980); CompleteTransition(manager); manager.OnTimer(window);
    assert(manager.m_panel == GameManager::Panel::Practice && !manager.m_practice);
    manager.ClosePanel();

    for (int round = 0; round < 5; ++round)
    {
        manager.currentStoryRound = round;
        manager.currentMiniGameIndex = round < 4 ? round + 1 : 4;
        assert(manager.EnterBranchStory(round % 3)); CompleteTransition(manager);
        manager.storyScene.finished = true; manager.OnMouseClick(0, 0);
        if (round == 4)
        {
            assert(manager.m_completedChoices[4] == -1 && !manager.m_profile.HasEvent(4, 1));
            CompleteTransition(manager);
            assert(manager.now_game_mode == GameManager::MiniGameTutor5);
            // 다섯 번째 사건은 실전 뒤 대화가 끝난 시점에만 완성된다.
            manager.currentMiniGameIndex = 5; manager.now_game_mode = GameManager::Story;
            manager.storyScene.SetDialogues(manager.storyData.GetPromiseStory(1, true));
            manager.storyScene.finished = true; manager.OnMouseClick(0, 0);
        }
        assert(manager.m_panel == GameManager::Panel::Notebook && manager.m_completedChoices[round] == round % 3);
        assert(manager.m_profile.HasEvent(round, round % 3) && manager.m_activeEvent == -1);
        SaveQualityFrame(manager, output / (L"quality_notebook_" + std::to_wstring(round + 1) + L".png"));
        if (round == 4) SaveQualityFrame(manager, output / L"quality_notebook_1366.png", true);
        manager.OnMouseClick(1600, 145); CompleteTransition(manager);
        assert(manager.m_panel == GameManager::Panel::None && !manager.m_exitConfirmOpen);
        if (round == 4) assert(manager.m_performanceIntroShown && manager.now_game_mode == GameManager::Story);
    }

    manager.now_game_mode = GameManager::EndingDialogue; manager.finalHeroineIndex = 0;
    manager.player = {90, 90, 90, 90}; manager.storyScene.finished = true;
    manager.AdvanceFinishedStory(); CompleteTransition(manager);
    assert(manager.now_game_mode == GameManager::Ending && (manager.m_profile.endings & 1u));
    manager.endingScene.scrollOffset = manager.endingScene.stopOffset;
    manager.OnMouseClick(760, 950); CompleteTransition(manager);
    assert(manager.now_game_mode == GameManager::Title);
    manager.OnMouseClick(950, 850);
    assert(manager.m_panel == GameManager::Panel::Gallery);
    SaveQualityFrame(manager, output / L"quality_gallery.png");
    const auto events = manager.m_profile.events, endings = manager.m_profile.endings;
    manager.OnMouseClick(900, 340); // 잠긴 첫 사건 하린 경로
    assert(!manager.m_replaying && manager.m_panel == GameManager::Panel::Gallery);
    manager.OnMouseClick(400, 340); CompleteTransition(manager);
    assert(manager.m_replaying && manager.now_game_mode == GameManager::Story);
    manager.storyScene.finished = true; manager.OnMouseClick(0, 0); CompleteTransition(manager); manager.OnTimer(window);
    assert(manager.m_panel == GameManager::Panel::Gallery && manager.m_profile.events == events);
    manager.OnMouseClick(600, 245); manager.OnMouseClick(400, 340); CompleteTransition(manager);
    assert(manager.m_replaying && manager.now_game_mode == GameManager::EndingDialogue);
    manager.storyScene.finished = true; manager.OnMouseClick(0, 0); CompleteTransition(manager);
    assert(manager.now_game_mode == GameManager::Ending && manager.m_profile.endings == endings);
    manager.endingScene.scrollOffset = manager.endingScene.stopOffset;
    manager.OnMouseClick(760, 950); CompleteTransition(manager); manager.OnTimer(window);
    assert(manager.m_panel == GameManager::Panel::Gallery && !manager.m_replaying);
    assert(manager.currentMiniGameIndex == 0 && manager.currentStoryRound == 0);
    for (const auto& character : manager.characters) assert(character.choice_time == 0);
    assert(manager.m_profile.events == events && manager.m_profile.endings == endings);
    loaded.Load(profilePath);
    assert(loaded.events == events && loaded.endings == endings && loaded.bestScores[0][4] == 100);
    manager.Shutdown();
    std::cout << "PASS: settings persistence, retry rollback, practice without rewards, five notebook events, gallery locks/replay/credits, 1920/1366 UI frames\n";
}
