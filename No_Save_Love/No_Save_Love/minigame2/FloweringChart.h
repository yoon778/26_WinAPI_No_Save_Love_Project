#pragma once
#include <vector>

// 130 BPM / 130ms 기준. 끝의 디지털 무음만 자르고 원곡의 잔향은 보존한다.
namespace FloweringChart
{
    inline constexpr int DurationMs = 252006;
    inline constexpr int ApproachMs = 1200;
    inline constexpr int CueStartBeat = 128;
    inline constexpr int ResponseBeat = 136;
    inline constexpr int Memory = 1;
    inline constexpr int Encore = 2;
    inline constexpr int Return = 4;
    inline constexpr int Curve = 8;
    inline constexpr int EncoreRepeat = 16;
    inline constexpr int Relay = 32;
    inline constexpr int Spinner = 64;

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

    inline int RelayPosition(int cue, int order)
    {
        constexpr int patterns[3][4] = {{0, 1, 2, 3}, {2, 0, 3, 1}, {1, 3, 0, 2}};
        return patterns[cue == 296 ? 1 : cue == 432 ? 2 : 0][order];
    }

    struct Note
    {
        int timeMs;
        int durationMs; // 0은 클릭, 양수는 슬라이더 또는 스피너의 진행 시간.
        float x, y, endX, endY; // 창 크기에 대한 비율. 오른쪽은 히로인 전용 영역.
        int flags = 0;
        int relayOrder = 0;
        int relayEndMs = 0;
    };

    inline std::vector<Note> Build()
    {
        std::vector<Note> notes;
        constexpr float xs[] = {0.18f, 0.37f, 0.61f, 0.47f};
        constexpr float ys[] = {0.43f, 0.65f, 0.45f, 0.33f};
        for (int beat = 8; beat < 536;)
        {
            // 중반과 후반에도 잠깐 숨을 고른 뒤 응원 패턴을 다시 받아친다.
            int responseBeat = -1;
            for (int cue : {128, 296, 432})
            {
                if (beat >= cue - 4 && beat < cue + 8) beat = cue + 8;
                if (beat >= cue + 8 && beat < cue + 16) responseBeat = cue + 8;
            }
            if (responseBeat >= 0)
            {
                for (int order = 0; order < 4; ++order)
                {
                    const int index = RelayPosition(responseBeat - 8, order);
                    notes.push_back({BeatTime(responseBeat), 0, xs[index], ys[index],
                        xs[index], ys[index], Relay, order, BeatTime(responseBeat + 8)});
                }
                beat = responseBeat + 10;
                continue;
            }
            // 회전 중 다른 노트를 요구하지 않고, 앞뒤에 손을 옮길 시간을 둔다.
            for (int spin : {208, 352, 512})
                if (beat >= spin - 2 && beat < spin) beat = spin;
            if (beat == 208 || beat == 352 || beat == 512)
            {
                notes.push_back({BeatTime(beat), BeatTime(beat + 8) - BeatTime(beat),
                    .40f, .48f, .40f, .48f, Spinner});
                beat += 10;
                continue;
            }
            const int step = beat < 64 ? 4 : 2;
            const bool returning = beat >= 176 && beat % 16 == 4;
            const bool slider = returning || beat % 16 == 8;
            const int index = static_cast<int>(notes.size()) % 4;
            const int hold = returning ? 4 : slider ? 2 : 0;
            float endY = ys[index];
            if (beat >= 64 && beat < 144) endY += index % 2 ? -0.17f : 0.17f;
            if (endY < 0.30f) endY = 0.30f;
            Note note = {BeatTime(beat), hold ? BeatTime(beat + hold) - BeatTime(beat) : 0,
                xs[index], ys[index], xs[index] + (index < 2 ? 0.16f : -0.16f), endY};
            if (slider && beat >= 64) note.flags |= Curve;
            if (!slider && ((beat >= 96 && beat < 124) || (beat >= 208 && beat % 16 == 12)))
                note.flags |= Memory;
            if (returning)
            {
                note.flags |= Return;
                note.endX = xs[(index + 1) % 4];
                note.endY = ys[(index + 1) % 4];
            }
            const bool encore = !slider && beat >= 144 && beat % 16 == 0;
            if (encore) note.flags |= Encore;
            notes.push_back(note);
            if (encore)
            {
                note.timeMs = BeatTime(beat + 1);
                note.flags |= EncoreRepeat;
                notes.push_back(note);
            }
            // 반복 입력 뒤에는 세 박자(약 1.385초)를 남겨 다음 위치로 이동한다.
            beat += returning ? 6 : slider || encore ? 4 : step;
        }
        return notes;
    }

    inline int MaximumScore(const std::vector<Note>& notes)
    {
        int units = 0;
        for (const auto& note : notes) units += note.durationMs > 0 && !(note.flags & Spinner) ? 2 : 1;
        return units * 300;
    }
}
