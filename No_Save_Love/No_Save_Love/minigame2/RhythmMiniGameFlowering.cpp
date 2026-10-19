#include "RhythmMiniGame.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    void Flower(Gdiplus::Graphics& g, float x, float y, float size, BYTE alpha, float angle)
    {
        using namespace Gdiplus;
        const auto saved = g.Save();
        g.TranslateTransform(x, y);
        g.RotateTransform(angle);
        SolidBrush petal(Color(alpha, 255, 178, 207));
        for (int i = 0; i < 5; ++i)
        {
            g.FillEllipse(&petal, -size * .34f, -size, size * .68f, size);
            g.RotateTransform(72.f);
        }
        SolidBrush center(Color(alpha, 255, 225, 151));
        g.FillEllipse(&center, -size * .23f, -size * .23f, size * .46f, size * .46f);
        g.Restore(saved);
    }

    void Label(Gdiplus::Graphics& g, const std::wstring& text, float x, float y,
        float size, Gdiplus::Color color, float width = 1200.f)
    {
        Gdiplus::Font font(L"맑은 고딕", size, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush brush(color);
        g.DrawString(text.c_str(), -1, &font, Gdiplus::RectF(x, y, width, size * 3.4f), nullptr, &brush);
    }
}

DWORD RhythmMiniGame::GetRhythmTime() const
{
    return isFlowering ? floweringTimeMs
        : (floweringPaused ? legacyPauseStarted : GetTickCount()) - legacyPausedMs;
}

int RhythmMiniGame::TimingWindow(int milliseconds) const
{
    return milliseconds * (difficulty == 0 ? 135 : difficulty == 2 ? 80 : 100) / 100;
}

int RhythmMiniGame::ApproachTime() const
{
    return difficulty == 0 ? 1560 : difficulty == 2 ? 1000 : FloweringChart::ApproachMs;
}

void RhythmMiniGame::UpdateFloweringClock()
{
    if (!floweringAudioReady || floweringPaused || isResultScreenActive) return;
    wchar_t position[64] = {};
    if (mciSendString(L"status rhythmBGM position", position, 64, nullptr) != 0)
    {
        floweringAudioReady = false;
        return;
    }
    floweringTimeMs = (std::min)(static_cast<DWORD>(_wtoi(position)), floweringDurationMs);
    // MP3 디코더의 마지막 프레임 반올림도 자연 종료로 처리한다. 중간 페이드/절단은 없다.
    if (floweringTimeMs + 1000 >= floweringDurationMs)
    {
        wchar_t mode[32]{};
        if (mciSendString(L"status rhythmBGM mode", mode, 32, nullptr) == 0
            && std::wstring(mode) == L"stopped") floweringTimeMs = floweringDurationMs;
    }
}

void RhythmMiniGame::SetPaused(bool paused)
{
    if ((isFlowering && !floweringAudioReady) || isResultScreenActive || floweringPaused == paused) return;
    if (paused)
    {
        if (isFlowering) UpdateFloweringClock();
        else legacyPauseStarted = GetTickCount();
    }
    else if (!isFlowering) legacyPausedMs += GetTickCount() - legacyPauseStarted;
    if (mciSendString(paused ? L"pause rhythmBGM" : L"resume rhythmBGM", nullptr, 0, nullptr) != 0)
        floweringAudioReady = false;
    floweringPaused = paused;
    floweringSpinner.hasAngle = false;
    // 메뉴나 창 밖에서 버튼을 뗐어도 복귀 후 가짜 홀드가 남지 않는다.
    if (!paused) isMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

void RhythmMiniGame::SpawnFloweringNotes(DWORD currentTime)
{
    while (floweringNextNote < floweringNotes.size())
    {
        const auto& note = floweringNotes[floweringNextNote];
        // 릴레이 응답은 예고가 끝나는 순간 네 원을 함께 활성화한다.
        if ((note.flags & FloweringChart::Relay) && currentTime < static_cast<DWORD>(note.timeMs)) break;
        if (currentTime + ApproachTime() < static_cast<DWORD>(note.timeMs)) break;
        ++floweringNextNote;
        // 지연된 프레임도 원래 음악 타임스탬프를 유지한다. 놓친 노트는 공통 판정에서 MISS 처리한다.
        if (note.flags & FloweringChart::Spinner)
        {
            floweringSpinner = {};
            floweringSpinner.active = true;
            floweringSpinner.startTime = note.timeMs;
            floweringSpinner.endTime = note.timeMs + note.durationMs;
        }
        else if (note.durationMs == 0)
        {
            for (auto& circle : hitCircles)
            {
                if (circle.isActive) continue;
                circle = {true, false, static_cast<int>(note.x * screenWidth),
                    static_cast<int>(note.y * screenHeight),
                    static_cast<DWORD>(note.timeMs - ApproachTime()),
                    static_cast<DWORD>(note.timeMs), note.flags,
                    note.relayOrder, static_cast<DWORD>(note.relayEndMs)};
                ++hitCircleCount;
                break;
            }
        }
        else
        {
            for (auto& slider : sliders)
            {
                if (slider.isActive) continue;
                slider = {};
                slider.isActive = true;
                slider.startX = static_cast<int>(note.x * screenWidth);
                slider.startY = static_cast<int>(note.y * screenHeight);
                slider.endX = static_cast<int>(note.endX * screenWidth);
                slider.endY = static_cast<int>(note.endY * screenHeight);
                slider.spawnTime = note.timeMs - ApproachTime();
                slider.hitTime = note.timeMs;
                slider.duration = note.durationMs;
                slider.floweringCurved = (note.flags & FloweringChart::Curve) != 0;
                slider.floweringReturn = (note.flags & FloweringChart::Return) != 0;
                ++sliderCount;
                break;
            }
        }
    }
}

void RhythmMiniGame::GetSliderPosition(const Slider& slider, double progress, int& x, int& y) const
{
    progress = (std::clamp)(progress, 0.0, 1.0);
    if (isFlowering && slider.floweringReturn)
        progress = progress < .5 ? progress * 2 : (1 - progress) * 2;
    x = static_cast<int>(slider.startX + (slider.endX - slider.startX) * progress);
    y = static_cast<int>(slider.startY + (slider.endY - slider.startY) * progress);
    if (isFlowering && slider.floweringCurved)
        y += static_cast<int>(std::sin(progress * 3.14159265359) * screenHeight * .12
            * (slider.endX > slider.startX ? 1 : -1));
}

bool RhythmMiniGame::HideFloweringCircle(int index, DWORD now) const
{
    const auto& circle = hitCircles[index];
    // 위치를 기억할 시간이 지난 뒤 꽃이 흩어지고, 박자 직후까지 확실히 숨는다.
    if ((circle.floweringFlags & FloweringChart::Memory)
        && now + MemoryHideMs() >= circle.hitTime && now < circle.hitTime + 120)
        return true;
    // 앵콜의 두 번째 접근 원이 첫 번째 접근 원을 덮지 않도록 한다.
    if (circle.floweringFlags & FloweringChart::Encore)
        for (const auto& earlier : hitCircles)
            if (earlier.isActive && earlier.hitTime < circle.hitTime && earlier.x == circle.x && earlier.y == circle.y)
                return true;
    return false;
}

void RhythmMiniGame::RenderFloweringSliderPath(HDC hDC, const Slider& slider)
{
    Gdiplus::Graphics g(hDC);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::Point points[25];
    for (int i = 0; i <= 24; ++i)
        GetSliderPosition(slider, i / (slider.floweringReturn ? 48.0 : 24.0), points[i].X, points[i].Y);
    Gdiplus::Pen outer(Gdiplus::Color(245, 237, 251, 241), 90.f);
    Gdiplus::Pen inner(Gdiplus::Color(255, 67, 125, 121), 72.f);
    outer.SetLineJoin(Gdiplus::LineJoinRound);
    inner.SetLineJoin(Gdiplus::LineJoinRound);
    outer.SetStartCap(Gdiplus::LineCapRound); outer.SetEndCap(Gdiplus::LineCapRound);
    inner.SetStartCap(Gdiplus::LineCapRound); inner.SetEndCap(Gdiplus::LineCapRound);
    g.DrawLines(&outer, points, 25);
    g.DrawLines(&inner, points, 25);
}

void RhythmMiniGame::ProcessFloweringSpin(int x, int y)
{
    auto& spin = floweringSpinner;
    if (!isFlowering || !spin.active || spin.cleared || floweringPaused || !isMouseDown
        || floweringTimeMs < spin.startTime || floweringTimeMs >= spin.endTime)
    {
        spin.hasAngle = false;
        return;
    }
    const double dx = x - screenWidth * .40, dy = y - screenHeight * .48;
    const double radius = std::hypot(dx, dy);
    if (radius < 30 || radius > screenHeight * .36)
    {
        spin.hasAngle = false;
        return;
    }
    const double angle = std::atan2(dy, dx);
    if (spin.hasAngle)
    {
        const double delta = std::remainder(angle - spin.angle, 6.28318530718);
        // 순간 이동/중앙 왕복으로 회전량을 채우지 못하게 한다.
        if (std::abs(delta) <= 1.0) spin.rotation += delta;
    }
    spin.angle = angle;
    spin.hasAngle = true;
}

void RhythmMiniGame::UpdateFloweringSpinner(DWORD now)
{
    auto& spin = floweringSpinner;
    if (!spin.active || now < spin.startTime) return;
    if (!spin.cleared && std::abs(spin.rotation) >= (difficulty + 2) * 6.28318530718)
    {
        spin.cleared = true;
        score += 300;
        AddCombo();
        PlayHitSound();
        lastJudge = JUDGE_PERFECT;
        judgeX = static_cast<int>(screenWidth * .40);
        judgeY = static_cast<int>(screenHeight * .48);
        judgeDisplayStartTime = now;
    }
    if (now >= spin.endTime)
    {
        if (!spin.cleared)
        {
            lastJudge = JUDGE_MISS;
            ResetCombo();
            PlayMissSound();
            judgeX = static_cast<int>(screenWidth * .40);
            judgeY = static_cast<int>(screenHeight * .48);
            judgeDisplayStartTime = now;
        }
        spin.active = false;
        spin.hasAngle = false;
    }
}

void RhythmMiniGame::RenderFloweringStage(HDC hDC)
{
    using namespace Gdiplus;
    const int time = static_cast<int>(floweringTimeMs);
    Graphics g(hDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    const float w = static_cast<float>(screenWidth), h = static_cast<float>(screenHeight);
    const float seconds = time / 1000.f;
    const int phase = FloweringChart::Phase(time);
    const float bloom = (std::clamp)((seconds - 64.f) / 12.f, 0.f, 1.f);
    LinearGradientBrush backdrop(PointF(0, 0), PointF(0, h),
        Color(255, 13, 25, 48), Color(255, 25 + int(bloom * 20), 44 + int(bloom * 50), 66));
    g.FillRectangle(&backdrop, 0.f, 0.f, w, h);

    // 조명은 노트 뒤에서만 움직이며, 판정 위치를 가리지 않는다.
    for (int i = 0; i < 5; ++i)
    {
        const float top = w * (.10f + i * .20f);
        const float swing = std::sin(seconds * .45f + i) * w * .035f;
        PointF beam[] = {PointF(top, 0), PointF(top - w * .12f + swing, h * .87f),
            PointF(top + w * .12f + swing, h * .87f)};
        SolidBrush light(Color(14 + int(bloom * 15), 151, 219, 210));
        g.FillPolygon(&light, beam, 3);
    }
    SolidBrush floor(Color(255, 13, 26, 39));
    g.FillRectangle(&floor, 0.f, h * .86f, w, h * .14f);
    Pen edge(Color(160, 115, 210, 195), 2.f);
    g.DrawLine(&edge, 0.f, h * .86f, w, h * .86f);

    // 성공으로 남은 꽃은 콤보가 끊겨도 사라지지 않는다.
    const int flowers = (std::min)(score / 300, 44);
    for (int i = 0; i < flowers; ++i)
    {
        const float x = w * (.04f + .91f * ((i * 13) % 44) / 44.f);
        const float y = h * (.91f + ((i * 7) % 5) * .014f);
        Flower(g, x, y, h * (.015f + (i % 3) * .003f), 220, i * 37.f);
    }
    const int particles = phase == 0 ? 10 : (phase == 1 ? 20 : 38);
    for (int i = 0; i < particles; ++i)
    {
        const float x = std::fmod(i * .137f + seconds * (phase == 1 ? .038f : .013f), 1.f) * w;
        const float y = std::fmod(i * .113f + seconds * .023f, 1.f) * h;
        Flower(g, x, y, h * .008f, 55, seconds * 25 + i * 23.f);
    }

    const int relay = FloweringChart::RelayCueBeat(time);
    const bool cue = relay >= 0 && time < FloweringChart::BeatTime(relay + 8);
    const bool smiling = cue || combo >= 5;
    Image* heroine = smiling ? floweringHeroineSmile : floweringHeroineNormal;
    const float pulse = std::sin(seconds * 130.f / 60.f * 6.2831853f);
    if (heroine && heroine->GetLastStatus() == Ok)
    {
        const float height = h * (cue ? .72f : .62f);
        const float width = height * heroine->GetWidth() / heroine->GetHeight();
        const float centerX = w * .85f;
        const float bottom = h * .87f + pulse * h * .004f;
        const auto saved = g.Save();
        g.TranslateTransform(centerX, bottom);
        g.RotateTransform(pulse * (cue ? 1.2f : .55f));
        g.DrawImage(heroine, -width / 2, -height, width, height);
        g.Restore(saved);
    }

    const wchar_t* names[] = {L"한세아", L"유하린", L"서이린"};
    const wchar_t* cheers[] = {L"급하지 않게. 네 박자로 해.", L"좋아! 다음 박자도 같이!", L"천천히 해도, 듣고 있어요."};
    SolidBrush panel(Color(225, 17, 32, 51));
    g.FillRectangle(&panel, w * .725f, h * .02f, w * .255f, h * .12f);
    Label(g, names[floweringHeroine], w * .745f, h * .035f, h * .026f, Color(255, 255, 196, 215), w * .22f);
    Label(g, cue ? L"이 네 박자를 기억해 줘!" : cheers[floweringHeroine],
        w * .745f, h * .08f, h * .021f, Color(255, 237, 243, 244), w * .22f);

    Label(g, L"FLOWERING  /  금요일의 작은 무대", w * .19f, h * .033f,
        h * .033f, Color(255, 247, 226, 220), w * .71f);
    const wchar_t* chapters[] = {L"01  씨앗 · 천천히 박자를 맞춰요", L"02  바람 · 기울어진 꽃길을 따라가요", L"03  함께 피우는 봄 · 익힌 기믹을 이어가요", L"04  피날레 · 마지막 음까지 함께"};
    Label(g, chapters[phase], w * .19f, h * .083f, h * .022f, Color(255, 153, 218, 202), w * .7f);

    // 슬라이더를 실제로 따라갈 때만 꽃길이 자란다.
    for (const auto& slider : sliders)
    {
        if (!slider.isActive || !slider.isStarted || slider.isFailed) continue;
        const float progress = (std::clamp)((time - slider.slideStartTime) / float(slider.duration), 0.f, 1.f);
        for (int i = 0; i <= static_cast<int>(progress * 9); ++i)
        {
            const float p = i / 9.f;
            int x, y;
            GetSliderPosition(slider, p, x, y);
            Flower(g, static_cast<float>(x), y - h * .065f, h * .015f, 220, i * 35.f);
        }
    }
}

void RhythmMiniGame::RenderFloweringOverlay(HDC hDC)
{
    using namespace Gdiplus;
    const int time = static_cast<int>(floweringTimeMs);
    Graphics g(hDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    const float w = static_cast<float>(screenWidth), h = static_cast<float>(screenHeight);
    const int relay = FloweringChart::RelayCueBeat(time);
    const int cueStart = relay < 0 ? -10000 : FloweringChart::BeatTime(relay);
    const int response = relay < 0 ? -5000 : FloweringChart::BeatTime(relay + 8);
    std::wstring mechanic;
    if (time >= FloweringChart::BeatTime(60) && time < FloweringChart::BeatTime(92))
        mechanic = L"바람 슬라이더 · 곧은 길도 곡선 꽃길도, 공을 끝까지 따라가요";
    else if (time >= FloweringChart::BeatTime(92) && time < FloweringChart::BeatTime(128))
        mechanic = L"기억 꽃 · 보라색 꽃잎이 흩어지면 원이 숨어요. 기억한 자리에서 박자에 맞춰 클릭!";
    else if (time >= FloweringChart::BeatTime(144) && time < FloweringChart::BeatTime(172))
        mechanic = L"앵콜 노트 · 돌아오는 화살표에 맞춰 같은 자리에서 두 번! 그다음은 세 박자 쉬어요";
    else if (time >= FloweringChart::BeatTime(172) && time < FloweringChart::BeatTime(208))
        mechanic = L"왕복 슬라이더 · 누른 채 공을 따라가고, 끝의 화살표에서 돌아와요";
    else if (time >= FloweringChart::BeatTime(208) && time < FloweringChart::BeatTime(224))
        mechanic = L"꽃바람 스피너 · 버튼을 누른 채 중심 둘레를 빙글빙글! 어느 방향도 좋아요";
    else if (time >= FloweringChart::BeatTime(224))
        mechanic = L"혼합 무대 · 기억 꽃 · 앵콜 · 왕복 꽃길 · 스피너 / 히로인 릴레이도 돌아옵니다";
    if (!mechanic.empty() && relay < 0 && !floweringSpinner.active)
        Label(g, mechanic, w * .07f, h * .80f, h * .021f, Color(255, 224, 229, 244), w * .64f);

    for (const auto& slider : sliders)
    {
        if (!slider.isActive || !slider.floweringReturn) continue;
        const float progress = slider.isStarted
            ? (std::clamp)((time - static_cast<int>(slider.slideStartTime)) / float(slider.duration), 0.f, 1.f) : 0.f;
        const float x = static_cast<float>(slider.endX), y = static_cast<float>(slider.endY);
        SolidBrush marker(Color(245, 25, 76, 89));
        Pen rim(Color(255, 159, 250, 220), 3.f);
        g.FillEllipse(&marker, x - 31, y - 31, 62.f, 62.f);
        g.DrawEllipse(&rim, x - 31, y - 31, 62.f, 62.f);
        Pen arrow(Color(255, 235, 255, 230), 4.f);
        arrow.SetEndCap(LineCapArrowAnchor);
        g.DrawArc(&arrow, x - 17, y - 17, 34.f, 34.f, 45.f, 270.f);
        const wchar_t* step = progress >= .5f ? L"돌아오는 길 · 끝까지 누르고 따라와요" : L"왕복 꽃길 · 화살표에서 되돌아와요";
        Label(g, step, w * .17f, h * .745f, h * .023f, Color(255, 173, 246, 224), w * .53f);
        SolidBrush track(Color(255, 42, 70, 80)), fill(Color(255, 144, 231, 206));
        g.FillRectangle(&track, w * .19f, h * .785f, w * .40f, 5.f);
        g.FillRectangle(&fill, w * .19f, h * .785f, w * .40f * progress, 5.f);
    }

    if (floweringSpinner.active)
    {
        const auto& spin = floweringSpinner;
        const float x = w * .40f, y = h * .48f, radius = h * .175f;
        const bool ready = time >= static_cast<int>(spin.startTime);
        const float fill = static_cast<float>((std::min)(1.0, std::abs(spin.rotation) / ((difficulty + 2) * 6.28318530718)));
        const float left = (std::clamp)((static_cast<int>(spin.endTime) - time)
            / float(spin.endTime - spin.startTime), 0.f, 1.f);
        SolidBrush backdrop(Color(240, 17, 37, 58));
        g.FillEllipse(&backdrop, x - radius, y - radius, radius * 2, radius * 2);
        Pen base(Color(255, 62, 89, 115), 12.f), progress(Color(255, 165, 238, 219), 12.f);
        g.DrawEllipse(&base, x - radius, y - radius, radius * 2, radius * 2);
        if (fill > 0) g.DrawArc(&progress, x - radius, y - radius, radius * 2, radius * 2, -90.f, 360 * fill);
        Pen timer(Color(200, 255, 212, 176), 3.f);
        g.DrawArc(&timer, x - radius - 17, y - radius - 17, (radius + 17) * 2, (radius + 17) * 2, -90.f, 360 * left);
        const float angle = static_cast<float>(spin.rotation);
        for (int petal = 0; petal < 10; ++petal)
        {
            const float a = angle + petal * .6283185f;
            Flower(g, x + std::cos(a) * radius * .70f, y + std::sin(a) * radius * .70f,
                h * .014f, ready ? 210 : 90, a * 57.2958f);
        }
        Flower(g, x, y - h * .026f, h * .044f, 240, angle * 57.2958f);
        const std::wstring status = spin.cleared ? L"BLOOM!" : ready
            ? std::to_wstring(static_cast<int>(fill * 100)) + L"%" : L"준비";
        Label(g, status, x - h * .045f, y + h * .032f, h * .028f, Color(255, 235, 245, 240), h * .15f);
        Label(g, spin.cleared ? L"꽃바람 완성! 다음 박자를 기다려요"
            : L"누른 채 중심 둘레를 돌려요 · 어느 방향도 OK", w * .14f, h * .745f,
            h * .025f, Color(255, 183, 240, 224), w * .6f);
        Label(g, L"꽃바람 스피너 · " + std::to_wstring(difficulty + 2) + L"바퀴", w * .29f, h * .235f,
            h * .025f, Color(255, 255, 220, 192), w * .4f);
    }

    for (int i = 0; i < MAX_HIT_CIRCLES; ++i)
    {
        const auto& note = hitCircles[i];
        if (!note.isActive) continue;
        if (note.floweringFlags & FloweringChart::Memory)
        {
            const float x = static_cast<float>(note.x), y = static_cast<float>(note.y);
            const float radius = hitCircleImg.GetWidth() * .5f;
            const int vanish = static_cast<int>(note.hitTime) - MemoryHideMs();
            const int scatter = time - vanish;
            if (scatter >= 0 && scatter < 260)
            {
                const float p = scatter / 260.f;
                for (int petal = 0; petal < 8; ++petal)
                {
                    const float angle = petal * .785398f + p * .5f;
                    const float travel = radius * (.55f + p * 1.8f);
                    SolidBrush petalBrush(Color(static_cast<BYTE>(210 * (1 - p)), 213, 168, 255));
                    g.FillEllipse(&petalBrush, x + std::cos(angle) * travel - 7,
                        y + std::sin(angle) * travel - 7, 14.f, 14.f);
                }
            }
            if (HideFloweringCircle(i, time)) continue;
            SolidBrush halo(Color(55, 186, 121, 255));
            g.FillEllipse(&halo, x - radius - 13, y - radius - 13, (radius + 13) * 2, (radius + 13) * 2);
            LinearGradientBrush face(PointF(x, y - radius), PointF(x, y + radius),
                Color(255, 121, 67, 161), Color(255, 55, 31, 92));
            g.FillEllipse(&face, x - radius, y - radius, radius * 2, radius * 2);
            Pen outline(Color(255, 225, 190, 255), 4.f);
            g.DrawEllipse(&outline, x - radius, y - radius, radius * 2, radius * 2);
            const float remaining = (std::clamp)((static_cast<int>(note.hitTime) - time)
                / float(ApproachTime()), 0.f, 1.f);
            const float approach = radius * (1 + .8f * remaining);
            g.DrawEllipse(&outline, x - approach, y - approach, approach * 2, approach * 2);
            Flower(g, x, y, radius * .55f, 245, time * .025f);
            Label(g, L"기억 꽃", x - 42, y + radius * 1.8f + 12, 20.f, Color(255, 229, 199, 255), 150.f);
        }
        if (note.floweringFlags & FloweringChart::Relay)
        {
            const float radius = hitCircleImg.GetWidth() * .5f;
            SolidBrush face(Color(255, 42, 87, 107));
            Pen outline(Color(255, 150, 240, 220), 4.f);
            g.FillEllipse(&face, note.x - radius, note.y - radius, radius * 2, radius * 2);
            g.DrawEllipse(&outline, note.x - radius, note.y - radius, radius * 2, radius * 2);
            Flower(g, static_cast<float>(note.x), static_cast<float>(note.y), radius * .48f, 240, 0);
        }
        if (HideFloweringCircle(i, time)) continue;
        if (note.floweringFlags & FloweringChart::Encore)
        {
            const bool repeat = (note.floweringFlags & FloweringChart::EncoreRepeat) != 0;
            const float x = static_cast<float>(note.x), y = static_cast<float>(note.y);
            const float radius = hitCircleImg.GetWidth() * .5f;
            // 반복 원은 첫 타격 시점에서 다시 줄어든다. 조기 판정에도 원이 갑자기 튀지 않는다.
            const int approachMs = repeat ? FloweringChart::BeatTime(1) - FloweringChart::BeatTime(0)
                : FloweringChart::ApproachMs;
            const float remaining = (std::clamp)((static_cast<int>(note.hitTime) - time) / float(approachMs), 0.f, 1.f);
            const float approach = radius * (1.f + .8f * remaining);
            SolidBrush glow(Color(35, 255, 205, 145));
            g.FillEllipse(&glow, x - radius - 12, y - radius - 12, (radius + 12) * 2, (radius + 12) * 2);
            LinearGradientBrush face(PointF(x, y - radius), PointF(x, y + radius),
                Color(255, 55, 78, 104), Color(255, 30, 43, 69));
            g.FillEllipse(&face, x - radius, y - radius, radius * 2, radius * 2);
            Pen outline(Color(255, 255, 221, 169), 3.f);
            Pen timing(Color(230, 255, 218, 163), 3.f);
            g.DrawEllipse(&outline, x - radius, y - radius, radius * 2, radius * 2);
            g.DrawEllipse(&timing, x - approach, y - approach, approach * 2, approach * 2);
            // osu!의 반복 화살표처럼 방향을 읽을 수 있는 원호. 그림/스킨은 직접 그린다.
            Pen arrow(Color(255, 255, 232, 195), 4.f);
            arrow.SetStartCap(LineCapRound); arrow.SetEndCap(LineCapArrowAnchor);
            g.DrawArc(&arrow, x - 19, y - 19, 38.f, 38.f, -60.f, 275.f);
            SolidBrush done(Color(255, 255, 221, 169));
            SolidBrush pending(Color(255, 94, 110, 135));
            g.FillEllipse(&done, x - 14, y + radius + 15, 9.f, 9.f);
            g.FillEllipse(repeat ? &done : &pending, x + 5, y + radius + 15, 9.f, 9.f);
            Label(g, repeat ? L"한 번 더" : L"앵콜 · 두 번", x - 70, y + radius + 32,
                21.f, Color(255, 255, 231, 199), 170.f);
        }
    }
    if (time >= cueStart && time < response)
    {
        const int cue = (std::min)(3, static_cast<int>((time - cueStart) * 130 / 120000));
        const int position = FloweringChart::RelayPosition(relay, cue);
        const float xs[] = {.18f, .37f, .61f, .47f}, ys[] = {.43f, .65f, .45f, .33f};
        Flower(g, w * xs[position], h * ys[position], h * .05f, 220, 0);
        Label(g, std::to_wstring(cue + 1), w * xs[position] - h * .012f, h * ys[position] - h * .024f,
            h * .032f, Color(255, 35, 41, 62), h * .08f);
        Label(g, L"히로인 릴레이 · 순서를 기억해요. 잠시 뒤 네 원이 함께 켜집니다",
            w * .12f, h * .80f, h * .023f, Color(255, 255, 214, 186), w * .58f);
    }
    else if (time >= response && relay >= 0 && time < FloweringChart::BeatTime(relay + 16))
    {
        int remaining = 0;
        for (const auto& note : hitCircles)
            if (note.isActive && (note.floweringFlags & FloweringChart::Relay)) ++remaining;
        const std::wstring hint = time < static_cast<int>(relayErrorUntil)
            ? L"순서가 달라요. 방금 본 순서를 떠올려요!"
            : remaining ? L"기억한 순서대로 클릭!  " + std::to_wstring(4 - remaining) + L" / 4"
            : L"릴레이 성공! 네 박자가 하나로 이어졌어요";
        Label(g, hint, w * .12f, h * .80f, h * .026f,
            time < static_cast<int>(relayErrorUntil) ? Color(255, 255, 156, 169) : Color(255, 255, 214, 186), w * .6f);
        const float progress = (std::clamp)((FloweringChart::BeatTime(relay + 16) - time)
            / float(FloweringChart::BeatTime(8) - FloweringChart::BeatTime(0)), 0.f, 1.f);
        SolidBrush timer(Color(255, 151, 231, 210));
        g.FillRectangle(&timer, w * .17f, h * .85f, w * .44f * progress, 4.f);
    }

    const int remaining = (static_cast<int>(floweringDurationMs) - time + 999) / 1000;
    SolidBrush timerPanel(Color(235, 13, 26, 39));
    g.FillRectangle(&timerPanel, w * .715f, h * .91f, w * .27f, h * .06f);
    Label(g, L"LUCY · 개화     " + std::to_wstring(remaining / 60) + L":"
        + (remaining % 60 < 10 ? L"0" : L"") + std::to_wstring(remaining % 60),
        w * .725f, h * .92f, h * .022f, Color(255, 204, 225, 224), w * .26f);
    if (time < 3200)
        Label(g, L"숨을 고르고, 첫 박자를 기다려요", w * .19f, h * .42f,
            h * .04f, Color(255, 255, 227, 209), w * .51f);
    if (!floweringAudioReady && !isResultScreenActive)
        Label(g, L"음원을 재생하지 못했습니다.\nF6로 공연을 다시 시작해 주세요.", w * .13f, h * .40f,
            h * .033f, Color(255, 255, 202, 202), w * .55f);
    if (isResultScreenActive)
    {
        SolidBrush veil(Color(225, 12, 25, 42));
        g.FillRectangle(&veil, w * .1f, h * .32f, w * .59f, h * .36f);
        Label(g, L"우리의 봄이 무대에 남았다", w * .15f, h * .36f, h * .039f,
            Color(255, 255, 218, 212), w * .5f);
        Label(g, std::to_wstring(resultScore100) + L" / 100   " + GetGradeText(),
            w * .25f, h * .46f, h * .06f, Color(255, 184, 237, 206), w * .43f);
        Label(g, L"성공 " + std::to_wstring(floweringHitUnits) + L" / " + std::to_wstring(floweringMaximumScore / 300)
            + L"   ·   성공률 90% + 박자 정확도 10%", w * .15f, h * .58f,
            h * .023f, Color(255, 229, 234, 239), w * .5f);
    }
}
