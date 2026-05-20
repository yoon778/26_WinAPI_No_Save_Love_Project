#pragma once

#include <vector>
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

    //엔딩 스토리 반환
    const std::vector<DialogueLineInfo>& GetEndingStory(int endingType, int heroineIndex) const;

    // 최종 선택 화면 전에 보여줄 대사를 반환한다.
    const std::vector<DialogueLineInfo>& GetFinalChoiceIntroStory(int finalChoiceIndex) const;

private:
    static const int HEROINE_COUNT = 3;
    static const int STORY_ROUND_COUNT = 4; 

    static const int ENDING_TYPE_COUNT = 3;
    static const int FINAL_CHOICE_INTRO_COUNT = 4;

    // [엔딩종류][히로인번호]
    // 0 = Happy, 1 = Bad, 2 = Hidden
    std::vector<DialogueLineInfo> endingStories[ENDING_TYPE_COUNT][HEROINE_COUNT];

    // 인트로 대사 묶음이다.
    std::vector<DialogueLineInfo> introStory;

    // 최종 선택 직전에 보여줄 대사 묶음이다.
    // 0 = 한세아, 1 = 유하린, 2 = 서이린, 3 = 새누 히든
    std::vector<DialogueLineInfo> finalChoiceIntroStories[FINAL_CHOICE_INTRO_COUNT];

    // 잘못된 값이 들어왔을 때 반환할 빈 대사 묶음이다.
    std::vector<DialogueLineInfo> emptyStory;

    // 4회차 × 3명 = 총 12개의 분기 스토리 묶음이다.
    std::vector<DialogueLineInfo> storyScripts[STORY_ROUND_COUNT][HEROINE_COUNT];

};
