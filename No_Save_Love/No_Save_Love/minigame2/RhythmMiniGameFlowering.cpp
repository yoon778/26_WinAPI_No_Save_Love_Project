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
    return isFlowering ? floweringTimeMs : GetTickCount();
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
    floweringTimeMs = (std::min)(static_cast<DWORD>(_wtoi(position)),
        static_cast<DWORD>(FloweringChart::DurationMs));
    // 공연 편집의 마지막을 부드럽게 마친다. 원본 파일 자체는 변경하지 않는다.
    if (floweringTimeMs > FloweringChart::DurationMs - 1500)
    {
        const int volume = (FloweringChart::DurationMs - floweringTimeMs) * 250 / 1500;
        wchar_t command[80];
        swprintf_s(command, L"setaudio rhythmBGM volume to %d", volume);
        mciSendString(command, nullptr, 0, nullptr);
    }
}

void RhythmMiniGame::SetPaused(bool paused)
{
    if (!isFlowering || !floweringAudioReady || isResultScreenActive || floweringPaused == paused) return;
    if (paused) UpdateFloweringClock();
    if (mciSendString(paused ? L"pause rhythmBGM" : L"resume rhythmBGM", nullptr, 0, nullptr) != 0)
        floweringAudioReady = false;
    floweringPaused = paused;
}

void RhythmMiniGame::SpawnFloweringNotes(DWORD currentTime)
{
    while (floweringNextNote < floweringNotes.size())
    {
        const auto& note = floweringNotes[floweringNextNote];
        if (currentTime + FloweringChart::ApproachMs < static_cast<DWORD>(note.timeMs)) break;
        ++floweringNextNote;
        // 지연된 프레임도 원래 음악 타임스탬프를 유지한다. 놓친 노트는 공통 판정에서 MISS 처리한다.
        if (note.durationMs == 0)
        {
            for (auto& circle : hitCircles)
            {
                if (circle.isActive) continue;
                circle = {true, false, static_cast<int>(note.x * screenWidth),
                    static_cast<int>(note.y * screenHeight),
                    static_cast<DWORD>(note.timeMs - FloweringChart::ApproachMs),
                    static_cast<DWORD>(note.timeMs), note.flags};
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
                slider.spawnTime = note.timeMs - FloweringChart::ApproachMs;
                slider.hitTime = note.timeMs;
                slider.duration = note.durationMs;
                slider.floweringCurved = (note.flags & FloweringChart::Curve) != 0;
                ++sliderCount;
                break;
            }
        }
    }
}

void RhythmMiniGame::GetSliderPosition(const Slider& slider, double progress, int& x, int& y) const
{
    progress = (std::clamp)(progress, 0.0, 1.0);
    x = static_cast<int>(slider.startX + (slider.endX - slider.startX) * progress);
    y = static_cast<int>(slider.startY + (slider.endY - slider.startY) * progress);
    if (isFlowering && slider.floweringCurved)
        y += static_cast<int>(std::sin(progress * 3.14159265359) * screenHeight * .12
            * (slider.endX > slider.startX ? 1 : -1));
}

bool RhythmMiniGame::HideFloweringCircle(int index, DWORD now) const
{
    const auto& circle = hitCircles[index];
    // 기억 꽃은 마지막 250ms만 숨긴다. 판정 시간과 위치는 바꾸지 않는다.
    if ((circle.floweringFlags & FloweringChart::Memory) && now < circle.hitTime && now + 250 >= circle.hitTime)
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
    for (int i = 0; i <= 24; ++i) GetSliderPosition(slider, i / 24.0, points[i].X, points[i].Y);
    Gdiplus::Pen outer(Gdiplus::Color(245, 237, 251, 241), 90.f);
    Gdiplus::Pen inner(Gdiplus::Color(255, 67, 125, 121), 72.f);
    outer.SetLineJoin(Gdiplus::LineJoinRound);
    inner.SetLineJoin(Gdiplus::LineJoinRound);
    g.DrawLines(&outer, points, 25);
    g.DrawLines(&inner, points, 25);
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

    const bool cue = time >= FloweringChart::BeatTime(128)
        && time < FloweringChart::BeatTime(136);
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
    const wchar_t* chapters[] = {L"01  씨앗 · 천천히 박자를 맞춰요", L"02  바람 · 기울어진 꽃길을 따라가요", L"03  개화 · 함께 맞춘 박자가 꽃이 됩니다"};
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
    const int cueStart = FloweringChart::BeatTime(FloweringChart::CueStartBeat);
    const int response = FloweringChart::BeatTime(FloweringChart::ResponseBeat);
    std::wstring mechanic;
    if (time >= FloweringChart::BeatTime(60) && time < FloweringChart::BeatTime(92))
        mechanic = L"바람 슬라이더 · 곧은 길도 곡선 꽃길도, 공을 끝까지 따라가요";
    else if (time >= FloweringChart::BeatTime(92) && time < cueStart)
        mechanic = L"기억 꽃 · 보라색 원은 마지막 순간 숨어요. 위치와 박자를 기억!";
    else if (time >= FloweringChart::BeatTime(144) && time < FloweringChart::BeatTime(172))
        mechanic = L"앵콜 노트 · ×2 꽃은 같은 자리에서 한 박자 간격으로 두 번!";
    else if (time >= FloweringChart::BeatTime(172) && time < FloweringChart::BeatTime(208))
        mechanic = L"거울 무대 · 다음 배치가 좌우로 바뀝니다. 이미 나온 원은 그대로!";
    else if (time >= FloweringChart::BeatTime(208))
        mechanic = L"함께 피우는 봄 · 꽃길, 기억 꽃, 앵콜을 이어가요";
    if (!mechanic.empty())
        Label(g, mechanic, w * .07f, h * .80f, h * .021f, Color(255, 224, 229, 244), w * .64f);

    for (int i = 0; i < MAX_HIT_CIRCLES; ++i)
    {
        const auto& note = hitCircles[i];
        if (!note.isActive || HideFloweringCircle(i, time)) continue;
        if (note.floweringFlags & FloweringChart::Memory)
        {
            Pen ring(Color(255, 207, 161, 255), 5.f);
            g.DrawEllipse(&ring, note.x - 58, note.y - 58, 116, 116);
        }
        if (note.floweringFlags & FloweringChart::Encore)
            Label(g, note.floweringFlags & FloweringChart::EncoreRepeat ? L"2/2" : L"×2",
                note.x - 28.f, note.y - 24.f, 34.f, Color(255, 255, 226, 124), 90.f);
    }
    if (time >= cueStart && time < response)
    {
        const int cue = (std::min)(3, static_cast<int>((time - cueStart) * 130 / 120000));
        const float xs[] = {.18f, .37f, .61f, .47f}, ys[] = {.43f, .65f, .45f, .33f};
        Flower(g, w * xs[cue], h * ys[cue], h * .05f, 220, 0);
        Label(g, std::to_wstring(cue + 1), w * xs[cue] - h * .012f, h * ys[cue] - h * .024f,
            h * .032f, Color(255, 35, 41, 62), h * .08f);
        Label(g, L"응원 보기 · 꽃은 누르지 않아요. 이어지는 원을 같은 순서로!",
            w * .12f, h * .80f, h * .023f, Color(255, 255, 214, 186), w * .58f);
    }
    else if (time >= response && time < FloweringChart::BeatTime(144))
        Label(g, L"이제 네 차례!  1 · 2 · 3 · 4", w * .17f, h * .80f, h * .03f, Color(255, 255, 214, 186), w * .5f);

    const int remaining = (FloweringChart::DurationMs - time + 999) / 1000;
    SolidBrush timerPanel(Color(235, 13, 26, 39));
    g.FillRectangle(&timerPanel, w * .715f, h * .91f, w * .27f, h * .06f);
    Label(g, L"LUCY · 개화     " + std::to_wstring(remaining / 60) + L":"
        + (remaining % 60 < 10 ? L"0" : L"") + std::to_wstring(remaining % 60),
        w * .725f, h * .92f, h * .022f, Color(255, 204, 225, 224), w * .26f);
    if (time < 3200)
        Label(g, L"숨을 고르고, 첫 박자를 기다려요", w * .19f, h * .42f,
            h * .04f, Color(255, 255, 227, 209), w * .51f);
    if (!floweringAudioReady && !isResultScreenActive)
        Label(g, L"음원을 재생하지 못했습니다.\nF5로 공연을 다시 시작해 주세요.", w * .13f, h * .40f,
            h * .033f, Color(255, 255, 202, 202), w * .55f);
    if (isResultScreenActive)
    {
        SolidBrush veil(Color(225, 12, 25, 42));
        g.FillRectangle(&veil, w * .1f, h * .32f, w * .59f, h * .36f);
        Label(g, L"우리의 봄이 무대에 남았다", w * .15f, h * .36f, h * .039f,
            Color(255, 255, 218, 212), w * .5f);
        Label(g, std::to_wstring(resultScore100) + L" / 100   " + GetGradeText(),
            w * .25f, h * .46f, h * .06f, Color(255, 184, 237, 206), w * .43f);
        Label(g, L"박수가 잦아들면, 전하고 싶던 말을 하러 갑니다.", w * .15f, h * .58f,
            h * .023f, Color(255, 229, 234, 239), w * .5f);
    }
}
