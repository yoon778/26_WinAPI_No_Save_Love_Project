#pragma once
#include <vector>

// 제공된 원곡 전체. 130 BPM / 130ms 기준, 마지막 음표 이후 원곡의 여운까지 재생한다.
namespace FloweringChart
{
    inline constexpr int DurationMs = 272341;
    inline constexpr int ApproachMs = 1200;
    inline constexpr int CueStartBeat = 128;
    inline constexpr int ResponseBeat = 136;
    inline constexpr int Memory = 1;
    inline constexpr int Encore = 2;
    inline constexpr int Link = 4;
    inline constexpr int Curve = 8;
    inline constexpr int EncoreRepeat = 16;
    inline constexpr int LinkFollow = 32;

    inline int BeatTime(int beat) { return 130 + (beat * 60000 + 65) / 130; }
    inline int Phase(int timeMs)
    {
        if (timeMs < BeatTime(64)) return 0;
        if (timeMs < BeatTime(208)) return 1;
        if (timeMs < BeatTime(416)) return 2;
        return 3;
    }

    inline int RelayCueBeat(int timeMs)
    {
        for (int beat : {128, 296, 432})
            if (timeMs >= BeatTime(beat) && timeMs < BeatTime(beat + 16)) return beat;
        return -1;
    }

    struct Note
    {
        int timeMs;
        int durationMs; // 0은 클릭, 양수는 누른 채 따라가는 슬라이더.
        float x, y, endX, endY; // 창 크기에 대한 비율. 오른쪽은 히로인 전용 영역.
        int flags = 0;
    };

    inline std::vector<Note> Build()
    {
        std::vector<Note> notes;
        constexpr float xs[] = {0.18f, 0.37f, 0.61f, 0.47f};
        constexpr float ys[] = {0.43f, 0.65f, 0.45f, 0.33f};
        for (int beat = 8; beat < 580;)
        {
            // 중반과 후반에도 잠깐 숨을 고른 뒤 응원 패턴을 다시 받아친다.
            int responseBeat = -1;
            for (int cue : {128, 296, 432})
            {
                if (beat >= cue - 4 && beat < cue + 8) beat = cue + 8;
                if (beat >= cue + 8 && beat < cue + 16) responseBeat = cue + 8;
            }
            const bool response = responseBeat >= 0;
            const int step = beat < 64 ? 4 : 2;
            const bool slider = !response && beat % 16 == 8;
            const int index = response ? (beat - responseBeat) / 2 : static_cast<int>(notes.size()) % 4;
            const int hold = slider ? 2 : 0;
            float endY = ys[index];
            if (beat >= 64 && beat < 144) endY += index % 2 ? -0.17f : 0.17f;
            if (endY < 0.30f) endY = 0.30f;
            Note note = {BeatTime(beat), hold ? BeatTime(beat + hold) - BeatTime(beat) : 0,
                xs[index], ys[index], xs[index] + (index < 2 ? 0.16f : -0.16f), endY};
            if (slider && beat >= 64) note.flags |= Curve;
            if (!response && !slider && ((beat >= 96 && beat < 124) || (beat >= 208 && beat % 16 == 12)))
                note.flags |= Memory;
            const bool link = !response && !slider && beat >= 176 && beat % 16 == 4;
            if (link)
            {
                note.flags = Link;
                note.endX = xs[(index + 1) % 4];
                note.endY = ys[(index + 1) % 4];
            }
            const bool encore = !response && !slider && beat >= 144 && beat % 16 == 0;
            if (encore) note.flags |= Encore;
            notes.push_back(note);
            if (encore)
            {
                note.timeMs = BeatTime(beat + 1);
                note.flags |= EncoreRepeat;
                notes.push_back(note);
            }
            else if (link)
            {
                note.timeMs = BeatTime(beat + 2);
                note.x = note.endX;
                note.y = note.endY;
                note.flags |= LinkFollow;
                notes.push_back(note);
            }
            // 반복 입력 뒤에는 세 박자(약 1.385초)를 남겨 다음 위치로 이동한다.
            beat += slider || encore || link ? 4 : step;
        }
        return notes;
    }

    inline int MaximumScore(const std::vector<Note>& notes)
    {
        int units = 0;
        for (const auto& note : notes) units += note.durationMs > 0 ? 2 : 1;
        return units * 300;
    }
}
