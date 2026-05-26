#pragma once

#include <windows.h>
#include <string>
#include <array>
#include <vector>

#include <functional> // 페이드 인 아웃 구현에 필요

// 장면들

#include "Scenes/TitleScene.h"
#include "Scenes/NameInputScene.h"
#include "Scenes/StoryScene.h"
#include "Scenes/ChoiceScene.h"
#include "Scenes/ResultScene.h"
#include "Scenes/FinalChoiceScene.h"
#include "Scenes/EndingScene.h"


// 데이터
#include "StoryData.h"

// 효과
#include "SceneTransition.h"


// 미니 게임
#include"Scenes/MiniGame1TutorialScene.h"
#include "minigame1/PCroomgame.h" // 피시방 미니게임 헤더파일
#include "minigame4/avoidgame.h" // 유혹 피하기 미니게임 헤더파일
#include "minigame2/RhythmMiniGame.h"
#include "minigame3/schoolrun.h"


class GameManager
{
public:
    void Initialize(HWND hWnd);
    void Shutdown();
    void OnMouseClick(int x, int y);
    void Render(HDC hdc);
    void OnChar(wchar_t inputChar);
    void OnKeyDown(WPARAM wParam);
    void OnKeyUp(WPARAM wParam);
    void OnTimer(HWND hWnd);
    void OnMouseUp(int x, int y);
    void OnMouseMove(int x, int y);

private:
    HWND m_hWnd = nullptr;

    enum game_mode_info
    {
        Title, 
        NameInput,
        Story,
        MiniGameTutor1,
        MiniGameTutor2,
        MiniGameTutor3,
        MiniGameTutor4,
        MiniGame1,
        MiniGame2,
        MiniGame3,
        MiniGame4,
        Result,
        Choice,
        FinalChoice,
        EndingDialogue,
        Ending
    };

    enum pending_minigame_release
    {
        PendingMiniGameReleaseNone,
        PendingMiniGameRelease1,
        PendingMiniGameRelease2,
        PendingMiniGameRelease3,
        PendingMiniGameRelease4
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
    EndingScene endingScene;

    MiniGame1TutorialScene minigam1_tutorial1;

    PCroomgame minigame1;
    RhythmMiniGame minigame2;
    schoolrun minigame3;
    avoidgame minigame4;
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
    int testMiniGameScore = 70;

    // Choice 선택 결과가 중복 반영되는 것을 막기 위한 플래그
    bool m_choiceApplied = false;

    // 최종 선택 전용 도입 StoryScene을 이미 보여줬는지 확인한다.
    bool m_finalChoiceIntroShown = false;

    // 엔딩 종류를 계산한다.
    // 0 = Happy, 1 = Bad, 2 = Hidden
    int CalculateEndingType(int heroineIndex) const;

    // 가장 많이 선택된 히로인 번호를 저장한다.
    int finalHeroineIndex = 0;

    // 최종 선택 도입 대사 번호다.
    // 0 = 한세아, 1 = 유하린, 2 = 서이린, 3 = 새누
    int finalChoiceIntroIndex = 0;

    // 기본 이름 값
    std::wstring playerName = L"윤서";

    bool m_isMiniGame2Initialized = false;
    pending_minigame_release m_pendingMiniGameRelease = PendingMiniGameReleaseNone;
     
private:

    // 선택된 히로인에 맞는 다음 StoryScene으로 들어간다.
    bool EnterBranchStory(int selectedHeroineIndex);

    // 미니게임 결과 화면으로 들어간다.
    void EnterResult(int whichGame, int score);

    // 현재 미니게임 번호에 맞는 튜토리얼 모드로 들어간다.
    void EnterCurrentMiniGameTutorial();

    // 현재 미니게임 번호에 맞는 실제 게임 모드로 들어간다.
    void StartCurrentMiniGame();

    // F1~F4 디버그 입력으로 원하는 미니게임에 바로 들어간다.
    void DebugEnterMiniGameByIndex(int miniGameIndex);

    // 현재 미니게임 번호에 맞는 튜토리얼 모드를 반환한다.
    game_mode_info GetTutorialModeByIndex(int miniGameIndex) const;

    // 현재 미니게임 번호에 맞는 실제 게임 모드를 반환한다.
    game_mode_info GetMiniGameModeByIndex(int miniGameIndex) const;

    // 현재 실행 중인 미니게임의 마우스 입력을 처리한다.
    void HandleCurrentMiniGameMouse(int x, int y);

    // 현재 실행 중인 미니게임의 키보드 입력을 처리한다.
    void HandleCurrentMiniGameKey(wchar_t inputChar);

    // 현재 실행 중인 미니게임을 갱신한다.
    void UpdateCurrentMiniGame();

    // 현재 실행 중인 미니게임을 출력한다.
    void RenderCurrentMiniGame(HDC hDC);

    // 미니게임이 끝났을 때 점수를 계산하고 ResultScene으로 이동한다.
    void FinishCurrentMiniGameIfNeeded();

    // Scene 전환이 실제로 끝난 뒤 미니게임 리소스를 정리한다.
    void ReleasePendingMiniGame();

    // ResultScene이 계산한 상승 스탯을 실제 player에 반영한다.
    void ApplyStatGain(const Player_state& plusState);

    // 히로인 선택 횟수를 증가시킨다.
    void AddChoiceCount(std::array<CharacterInfo, HEROINE_COUNT>& characters, int selectedCharacter);

    // 최종 히로인 계산한뒤 finalChoice에게 건네주면서 finalChoice실행
    void EnterFinalChoice();

    // 최종 히로인과 스탯을 바탕으로 엔딩 대사를 StoryScene에 넣는다.
    void EnterEndingStory();

    
 private: // 페이드 인 아웃

     // 전체 Scene 전환 페이드
    SceneTransition sceneTransition;

    // 페이드가 완전히 검은 화면이 되었을 때 이동할 다음 모드
    game_mode_info pendingGameMode;

    // 예약된 Scene 변경이 있는지 확인
    bool hasPendingSceneChange = false;

    // 검은 화면이 된 순간 실행할 준비 작업
    std::function<void()> pendingSceneSetup;

    // 바로 Scene을 바꾸지 않고 페이드 전환을 요청한다.
    void RequestSceneChange(game_mode_info nextMode);

    // 화면이 완전히 검어진 순간 실제 Scene 변경을 적용한다.
    void ApplyPendingSceneChange();

private: // 종료 확인 팝업

    bool m_exitConfirmOpen = false;
    RECT m_exitPopupRect = { 610, 360, 1310, 720 };
    RECT m_exitYesButtonRect = { 720, 585, 910, 655 };
    RECT m_exitNoButtonRect = { 1010, 585, 1200, 655 };

    void RenderExitConfirmPopup(HDC hDC);
    void DrawExitConfirmButton(HDC hDC, const RECT& rect, const wchar_t* text);
    bool IsPointInsideRect(const RECT& rect, int x, int y) const;

};
