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
    if (simulated && std::wstring(command) == L"status rhythmBGM mode")
    {
        swprintf_s(result, size, L"%s", musicPosition >= FloweringChart::DurationMs ? L"stopped" : L"playing");
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
#include "../No_Save_Love/Scenes/PromiseScene.cpp"
#include "promise_check.h"
#include "audio_check.h"

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
    std::cout << std::unitbuf;
    assert(argc == 2);
    const std::filesystem::path output(argv[1]);
    std::filesystem::create_directories(output);
    HWND window = CreateWindowW(L"STATIC", L"Flowering check", WS_POPUP,
        0, 0, 1920, 1080, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    assert(window);
    CheckEffectSounds(window);
    CheckPromises(output);
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
            if (note.flags & FloweringChart::EncoreRepeat)
                assert(chart[i + 1].timeMs - note.timeMs >= 1384);
        }
    }
    assert(flags == (FloweringChart::Memory | FloweringChart::Encore | FloweringChart::Link
        | FloweringChart::Curve | FloweringChart::EncoreRepeat | FloweringChart::LinkFollow));

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
    // 실제 MP3 원곡 끝의 마지막 디코딩 프레임에서도 자연 종료한다.
    wchar_t playEnd[100];
    swprintf_s(playEnd, L"play rhythmBGM from %lu", game.floweringDurationMs - 1000);
    assert(game.floweringDurationMs >= 272000 && game.floweringDurationMs <= 273000);
    assert(mciSendStringW(playEnd, nullptr, 0, nullptr) == 0);
    Sleep(1300);
    game.Update();
    assert(game.isResultScreenActive);
    game.Release();
    std::cout << "PASS: actual MP3 open/play/position/pause/resume/end\n";

    int lateNotes = 0, lateFlags = 0;
    for (const auto& note : chart) if (note.timeMs >= 120000)
    { ++lateNotes; lateFlags |= note.flags; }
    assert(lateNotes >= 100 && (lateFlags & flags) == flags);
    assert(FloweringChart::RelayCueBeat(FloweringChart::BeatTime(296)) == 296);
    assert(FloweringChart::RelayCueBeat(FloweringChart::BeatTime(432)) == 432);
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
    const int snapshots[] = {10000, 35000, 45000, 59900, 66300, 66800, 68000, 70000, 83500, 104000, 139000, 175000, 202000, 240000, 271000};
    size_t nextNote = 0;
    for (musicPosition = 0; musicPosition <= FloweringChart::DurationMs + 1000; musicPosition += 10)
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

    // 실제 입력 경로로 전체 GOOD/BAD 성공을 재현한다. 단순 만점만으로 등급을 검증하지 않는다.
    for (int delay : {140, 220})
    {
        musicPosition = 0;
        assert(game.Init(window, true, 2));
        size_t next = 0;
        for (musicPosition = 0; musicPosition <= FloweringChart::DurationMs + 1000; musicPosition += 10)
        {
            for (const auto& slider : game.sliders)
                if (slider.isActive && slider.isStarted)
                    game.GetSliderPosition(slider, (musicPosition - slider.slideStartTime) / double(slider.duration),
                        game.mouseX, game.mouseY);
            game.Update();
            while (next < chart.size() && chart[next].timeMs + delay <= musicPosition)
            {
                const auto& note = chart[next++];
                game.OnMouseDown(static_cast<int>(note.x * 1920), static_cast<int>(note.y * 1080));
                if (note.durationMs == 0) game.OnMouseUp(game.mouseX, game.mouseY);
            }
        }
        assert(game.floweringHitUnits == game.floweringMaximumScore / 300);
        assert(game.GetResultScore100() >= 90 && std::wstring(game.GetGradeText()) == L"S");
        std::cout << "PASS: full song delay " << delay << "ms, grade S, score " << game.GetResultScore100() << "\n";
        game.Release();
    }

    musicPosition = 0;
    assert(game.Init(window, true, 2));
    for (musicPosition = 0; musicPosition <= FloweringChart::DurationMs + 1000; musicPosition += 100) game.Update();
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

    // ESC 메뉴는 시스템 커서를 보이고 게임으로 돌아가면 원래 커서를 사용한다.
    // ShowCursor 카운터를 관찰한 직후 복구하여 반복 개폐의 누적 오류도 확인한다.
    const auto cursorVisible = []() { const int count = ShowCursor(TRUE); ShowCursor(FALSE); return count > 0; };
    for (int slot : {1, 5})
    {
        GameManager manager;
        manager.Initialize(window); CompleteTransition(manager);
        manager.DebugEnterMiniGameByIndex(slot); CompleteTransition(manager);
        assert(!cursorVisible());
        for (int cycle = 0; cycle < 3; ++cycle)
        {
            manager.OnKeyDown(VK_ESCAPE);
            assert(manager.m_exitConfirmOpen && cursorVisible());
            const int cursorX = manager.minigame2.mouseX;
            manager.OnMouseMove(720, 830);
            assert(manager.minigame2.mouseX == cursorX);
            if (cycle == 1) manager.OnKeyDown(VK_ESCAPE);
            else manager.OnMouseClick(720, 830);
            assert(!manager.m_exitConfirmOpen && !cursorVisible());
        }
        manager.Shutdown();
        assert(cursorVisible());
    }
    std::cout << "PASS: both rhythm pause menus restore cursor after repeated keyboard/click close\n";

    for (int heroine = 0; heroine < 3; ++heroine)
    {
        GameManager manager;
        manager.Initialize(window);
        CompleteTransition(manager);
        audioManager.SetSfxVolume(L"click", 0);
        audioManager.SetSfxVolume(L"defense_shot", 0);
        audioManager.SetSfxVolume(L"defense_support", 0);
        manager.currentMiniGameIndex = 4;
        manager.currentStoryRound = 4;
        manager.player = {50, 50, 50, 50};
        manager.characters[heroine].choice_time = 3;
        manager.characters[(heroine + 1) % 3].choice_time = 1;
        manager.now_game_mode = GameManager::Story;
        manager.storyScene.finished = true;
        manager.OnMouseClick(0, 0);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Choice && !manager.m_promiseIntroShown);
        // 다섯 번째 동행은 기존 최다 선택 인물과 다르게 고를 수 있다.
        const int partner = (heroine + 1) % 3;
        manager.OnMouseClick(800, 200 + partner * 310); CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Story && manager.currentStoryRound == 5);
        assert(manager.promiseHeroineIndex == partner && manager.characters[partner].choice_time == 2);
        assert(!manager.EnterBranchStory(partner)); // 여섯 번째 선택은 없다.
        manager.storyScene.finished = true;
        manager.OnMouseClick(0, 0); CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::MiniGameTutor5);
        assert(manager.promiseScene.heroine == partner && manager.promiseScene.IsTutorial());
        manager.OnMouseClick(960, 838); CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::MiniGame5 && !manager.promiseScene.IsTutorial());
        manager.OnMouseClick(130,880);
        assert(manager.promiseScene.selected == 0);
        manager.OnKeyDown(VK_ESCAPE);
        assert(manager.m_exitConfirmOpen && manager.promiseScene.selected == -1 && cursorVisible());
        const float pausedTime=manager.promiseScene.practiceTime;
        manager.OnMouseClick(860,295); manager.OnTimer(window);
        assert(manager.promiseScene.practiceTime == pausedTime);
        manager.promiseScene.lastTick=0;
        manager.OnKeyDown(VK_ESCAPE);
        assert(GetTickCount64()-manager.promiseScene.lastTick < 1000);
        manager.promiseScene.BeginBattle();
        for(int tick=0;tick<11000 && !manager.promiseScene.IsFinished();++tick) PlayDefenseStep(manager.promiseScene,0);
        assert(manager.promiseScene.victory);
        const int defenseScore=manager.promiseScene.GetScore();
        manager.FinishCurrentMiniGameIfNeeded();
        assert(manager.currentMiniGameIndex == 5);
        const auto puzzleGain = StatBalance::CalculateGain(4, defenseScore);
        assert(manager.player.money == 50 + puzzleGain.money);
        assert(manager.player.speech == 50 + puzzleGain.speech);
        manager.FinishCurrentMiniGameIfNeeded(true); // 전환 중 중복 완료 방지
        assert(manager.currentMiniGameIndex == 5 && manager.player.money == 50 + puzzleGain.money);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Result && !manager.promiseScene.portrait);
        manager.OnMouseClick(960, 980); CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Story && !manager.m_performanceIntroShown);
        assert(manager.storyScene.dialogues.size() == manager.storyData.GetPromiseStory(partner, true).size() + 1);
        manager.storyScene.finished = true; manager.OnMouseClick(0, 0);
        assert(manager.m_performanceIntroShown && manager.finalHeroineIndex == heroine);
        assert(!manager.storyData.GetPerformanceStory(heroine, false).empty());
        manager.storyScene.finished = true;
        manager.OnMouseClick(0, 0);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::MiniGameTutor6);
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
        assert(manager.now_game_mode == GameManager::MiniGame6);
        assert(manager.minigame2.floweringHeroine == heroine);
        musicPosition = 70000;
        manager.minigame2.Update();
        SaveFrame(manager.minigame2, output / (L"heroine_" + std::to_wstring(heroine) + L".png"));
        manager.minigame2.score = manager.minigame2.floweringMaximumScore;
        manager.minigame2.floweringHitUnits = manager.minigame2.floweringMaximumScore / 300;
        manager.FinishCurrentMiniGameIfNeeded(true);
        assert(manager.currentMiniGameIndex == 6);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Result);
        const auto performanceGain = StatBalance::CalculateGain(5, 100);
        assert(manager.player.speech == 50 + puzzleGain.speech + performanceGain.speech);
        assert(manager.player.charm == 50 + puzzleGain.charm + performanceGain.charm);
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
        // 배속으로 끝난 엔딩 대사도 타이머에서 크레딧으로 전환한다.
        manager.storyScene.finished = true;
        manager.OnTimer(window);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Ending);
        manager.OnKeyDown('F');
        assert(manager.endingScene.isFastScroll);
        manager.endingScene.Update();
        assert(manager.endingScene.scrollOffset == manager.endingScene.fastScrollSpeed);
        manager.OnMouseClick(1750, 44);
        assert(!manager.endingScene.isFastScroll);
        manager.OnKeyDown(VK_SPACE);
        assert(manager.endingScene.isFastScroll);
        manager.endingScene.scrollOffset = manager.endingScene.stopOffset;
        if (heroine == 0)
        {
            CImage credits;
            credits.Create(1920, 1080, 24);
            auto creditsDC = credits.GetDC();
            manager.Render(creditsDC);
            credits.ReleaseDC();
            assert(SUCCEEDED(credits.Save((output / L"credits_return.png").c_str())));
        }
        manager.OnMouseClick(780, 950);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Title);
        assert(manager.currentMiniGameIndex == 0 && manager.currentStoryRound == 0);
        assert(!manager.m_performanceIntroShown && !manager.m_finalChoiceIntroShown && !manager.m_promiseIntroShown);
        assert(manager.player.speech == 0 && manager.player.charm == 0);
        assert(manager.storyScene.history.empty() && !manager.titleScene.IsStartClicked());
        for (const auto& character : manager.characters) assert(character.choice_time == 0);
        manager.OnMouseClick(960, 720);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::NameInput && !manager.nameinputScene.IsFinished());
        manager.OnChar(L'가');
        manager.OnChar(L'\r');
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Story);
        assert(manager.playerName == L"가" && !manager.storyScene.IsFinished());
        for (int i = 0; i < 50 && manager.storyScene.fadeState != FadeState::None; ++i) manager.storyScene.Update();
        manager.OnKeyDown(VK_SPACE);
        assert(manager.storyScene.currentDialogueIndex == 0 && manager.storyScene.isTypingFinished);
        manager.OnKeyDown(VK_SPACE);
        assert(manager.storyScene.currentDialogueIndex == 1);
        manager.OnKeyDown('F');
        assert(manager.storyScene.fastForward);
        manager.OnKeyDown(VK_ESCAPE);
        assert(manager.m_exitConfirmOpen && !manager.storyScene.fastForward);
        const auto indexBeforeMenu = manager.storyScene.currentDialogueIndex;
        manager.OnKeyDown(VK_SPACE); manager.OnTimer(window);
        assert(manager.storyScene.currentDialogueIndex == indexBeforeMenu);
        if (heroine == 0)
        {
            CImage help; help.Create(1920, 1080, 24);
            HDC helpDC = help.GetDC(); manager.Render(helpDC); help.ReleaseDC();
            assert(SUCCEEDED(help.Save((output / L"keyboard_help.png").c_str())));
        }
        manager.OnMouseClick(720, 830);
        assert(!manager.m_exitConfirmOpen);
        manager.OnKeyDown('L');
        assert(manager.storyScene.historyOpen);
        manager.OnKeyDown(VK_ESCAPE);
        assert(!manager.storyScene.historyOpen && !manager.m_exitConfirmOpen);
        manager.OnKeyDown('S');
        assert(manager.storyScene.history.size() == manager.storyData.GetIntroStory().size());
        assert(manager.currentMiniGameIndex == 0 && manager.currentStoryRound == 0);
        CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::MiniGameTutor1);
        for (const auto& character : manager.characters) assert(character.choice_time == 0);
        // 스킵 버튼이 엔딩 대사에서는 크레딧으로 이동한다.
        manager.now_game_mode = GameManager::EndingDialogue;
        manager.storyScene.SetDialogues(manager.storyData.GetEndingStory(0, heroine));
        manager.OnMouseClick(1500, 44); CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Ending);
        manager.Shutdown();
    }
    DestroyWindow(window);
    {
        GameManager manager;
        manager.Initialize(window); CompleteTransition(manager);
        manager.characters[0].choice_time=2; manager.characters[1].choice_time=2;
        manager.characters[2].choice_time=1; manager.PrepareFinalChoiceIntro();
        assert(manager.finalHeroineIndex==0); // 다섯 선택의 동률 기준
        manager.characters[1].choice_time=3; manager.characters[2].choice_time=0;
        manager.PrepareFinalChoiceIntro(); assert(manager.finalHeroineIndex==1);
        manager.promiseHeroineIndex=2; manager.m_promiseWon=false;
        manager.currentStoryRound=5; manager.currentMiniGameIndex=5;
        manager.now_game_mode=GameManager::Result; manager.resultScene.SetResult(4,40);
        manager.OnMouseClick(960,980); CompleteTransition(manager);
        assert(manager.now_game_mode==GameManager::Story);
        assert(manager.storyScene.dialogues.front().speaker==L"서이린");
        assert(manager.storyScene.dialogues.front().text.find(L"다음에는")!=std::wstring::npos);
        manager.Shutdown();
    }
    std::cout << "PASS: three heroines, tutorial, reward once, ending, credit speed, title return, new game keyboard/log\n";
}
