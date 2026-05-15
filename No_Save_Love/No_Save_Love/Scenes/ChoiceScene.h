#pragma once

#include <windows.h>
#include <string>
#include <atlimage.h>
#include <vector>
class ChoiceScene
{
public:


    // 선택지 화면을 처음 준비할 때 호출한다.
    void Initialize();

    // 선택지 화면에 다시 들어올 때 선택 상태를 초기화한다.
    void Reset();

    // 선택지 화면에서 마우스를 클릭했을 때 호출한다.
    void HandleChoiceClick(int x, int y);

    // 선택지 화면을 출력한다.
    void RenderChoice(HDC hDC);

    // 선택지를 눌렀는지 확인한다.
    bool HasSelected() const;

    // 선택한 선택지 번호를 반환한다.
    int GetSelectedIndex() const;

private:
    ATL::CImage choice_back_ground;

    // 선택지는 현재 3개로 고정한다.
    static const int HEROINE_COUNT = 3;

    // 선택했는지 여부를 저장한다.
    // 함수 이름 HasSelected와 겹치지 않도록 변수 이름을 다르게 한다.
    bool m_hasSelected = false;

    // 선택된 캐릭터 번호다.
    // 아무것도 선택하지 않았으면 -1이다.
    int selectedCharacter = -1;

    // 선택지 문구다.
    std::wstring choiceTexts[HEROINE_COUNT] =
    {
        L"한세아",
        L"유하린",
        L"서이린"
    };

    // 선택지 클릭 판정 영역이다.
    RECT choiceHitBox[HEROINE_COUNT] =
    {
        { 610, 220, 1290, 345 },
        { 610, 420, 1290, 545 },
        { 610, 620, 1290, 745 }
    };

    // 선택지 표시용 사다리꼴 좌표다.
    POINT choiceBox[HEROINE_COUNT][4] =
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

};