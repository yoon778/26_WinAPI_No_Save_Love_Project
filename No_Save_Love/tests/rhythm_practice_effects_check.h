#pragma once
#include <atlimage.h>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "../No_Save_Love/minigame2/RhythmPracticeEffects.h"

// 실제 1920x1080 GDI 프레임을 비교한다. 창이나 음원, 게임 상태는 변경하지 않는다.
inline void CheckRhythmPracticeEffects(const std::filesystem::path& output)
{
    using RhythmPracticeEffects::State;
    std::filesystem::create_directories(output);
    CImage frame;
    assert(frame.Create(1920, -1080, 32));
    const RECT canvas{0, 0, 1920, 1080};
    const auto snapshot = [&](const State& state) {
        HDC dc = frame.GetDC();
        assert(PatBlt(dc, 0, 0, 1920, 1080, BLACKNESS));
        const COLORREF sentinel = RGB(13, 97, 173);
        SetDCBrushColor(dc, sentinel);
        RhythmPracticeEffects::Render(dc, state, 458);
        assert(GetDCBrushColor(dc) == sentinel);
        GdiFlush();
        frame.ReleaseDC();
        std::vector<uint32_t> pixels(1920 * 1080);
        for (int y = 0; y < 1080; ++y)
            std::memcpy(pixels.data() + y * 1920, frame.GetPixelAddress(0, y), 1920 * sizeof(uint32_t));
        // 가운데 노트 영역은 이 배경 연출이 직접 칠하지 않는다.
        for (int y = 80; y < 900; ++y)
            for (int x = 90; x < 1830; ++x)
                assert((pixels[y * 1920 + x] & 0x00ffffff) == 0);
        return pixels;
    };
    const auto difference = [](const auto& first, const auto& second) {
        size_t count = 0;
        for (size_t i = 0; i < first.size(); ++i) count += first[i] != second[i];
        return count;
    };
    const auto changedIn = [](const auto& first, const auto& second, RECT area) {
        for (int y = area.top; y < area.bottom; ++y)
            for (int x = area.left; x < area.right; ++x)
                if (first[y * 1920 + x] != second[y * 1920 + x]) return true;
        return false;
    };

    std::array<State, 6> states;
    for (State& state : states) state.songTime = 1000;
    states[1].successAge = 80;
    states[2].missAge = 90;
    states[3].combo = 20; states[3].comboAge = 340;
    states[4].sliderProgress = .52;
    states[5].windowStage = 1;
    const auto baseline = snapshot(states[0]);
    Sleep(25); // 실제 시간이 흘러도 같은 게임 시각이면 일시정지 화면은 같아야 한다.
    assert(baseline == snapshot(states[0]));
    State advanced = states[0]; advanced.songTime += 127;
    const size_t timeDifference = difference(baseline, snapshot(advanced));
    assert(timeDifference > 0);
    const char* names[] = {"baseline", "success", "miss", "combo", "slider", "small-window"};
    std::ostringstream report;
    report << "time_advance_changed_pixels=" << timeDifference << '\n';
    for (size_t i = 1; i < states.size(); ++i)
    {
        const auto pixels = snapshot(states[i]);
        const size_t changed = difference(baseline, pixels);
        assert(changed > 0);
        assert(pixels == snapshot(states[i]));
        report << names[i] << "_changed_pixels=" << changed << '\n';
    }
    State expired = states[0];
    expired.successAge = 430; expired.missAge = 650; expired.comboAge = 1000;
    assert(baseline == snapshot(expired));
    State comboWithoutBurst = states[3]; comboWithoutBurst.comboAge = MAXDWORD;
    assert(changedIn(snapshot(states[3]), snapshot(comboWithoutBurst), {500, 900, 1450, 980}));
    State sliderLater = states[4]; sliderLater.sliderProgress = .82;
    assert(snapshot(states[4]) != snapshot(sliderLater));
    auto previousStage = snapshot(states[5]);
    assert(changedIn(baseline, previousStage, {0, 100, 80, 900}));
    for (int stage = 2; stage <= 3; ++stage)
    {
        State next = states[5]; next.windowStage = stage;
        const auto pixels = snapshot(next);
        assert(previousStage != pixels);
        const RECT edge = stage == 2 ? RECT{0, 100, 80, 900} : RECT{300, 0, 1600, 80};
        assert(changedIn(baseline, pixels, edge));
        State moved = next; moved.songTime += 127;
        assert(changedIn(pixels, snapshot(moved), edge));
        report << "window_stage_" << stage << "_changed_pixels=" << difference(previousStage, pixels) << '\n';
        previousStage = pixels;
    }

    CImage background;
    assert(SUCCEEDED(background.Load(L"resource\\minigame2\\background\\bg1.png")));
    const auto saveFrame = [&](const State& state, const std::wstring& name) {
        HDC dc = frame.GetDC();
        FillRect(dc, &canvas, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        background.Draw(dc, 0, 0, 1920, 1080);
        RhythmPracticeEffects::Render(dc, state, 458);
        GdiFlush(); frame.ReleaseDC();
        assert(SUCCEEDED(frame.Save((output / name).c_str())));
    };
    for (size_t i = 0; i < states.size(); ++i)
        saveFrame(states[i], L"state" + std::to_wstring(i) + L".png");
    for (int stage = 2; stage <= 3; ++stage)
    {
        State state = states[5]; state.windowStage = stage;
        saveFrame(state, L"window_stage" + std::to_wstring(stage) + L".png");
    }

    // 모든 연출을 동시에 켜서 helper 자체의 최악 비용을 측정한다. 배경 디코딩/저장은 제외한다.
    State busy = states[1]; busy.combo = 30; busy.comboAge = 340;
    busy.missAge = 90; busy.sliderProgress = .52; busy.windowStage = 1;
    HDC dc = frame.GetDC();
    for (int i = 0; i < 40; ++i) RhythmPracticeEffects::Render(dc, busy, 458);
    GdiFlush();
    const DWORD before = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    assert(before > 0);
    std::vector<double> milliseconds;
    milliseconds.reserve(1500);
    const auto limitStart = std::chrono::steady_clock::now();
    for (int i = 0; i < 1500; ++i)
    {
        busy.songTime += 17;
        const auto start = std::chrono::steady_clock::now();
        RhythmPracticeEffects::Render(dc, busy, 458);
        GdiFlush();
        const auto end = std::chrono::steady_clock::now();
        milliseconds.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        if (end - limitStart > std::chrono::seconds(2)) break;
    }
    const DWORD after = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    frame.ReleaseDC();
    assert(before == after);
    assert(milliseconds.size() >= 100);
    double total = 0;
    for (const double time : milliseconds) total += time;
    std::sort(milliseconds.begin(), milliseconds.end());
    report << "render_count=" << milliseconds.size() << "\ngdi_before=" << before << "\ngdi_after=" << after
        << "\neffect_only_mean_ms=" << total / milliseconds.size()
        << "\neffect_only_p95_ms=" << milliseconds[(milliseconds.size() * 95) / 100]
        << "\neffect_only_max_ms=" << milliseconds.back() << '\n';
    std::ofstream metrics(output / L"metrics.txt"); metrics << report.str();
    assert(metrics.good());
    std::cout << "PASS: rhythm practice effects pixels, frozen time, event expiry, stage distinction, protected note area, DC state and GDI stability\n"
        << report.str();
}
