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
#include <cmath>
#define private public
#include "../No_Save_Love/GameManager.h"
#undef private

// 실제 음원 재생 검증 후, 동일한 production 판정에 음악 시각만 공급해 전곡을 검사한다.
bool simulated = false;
int musicPosition = 0;
SHORT simulatedLeftButton = 0;
SHORT WINAPI TestAsyncKeyState(int key)
{
    if (simulated && key == VK_LBUTTON) return simulatedLeftButton;
    return GetAsyncKeyState(key);
}
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
    if (simulated && (std::wstring(command) == L"pause rhythmBGM" || std::wstring(command) == L"resume rhythmBGM")) return 0;
    return mciSendStringW(command, result, size, callback);
}
#define mciSendStringW TestMci
#include "../No_Save_Love/minigame2/RhythmMiniGame.cpp"
#define GetAsyncKeyState TestAsyncKeyState
#include "../No_Save_Love/minigame2/RhythmMiniGameFlowering.cpp"
#undef GetAsyncKeyState
#include "../No_Save_Love/GameManager.cpp"
#include "../No_Save_Love/GameManagerMenus.cpp"
#undef mciSendStringW
#include "../No_Save_Love/Scenes/PromiseScene.cpp"
#include "promise_check.h"
#include "audio_check.h"
#include "combat_modes_check.h"
#include "rhythm_practice_effects_check.h"

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

#include "quality_check.h"

// Drive held objects through the same pointer route as a player. Ordinary clicks
// still happen at their chart time below; spinners need actual circular motion.
void FollowFloweringObjects(RhythmMiniGame& game, int now)
{
    for (const auto& slider : game.sliders)
    {
        if (!slider.isActive || !slider.isStarted) continue;
        int x, y;
        game.GetSliderPosition(slider, (now - slider.slideStartTime) / double(slider.duration), x, y);
        game.OnMouseMove(x, y);
    }
    const auto& spinner = game.floweringSpinner;
    if (spinner.active && now >= spinner.startTime && now < spinner.endTime)
    {
        const double angle = (now - spinner.startTime) * .009;
        game.OnMouseMove(static_cast<int>(1920 * .40 + std::cos(angle) * 180),
            static_cast<int>(1080 * .48 + std::sin(angle) * 180));
    }
}

void CheckFloweringMechanics(HWND window, const std::filesystem::path& output,
    const std::vector<FloweringChart::Note>& chart)
{
    RhythmMiniGame game;
    const auto start = [&](std::vector<FloweringChart::Note> notes)
    {
        musicPosition = 0;
        assert(game.Init(window, true, 0));
        game.floweringNotes = std::move(notes);
        game.floweringMaximumScore = FloweringChart::MaximumScore(game.floweringNotes);
    };
    start({});
    game.hitCircles[0] = {true, false, 400, 400, 0, 2000, FloweringChart::Memory};
    for (int difficulty = 0; difficulty < 3; ++difficulty)
    {
        game.difficulty = difficulty;
        const DWORD hiddenAt = 2000 - (difficulty == 0 ? 450 : difficulty == 1 ? 550 : 650);
        assert(!game.HideFloweringCircle(0, hiddenAt - 1));
        assert(game.HideFloweringCircle(0, hiddenAt));
        assert(game.HideFloweringCircle(0, 2000));
        assert(game.HideFloweringCircle(0, 2119));
        assert(!game.HideFloweringCircle(0, 2120));
    }
    game.difficulty = 1;
    game.hitCircles[0].floweringFlags = FloweringChart::Encore;
    game.hitCircles[1] = {true, false, 400, 400, 0, 2462, FloweringChart::Encore | FloweringChart::EncoreRepeat};
    assert(game.HideFloweringCircle(1, 1800));
    game.hitCircles[0].isActive = false;
    assert(!game.HideFloweringCircle(1, 2000));
    game.Release();

    for (int cue : {128, 296, 432})
    {
        std::vector<FloweringChart::Note> relay;
        for (const auto& note : chart)
            if ((note.flags & FloweringChart::Relay) && note.timeMs == FloweringChart::BeatTime(cue + 8))
                relay.push_back(note);
        assert(relay.size() == 4);
        start(relay);
        musicPosition = relay[0].timeMs - 1;
        game.Update();
        if (cue == 128) SaveFrame(game, output / L"relay_preview.png");
        game.OnMouseDown(static_cast<int>(relay[0].x * 1920), static_cast<int>(relay[0].y * 1080));
        game.OnMouseUp(0, 0);
        assert(game.hitCircleCount == 0 && game.score == 0);
        musicPosition = relay[0].timeMs;
        game.Update();
        assert(game.hitCircleCount == 4);
        if (cue == 128) SaveFrame(game, output / L"relay_four_active.png");
        for (int order = 0; order < 4; ++order)
            assert(std::count_if(std::begin(game.hitCircles), std::end(game.hitCircles),
                [&](const HitCircle& circle) { return circle.isActive && circle.relayOrder == order; }) == 1);
        game.combo = 5;
        game.OnMouseDown(static_cast<int>(relay[1].x * 1920), static_cast<int>(relay[1].y * 1080));
        game.OnMouseUp(0, 0);
        assert(game.hitCircleCount == 4 && game.score == 0 && game.combo == 0);
        assert(game.relayErrorUntil > game.floweringTimeMs);
        if (cue == 128) SaveFrame(game, output / L"relay_wrong_order.png");
        for (int order = 0; order < 4; ++order)
        {
            musicPosition = relay[0].timeMs + order * 600;
            game.Update();
            game.OnMouseDown(static_cast<int>(relay[order].x * 1920), static_cast<int>(relay[order].y * 1080));
            game.OnMouseUp(0, 0);
            assert(game.hitCircleCount == 3 - order && game.floweringHitUnits == order + 1);
        }
        assert(game.score == 1200);
        game.Release();
        start(relay);
        musicPosition = relay[0].relayEndMs + 1;
        game.Update();
        assert(game.hitCircleCount == 0 && game.floweringHitUnits == 0 && game.missedUnits == 4);
        game.Release();
        if (cue == 432)
        {
            start(relay);
            musicPosition = relay[0].timeMs;
            game.Update();
            assert(game.hitCircleCount == 4);
            game.Release();
            assert(game.Init(window, false));
            game.CreateHitCircle(400, 400);
            assert(game.hitCircles[0].isActive && game.hitCircles[0].floweringFlags == 0);
            assert(game.hitCircles[0].relayEndTime == 0 && game.hitCircles[0].relayOrder == 0);
            game.Update();
            assert(game.hitCircles[0].isActive && game.missedUnits == 0);
            game.Release();
        }
    }

    const auto returning = *std::find_if(chart.begin(), chart.end(),
        [](const auto& note) { return (note.flags & FloweringChart::Return) != 0; });
    for (bool releaseEarly : {false, true})
    {
        start({returning});
        musicPosition = returning.timeMs;
        game.Update();
        const auto& slider = game.sliders[0];
        assert(slider.isActive && slider.floweringReturn);
        POINT positions[5]{};
        for (int i = 0; i < 5; ++i)
        {
            int x, y;
            game.GetSliderPosition(slider, i * .25, x, y);
            positions[i] = {x, y};
        }
        assert(positions[0].x == slider.startX && positions[0].y == slider.startY);
        assert(positions[2].x == slider.endX && positions[2].y == slider.endY);
        assert(positions[4].x == slider.startX && positions[4].y == slider.startY);
        assert(positions[1].x == positions[3].x && positions[1].y == positions[3].y);
        game.OnMouseDown(slider.startX, slider.startY);
        if (releaseEarly) game.OnMouseUp(slider.startX, slider.startY);
        for (musicPosition += 10; musicPosition <= returning.timeMs + returning.durationMs + 20; musicPosition += 10)
        {
            FollowFloweringObjects(game, musicPosition);
            game.Update();
        }
        assert(game.score == (releaseEarly ? 300 : 600));
        assert(game.floweringHitUnits == (releaseEarly ? 1 : 2));
        assert(game.missedUnits == (releaseEarly ? 1 : 0));
        game.Release();
    }

    const auto spinnerNote = *std::find_if(chart.begin(), chart.end(),
        [](const auto& note) { return (note.flags & FloweringChart::Spinner) != 0; });
    for (int direction : {-1, 1})
    {
        start({spinnerNote});
        const int centerX = static_cast<int>(1920 * .40), centerY = static_cast<int>(1080 * .48);
        const auto move = [&](double angle, int radius = 180)
        {
            game.OnMouseMove(centerX + static_cast<int>(std::cos(angle) * radius),
                centerY + static_cast<int>(std::sin(angle) * radius));
        };
        musicPosition = spinnerNote.timeMs - 1;
        game.Update();
        game.OnMouseDown(centerX + 180, centerY);
        for (int i = 0; i < 50; ++i) move(i * .2);
        assert(game.floweringSpinner.rotation == 0 && game.score == 0);
        musicPosition = spinnerNote.timeMs;
        game.Update();
        move(0); move(.2);
        const double beforeRelease = game.floweringSpinner.rotation;
        game.OnMouseUp(centerX + 180, centerY);
        for (int i = 0; i < 50; ++i) move(i * .2);
        assert(game.floweringSpinner.rotation == beforeRelease);
        game.OnMouseDown(centerX + 180, centerY);
        move(0);
        assert(game.floweringSpinner.hasAngle);
        game.SetPaused(true);
        assert(!game.floweringSpinner.hasAngle);
        for (int i = 0; i < 50; ++i) move(i * .2);
        assert(game.floweringSpinner.rotation == beforeRelease);
        // A release inside the ESC menu bypasses game.OnMouseUp. Resume must
        // reconcile the physical button rather than preserve the stale hold.
        simulatedLeftButton = 0;
        game.SetPaused(false);
        assert(!game.isMouseDown && !game.floweringSpinner.hasAngle);
        for (int i = 0; i < 50; ++i) move(i * .2);
        assert(game.floweringSpinner.rotation == beforeRelease);
        game.SetPaused(true);
        simulatedLeftButton = static_cast<SHORT>(0x8000);
        game.SetPaused(false);
        assert(game.isMouseDown && !game.floweringSpinner.hasAngle);
        simulatedLeftButton = 0;
        for (int i = 0; i < 20; ++i) move(i * .2, 10);
        for (int i = 0; i < 20; ++i) move(i * .2, 500);
        move(0); move(2.5);
        assert(game.floweringSpinner.rotation == beforeRelease);
        game.floweringSpinner.hasAngle = false;
        for (int i = 0; i <= 240; ++i)
        {
            musicPosition = spinnerNote.timeMs + i * 10;
            game.Update();
            move(direction * i * .12);
        }
        game.Update();
        assert(game.floweringSpinner.cleared && game.score == 300 && game.floweringHitUnits == 1);
        if (direction == 1) SaveFrame(game, output / L"spinner_clear.png");
        musicPosition = spinnerNote.timeMs + spinnerNote.durationMs + 1;
        game.Update();
        assert(!game.floweringSpinner.active && game.score == 300);
        game.Release();
    }
    start({spinnerNote});
    musicPosition = spinnerNote.timeMs + spinnerNote.durationMs + 1;
    game.Update();
    assert(!game.floweringSpinner.active && game.score == 0 && game.missedUnits == 1);
    game.Release();
    std::cout << "PASS: memory visibility, ordered relays and practice restart, return tracking/release, spinner directions/pause-resume/input guards/miss\n";
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
    CheckCombatModes(window);
    CheckRhythmPracticeEffects(output / L"rhythm_practice_effects");
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
            // 릴레이의 네 원만 동시에 활성화한다. 나머지 기믹은 이동 여유를 보장한다.
            if ((note.flags & FloweringChart::Relay) && note.timeMs == chart[i + 1].timeMs)
                assert((chart[i + 1].flags & FloweringChart::Relay) && chart[i + 1].relayOrder == note.relayOrder + 1);
            else
            {
                const int endTime = note.flags & FloweringChart::Relay ? note.relayEndMs : note.timeMs + note.durationMs;
                assert(endTime + 350 <= chart[i + 1].timeMs);
            }
            if (note.flags & FloweringChart::EncoreRepeat)
                assert(chart[i + 1].timeMs - note.timeMs >= 1384);
        }
    }
    assert(flags == (FloweringChart::Memory | FloweringChart::Encore | FloweringChart::Return
        | FloweringChart::Curve | FloweringChart::EncoreRepeat | FloweringChart::Relay | FloweringChart::Spinner));
    assert(std::count_if(chart.begin(), chart.end(), [](const auto& note) { return (note.flags & FloweringChart::Relay) != 0; }) == 12);
    assert(std::count_if(chart.begin(), chart.end(), [](const auto& note) { return (note.flags & FloweringChart::Spinner) != 0; }) == 3);

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
    // 끝의 디지털 무음을 제거한 MP3도 마지막 디코딩 프레임에서 자연 종료한다.
    wchar_t playEnd[100];
    swprintf_s(playEnd, L"play rhythmBGM from %lu", game.floweringDurationMs - 1000);
    assert(game.floweringDurationMs >= 251900 && game.floweringDurationMs <= 252200);
    assert(mciSendStringW(playEnd, nullptr, 0, nullptr) == 0);
    Sleep(1300);
    game.Update();
    assert(game.isResultScreenActive);
    game.Release();
    std::cout << "PASS: actual MP3 open/play/position/pause/resume/end\n";

    int lateNotes = 0, lateFlags = 0;
    for (const auto& note : chart) if (note.timeMs >= 120000)
    { ++lateNotes; lateFlags |= note.flags; }
    assert(lateNotes >= 70 && (lateFlags & flags) == flags);
    assert(FloweringChart::RelayCueBeat(FloweringChart::BeatTime(296)) == 296);
    assert(FloweringChart::RelayCueBeat(FloweringChart::BeatTime(432)) == 432);
    simulated = true;
    CheckFloweringMechanics(window, output, chart);
    musicPosition = 0;
    assert(game.Init(window, true, 1));
    assert(game.floweringMaximumScore == FloweringChart::MaximumScore(chart));
    const int snapshots[] = {10000, 35000, 45000, 59900, 63000, 66300, 66800, 68000, 70000, 83500, 97000, 104000, 139000, 175000, 202000, 238000, 251000};
    const auto firstWith = [&](int flag) -> const FloweringChart::Note& {
        return *std::find_if(chart.begin(), chart.end(), [&](const auto& note) { return (note.flags & flag) != 0; });
    };
    const auto& memory = firstWith(FloweringChart::Memory);
    const auto& returning = firstWith(FloweringChart::Return);
    const auto& spinner = firstWith(FloweringChart::Spinner);
    const std::vector<std::pair<int, const wchar_t*>> mechanicFrames = {
        {memory.timeMs - game.MemoryHideMs() - 80, L"memory_before_hide.png"},
        {memory.timeMs - game.MemoryHideMs() + 120, L"memory_scatter.png"},
        {memory.timeMs - 100, L"memory_hidden.png"},
        {returning.timeMs + returning.durationMs / 2, L"return_turnaround.png"},
        {returning.timeMs + returning.durationMs * 3 / 4, L"return_backtrack.png"},
        {spinner.timeMs + 1050, L"spinner_half.png"}
    };
    size_t nextNote = 0;
    for (musicPosition = 0; musicPosition <= FloweringChart::DurationMs + 1000; musicPosition += 10)
    {
        FollowFloweringObjects(game, musicPosition);
        game.Update();
        while (nextNote < chart.size() && chart[nextNote].timeMs <= musicPosition)
        {
            const auto& note = chart[nextNote++];
            game.OnMouseDown(static_cast<int>(note.x * 1920), static_cast<int>(note.y * 1080));
            if (note.durationMs == 0) game.OnMouseUp(game.mouseX, game.mouseY);
        }
        for (int snapshot : snapshots)
            if (musicPosition == snapshot) SaveFrame(game, output / (std::to_wstring(snapshot) + L".png"));
        for (const auto& [time, name] : mechanicFrames)
            if (musicPosition == (time + 9) / 10 * 10) SaveFrame(game, output / name);
    }
    assert(game.score == game.floweringMaximumScore);
    assert(game.GetResultScore100() == 100);
    assert(game.hitCircleCount == 0 && game.sliderCount == 0);
    assert(!game.hasWindowGimmickTriggered && !game.hasWindowJumpGimmickTriggered);
    game.resultScreenStartTime = GetTickCount() - RESULT_SCREEN_DURATION;
    game.Update();
    assert(game.IsGameOver());
    game.Release();
    std::cout << "PASS: " << chart.size() << " chart objects, perfect " << FloweringChart::MaximumScore(chart) << ", six mechanics, natural finish\n";

    // 실제 입력 경로로 전체 GOOD/BAD 성공을 재현한다. 단순 만점만으로 등급을 검증하지 않는다.
    for (int delay : {140, 220})
    {
        musicPosition = 0;
        assert(game.Init(window, true, 2));
        size_t next = 0;
        for (musicPosition = 0; musicPosition <= FloweringChart::DurationMs + 1000; musicPosition += 10)
        {
            FollowFloweringObjects(game, musicPosition);
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
    assert(game.missedUnits == game.floweringMaximumScore / 300);
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
        assert(manager.GetUpdateInterval()==16);
        manager.OnMouseClick(130,880);
        assert(manager.promiseScene.selected == 0);
        manager.OnKeyDown(VK_ESCAPE);
        assert(manager.m_exitConfirmOpen && manager.promiseScene.selected == -1 && cursorVisible());
        assert(manager.GetUpdateInterval()==16);
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
        assert(manager.GetUpdateInterval()==16);
        manager.OnMouseClick(960, 980); CompleteTransition(manager);
        assert(manager.now_game_mode == GameManager::Story && !manager.m_performanceIntroShown);
        assert(manager.storyScene.dialogues.size() == manager.storyData.GetPromiseStory(partner, true).size() + 1);
        manager.storyScene.finished = true; manager.OnMouseClick(0, 0);
        assert(manager.m_panel == GameManager::Panel::Notebook);
        assert(manager.m_completedChoices[4] == partner);
        manager.OnMouseClick(1600, 145);
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
    CheckQualityFeatures(window, output);
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
