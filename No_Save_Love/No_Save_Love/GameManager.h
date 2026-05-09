#pragma once

#include <windows.h>
#include <string>
#include <array>
#include <vector>

#include "Scenes/StoryScene.h"
#include "Scenes/ChoiceScene.h"
#include "Scenes/ResultScene.h"
#include "StoryData.h"

class GameManager
{
public:
    void Initialize(HWND hWnd);
    void Shutdown();
    void OnMouseClick(int x, int y);
    void Render(HDC hdc);

private:
    HWND m_hWnd = nullptr;

    enum game_mode_info
    {
        Dialogue,
        Result,
        Choice,
        Ending
    };

    game_mode_info now_game_mode = game_mode_info::Dialogue;

private:
    StoryScene storyScene;
    ResultScene resultScene;
    ChoiceScene choiceScene;
    StoryData storyData;
private:
    static const int HEROINE_COUNT = 3;
    static const int STORY_ROUND_COUNT = 4;

    struct CharacterInfo
    {
        std::wstring name; // 히로인 이름
        int choice_time;   // 선택된 횟수
    };

    std::array<CharacterInfo, HEROINE_COUNT> characters;

    // 플레이어 현재 총 스탯
    Player_state player;

private:
    // 4번의 선택 × 3명의 히로인 = 12개의 스토리 묶음
    std::vector<DialogueLineInfo> storyScripts[STORY_ROUND_COUNT][HEROINE_COUNT];

    // 현재 몇 번째 선택 이후 스토리인지
    int currentStoryRound = 0;

    // 현재 몇 번째 미니게임인지
    int currentMiniGameIndex = 0;

    // 임시 미니게임 점수
    int testMiniGameScore = 85;

    // Choice 선택 결과가 중복 반영되는 것을 막기 위한 플래그
    bool m_choiceApplied = false;

private:
    // 12개의 스토리 데이터를 준비한다.
    void InitializeStoryScripts();

    // 선택된 히로인에 맞는 다음 StoryScene으로 들어간다.
    void EnterBranchStory(int selectedHeroineIndex);

    // 미니게임 결과 화면으로 들어간다.
    void EnterResult(int whichGame, int score);

    // ResultScene이 계산한 상승 스탯을 실제 player에 반영한다.
    void ApplyStatGain(const Player_state& plusState);

    // 히로인 선택 횟수를 증가시킨다.
    void AddChoiceCount(std::array<CharacterInfo, HEROINE_COUNT>& characters, int selectedCharacter);
};