#pragma once
#include <windows.h>
#include <algorithm>
#include <cmath>

// 노트보다 먼저 그리는 노래방 조명. 입자 저장이나 별도 시계 없이 판정 상태만 읽는다.
namespace RhythmPracticeEffects
{
    struct State
    {
        DWORD songTime = 0;
        DWORD successAge = MAXDWORD, missAge = MAXDWORD, comboAge = MAXDWORD;
        int combo = 0;
        int windowStage = 0; // 0: 기본, 1: 작은 창, 2: 창 점프, 3: 슬라이더 이동
        double sliderProgress = -1.0;
    };

    inline double Pulse(DWORD age, DWORD duration)
    {
        if (age >= duration) return 0.0;
        const double remaining = 1.0 - static_cast<double>(age) / duration;
        return remaining * remaining;
    }

    inline COLORREF Mix(COLORREF from, COLORREF to, double amount)
    {
        amount = (std::clamp)(amount, 0.0, 1.0);
        return RGB(static_cast<int>(GetRValue(from) + (GetRValue(to) - GetRValue(from)) * amount),
            static_cast<int>(GetGValue(from) + (GetGValue(to) - GetGValue(from)) * amount),
            static_cast<int>(GetBValue(from) + (GetBValue(to) - GetBValue(from)) * amount));
    }

    inline void Fill(HDC dc, RECT rect, COLORREF color)
    {
        SetDCBrushColor(dc, color);
        FillRect(dc, &rect, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
    }

    inline void GlowBar(HDC dc, RECT rect, COLORREF color)
    {
        RECT glow = rect;
        InflateRect(&glow, 3, 3);
        Fill(dc, glow, Mix(RGB(24, 15, 41), color, .22));
        Fill(dc, rect, color);
    }

    inline void Render(HDC dc, const State& state, DWORD beatInterval)
    {
        const COLORREF oldBrushColor = GetDCBrushColor(dc);
        beatInterval = (std::max)(DWORD(1), beatInterval);
        const double beat = static_cast<double>(state.songTime % beatInterval) / beatInterval;
        const double success = Pulse(state.successAge, 430);
        const double miss = Pulse(state.missAge, 650);
        const double energy = .35 + (std::min)(state.combo, 30) / 60.0;
        const COLORREF colors[] = {RGB(211, 128, 220), RGB(235, 182, 111), RGB(121, 180, 235), RGB(102, 216, 199)};
        const COLORREF base = colors[(std::clamp)(state.windowStage, 0, 3)];
        const COLORREF color = Mix(base, RGB(178, 106, 128), miss);
        const COLORREF dim = Mix(RGB(27, 18, 46), color, .35);
        const COLORREF bright = Mix(color, RGB(255, 245, 221), success * .8);

        // 하단 중앙만 사용해 좌우의 기존 콤보/실수 캐릭터와 겹치지 않는다.
        Fill(dc, {380, 1040, 1540, 1043}, dim);
        for (int i = 0; i < 24; ++i)
        {
            const double wave = .5 + .5 * std::sin(beat * 6.283185 + i * .57);
            const int height = 6 + static_cast<int>((14 + 30 * energy) * wave * (1 - .7 * miss) + 10 * success);
            const int x = 606 + i * 30;
            Fill(dc, {x, 1024 - height, x + 12, 1024}, Mix(dim, bright, .45 + .55 * wave));
        }

        // 성공한 박자가 무대 양옆으로 퍼진다. 클릭 대상처럼 보이는 원은 사용하지 않는다.
        if (success > 0.0)
        {
            const int spread = static_cast<int>((1.0 - success) * 510);
            const COLORREF pulseColor = Mix(dim, bright, success);
            GlowBar(dc, {930 - spread, 1040, 960 - spread, 1043}, pulseColor);
            GlowBar(dc, {960 + spread, 1040, 990 + spread, 1043}, pulseColor);
        }

        // 누른 채 정확히 따라가는 동안에만 청록색 조명이 끝까지 이어진다.
        if (state.sliderProgress >= 0.0)
        {
            const int head = 720 + static_cast<int>((std::clamp)(state.sliderProgress, 0.0, 1.0) * 480);
            Fill(dc, {720, 964, 1200, 967}, RGB(45, 74, 88));
            Fill(dc, {720, 964, head, 967}, RGB(112, 219, 203));
            GlowBar(dc, {head - 4, 961, head + 4, 970}, RGB(220, 253, 232));
        }

        // 10콤보 축하는 기존 캐릭터 뒤에 최대 12개의 작은 별빛만 더한다.
        if (state.comboAge < 1000)
        {
            const double t = static_cast<double>(state.comboAge) / 1000.0;
            for (int i = 0; i < 12; ++i)
            {
                const int x = 608 + i * 64 + static_cast<int>((i % 2 ? 1 : -1) * t * 26);
                const int y = 994 - static_cast<int>(std::sin(t * 3.141593) * (38 + i % 3 * 19));
                const int radius = 1 + static_cast<int>((1 - t) * 3);
                const COLORREF sparkle = Mix(dim, RGB(255, 231, 173), 1 - t);
                Fill(dc, {x - radius, y - 1, x + radius + 1, y + 2}, sparkle);
                Fill(dc, {x - 1, y - radius, x + 2, y + radius + 1}, sparkle);
            }
        }

        // 창 기믹마다 가장자리 조명의 움직임도 달라지며 화면이나 노트는 흔들지 않는다.
        if (state.windowStage != 0)
        {
            const DWORD cycleDuration = beatInterval * 4;
            const double cycle = static_cast<double>(state.songTime % cycleDuration) / cycleDuration;
            for (int i = 0; i < 6; ++i)
            {
                const double phase = std::fmod(cycle + i / 6.0, 1.0);
                if (state.windowStage == 1)
                {
                    const int inset = 18 + static_cast<int>((.5 + .5 * std::sin(phase * 6.283185)) * 28);
                    const int y = 220 + i * 92;
                    GlowBar(dc, {inset, y, inset + 3, y + 31}, Mix(dim, color, phase));
                    GlowBar(dc, {1917 - inset, y, 1920 - inset, y + 31}, Mix(dim, color, phase));
                }
                else if (state.windowStage == 2)
                {
                    const int y = 160 + static_cast<int>(phase * 650);
                    GlowBar(dc, {20, y, 24, y + 30}, color);
                    GlowBar(dc, {1896, 940 - y, 1900, 970 - y}, color);
                }
                else
                {
                    const int x = 400 + static_cast<int>(phase * 1120);
                    GlowBar(dc, {x, 22, x + 28, 25}, Mix(dim, color, .5 + .5 * std::sin(phase * 3.141593)));
                }
            }
        }
        SetDCBrushColor(dc, oldBrushColor);
    }
}
