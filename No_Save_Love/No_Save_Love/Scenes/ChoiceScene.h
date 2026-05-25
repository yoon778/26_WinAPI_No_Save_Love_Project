#pragma once

#include <windows.h>
#include <string>
#include <atlimage.h>

#include "ResultScene.h"

class ChoiceScene
{
public:


    // 선택지 화면을 처음 준비할 때 호출한다.
    void Initialize();

    // 선택지 화면에서 사용한 이미지 리소스를 해제한다.
    void Shutdown();

    // 선택지 화면에 다시 들어올 때 선택 상태를 초기화한다.
    void Reset();

    // GameManager가 현재 플레이어 스탯을 알려줄 때 호출한다.
    void SetPlayerState(const Player_state& playerState);

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
    ATL::CImage heroineFaceImages[3];

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

    // 선택지 카드에 표시할 짧은 히로인 설명이다.
    std::wstring heroineDescriptions[HEROINE_COUNT] =
    {
        L"차분하고 완벽주의적인 학생회장",
        L"밝고 장난스러운 분위기 메이커",
        L"조용하고 감성적인 문예부 소녀"
    };

    // 선택지 카드 하단에 표시할 스탯 힌트다.
    std::wstring heroineHints[HEROINE_COUNT] =
    {
        L"Hint: 재력과 대화 스킬을 중요하게 봅니다.",
        L"Hint: 매력과 대화 스킬을 중요하게 봅니다.",
        L"Hint: 외모와 매력을 중요하게 봅니다."
    };

    // 왼쪽 스탯 패널에 표시할 현재 플레이어 스탯이다.
    Player_state currentPlayerState{};

    // 왼쪽 스탯 패널 영역이다.
    RECT statPanelRect = { 0, 0, 360, 1080 };

    // 선택지 클릭 판정 영역이다.
    RECT choiceHitBox[HEROINE_COUNT] =
    {
        { 540, 100, 1780, 350 },
        { 540, 410, 1780, 660 },
        { 540, 720, 1780, 970 }
    };

private:
    // 왼쪽 스탯 패널 전체를 출력한다.
    void DrawStatPanel(HDC hDC);

    // 스탯 한 줄과 게이지 바를 출력한다.
    void DrawStatRow(HDC hDC, const std::wstring& statName, int statValue, int top);

    // 오른쪽 히로인 선택 카드 하나를 출력한다.
    void DrawChoiceCard(HDC hDC, int index);

    // 선택 카드 왼쪽에 히로인 얼굴 이미지를 출력한다.
    void DrawHeroineFace(HDC hDC, int index, const RECT& iconRect);

    // 사선 카드에 사용할 폴리곤을 그린다.
    void DrawSkewedCard(HDC hDC, const RECT& rect, COLORREF fillColor, COLORREF lineColor);

    // 폰트 생성과 DrawTextW 호출을 한 곳에서 처리한다.
    void DrawTextInRect(HDC hDC, const std::wstring& text, RECT rect, int fontSize, COLORREF color, UINT format, int fontWeight = FW_NORMAL);

};
