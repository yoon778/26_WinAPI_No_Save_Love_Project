#include "EndingScene.h"

void EndingScene::Initialize(const std::vector<std::wstring>& credits)
{
    creditLines = credits;
    Reset();
}

void EndingScene::Shutdown()
{
    if (!endingImage.IsNull())
    {
        endingImage.Destroy();
    }
}

void EndingScene::Reset()
{
    scrollOffset = 0;
    isFastScroll = false;
    exitRequested = false;

    int totalHeight = static_cast<int>(creditLines.size()) * lineSpacing;
    stopOffset = startY + totalHeight - 520;
}

void EndingScene::SetEndingImage(int endingType, int heroineIndex)
{
    if (!endingImage.IsNull())
    {
        endingImage.Destroy();
    }

    std::wstring imagePath = GetEndingImagePath(endingType, heroineIndex);

    if (!imagePath.empty())
    {
        endingImage.Load(imagePath.c_str());
    }
}

void EndingScene::OnMouseClick(int x, int y)
{
    POINT mousePoint = { x, y };

    if (IsCreditFinished())
    {
        if (PtInRect(&exitButtonRect, mousePoint))
        {
            exitRequested = true;
        }

        return;
    }

    isFastScroll = true;
}

bool EndingScene::IsExitRequested() const
{
    return exitRequested;
}

void EndingScene::Update()
{
    if (scrollOffset < stopOffset)
    {
        int currentScrollSpeed = isFastScroll ? fastScrollSpeed : scrollSpeed;
        scrollOffset += currentScrollSpeed;

        if (scrollOffset > stopOffset)
        {
            scrollOffset = stopOffset;
        }
    }
}

void EndingScene::Render(HDC hDC)
{
    DrawBackground(hDC);

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

    if (IsCreditFinished())
    {
        DrawExitButton(hDC);
    }
}

std::wstring EndingScene::GetEndingImagePath(int endingType, int heroineIndex) const
{
    if (endingType == 2)
    {
        return L"resource\\endingscene\\hidden_end.png";
    }

    if (endingType == 0)
    {
        if (heroineIndex == 0)
        {
            return L"resource\\endingscene\\hansea_happyend.png";
        }
        else if (heroineIndex == 1)
        {
            return L"resource\\endingscene\\harin_happyend.png";
        }
        else if (heroineIndex == 2)
        {
            return L"resource\\endingscene\\seoirin_happyend.png";
        }
    }
    else if (endingType == 1)
    {
        if (heroineIndex == 0)
        {
            return L"resource\\endingscene\\hansea_badend.png";
        }
        else if (heroineIndex == 1)
        {
            return L"resource\\endingscene\\harin_badend.png";
        }
        else if (heroineIndex == 2)
        {
            return L"resource\\endingscene\\seoirin_badend.png";
        }
    }

    return L"";
}

void EndingScene::DrawBackground(HDC hDC)
{
    RECT backgroundRect = { 0, 0, 1920, 1080 };

    if (!endingImage.IsNull())
    {
        endingImage.Draw(hDC, 0, 0, 1920, 1080);
    }
    else
    {
        HBRUSH backgroundBrush = CreateSolidBrush(RGB(8, 8, 14));
        FillRect(hDC, &backgroundRect, backgroundBrush);
        DeleteObject(backgroundBrush);
    }

    HBRUSH overlayBrush = CreateSolidBrush(RGB(0, 0, 0));
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 80;

    HDC memoryDC = CreateCompatibleDC(hDC);
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memoryDC, overlayBitmap));
    FillRect(memoryDC, &backgroundRect, overlayBrush);

    AlphaBlend(hDC, 0, 0, 1920, 1080, memoryDC, 0, 0, 1920, 1080, blend);

    SelectObject(memoryDC, oldBitmap);
    DeleteObject(overlayBitmap);
    DeleteDC(memoryDC);
    DeleteObject(overlayBrush);
}

bool EndingScene::IsCreditFinished() const
{
    return scrollOffset >= stopOffset;
}

void EndingScene::DrawExitButton(HDC hDC)
{
    HBRUSH buttonBrush = CreateSolidBrush(RGB(20, 20, 28));
    HPEN buttonPen = CreatePen(PS_SOLID, 2, RGB(245, 225, 245));

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, buttonBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, buttonPen));

    RoundRect(
        hDC,
        exitButtonRect.left,
        exitButtonRect.top,
        exitButtonRect.right,
        exitButtonRect.bottom,
        18,
        18
    );

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    DeleteObject(buttonBrush);
    DeleteObject(buttonPen);

    HFONT buttonFont = CreateFontW(
        32,
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
        L"Malgun Gothic"
    );

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, buttonFont));

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(245, 245, 250));
    DrawTextW(
        hDC,
        L"게임 종료",
        -1,
        &exitButtonRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, oldFont);
    DeleteObject(buttonFont);
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
    else if (index == 4 || index == 7 || index == 10 || index == 13 ||
        index == 16 || index == 19 || index == 22 || index == 27 ||
        index == 38 || index == 43 || index == 49 || index == 54 ||
        index == 60)
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
