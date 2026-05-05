#pragma once

#include <windows.h>
#include <string>
#include <array>

#include "Scenes/StoryScene.h"

class GameManager
{
public:
    bool Initialize(HWND hWnd);
    void Shutdown();
    void OnMouseClick(int x, int y);
    void Render(HDC hdc);
private:
    HWND m_hWnd = nullptr;

    enum game_mode_info
    {
        Dialogue,
        Choice,
        Result
    };

    game_mode_info now_game_mode = game_mode_info::Dialogue;

private:
    // 대사 화면 담당 객체다.
    StoryScene storyScene;

private:
    // 선택지 처리 함수
    void HandleChoiceClick(int x, int y);

    // 결과 처리 함수
    void HandleResultClick();

    // 선택지 화면 출력
    void RenderChoice(HDC hDC);

    // 결과 화면 출력
    void RenderResult(HDC hDC);

private:
    static const int HEROINE_COUNT = 3;

    // 히로인 이름 목록이다.
    std::wstring heroineNames[HEROINE_COUNT] = {
        L"한세아",
        L"유하린",
        L"서이린"
    };

    struct CharacterInfo
    {
        std::wstring name; // 캐릭터 이름
        int affection;     // 현재 호감도
    };

    std::array<CharacterInfo, HEROINE_COUNT> characters;
    int choiceAffectionValue = 10;
    
    // 선택지 클릭 판정 영역이다.
    RECT choiceHitBox[3] =
    {
        { 610, 220, 1290, 345 },
        { 610, 420, 1290, 545 },
        { 610, 620, 1290, 745 }
    };

    // 선택지 표시용 사다리꼴 좌표다.
    POINT choiceBox[3][4] =
    {
        {
            { 670, 220 },
            { 1290, 220 },
            { 1235, 345 },
            { 610, 345 }
        },
        {
            { 670, 420 },
            { 1290, 420 },
            { 1235, 545 },
            { 610, 545 }
        },
        {
            { 670, 620 },
            { 1290, 620 },
            { 1235, 745 },
            { 610, 745 }
        }
    };

    // 선택된 캐릭터 번호다.
    int selectedCharacter = -1;

    // 결과 화면 출력용 대화창 영역이다.
    RECT dialogueBox = { 100, 830, 1820, 1030 };

    // 결과 문구 출력 영역이다.
    RECT textRect = { 520, 885, 1600, 965 };
};