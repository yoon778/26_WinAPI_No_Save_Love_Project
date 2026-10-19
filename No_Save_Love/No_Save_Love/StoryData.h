#pragma once

#include <vector>
#include <array>
#include "Scenes/StoryScene.h"

class StoryData
{
public:
    // 모든 스토리 데이터를 준비한다.
    void Initialize();

    // 인트로 대사를 반환한다.
    const std::vector<DialogueLineInfo>& GetIntroStory() const;

    // 선택 회차와 히로인 번호에 맞는 분기 스토리를 반환한다.
    const std::vector<DialogueLineInfo>& GetBranchStory(int roundIndex, int heroineIndex) const;
    // 선택하지 않은 사건의 기억을 공유한 것처럼 말하지 않도록 현재 대사를 보정한다.
    std::vector<DialogueLineInfo> GetContextualBranchStory(int roundIndex, int heroineIndex,
        const std::array<int, 5>& completedChoices) const;
    std::vector<DialogueLineInfo> GetContextualEndingStory(int endingType, int heroineIndex,
        const std::array<int, 5>& completedChoices) const;
    std::vector<DialogueLineInfo> GetContextualPerformanceStory(int heroineIndex, bool after,
        const std::array<int, 5>& completedChoices) const;
    std::wstring GetEventTitle(int roundIndex, int heroineIndex) const;
    std::wstring GetEventSummary(int roundIndex, int heroineIndex) const;
    std::wstring GetEventBackgroundKey(int roundIndex, int heroineIndex) const;

    //엔딩 스토리 반환
    const std::vector<DialogueLineInfo>& GetEndingStory(int endingType, int heroineIndex) const;

    // 엔딩 크레딧을 반환한다.
    const std::vector<std::wstring>& GetEndingCredits() const;

    // 최종 선택 화면 전에 보여줄 대사를 반환한다.
    const std::vector<DialogueLineInfo>& GetFinalChoiceIntroStory(int finalChoiceIndex) const;
    const std::vector<DialogueLineInfo>& GetPerformanceStory(int heroineIndex, bool after) const;
    const std::vector<DialogueLineInfo>& GetPromiseStory(int heroineIndex, bool after) const;

private:
    static const int HEROINE_COUNT = 3;
    static const int STORY_ROUND_COUNT = 5;

    static const int ENDING_TYPE_COUNT = 3;
    static const int FINAL_CHOICE_INTRO_COUNT = 4;

    // [엔딩종류][히로인번호]
    // 0 = Happy, 1 = Bad, 2 = Hidden
    std::vector<DialogueLineInfo> endingStories[ENDING_TYPE_COUNT][HEROINE_COUNT];

    // 엔딩 화면에서 천천히 올라갈 크레딧 목록이다.
    std::vector<std::wstring> endingCredits;

    // 인트로 대사 묶음이다.
    std::vector<DialogueLineInfo> introStory;

    // 최종 선택 직전에 보여줄 대사 묶음이다.
    // 0 = 한세아, 1 = 유하린, 2 = 서이린, 3 = 새누 히든
    std::vector<DialogueLineInfo> finalChoiceIntroStories[FINAL_CHOICE_INTRO_COUNT];
    std::vector<DialogueLineInfo> performanceIntros[HEROINE_COUNT];
    std::vector<DialogueLineInfo> performanceOutros[HEROINE_COUNT];
    std::vector<DialogueLineInfo> promiseIntros[HEROINE_COUNT];
    std::vector<DialogueLineInfo> promiseOutros[HEROINE_COUNT];

    // 잘못된 값이 들어왔을 때 반환할 빈 대사 묶음이다.
    std::vector<DialogueLineInfo> emptyStory;

    // 5회차 × 3명 = 총 15개의 분기 스토리 묶음이다.
    std::vector<DialogueLineInfo> storyScripts[STORY_ROUND_COUNT][HEROINE_COUNT];

};
