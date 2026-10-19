#pragma once

// flowering_check.cpp와 동일하게 production 클래스를 private 공개 상태로 포함한다.
void CheckCombatModes(HWND window)
{
    const int savedDifficulty = GameOptions::difficulty;
    PromiseScene defense;
    float lastInterval = 100, lastEnergy = 200;
    for (int mode = 0; mode < 3; ++mode)
    {
        GameOptions::difficulty = mode;
        defense.Initialize(0);
        defense.BeginBattle();
        assert(defense.energy < lastEnergy);
        lastEnergy = defense.energy;
        defense.spawnTimer = 0;
        defense.SpawnWave();
        assert(defense.spawnTimer < lastInterval);
        lastInterval = defense.spawnTimer;
        assert(defense.GetResultDetails().size() == 3);
        if (mode == 1) assert(defense.energy == 110 && std::abs(defense.spawnTimer - 4.2f) < .001f);
        defense.Shutdown();
    }

    GameOptions::difficulty = 1;
    RhythmMiniGame rhythm;
    assert(rhythm.Init(window, false));
    assert(rhythm.screenWidth == 1920 && rhythm.screenHeight == 1080);
    const auto originalDuration = RHYTHM_GAME_DURATION;
    int previousWindow = 1000;
    for (int mode = 0; mode < 3; ++mode)
    {
        rhythm.difficulty = mode;
        assert(rhythm.TimingWindow(250) < previousWindow);
        previousWindow = rhythm.TimingWindow(250);
        if (mode == 1) assert(rhythm.TimingWindow(80) == 80);
    }
    rhythm.SetPaused(true);
    const auto frozen = rhythm.GetRhythmTime();
    Sleep(90);
    assert(rhythm.GetRhythmTime() == frozen);
    rhythm.SetPaused(false);
    assert(rhythm.GetRhythmTime() - frozen < 60);
    assert(RHYTHM_GAME_DURATION == originalDuration);

    // 창 기믹도 논리 좌표를 유지하며 실제 클릭 대상 크기를 보존한다.
    rhythm.TriggerWindowGimmick(rhythm.GetRhythmTime());
    assert(rhythm.screenWidth == 1920 && rhythm.screenHeight == 1080);
    assert(rhythm.WindowNoteScale() > 1.0);
    rhythm.Release();

    assert(rhythm.Init(window, true));
    const auto noteTime = rhythm.floweringNotes.front().timeMs;
    int lastJudgmentMs = 0;
    for (const auto& note : rhythm.floweringNotes)
        lastJudgmentMs = (std::max)(lastJudgmentMs,
            (note.flags & FloweringChart::Relay) ? note.relayEndMs : note.timeMs + note.durationMs);
    int previousApproach = 2000;
    for (int mode = 0; mode < 3; ++mode)
    {
        rhythm.difficulty = mode;
        assert(rhythm.ApproachTime() < previousApproach);
        previousApproach = rhythm.ApproachTime();
        rhythm.ClearAllNotes();
        rhythm.floweringNextNote = 0;
        rhythm.SpawnFloweringNotes(noteTime - rhythm.ApproachTime());
        assert(rhythm.sliderCount == 1);
        assert(rhythm.sliders[0].hitTime == noteTime);
        assert(rhythm.floweringDurationMs >= static_cast<DWORD>(lastJudgmentMs + 350));
    }
    rhythm.hitUnits = rhythm.missedUnits = rhythm.combo = rhythm.maxCombo = 0;
    rhythm.score = 300;
    rhythm.AddCombo();
    rhythm.ResetCombo(2);
    const auto details = rhythm.GetResultDetails();
    assert(details[0].find(L"1회") != std::wstring::npos && details[0].find(L"2회") != std::wstring::npos);
    assert(details[1].find(L"33%") != std::wstring::npos);
    rhythm.Release();
    GameOptions::difficulty = savedDifficulty;
    std::cout << "PASS: combat difficulty order, normal defaults, rhythm pause clock, logical coordinates, unchanged song/chart, actual result counters\n";
}
