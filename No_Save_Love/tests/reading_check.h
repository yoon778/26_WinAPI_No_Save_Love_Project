// story_cutscene_check.cpp와 동일한 실제 Scene/GDI를 사용한다.
void CheckReading(StoryScene& scene, StoryData& data, CImage& frame, HDC dc,
    const std::filesystem::path& output)
{
    auto capture = [&](const wchar_t* name) {
        scene.Render(dc);
        assert(SUCCEEDED(frame.Save((output / name).c_str())));
    };
    scene.ClearHistory();
    scene.SetPlayerName(L"가나다라마바");
    std::vector<DialogueLineInfo> script;
    for (int i = 0; i < 12; ++i)
        script.push_back({L"학생회 후배", L"{PLAYER}님, 기록 " + std::to_wstring(i), L"hansea_room", L"hansea_normal"});
    scene.SetDialogues(script);
    FinishFade(scene);
    scene.OnKeyDown(VK_SPACE);
    assert(scene.isTypingFinished && scene.currentDialogueIndex == 0 && scene.history.size() == 1);
    capture(L"reading_long_name.png");
    scene.OnKeyDown(VK_SPACE);
    assert(scene.currentDialogueIndex == 1 && scene.history.size() == 1);
    scene.OnKeyDown('L');
    const int visible = scene.visibleTextCount;
    for (int i = 0; i < 10; ++i) { scene.Update(); scene.OnKeyDown(VK_SPACE); }
    assert(scene.historyOpen && scene.visibleTextCount == visible && scene.currentDialogueIndex == 1);
    scene.OnKeyDown(VK_ESCAPE);
    scene.OnKeyDown('F');
    assert(scene.fastForward && !scene.IsFinished());
    int fastTicks = 0;
    for (; fastTicks < 1000 && !scene.IsFinished(); ++fastTicks)
    {
        scene.lastCutsceneTick = GetTickCount64() - 60;
        scene.Update();
    }
    assert(scene.IsFinished() && scene.history.size() == script.size());
    assert(fastTicks <= 45); // 기존 4배속보다 빠르면서 모든 줄을 순서대로 기록한다.
    for (int i = 0; i < 12; ++i)
        assert(scene.history[i].text == L"가나다라마바님, 기록 " + std::to_wstring(i));
    scene.OnKeyDown('L');
    scene.OnMouseWheel(WHEEL_DELTA * 20);
    assert(scene.historyEnd == 5);
    scene.OnKeyDown(VK_END);
    assert(scene.historyEnd == 12);
    capture(L"reading_log.png");
    scene.OnKeyDown(VK_ESCAPE);
    scene.SetDialogues({
        {L"", L"이야기", L"school_day", L"none"},
        {L"시스템", L"직접 확인하는 안내", L"school_day", L"none", L"", L"", HeroineMotion::Idle, StoryPresentation::Tutorial}
    });
    assert(scene.history.size() == 12); // 장면 변경에도 지난 대사는 유지한다.
    FinishFade(scene);
    scene.OnKeyDown('F');
    for (int i = 0; i < 100; ++i) { scene.lastCutsceneTick = GetTickCount64() - 60; scene.Update(); }
    assert(scene.currentDialogueIndex == 1 && !scene.fastForward && !scene.IsFinished());
    scene.OnKeyDown('F');
    assert(!scene.fastForward);
    std::vector<DialogueLineInfo> tutorials;
    for (const auto& line : data.GetIntroStory())
        if (line.presentation == StoryPresentation::Tutorial) tutorials.push_back(line);
    assert(tutorials.size() == 6);
    scene.SetDialogues(tutorials);
    FinishFade(scene);
    scene.OnKeyDown(VK_SPACE);
    capture(L"reading_tutorial.png");
    for (int i = 1; i < 6; ++i)
    {
        scene.OnKeyDown(VK_SPACE); scene.OnKeyDown(VK_SPACE);
        capture((L"reading_tutorial_" + std::to_wstring(i + 1) + L".png").c_str());
    }

    std::vector<DialogueLineInfo> messages;
    for (const auto& line : data.GetBranchStory(1, 0))
        if (line.presentation == StoryPresentation::Phone) messages.push_back(line);
    assert(messages.size() == 3);
    scene.SetDialogues({messages[0], messages[1]});
    FinishFade(scene);
    scene.OnKeyDown(VK_SPACE); scene.OnKeyDown(VK_SPACE); scene.OnKeyDown(VK_SPACE);
    capture(L"reading_phone.png");
    // 모두 읽은 전화 메시지도 로그에서 다시 볼 수 있다.
    assert(scene.history.back().presentation == StoryPresentation::Phone);
    std::vector<DialogueLineInfo> firstChat;
    for (const auto& line : data.GetIntroStory())
        if (line.presentation == StoryPresentation::Phone) firstChat.push_back(line);
    assert(firstChat.size() == 2 && firstChat.back().speaker == L"{PLAYER}");
    scene.SetDialogues(firstChat); FinishFade(scene);
    scene.OnKeyDown(VK_SPACE); scene.OnKeyDown(VK_SPACE); scene.OnKeyDown(VK_SPACE);
    capture(L"reading_phone_reply.png");
    for (auto effect : {AmbientEffect::Rain, AmbientEffect::Petals, AmbientEffect::Sunlight, AmbientEffect::Dust})
    {
        scene.SetDialogues({{L"", L"배경 연출 확인", L"school_day", L"none", L"", L"",
            HeroineMotion::Idle, StoryPresentation::Dialogue, effect}});
        FinishFade(scene); scene.OnKeyDown(VK_SPACE);
        scene.ambientSeconds = 0;
        scene.Render(dc);
        const auto first = Digest(frame, 120, 780);
        scene.ambientSeconds = 2;
        scene.Render(dc);
        assert(first != Digest(frame, 120, 780));
        assert(SUCCEEDED(frame.Save((output / (L"ambient_" + std::to_wstring(static_cast<int>(effect)) + L".png")).c_str())));
    }
    for (auto* heroine : {&scene.hansea, &scene.yuharin, &scene.seoirin})
    {
        assert(heroine->animation && heroine->animation->GetLastStatus() == Gdiplus::Ok);
        assert(heroine->animation->GetWidth() >= 1000 && heroine->animation->GetHeight() >= 1400);
    }
    // 도서관 왼쪽 책장에는 비가 내리지 않고 중앙 유리의 물방울만 움직인다.
    scene.SetDialogues({{L"", L"창밖의 비", L"library_day", L"none", L"", L"",
        HeroineMotion::Idle, StoryPresentation::Dialogue, AmbientEffect::Rain}});
    FinishFade(scene); scene.OnKeyDown(VK_SPACE);
    scene.ambientSeconds = 0; scene.Render(dc);
    const auto shelves = Digest(frame, 150, 700, 30, 650);
    const auto window = Digest(frame, 270, 610, 900, 1175);
    scene.ambientSeconds = 3; scene.Render(dc);
    assert(shelves == Digest(frame, 150, 700, 30, 650));
    assert(window != Digest(frame, 270, 610, 900, 1175));
    capture(L"reading_library_rain.png");
    assert(scene.historyButtonRect.bottom - scene.historyButtonRect.top == 40);
    // 같은 회차의 세 경로에서 같은 소품 애니메이션을 재사용하지 않는다.
    for (int round = 0; round < 4; ++round)
    {
        std::set<std::wstring> previous;
        for (int heroine = 0; heroine < 3; ++heroine)
        {
            std::set<std::wstring> effects;
            for (const auto& line : data.GetBranchStory(round, heroine))
                if (line.effectKey.starts_with(L"anim_")) effects.insert(line.effectKey);
            for (const auto& effect : effects) assert(previous.insert(effect).second);
        }
    }
    scene.ClearHistory();
    assert(scene.history.empty() && !scene.historyOpen && !scene.fastForward);
    // 스킵은 현재 묶음만 완료하고, 읽은 줄을 중복하지 않으며 건너뛴 줄도 보존한다.
    scene.SetDialogues({
        {L"한세아", L"이미 읽은 말", L"hansea_room", L"hansea_normal", L"", L"story1"},
        {L"{PLAYER}", L"건너뛴 {PLAYER}의 답장", L"library_day", L"none", L"", L"story3", HeroineMotion::Idle, StoryPresentation::Phone},
        {L"안내", L"안내도 로그에 남아요", L"", L"", L"", L"stop", HeroineMotion::Idle, StoryPresentation::Tutorial},
        {L"", L"마지막 줄", L"", L"", L"", L""}
    });
    FinishFade(scene); scene.OnKeyDown(VK_SPACE);
    assert(scene.history.size() == 1);
    scene.OnKeyDown('F');
    scene.OnMouseClick(1500, 44);
    assert(scene.IsFinished() && !scene.fastForward && scene.history.size() == 4);
    assert(scene.currentDialogueIndex == 3 && scene.currentBackgroundKey == L"library_day" && scene.currentCharacterKey.empty());
    assert(scene.ConsumePendingBgmKey() == L"stop" && scene.ConsumePendingBgmKey().empty());
    assert(scene.history[1].text.find(L"{PLAYER}") == std::wstring::npos);
    assert(scene.history[1].presentation == StoryPresentation::Phone && scene.history[2].presentation == StoryPresentation::Tutorial);
    scene.OnMouseClick(1500, 44); scene.OnKeyDown('S');
    assert(scene.history.size() == 4);
    scene.OnKeyDown('L'); capture(L"reading_skip_log.png"); scene.OnKeyDown(VK_ESCAPE);
    scene.SetDialogues({{L"", L"새 장면", L"school_day", L"none"}});
    scene.OnKeyDown('L'); scene.OnKeyDown('S');
    assert(!scene.IsFinished()); // 로그 안의 단축키는 진행시키지 않는다.
    scene.OnKeyDown(VK_ESCAPE);
    scene.fadeState = FadeState::FadeOut;
    scene.OnKeyDown('S');
    assert(scene.IsFinished() && scene.fadeState == FadeState::None && scene.history.size() == 5);
    scene.SetDialogues({}); scene.OnKeyDown('S'); assert(scene.IsFinished() && scene.history.size() == 5);
    scene.ClearHistory();
    scene.SetPlayerName(L"윤서");
    std::cout << "READING PASS: keyboard, 16x sequential log, pause, tutorial, phone, 4 ambient effects, continuous original-art blinking\n";
}
