#include "FinalChoiceScene.h"

void FinalChoiceScene::SetFinalHeroine(int heroineIndex, const std::wstring& heroineName)
{
    // 최종 히로인 번호를 저장한다.
    m_heroineIndex = heroineIndex;

    // 최종 히로인 이름을 저장한다.
    m_heroineName = heroineName;
}

void FinalChoiceScene::Reset()
{
    m_finished = false;
}

void FinalChoiceScene::OnMouseClick(int x, int y)
{

    m_finished = true;
}

bool FinalChoiceScene::IsFinished() const
{
    return m_finished;
}

void FinalChoiceScene::Render(HDC hDC)
{
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(30, 30, 40));

    HFONT font = CreateFontW(
        48,
        0,
        0,
        0,
        FW_BOLD,
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, font));

    std::wstring text =
        L"가장 많이 네 곁에 있었던 사람\n\n" +
        m_heroineName +
        L"\n\n클릭하면 마지막 이야기로 이동합니다.";

    RECT textRect = { 0, 0, 1920, 1080 };

    DrawTextW(
        hDC,
        text.c_str(),
        -1,
        &textRect,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK
    );

    SelectObject(hDC, oldFont);
    DeleteObject(font);
}