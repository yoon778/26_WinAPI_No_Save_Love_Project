#pragma once
#include <array>
#include <cassert>
#include <string>

// 실제 StoryData를 준비한 뒤 호출한다. 정본 회상과 플레이 중 보정이 서로 오염되지 않아야 한다.
void CheckStoryContinuity(const StoryData& data)
{
    auto contains = [](const std::vector<DialogueLineInfo>& lines, const wchar_t* text) {
        for (const auto& line : lines) if (line.text.find(text) != std::wstring::npos) return true;
        return false;
    };
    const std::array<int, 5> unseen{-1, -1, -1, -1, -1};
    assert(data.GetContextualBranchStory(-1, 0, unseen).empty());
    assert(data.GetContextualBranchStory(5, 0, unseen).empty());
    assert(data.GetContextualBranchStory(0, 3, unseen).empty());
    assert(data.GetEventTitle(-1, 0).empty() && data.GetEventSummary(0, 3).empty());
    assert(data.GetEventBackgroundKey(5, 0).empty());

    // 3^5개의 모든 선택 조합. 날짜는 옮기지 않고 연출·화자·줄 순서를 유지한다.
    for (int combination = 0; combination < 243; ++combination)
    {
        std::array<int, 5> choices{};
        int code = combination;
        for (int& choice : choices) { choice = code % 3; code /= 3; }
        for (int round = 0; round < 5; ++round)
            for (int heroine = 0; heroine < 3; ++heroine)
            {
                const auto& canonical = data.GetBranchStory(round, heroine);
                auto contextual = data.GetContextualBranchStory(round, heroine, choices);
                assert(!canonical.empty() && contextual.size() == canonical.size());
                assert(!data.GetEventTitle(round, heroine).empty());
                assert(!data.GetEventSummary(round, heroine).empty());
                assert(!data.GetEventBackgroundKey(round, heroine).empty());
                bool allSeen = true;
                for (int prior = 0; prior < round; ++prior) allSeen &= choices[prior] == heroine;
                for (size_t i = 0; i < canonical.size(); ++i)
                {
                    const auto& a = canonical[i]; const auto& b = contextual[i];
                    assert(a.speaker == b.speaker && a.backgroundKey == b.backgroundKey);
                    assert(a.characterKey == b.characterKey && a.effectKey == b.effectKey && a.bgmKey == b.bgmKey);
                    assert(a.heroineMotion == b.heroineMotion && a.presentation == b.presentation && a.ambient == b.ambient);
                    assert(!b.text.empty());
                    if (allSeen) assert(a.text == b.text);
                }
            }
    }

    auto irin = data.GetContextualBranchStory(3, 2, unseen);
    assert(!contains(irin, L"지워진 책갈피") && !contains(irin, L"더 닳으면"));
    assert(contains(irin, L"작년에 낸 글의 필명") && contains(irin, L"처음 빌려준 책갈피"));
    auto completed = unseen; completed[2] = 2;
    assert(contains(data.GetContextualBranchStory(3, 2, completed), L"지워진 책갈피"));
    assert(contains(data.GetBranchStory(3, 2), L"지워진 책갈피"));
    auto rain = data.GetContextualBranchStory(2, 2, unseen);
    assert(!contains(rain, L"검은 노트 사이에 끼워뒀던 쪽지"));
    assert(contains(rain, L"문집에서 글씨가 흐려진 쪽지"));
    auto harin = data.GetContextualBranchStory(1, 1, unseen);
    assert(!contains(harin, L"박수 담당") && !contains(harin, L"그래서 요즘 학교 사진"));
    assert(contains(data.GetContextualBranchStory(2, 1, unseen), L"금요일까지만 등교하고 이사"));
    assert(contains(data.GetContextualBranchStory(4, 1, unseen), L"토요일에 이사 가"));
    assert(!contains(data.GetContextualBranchStory(3, 0, unseen), L"또 묻혔어"));
    assert(contains(data.GetContextualEndingStory(0, 2, unseen), L"날짜와 번호를 적은 종이를 한 장씩"));
    completed[3] = 2;
    assert(!contains(data.GetContextualEndingStory(0, 2, completed), L"날짜와 번호를 적은 종이를 한 장씩"));
    assert(contains(data.GetContextualPerformanceStory(1, false, unseen), L"전학 전 마지막 등교"));
}

// private을 공개한 기존 Win32 검사 번역 단위에서 사용한다.
void CheckStoryElapsedTiming(StoryScene& scene)
{
    const int previousSpeed = GameOptions::textSpeed;
    const std::vector<DialogueLineInfo> script{{L"", std::wstring(300, L'가'), L"room", L"none"}};
    auto reset = [&]() {
        scene.SetDialogues(script);
        while (scene.fadeState != FadeState::None) scene.UpdateFade();
    };
    GameOptions::textSpeed = 1;
    reset();
    for (int i = 0; i < 16; ++i) scene.Update(0.06f);
    const int slowFrames = scene.visibleTextCount;
    reset();
    for (int i = 0; i < 60; ++i) scene.Update(0.016f);
    assert(slowFrames == 16 && scene.visibleTextCount == slowFrames);
    scene.SetPaused(true);
    const auto visible = scene.visibleTextCount;
    const auto ambient = scene.ambientSeconds;
    scene.Update(0.1f);
    scene.OnKeyDown(VK_SPACE);
    scene.OnMouseClick(1500, 44);
    assert(scene.visibleTextCount == visible && scene.ambientSeconds == ambient && !scene.fastForward);
    scene.SetPaused(false);
    scene.Update(0.06f);
    assert(scene.visibleTextCount == visible + 1);
    GameOptions::textSpeed = 2;
    reset();
    for (int i = 0; i < 60; ++i) scene.Update(0.016f);
    assert(scene.visibleTextCount == slowFrames * 2);
    GameOptions::textSpeed = previousSpeed;
    scene.SetDialogues({});
}
