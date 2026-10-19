#include <windows.h>
#include <atlimage.h>
#include <gdiplus.h>
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include <chrono>
#include "../No_Save_Love/AudioManager.h"
#include "../No_Save_Love/GameOptions.h"
#include "../No_Save_Love/DisplayGeometry.h"
#define private public
#include "../No_Save_Love/SceneTransition.h"
#include "../No_Save_Love/minigame1/PCroomgame.h"
#include "../No_Save_Love/minigame3/schoolrun.h"
#include "../No_Save_Love/minigame4/avoidgame.h"
#undef private
#include "../No_Save_Love/SceneTransition.cpp"
#include "../No_Save_Love/minigame1/PCroomgame.cpp"
#include "../No_Save_Love/minigame3/schoolrun.cpp"
#include "../No_Save_Love/minigame4/avoidgame.cpp"

AudioManager audioManager;

template<class Game> void Advance(Game& game, float duration, float frame)
{
    for (float left = duration; left > 0.000001f;)
    {
        const float step = (std::min)(left, frame);
        game.Update(step);
        left -= step;
    }
}

void CheckViewport()
{
    for (POINT size : { POINT{1366,768}, POINT{1280,1024}, POINT{2560,1080}, POINT{3840,2160}, POINT{800,450} })
    {
        const RECT view = DisplayGeometry::Fit(size.x, size.y);
        assert(view.left >= 0 && view.top >= 0 && view.right <= size.x && view.bottom <= size.y);
        const POINT center = DisplayGeometry::ToLogical({ (view.left + view.right) / 2, (view.top + view.bottom) / 2 }, view);
        assert(std::abs(center.x - 960) <= 2 && std::abs(center.y - 540) <= 2);
        POINT corner = DisplayGeometry::ToLogical({ view.left, view.top }, view);
        assert(corner.x == 0 && corner.y == 0);
        corner = DisplayGeometry::ToLogical({ view.right - 1, view.bottom - 1 }, view);
        assert(corner.x < 1920 && corner.y < 1080);
        assert(DisplayGeometry::ToLogical({ view.left - 1, view.top }, view).x < 0);
        assert(DisplayGeometry::ToLogical({ view.right, view.bottom }, view).x < 0);
    }
    assert(DisplayGeometry::Fit(0, 0).right == 0);
    assert(DisplayGeometry::ToLogical({ 0,0 }, {}).x == -1);
    std::cout << "PASS viewport: 720p/4:3/ultrawide/4K, edge and letterbox mouse mapping\n";
}

void CheckTransition()
{
    SceneTransition slow, fast;
    slow.Start(); fast.Start();
    Advance(slow, 0.3f, 0.06f); Advance(fast, 0.3f, 0.016f);
    assert(std::abs(slow.alpha - fast.alpha) < 0.01f);
    assert(!slow.IsBlack());
    Advance(slow, 0.4f, 0.06f); Advance(fast, 0.4f, 0.016f);
    assert(slow.IsBlack() && fast.IsBlack());
    slow.StartFadeIn(); fast.StartFadeIn();
    Advance(slow, 0.7f, 0.06f); Advance(fast, 0.7f, 0.016f);
    assert(!slow.IsActive() && !fast.IsActive());
    std::cout << "PASS transition: identical elapsed-time fade at 16/60ms\n";
}

void CheckCooking()
{
    GameOptions::difficulty = 1;
    PCroomgame slow, fast;
    slow.Init(); fast.Init();
    slow.ismoving = fast.ismoving = true;
    slow.targetPos = fast.targetPos = {1495, 0};
    Advance(slow, 0.3f, 0.06f); Advance(fast, 0.3f, 0.016f);
    assert(std::abs(slow.playerPos.y - fast.playerPos.y) <= 1);
    assert(std::abs(slow.timer - fast.timer) < 0.001);
    assert(slow.playerPos.y >= 849 && slow.playerPos.y <= 851);
    slow.curramen = slow.seatorder[0];
    slow.DeliverToSeat(0);
    assert(slow.successfulOrders == 1 && slow.score == 100);
    assert(slow.GetResultDetails().size() == 4);
    slow.Release(); fast.Release();
    double previousTime = 100.0;
    for (int difficulty = 0; difficulty < 3; ++difficulty)
    {
        GameOptions::difficulty = difficulty;
        PCroomgame game;
        game.Init();
        assert(game.timer < previousTime);
        previousTime = game.timer;
        Advance(game, static_cast<float>(game.timer) + 0.04f, 0.016f);
        assert(game.isfinished() && game.timer == 0.0);
        game.Release();
    }
    std::cout << "PASS cooking: smooth movement, elapsed timer, success details, 60/50/45s difficulty\n";
}

void CheckRunner()
{
    GameOptions::difficulty = 1;
    schoolrun slow, fast;
    slow.Reset(); fast.Reset();
    slow.TryJump(); fast.TryJump();
    Advance(slow, 0.3f, 0.06f); Advance(fast, 0.3f, 0.016f);
    assert(std::abs(slow.m_playerY - fast.m_playerY) < 0.02f);
    assert(std::abs(slow.m_backgroundOffset - fast.m_backgroundOffset) < 0.02f);
    assert(std::abs(slow.m_remainingTime - fast.m_remainingTime) < 0.001f);
    assert(slow.m_distanceScore == fast.m_distanceScore);
    assert(std::abs(slow.m_playerY - 542.0f) < 0.1f); // Original 5-tick jump position.
    for (int difficulty = 0; difficulty < 3; ++difficulty)
    {
        GameOptions::difficulty = difficulty;
        schoolrun game; game.Reset();
        assert(game.m_life == (difficulty == 0 ? 5 : difficulty == 1 ? 3 : 2));
        game.m_invincibleTime = 100.0f;
        Advance(game, 40.04f, 0.016f);
        assert(game.IsFinished() && game.m_remainingTime == 0.0f);
        assert(game.GetResultDetails().size() == 4);
    }
    std::cout << "PASS runner: original jump arc, scroll, 40s duration and 5/3/2 hearts\n";
}

void CheckAvoid()
{
    GameOptions::difficulty = 1;
    avoidgame slow, fast;
    slow.Reset(); fast.Reset();
    slow.OnKeyDown(VK_RIGHT); fast.OnKeyDown(VK_RIGHT);
    slow.Jump(); fast.Jump();
    Advance(slow, 0.3f, 0.06f); Advance(fast, 0.3f, 0.016f);
    assert(std::abs(slow.m_player.x - fast.m_player.x) < 0.03f);
    assert(std::abs(slow.m_player.y - fast.m_player.y) < 0.03f);
    assert(std::abs(slow.m_game.remainingTime - fast.m_game.remainingTime) < 0.001f);
    float previousWarning = 10.0f;
    for (int difficulty = 0; difficulty < 3; ++difficulty)
    {
        GameOptions::difficulty = difficulty;
        avoidgame game; game.Reset();
        assert(game.m_game.life == (difficulty == 0 ? 7 : difficulty == 1 ? 5 : 4));
        assert(game.GetScore() == 100);
        game.StartDeliverPattern();
        assert(game.m_deliver.warningTimer < previousWarning);
        previousWarning = game.m_deliver.warningTimer;
        game.DamagePlayer(avoidgame::HIT_IMAGE_GAME);
        assert(game.m_damageCount == 1 && game.GetScore() < 100);
        game.m_game.invincibleTimer = 100.0f;
        Advance(game, 50.04f, 0.016f);
        assert(game.IsFinished() && game.m_game.isSuccess);
        assert(game.GetResultDetails().size() == 3);
    }
    std::cout << "PASS avoidance: original movement/jump, warning difficulty, normalized hearts and 50s duration\n";
}

void CheckRendering()
{
    CImage canvas;
    canvas.Create(1920, 1080, 32);
    HDC dc = canvas.GetDC();
    auto measure = [&](const char* name, auto render)
    {
        for (int i = 0; i < 3; ++i) render();
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < 12; ++i) render();
        GdiFlush();
        const double milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() / 12.0;
        std::cout << "RENDER " << name << " average " << milliseconds << "ms (1920x1080 offscreen)\n";
    };
    PCroomgame cooking; cooking.Init();
    measure("cooking", [&] { cooking.PAINT(dc); });
    cooking.Release();
    schoolrun runner; runner.Initialize();
    Advance(runner, 1.0f, 0.016f);
    // Cropping transparent rows must produce exactly the same visible ground.
    CImage reference;
    reference.Create(1920, 1080, 32);
    HDC referenceDC = reference.GetDC();
    RECT bounds = {0, 0, 1920, 1080};
    FillRect(referenceDC, &bounds, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    FillRect(dc, &bounds, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    const int offset = static_cast<int>(runner.m_scrollOffset) % 1920 + 10;
    runner.m_groundImage.Draw(referenceDC, -offset, 0, 1970, 1080);
    runner.m_groundImage.Draw(referenceDC, 1920 - offset, 0, 1970, 1080);
    runner.RenderGround(dc);
    GdiFlush();
    for (int y = 0; y < 1080; ++y)
    {
        const DWORD* original = static_cast<const DWORD*>(reference.GetPixelAddress(0, y));
        const DWORD* cropped = static_cast<const DWORD*>(canvas.GetPixelAddress(0, y));
        for (int x = 0; x < 1920; ++x) assert((original[x] & 0xFFFFFF) == (cropped[x] & 0xFFFFFF));
    }
    reference.ReleaseDC();
    std::cout << "PASS runner ground crop: identical RGB pixels to original full image\n";
    measure("runner", [&] { runner.Render(dc); });
    runner.Release();
    avoidgame avoid; avoid.Initialize();
    avoid.StartReelsPattern();
    for (int i = 0; i < 12; ++i) avoid.SpawnFoodDrop();
    measure("avoidance with reels", [&] { avoid.Render(dc); });
    avoid.Release();
    canvas.ReleaseDC();
}

int main()
{
    CheckViewport(); CheckTransition(); CheckCooking(); CheckRunner(); CheckAvoid();
    GameOptions::difficulty = 1;
    CheckRendering();
    std::cout << "ALL TIMING AND DISPLAY CHECKS PASSED\n";
}
