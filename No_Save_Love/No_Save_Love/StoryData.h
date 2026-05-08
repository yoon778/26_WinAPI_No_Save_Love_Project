#pragma once

#include <vector>
#include "Scenes/StoryScene.h"

// 스토리 데이터를 관리하는 클래스다.
// GameManager가 직접 12개의 대사를 들고 있지 않도록 분리한다.
class StoryData
{
public:
    // 모든 스토리 데이터를 준비한다.
    void Initialize();

    // 선택 회차와 히로인 번호에 맞는 대사 묶음을 반환한다.
    const std::vector<DialogueLineInfo>& GetBranchStory(int roundIndex, int heroineIndex) const;

private:
    // 히로인은 3명이다.
    static const int HEROINE_COUNT = 3;

    // 선택 기회는 총 4번이다.
    static const int STORY_ROUND_COUNT = 4;

    // 잘못된 값이 들어왔을 때 반환할 빈 대사 묶음이다.
    std::vector<DialogueLineInfo> emptyStory;

    // 4회차 × 3명 = 총 12개의 스토리 묶음이다.
    std::vector<DialogueLineInfo> storyScripts[STORY_ROUND_COUNT][HEROINE_COUNT];
};