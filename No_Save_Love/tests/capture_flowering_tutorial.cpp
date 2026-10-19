#include <windows.h>
#include <mmsystem.h>
#include <atlimage.h>
#include <gdiplus.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#define private public
#include "../No_Save_Love/minigame2/RhythmMiniGame.h"
#undef private

// Capture production rendering at deterministic music times, without playing audio.
int musicPosition = 0;
MCIERROR CaptureMci(LPCWSTR command, LPWSTR result, UINT size, HWND)
{
    const std::wstring text(command);
    if (text == L"status rhythmBGM length")
        swprintf_s(result, size, L"%d", FloweringChart::DurationMs);
    else if (text == L"status rhythmBGM position")
        swprintf_s(result, size, L"%d", musicPosition);
    else if (text == L"status rhythmBGM mode")
        wcscpy_s(result, size, L"playing");
    return 0;
}
#define mciSendStringW CaptureMci
#include "../No_Save_Love/minigame2/RhythmMiniGame.cpp"
#include "../No_Save_Love/minigame2/RhythmMiniGameFlowering.cpp"
#undef mciSendStringW

namespace
{
    constexpr int Width = 900, Height = 500;

    void Fill(HDC dc, RECT rect, COLORREF color)
    {
        SetDCBrushColor(dc, color);
        FillRect(dc, &rect, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
    }

    void Caption(HDC dc, const wchar_t* text, RECT rect)
    {
        HFONT font = CreateFontW(-25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        HGDIOBJ previous = SelectObject(dc, font);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(230, 240, 237));
        DrawTextW(dc, text, -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
        SelectObject(dc, previous);
        DeleteObject(font);
    }

    void Capture(RhythmMiniGame& game, CImage& frame)
    {
        if (frame.IsNull()) frame.Create(1920, 1080, 24);
        const HDC dc = frame.GetDC();
        game.Render(dc);
        frame.ReleaseDC();
    }

    RECT SquareCrop(int centerX, int centerY, int side)
    {
        side = (std::min)(side, 1080);
        const int left = (std::clamp)(centerX - side / 2, 0, 1920 - side);
        const int top = (std::clamp)(centerY - side / 2, 0, 1080 - side);
        return {left, top, left + side, top + side};
    }

    RECT SliderCrop(RhythmMiniGame& game)
    {
        const auto& slider = game.sliders[0];
        int left = 1920, top = 1080, right = 0, bottom = 0;
        for (int i = 0; i <= 20; ++i)
        {
            int x, y;
            game.GetSliderPosition(slider, i / 20.0, x, y);
            left = (std::min)(left, x); right = (std::max)(right, x);
            top = (std::min)(top, y); bottom = (std::max)(bottom, y);
        }
        return SquareCrop((left + right) / 2, (top + bottom) / 2,
            (std::max)(right - left, bottom - top) + 260);
    }

    void DrawCrop(HDC destination, CImage& frame, RECT crop, RECT slot)
    {
        const double scale = (std::min)((slot.right - slot.left) / double(crop.right - crop.left),
            (slot.bottom - slot.top) / double(crop.bottom - crop.top));
        const int width = static_cast<int>((crop.right - crop.left) * scale);
        const int height = static_cast<int>((crop.bottom - crop.top) * scale);
        HDC source = frame.GetDC();
        SetStretchBltMode(destination, HALFTONE);
        SetBrushOrgEx(destination, 0, 0, nullptr);
        const BOOL drawn = StretchBlt(destination, slot.left + (slot.right - slot.left - width) / 2,
            slot.top + (slot.bottom - slot.top - height) / 2, width, height, source,
            crop.left, crop.top, crop.right - crop.left, crop.bottom - crop.top, SRCCOPY);
        frame.ReleaseDC();
        assert(drawn);
    }

    void SavePair(const std::filesystem::path& path, CImage& first, CImage& second,
        RECT crop, const wchar_t* firstLabel, const wchar_t* secondLabel)
    {
        CImage output;
        output.Create(Width, Height, 24);
        HDC dc = output.GetDC();
        Fill(dc, {0, 0, Width, Height}, RGB(13, 26, 43));
        Caption(dc, firstLabel, {12, 8, 444, 46});
        Caption(dc, secondLabel, {456, 8, 888, 46});
        DrawCrop(dc, first, crop, {12, 54, 444, 486});
        DrawCrop(dc, second, crop, {456, 54, 888, 486});
        Fill(dc, {449, 16, 451, 484}, RGB(67, 91, 111));
        output.ReleaseDC();
        assert(SUCCEEDED(output.Save(path.c_str())));
    }

    void Start(RhythmMiniGame& game, std::vector<FloweringChart::Note> notes)
    {
        game.ClearAllNotes();
        game.floweringNotes = std::move(notes);
        game.floweringNextNote = 0;
        game.floweringTimeMs = musicPosition = 0;
        game.floweringMaximumScore = FloweringChart::MaximumScore(game.floweringNotes);
        game.score = game.combo = game.maxCombo = 0;
        game.hitUnits = game.missedUnits = game.floweringHitUnits = 0;
        game.lastJudge = JUDGE_NONE;
        game.isMouseDown = false;
        game.mouseX = game.mouseY = -300;
        for (auto& point : game.cursorTrail) point.isActive = false;
    }

    void At(RhythmMiniGame& game, int time)
    {
        musicPosition = time;
        game.Update();
    }

    void FollowTo(RhythmMiniGame& game, int target)
    {
        while (musicPosition < target)
        {
            musicPosition = (std::min)(musicPosition + 10, target);
            const auto& slider = game.sliders[0];
            int x, y;
            game.GetSliderPosition(slider,
                (musicPosition - slider.slideStartTime) / double(slider.duration), x, y);
            game.OnMouseMove(x, y);
            game.Update();
        }
        assert(!game.sliders[0].isFailed);
    }
}

int wmain(int argc, wchar_t** argv)
{
    assert(argc == 3);
    const std::filesystem::path output = std::filesystem::absolute(argv[1]);
    std::filesystem::create_directories(output);
    HWND window = CreateWindowW(L"STATIC", L"Hidden tutorial capture", WS_POPUP,
        0, 0, 1920, 1080, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    assert(window && !IsWindowVisible(window));
    GameOptions::difficulty = 1;
    GameOptions::musicVolume = GameOptions::effectsVolume = 0;
    RhythmMiniGame game;
    assert(game.Init(window, true, 0));
    assert(!game.hitCircleImg.IsNull() && !game.sliderFollowCircleImg.IsNull());
    const auto chart = FloweringChart::Build();
    const auto find = [&](int flags, int excluded = 0)
    {
        const auto it = std::find_if(chart.begin(), chart.end(), [&](const auto& note)
            { return (note.flags & flags) == flags && !(note.flags & excluded); });
        assert(it != chart.end());
        return *it;
    };
    CImage first, second;

    const auto wind = find(FloweringChart::Curve, FloweringChart::Return);
    Start(game, {wind});
    At(game, wind.timeMs - 250);
    const RECT windCrop = SliderCrop(game);
    Capture(game, first);
    At(game, wind.timeMs);
    game.OnMouseDown(game.sliders[0].startX, game.sliders[0].startY);
    FollowTo(game, wind.timeMs + wind.durationMs * 3 / 5);
    Capture(game, second);
    SavePair(output / L"wind.png", first, second, windCrop, L"1. 시작 원 누르기", L"2. 누른 채 공 따라가기");

    const auto memory = find(FloweringChart::Memory);
    Start(game, {memory});
    At(game, memory.timeMs - game.MemoryHideMs() - 140);
    Capture(game, first);
    At(game, memory.timeMs - game.MemoryHideMs() + 50);
    assert(game.HideFloweringCircle(0, musicPosition));
    Capture(game, second);
    SavePair(output / L"memory.png", first, second,
        SquareCrop(static_cast<int>(memory.x * 1920), static_cast<int>(memory.y * 1080), 470),
        L"1. 보라색 꽃 위치 기억", L"2. 사라져도 같은 자리");

    const auto encore = find(FloweringChart::Encore, FloweringChart::EncoreRepeat);
    const auto encoreIt = std::find_if(chart.begin(), chart.end(), [&](const auto& note)
        { return (note.flags & FloweringChart::EncoreRepeat) && note.timeMs > encore.timeMs; });
    assert(encoreIt != chart.end());
    Start(game, {encore, *encoreIt});
    At(game, encore.timeMs - 140);
    Capture(game, first);
    At(game, encore.timeMs);
    game.OnMouseDown(static_cast<int>(encore.x * 1920), static_cast<int>(encore.y * 1080));
    game.OnMouseUp(-300, -300);
    At(game, encoreIt->timeMs - 140);
    assert(game.hitCircleCount == 1);
    Capture(game, second);
    SavePair(output / L"encore.png", first, second,
        SquareCrop(static_cast<int>(encore.x * 1920), static_cast<int>(encore.y * 1080) + 40, 470),
        L"1. 첫 박자 클릭", L"2. 같은 자리 한 번 더");

    const auto returnIt = std::find_if(chart.begin(), chart.end(), [](const auto& note)
        { return (note.flags & FloweringChart::Return) && note.y > .43f && note.endY > .43f; });
    assert(returnIt != chart.end());
    const auto returning = *returnIt;
    Start(game, {returning});
    At(game, returning.timeMs);
    RECT returnCrop = SliderCrop(game);
    returnCrop.bottom = (std::min)(returnCrop.bottom, 790L); // Keep the stage hint outside the close-up.
    game.OnMouseDown(game.sliders[0].startX, game.sliders[0].startY);
    FollowTo(game, returning.timeMs + returning.durationMs / 5);
    Capture(game, first);
    FollowTo(game, returning.timeMs + returning.durationMs * 4 / 5);
    Capture(game, second);
    SavePair(output / L"return.png", first, second, returnCrop, L"1. 화살표까지 따라가기", L"2. 누른 채 되돌아오기");

    const auto spinner = find(FloweringChart::Spinner);
    Start(game, {spinner});
    At(game, spinner.timeMs);
    const int centerX = static_cast<int>(1920 * .40), centerY = static_cast<int>(1080 * .48);
    const int spinRadius = static_cast<int>(game.screenHeight * .25);
    game.OnMouseDown(centerX + spinRadius, centerY);
    bool capturedSpin = false;
    for (int i = 0; i <= 300 && !game.floweringSpinner.cleared; ++i)
    {
        At(game, spinner.timeMs + i * 10);
        game.OnMouseMove(centerX + static_cast<int>(std::cos(i * .12) * spinRadius),
            centerY + static_cast<int>(std::sin(i * .12) * spinRadius));
        game.Update();
        if (!capturedSpin && game.floweringSpinner.rotation >= 4.0)
        {
            Capture(game, first);
            capturedSpin = true;
        }
    }
    assert(capturedSpin && game.floweringSpinner.cleared && game.score == 300);
    Capture(game, second);
    SavePair(output / L"spinner.png", first, second, SquareCrop(centerX, centerY, 620),
        L"1. 누른 채 빙글빙글", L"2. 꽃바람 완성");

    std::vector<FloweringChart::Note> relay;
    for (const auto& note : chart)
        if ((note.flags & FloweringChart::Relay) && note.timeMs == FloweringChart::BeatTime(136)) relay.push_back(note);
    assert(relay.size() == 4);
    Start(game, relay);
    CImage relayOutput;
    relayOutput.Create(Width, Height, 24);
    HDC relayDC = relayOutput.GetDC();
    Fill(relayDC, {0, 0, Width, Height}, RGB(13, 26, 43));
    Caption(relayDC, L"1. 나타나는 순서 기억", {12, 4, 888, 42});
    for (int i = 0; i < 4; ++i)
    {
        At(game, FloweringChart::BeatTime(129 + i * 2));
        Capture(game, first);
        DrawCrop(relayDC, first, {230, 255, 1290, 810},
            {18 + i * 220, 42, 222 + i * 220, 156});
        const std::wstring order = std::to_wstring(i + 1);
        Caption(relayDC, order.c_str(), {18 + i * 220, 158, 222 + i * 220, 187});
    }
    At(game, relay[0].timeMs + 100);
    assert(game.hitCircleCount == 4);
    Capture(game, second);
    Caption(relayDC, L"2. 네 꽃을 기억한 순서대로 클릭", {12, 190, 888, 230});
    DrawCrop(relayDC, second, {230, 255, 1290, 810}, {12, 235, 888, 494});
    relayOutput.ReleaseDC();
    assert(SUCCEEDED(relayOutput.Save((output / L"relay.png").c_str())));

    CImage sheet;
    sheet.Create(1800, 1650, 24);
    HDC sheetDC = sheet.GetDC();
    Fill(sheetDC, {0, 0, 1800, 1650}, RGB(13, 26, 43));
    int index = 0;
    for (const auto* name : {L"wind", L"memory", L"relay", L"encore", L"return", L"spinner"})
    {
        CImage saved;
        assert(SUCCEEDED(saved.Load((output / (std::wstring(name) + L".png")).c_str())));
        assert(saved.GetWidth() == Width && saved.GetHeight() == Height);
        const int x = index % 2 * 900, y = index / 2 * 550;
        Caption(sheetDC, name, {x, y, x + 900, y + 42});
        DrawCrop(sheetDC, saved, {0, 0, Width, Height}, {x, y + 45, x + 900, y + 545});
        ++index;
        std::wcout << L"Captured " << name << L" (900x500)\n";
    }
    sheet.ReleaseDC();
    assert(SUCCEEDED(sheet.Save(std::filesystem::absolute(argv[2]).c_str())));
    game.Release();
    DestroyWindow(window);
}
