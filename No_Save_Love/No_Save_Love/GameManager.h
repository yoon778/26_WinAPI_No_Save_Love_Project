#pragma once

#include <windows.h>
#include <string>
#include <array>

#include "Scenes/StoryScene.h"
#include "Scenes/ChoiceScene.h"
#include "Scenes/ResultScene.h"

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
        Choice
    };

    game_mode_info now_game_mode = game_mode_info::Dialogue;

private:
    // 장면 담당 객체들
    StoryScene storyScene;
    ResultScene resultScene;
    ChoiceScene choiceScene;

private:
    static const int HEROINE_COUNT = 3;

    struct CharacterInfo
    {
        std::wstring name; // 히로인 이름
        int choice_time;   // 선택된 횟수
    };

    // 히로인 선택 횟수 저장
    std::array<CharacterInfo, HEROINE_COUNT> characters;

    // 플레이어 현재 총 스탯
    Player_state player;

private:
    // 현재는 미니게임이 없으므로 테스트용 값으로 둔다.
    int currentMiniGameIndex = 0; // 0: PC방 알바
    int testMiniGameScore = 85;   // 임시 점수

private:
    // 선택한 히로인의 선택 횟수를 증가시킨다.
    void AddChoiceCount(std::array<CharacterInfo, HEROINE_COUNT>& characters, int selectedCharacter);

    // ResultScene으로 들어갈 때 호출한다.
    void EnterResult(int whichGame, int score);

    // ResultScene이 계산한 상승 스탯을 실제 player 스탯에 반영한다.
    void ApplyStatGain(const Player_state& plusState);

    // Result 화면 클릭 처리
    void HandleResultClick();
};