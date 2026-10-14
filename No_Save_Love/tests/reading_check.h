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
    for (int i = 0; i < 1000 && !scene.IsFinished(); ++i)
    {
        scene.lastCutsceneTick = GetTickCount64() - 60;
        scene.Update();
    }
    assert(scene.IsFinished() && scene.history.size() == script.size());
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
    assert(tutorials.size() == 3);
    scene.SetDialogues(tutorials);
    FinishFade(scene);
    scene.OnKeyDown(VK_SPACE);
    capture(L"reading_tutorial.png");

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
    scene.ClearHistory();
    assert(scene.history.empty() && !scene.historyOpen && !scene.fastForward);
    scene.SetPlayerName(L"윤서");
    std::cout << "READING PASS: keyboard, 4x sequential log, pause, tutorial, phone, 4 ambient effects, 18 sprite frames\n";
}
