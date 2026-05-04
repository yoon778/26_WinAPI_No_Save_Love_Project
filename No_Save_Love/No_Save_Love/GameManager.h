#pragma once

#include <windows.h>
#include <string>
#include <vector>

class GameManager
{
public:
    // 게임 시작
    bool Initialize(HWND hWnd);

    // 게임 종료
    void Shutdown();

    // 마우스 클릭이 발생했을 때
    void OnMouseClick(int x, int y);

    // 화면을 그릴 때
    void Render(HDC hdc);

   
private:
    // 현재 게임이 실행되는 윈도우 핸들
    HWND m_hWnd = nullptr;

    std::wstring heroineNames[3] = {
    L"한세아",
    L"유하린",
    L"서이린"
    };
//gamemode ====================================================================
    enum game_mode_info
    {
        Dialogue,
        Choice,
        Result
    };
    game_mode_info now_game_mode = game_mode_info::Dialogue;

//render ====================================================================
// diagoule_mode
    // 대사 목록
    struct DialogueLine_info {
        std::wstring speaker;
        std::wstring text;
    };
    std::vector<DialogueLine_info> dialogues;

    //speaker 마다의 폰트 색상
    struct SpeakerStyle
    {
        COLORREF name_color; // 이름
        COLORREF text_color; // 대화
        COLORREF accent;    // 강조
        COLORREF outline;   // 테두리
    };
    SpeakerStyle hansea_font_style = { RGB(238, 242, 248), RGB(120, 155, 210) ,RGB(185, 205, 240),RGB(70, 95, 140) };
    SpeakerStyle yuharin_font_style = { RGB(245, 242, 235), RGB(170, 70, 75) ,RGB(210, 120, 130),RGB(120, 80, 85) };
    SpeakerStyle seoirin_font_style = { RGB(250, 250, 252), RGB(85, 120, 200) ,RGB(140, 180, 240),RGB(60, 90, 160) };
    
    // 현재 출력 중인 대사 번호
    int m_currentDialogueIndex = 0;

    // 대화창 영역
    RECT dialogue_box = { 100, 830, 1820, 1030 };

    // 이름창 영역
    RECT name_box = { 280, 885, 430, 950 };

    // 대사 텍스트 영역
    RECT text_rect = { 520, 885, 1600, 965 };
    
    // 대화창 위에 색있는 줄 하나
    int lineY = 825;

//choice_mode
    RECT choiceHitBox[3] =
    {
        { 610, 220, 1290, 345 }, // 한세아
        { 610, 420, 1290, 545 }, // 유하린
        { 610, 620, 1290, 745 }  // 서이린
    };

    POINT shoiceBox[3][4] =
    {
        {// 한세아
            { 670, 220 },  // 왼쪽 위
            { 1290, 220 }, // 오른쪽 위
            { 1235, 345 }, // 오른쪽 아래
            { 610, 345 }   // 왼쪽 아래
        },
        {// 유하린
            { 670, 420 },  // 왼쪽 위
            { 1290, 420 }, // 오른쪽 위
            { 1235, 545 }, // 오른쪽 아래
            { 610, 545 }   // 왼쪽 아래
        },
        { // 서이린
            { 670, 620 },  // 왼쪽 위
            { 1290, 620 }, // 오른쪽 위
            { 1235, 745 }, // 오른쪽 아래
            { 610, 745 }   // 왼쪽 아래
        }
    };
//result
    int selectedCharacter = -1;

};