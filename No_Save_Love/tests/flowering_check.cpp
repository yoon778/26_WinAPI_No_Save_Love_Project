#include <windows.h>
#include <mmsystem.h>
#include <atlimage.h>
#include <gdiplus.h>
#include <array>
#include <vector>
#include <span>
#include <string>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <cassert>
#include <iostream>
#define private public
#include "../No_Save_Love/GameManager.h"
#undef private

// 실제 음원 재생 검증 후, 동일한 production 판정에 음악 시각만 공급해 전곡을 검사한다.
bool simulated = false;
int musicPosition = 0;
MCIERROR TestMci(LPCWSTR command, LPWSTR result, UINT size, HWND callback)
{
    if (simulated && std::wstring(command) == L"status rhythmBGM position")
    {
        swprintf_s(result, size, L"%d", musicPosition);
        return 0;
    }
    if (simulated && std::wstring(command).starts_with(L"play ")) return 0;
    return mciSendStringW(command, result, size, callback);
}
#define mciSendStringW TestMci
#include "../No_Save_Love/minigame2/RhythmMiniGame.cpp"
#include "../No_Save_Love/minigame2/RhythmMiniGameFlowering.cpp"
#include "../No_Save_Love/GameManager.cpp"
#undef mciSendStringW

void SaveFrame(RhythmMiniGame& game, const std::filesystem::path& path)
{
    CImage frame;
    frame.Create(1920, 1080, 24);
    HDC dc = frame.GetDC();
    game.Render(dc);
    frame.ReleaseDC();
    assert(SUCCEEDED(frame.Save(path.c_str())));
}

void CompleteTransition(GameManager& manager)
{
    if (manager.hasPendingSceneChange) manager.ApplyPendingSceneChange();
    manager.sceneTransition.state = SceneTransitionState::None;
    manager.sceneTransition.alpha = 0;
}

int wmain(int argc, wchar_t** argv)
{
    assert(argc == 2);
    const std::filesystem::path output(argv[1]);
    std::filesystem::create_directories(output);
    HWND window = CreateWindowW(L"STATIC", L"Flowering check", WS_POPUP,
        0, 0, 1920, 1080, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    assert(window);
    const auto chart = FloweringChart::Build();
    int flags = 0;
    for (size_t i = 0; i < chart.size(); ++i)
    {
        const auto& note = chart[i];
        flags |= note.flags;
        assert(note.timeMs > FloweringChart::ApproachMs);
        assert(note.timeMs + note.durationMs + 350 < FloweringChart::DurationMs);
        assert(note.x >= .12f && note.x <= .68f && note.endX >= .12f && note.endX <= .68f);
        assert(note.y >= .30f && note.y <= .70f && note.endY >= .30f && note.endY <= .70f);
        if (i + 1 < chart.size())
        {
            // 클릭 하나로 동시에 다른 위치를 누르거나, 홀드 중 다음 노트를 누를 필요가 없어야 한다.
            assert(note.timeMs + note.durationMs + 350 <= chart[i + 1].timeMs);
        }
    }
    assert(flags == (FloweringChart::Memory | FloweringChart::Encore | FloweringChart::Mirror
        | FloweringChart::Curve | FloweringChart::EncoreRepeat));

    RhythmMiniGame game;
    assert(game.Init(window, true, 0));
    mciSendStringW(L"setaudio rhythmBGM volume to 0", nullptr, 0, nullptr);
    Sleep(120);
    game.Update();
    assert(game.floweringTimeMs > 0);
    game.SetPaused(true);
    const auto pauseTime = game.floweringTimeMs;
    Sleep(100);
    game.Update();
    assert(game.floweringTimeMs == pauseTime);
    game.SetPaused(false);
    Sleep(100);
    game.Update();
    assert(game.floweringTimeMs > pauseTime);
    // MP3의 마지막 디코딩 프레임에서도 120초 공연이 정상적으로 종료되는지 확인한다.
    assert(mciSendStringW(L"play rhythmBGM from 119000 to 120000", nullptr, 0, nullptr) == 0);
    Sleep(1300);
    game.Update();
    assert(game.isResultScreenActive);
    game.Release();
    std::cout << "PASS: actual MP3 open/play/position/pause/resume/end\n";

    simulated = true;
    musicPosition = 0;
    assert(game.Init(window, true, 1));
    assert(game.floweringMaximumScore == FloweringChart::MaximumScore(chart));
    game.hitCircles[0] = {true, false, 400, 400, 0, 2000, FloweringChart::Memory};
    assert(!game.HideFloweringCircle(0, 1749));
    assert(game.HideFloweringCircle(0, 1750));
    assert(!game.HideFloweringCircle(0, 2000));
    game.hitCircles[0].floweringFlags = FloweringChart::Encore;
    game.hitCircles[1] = {true, false, 400, 400, 0, 2462, FloweringChart::Encore | FloweringChart::EncoreRepeat};
    assert(game.HideFloweringCircle(1, 1800));
    game.hitCircles[0].isActive = false;
    assert(!game.HideFloweringCircle(1, 2000));
    game.ClearAllNotes();
    const int snapshots[] = {10000, 35000, 45000, 59900, 70000, 83500, 104000, 120000};
    size_t nextNote = 0;
    for (musicPosition = 0; musicPosition <= FloweringChart::DurationMs; musicPosition += 10)
    {
        for (const auto& slider : game.sliders)
        {
            if (!slider.isActive || !slider.isStarted) continue;
            const double progress = (musicPosition - slider.slideStartTime) / double(slider.duration);
            game.GetSliderPosition(slider, progress, game.mouseX, game.mouseY);
        }
        game.Update();
        while (nextNote < chart.size() && chart[nextNote].timeMs <= musicPosition)
        {
            const auto& note = chart[nextNote++];
            game.OnMouseDown(static_cast<int>(note.x * 1920), static_cast<int>(note.y * 1080));
            if (note.durationMs == 0) game.OnMouseUp(game.mouseX, game.mouseY);
        }
        for (int snapshot : snapshots)
            if (musicPosition == snapshot) SaveFrame(game, output / (std::to_wstring(snapshot) + L".png"));
    }
    assert(game.score == game.floweringMaximumScore);
    assert(game.GetResultScore100() == 100);
    assert(game.hitCircleCount == 0 && game.sliderCount == 0);
    assert(!game.hasWindowGimmickTriggered && !game.hasWindowJumpGimmickTriggered);
    game.resultScreenStartTime = GetTickCount() - RESULT_SCREEN_DURATION;
    game.Update();
    assert(game.IsGameOver());
    game.Release();
    std::cout << "PASS: " << chart.size() << " chart objects, perfect " << FloweringChart::MaximumScore(chart) << ", five mechanics, natural finish\n";

    musicPosition = 0;
    assert(game.Init(window, true, 2));
    for (musicPosition = 0; musicPosition <= FloweringChart::DurationMs; musicPosition += 100) game.Update();
    assert(game.GetResultScore100() == 0);
    game.Release();
    // 실패 후 재시작에서도 이전 채보/점수/시간이 남지 않는다.
    musicPosition = 0;
    assert(game.Init(window, true, 2));
    assert(game.floweringNextNote == 0 && game.score == 0 && game.floweringTimeMs == 0);
    game.Release();

    // 기존 미니게임2는 창 기믹과 기존 콤보 보너스 계산을 유지한다.
    assert(game.Init(window));
    assert(!game.isFlowering);
    game.CreateHitCircle(400, 400);
    game.hitCircles[0].hitTime = GetTickCount();
    game.OnMouseDown(400, 400);
    assert(game.score == 300);
    game.score = RESULT_SCORE_MAX - MAX_COMBO_BONUS_SCORE;
    game.maxCombo = MAX_COMBO_TARGET + 1;
    game.CalculateResult();
    assert(game.GetResultScore100() == 100);
    game.TriggerWindowGimmick(GetTickCount());
    game.Release();
    RECT restored;
    GetClientRect(window, &restored);
    assert(restored.right == 1920 && restored.bottom == 1080);
    std::cout << "PASS: miss/restart, legacy rhythm score and window restoration\n";

    for (int heroine = 0; heroine < 3; ++heroine)
    {
        GameManager manager;
        manager.Initialize(window);
        CompleteTransition(manager);
        audioManager.SetSfxVolume(L"click", 0);
        manager.currentMiniGameIndex = 4;
        manager.currentStoryRound = 4;
        manager.player = {50, 50, 50, 50};
        manager.characters[heroine].choice_time = 3;
        manager.characters[(heroine + 1) % 3].choice_time = 1;
        manager.now_game_mode = GameManager::Story;
        manager.storyScene.finished = true;
        manager.OnMouseClick(0, 0);
        assert(manager.m_performanceIntroShown && manager.finalHeroineIndex == heroine);
        assert(!manager.storyData.GetPerformanceStory(heroine, false).empty());
        manager.storyScene.finished = true;
        manager.OnMouseClick(0, 0);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::MiniGameTutor5);
        assert(manager.minigame2TutorialScene.floweringMode);
        CImage tutorial;
        tutorial.Create(1920, 1080, 24);
        manager.minigame2TutorialScene.currentPage = 1;
        auto dc = tutorial.GetDC();
        manager.Render(dc);
        tutorial.ReleaseDC();
        if (heroine == 0) assert(SUCCEEDED(tutorial.Save((output / L"tutorial.png").c_str())));
        musicPosition = 0;
        manager.OnMouseClick(1800, 60); // 튜토리얼 SKIP
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::MiniGame5);
        assert(manager.minigame2.floweringHeroine == heroine);
        musicPosition = 70000;
        manager.minigame2.Update();
        SaveFrame(manager.minigame2, output / (L"heroine_" + std::to_wstring(heroine) + L".png"));
        manager.minigame2.score = manager.minigame2.floweringMaximumScore;
        manager.FinishCurrentMiniGameIfNeeded(true);
        assert(manager.currentMiniGameIndex == 5);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Result);
        assert(manager.player.speech == 81 && manager.player.charm == 72);
        manager.OnMouseClick(960, 980);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Story); // 다섯 번째 Choice가 아님
        assert(manager.characters[heroine].choice_time == 3);
        manager.storyScene.finished = true;
        manager.OnMouseClick(0, 0);
        assert(manager.m_finalChoiceIntroShown && manager.finalChoiceIntroIndex == heroine);
        assert(manager.CalculateEndingType(heroine) == 0);
        manager.storyScene.finished = true;
        manager.OnMouseClick(0, 0);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::EndingDialogue);
        manager.Shutdown();
    }
    DestroyWindow(window);
    std::cout << "PASS: three heroines, tutorial, fifth reward once, four choices, updated ending\n";
}
