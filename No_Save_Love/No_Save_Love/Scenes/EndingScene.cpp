#include "EndingScene.h"

void EndingScene::Initialize(const std::vector<std::wstring>& credits)
{
    creditLines = credits;
    Reset();
}

void EndingScene::Reset()
{
    scrollOffset = 0;

    int totalHeight = static_cast<int>(creditLines.size()) * lineSpacing;
    stopOffset = startY + totalHeight - 520;
}

void EndingScene::Update()
{
    if (scrollOffset < stopOffset)
    {
        scrollOffset += scrollSpeed;

        if (scrollOffset > stopOffset)
        {
            scrollOffset = stopOffset;
        }
    }
}

void EndingScene::Render(HDC hDC)
{
    RECT backgroundRect = { 0, 0, 1920, 1080 };
    HBRUSH backgroundBrush = CreateSolidBrush(RGB(8, 8, 14));
    FillRect(hDC, &backgroundRect, backgroundBrush);
    DeleteObject(backgroundBrush);

    SetBkMode(hDC, TRANSPARENT);

    for (int i = 0; i < static_cast<int>(creditLines.size()); i++)
    {
        int y = startY - scrollOffset + (i * lineSpacing);

        if (y < -120 || y > 1140)
        {
            continue;
        }

        DrawCreditLine(hDC, creditLines[i], y, i);
    }
}

void EndingScene::DrawCreditLine(HDC hDC, const std::wstring& text, int y, int index)
{
    if (text.empty())
    {
        return;
    }

    int fontSize = 34;
    int fontWeight = FW_NORMAL;
    COLORREF textColor = RGB(235, 235, 242);

    if (index == 0)
    {
        fontSize = 58;
        fontWeight = FW_BOLD;
        textColor = RGB(255, 210, 230);
    }
    else if (index == 2 || text == L"THE END")
    {
        fontSize = 46;
        fontWeight = FW_BOLD;
        textColor = RGB(245, 225, 245);
    }
    else if (index == 4 || index == 7 || index == 10 || index == 13 || index == 20)
    {
        fontSize = 30;
        fontWeight = FW_BOLD;
        textColor = RGB(185, 160, 220);
    }

    HFONT creditFont = CreateFontW(
        fontSize,
        0,
        0,
        0,
        fontWeight,
        FALSE,
        FALSE,
        FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Malgun Gothic"
    );

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, creditFont));

    RECT textRect = { 0, y, 1920, y + 80 };
    SetTextColor(hDC, textColor);
    DrawTextW(
        hDC,
        text.c_str(),
        -1,
        &textRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, oldFont);
    DeleteObject(creditFont);
}
