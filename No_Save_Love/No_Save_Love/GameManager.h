#pragma once

#include <windows.h>
#include <string>
#include <array>
#include <vector>

#include "Scenes/TitleScene.h"
#include "Scenes/NameInputScene.h"
#include "Scenes/StoryScene.h"
#include "Scenes/ChoiceScene.h"
#include "Scenes/ResultScene.h"
#include "Scenes/FinalChoiceScene.h"
#include "Scenes/EndingScene.h"


#include "StoryData.h"

class GameManager
{
public:
    void Initialize(HWND hWnd);
    void Shutdown();
    void OnMouseClick(int x, int y);
    void Render(HDC hdc);
    void OnChar(wchar_t inputChar);
    void OnTimer();

private:
    HWND m_hWnd = nullptr;

    enum game_mode_info
    {
        Title, 
        NameInput,
        Story,
        MiniGame,
        Result,
        Choice,
        FinalChoice,
        EndingDialogue,
        Ending
    };

    game_mode_info now_game_mode = game_mode_info::Story;

private:
    TitleScene titleScene;
    NameInputScene nameinputScene;
    StoryScene storyScene;
    ResultScene resultScene;
    ChoiceScene choiceScene;
    StoryData storyData;
    FinalChoiceScene finalChoiceScene;

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

    // 현재 몇 번째 선택 이후 스토리인지
    int currentStoryRound = 0;

    // 현재 몇 번째 미니게임인지
    int currentMiniGameIndex = 0;

    // 임시 미니게임 점수
    int testMiniGameScore = 30;

    // Choice 선택 결과가 중복 반영되는 것을 막기 위한 플래그
    bool m_choiceApplied = false;

    // 엔딩 종류를 계산한다.
    // 0 = Happy, 1 = Bad, 2 = Hidden
    int CalculateEndingType(int heroineIndex) const;

    // 가장 많이 선택된 히로인 번호를 저장한다.
    int finalHeroineIndex = 0;

    // 기본 이름 값
    std::wstring playerName = L"윤서";
     
private:

    // 선택된 히로인에 맞는 다음 StoryScene으로 들어간다.
    bool EnterBranchStory(int selectedHeroineIndex);

    // 미니게임 결과 화면으로 들어간다.
    void EnterResult(int whichGame, int score);

    // ResultScene이 계산한 상승 스탯을 실제 player에 반영한다.
    void ApplyStatGain(const Player_state& plusState);

    // 히로인 선택 횟수를 증가시킨다.
    void AddChoiceCount(std::array<CharacterInfo, HEROINE_COUNT>& characters, int selectedCharacter);

    // 최종 히로인 계산한뒤 finalChoice에게 건네주면서 finalChoice실행
    void EnterFinalChoice();

    // 최종 히로인과 스탯을 바탕으로 엔딩 대사를 StoryScene에 넣는다.
    void EnterEndingStory();

    // 키보드 입력 처리
   
};