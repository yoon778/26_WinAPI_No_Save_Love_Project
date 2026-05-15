#include "ChoiceScene.h"

void ChoiceScene::Initialize()
{
    choice_back_ground.Load(L"resource\\background\\choicescene_background.png");
    Reset();
}

void ChoiceScene::Reset()
{
    // 선택지 화면에 다시 들어올 때 이전 선택 기록을 지운다. 
    m_hasSelected = false;
    selectedCharacter = -1;
}

void ChoiceScene::HandleChoiceClick(int x, int y)
{
    // 마우스 좌표를 POINT 구조체로 만든다.
    POINT mousePoint = { x, y };

    // 선택지 3개 중 어느 것을 클릭했는지 검사한다.
    for (int i = 0; i < HEROINE_COUNT; i++)
    {
        // 마우스 좌표가 선택지 클릭 영역 안에 있으면 선택 처리한다.
        if (PtInRect(&choiceHitBox[i], mousePoint))
        {
            m_hasSelected = true;
            selectedCharacter = i;
            break;
        }
    }
}

void ChoiceScene::RenderChoice(HDC hDC)
{
    //배경 그리기
    choice_back_ground.Draw(hDC, 0, 0, 1920, 1080);

    // 선택지 박스 내부 색상 브러시를 만든다.
    HBRUSH whiteBrush = CreateSolidBrush(RGB(245, 245, 245));

    // 선택지 박스 테두리 펜을 만든다.
    HPEN bluePen = CreatePen(PS_SOLID, 5, RGB(70, 110, 230));

    // 기존 GDI 객체를 저장하고 새 객체를 적용한다.
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, whiteBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, bluePen));

    // 선택지 박스 3개를 그린다.
    for (int i = 0; i < HEROINE_COUNT; i++)
    {
        Polygon(hDC, choiceBox[i], 4);
    }

    // 원래 브러시와 펜으로 복구한다.
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    // 직접 만든 브러시와 펜은 삭제한다.
    DeleteObject(whiteBrush);
    DeleteObject(bluePen);

    // 선택지 텍스트용 폰트를 만든다.
    HFONT choiceFont = CreateFontW(
        42,
        0,
        0,
        0,
        600,
        FALSE,
        FALSE,
        FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    // 기존 폰트를 저장하고 새 폰트를 적용한다.
    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, choiceFont));

    // 글자 배경을 투명하게 한다.
    SetBkMode(hDC, TRANSPARENT);

    // 글자 색상을 지정한다.
    SetTextColor(hDC, RGB(30, 30, 40));

    // 선택지 문구를 출력한다.
    for (int i = 0; i < HEROINE_COUNT; i++)
    {
        RECT textRect = choiceHitBox[i];

        DrawTextW(
            hDC,
            choiceTexts[i].c_str(),
            -1,
            &textRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE
        );
    }

    // 원래 폰트로 복구하고 만든 폰트를 삭제한다.
    SelectObject(hDC, oldFont);
    DeleteObject(choiceFont);
}

bool ChoiceScene::HasSelected() const
{
    return m_hasSelected;
}

int ChoiceScene::GetSelectedIndex() const
{
    return selectedCharacter;
}