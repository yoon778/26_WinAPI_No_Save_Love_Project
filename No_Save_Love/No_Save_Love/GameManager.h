#pragma once

#include <windows.h>
#include <string>
#include <array>

#include "StoryScene.h"
#include "ChoiceScene.h"

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
        Choice,
        Result
    };

    game_mode_info now_game_mode = game_mode_info::Dialogue;

private:
    // 대사 화면 담당 객체다.
    StoryScene storyScene;
    ChoiceScene choiceScene;


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
    

    // 결과 화면 출력용 대화창 영역이다.
    RECT dialogueBox = { 100, 830, 1820, 1030 };

    // 결과 문구 출력 영역이다.
    RECT textRect = { 520, 885, 1600, 965 };

private:


    // 결과 처리 함수
    void HandleResultClick();

    // 결과 화면 출력
    void RenderResult(HDC hDC);

    // choice 호감도 반영
    void AddAffection(std::array<CharacterInfo, HEROINE_COUNT>& characters , int selectedcharacter);

};