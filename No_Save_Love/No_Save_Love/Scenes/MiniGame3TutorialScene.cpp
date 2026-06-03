#include "MiniGame3TutorialScene.h"

void MiniGame3TutorialScene::Initialize()
{
    Shutdown();
    Reset();
    guideImage.Load(L"resource\\minigame3\\minigame3_tuto_image.PNG");
}

void MiniGame3TutorialScene::Shutdown()
{
    if (!guideImage.IsNull())
    {
        guideImage.Destroy();
    }
}

void MiniGame3TutorialScene::Reset()
{
    currentPage = 0;
    finished = false;
}

void MiniGame3TutorialScene::Render(HDC hDC)
{
    DrawBackground(hDC);
    DrawHeader(hDC);

    if (currentPage == 0)
    {
        DrawPageControlGuide(hDC);
    }
    else
    {
        DrawPageGoalGuide(hDC);
    }

    DrawBottomButtons(hDC);
}

void MiniGame3TutorialScene::OnMouseClick(int x, int y)
{
    if (finished)
    {
        return;
    }

    if (IsInsideRect(skipButtonRect, x, y))
    {
        finished = true;
        return;
    }

    if (currentPage > 0 && IsInsideRect(prevButtonRect, x, y))
    {
        GoPrev();
        return;
    }

    if (IsInsideRect(nextButtonRect, x, y))
    {
        GoNext();
    }
}

bool MiniGame3TutorialScene::IsFinished() const
{
    return finished;
}

int MiniGame3TutorialScene::GetCurrentPage() const
{
    return currentPage;
}

bool MiniGame3TutorialScene::IsInsideRect(const RECT& rect, int x, int y) const
{
    return x >= rect.left &&
           x <= rect.right &&
           y >= rect.top &&
           y <= rect.bottom;
}

bool MiniGame3TutorialScene::IsValidImage(const CImage& image) const
{
    return !image.IsNull();
}

void MiniGame3TutorialScene::DrawBackground(HDC hDC)
{
    RECT bgRect = { 0, 0, screenWidth, screenHeight };
    HBRUSH bgBrush = CreateSolidBrush(RGB(18, 16, 22));
    FillRect(hDC, &bgRect, bgBrush);
    DeleteObject(bgBrush);

    RECT mainPanel = { 70, 120, 1850, 940 };
    DrawRoundCard(hDC, mainPanel, RGB(32, 29, 40), RGB(90, 78, 110));

    HPEN linePen = CreatePen(PS_SOLID, 4, RGB(255, 205, 80));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, linePen));

    MoveToEx(hDC, 100, 145, nullptr);
    LineTo(hDC, 1820, 145);

    SelectObject(hDC, oldPen);
    DeleteObject(linePen);
}

void MiniGame3TutorialScene::DrawHeader(HDC hDC)
{
    RECT titleRect = { 100, 35, 1550, 110 };
    DrawTextInRect(
        hDC,
        L"미니게임 3 - 등교길",
        titleRect,
        46,
        RGB(255, 220, 110),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT subTitleRect = { 100, 92, 1650, 135 };
    DrawTextInRect(
        hDC,
        L"장애물을 피하며 제한시간 동안 학교까지 달려가세요.",
        subTitleRect,
        26,
        RGB(230, 230, 235),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_NORMAL
    );

    DrawButton(
        hDC,
        skipButtonRect,
        L"SKIP",
        RGB(65, 60, 75),
        RGB(245, 245, 250)
    );
}

void MiniGame3TutorialScene::DrawPageControlGuide(HDC hDC)
{
    RECT imageCard = { 110, 175, 1145, 905 };
    DrawRoundCard(hDC, imageCard, RGB(22, 21, 28), RGB(105, 95, 125));

    RECT imageRect = { 135, 205, 1120, 875 };
    if (IsValidImage(guideImage))
    {
        DrawImageFit(hDC, guideImage, imageRect);
    }
    else
    {
        DrawTextInRect(hDC, L"튜토리얼 이미지가 없습니다.", imageRect, 30, RGB(220, 220, 230), DT_CENTER | DT_VCENTER | DT_WORDBREAK, FW_BOLD);
    }

    RECT guideCard = { 1190, 175, 1810, 905 };
    DrawRoundCard(hDC, guideCard, RGB(42, 37, 48), RGB(135, 115, 90));

    RECT guideTitle = { 1230, 215, 1770, 270 };
    DrawTextInRect(hDC, L"조작 방법", guideTitle, 40, RGB(255, 220, 110), DT_LEFT | DT_VCENTER | DT_SINGLELINE, FW_BOLD);

    DrawGuideStep(hDC, 0, L"1. SPACE로 점프합니다. 한 번 더 누르면 2단 점프합니다.");
    DrawGuideStep(hDC, 1, L"2. 아래 방향키를 누르고 있으면 슬라이딩합니다.");
    DrawGuideStep(hDC, 2, L"3. 장애물에 맞으면 하트가 줄어듭니다.");

    RECT warning = { 1230, 710, 1770, 830 };
    DrawTextInRect(hDC, L"하트 3개 안에서 최대한 멀리 달리세요!", warning, 30, RGB(255, 210, 90), DT_CENTER | DT_VCENTER | DT_WORDBREAK, FW_BOLD);
}

void MiniGame3TutorialScene::DrawPageGoalGuide(HDC hDC)
{
    RECT imageCard = { 110, 175, 1145, 905 };
    DrawRoundCard(hDC, imageCard, RGB(22, 21, 28), RGB(105, 95, 125));

    RECT imageRect = { 135, 205, 1120, 875 };
    if (IsValidImage(guideImage))
    {
        DrawImageFit(hDC, guideImage, imageRect);
    }
    else
    {
        DrawTextInRect(hDC, L"튜토리얼 이미지가 없습니다.", imageRect, 30, RGB(220, 220, 230), DT_CENTER | DT_VCENTER | DT_WORDBREAK, FW_BOLD);
    }

    RECT guideCard = { 1190, 175, 1810, 905 };
    DrawRoundCard(hDC, guideCard, RGB(42, 37, 48), RGB(135, 115, 90));

    RECT guideTitle = { 1230, 215, 1770, 270 };
    DrawTextInRect(hDC, L"장애물 피하기", guideTitle, 40, RGB(255, 220, 110), DT_LEFT | DT_VCENTER | DT_SINGLELINE, FW_BOLD);

    DrawGoalLine(hDC, 0, L"공사 표지판과 바닥 장애물은 점프로 넘으세요.");
    DrawGoalLine(hDC, 1, L"공중 장애물은 슬라이딩으로 피하세요.");
    DrawGoalLine(hDC, 2, L"멀리 달릴수록 점수가 올라갑니다.");

    RECT warning = { 1230, 710, 1770, 830 };
    DrawTextInRect(hDC, L"착지 직전 점프도 입력 버퍼로 자연스럽게 이어집니다.", warning, 28, RGB(255, 210, 90), DT_CENTER | DT_VCENTER | DT_WORDBREAK, FW_BOLD);
}

void MiniGame3TutorialScene::DrawBottomButtons(HDC hDC)
{
    if (currentPage > 0)
    {
        DrawButton(hDC, prevButtonRect, L"< 이전", RGB(65, 60, 75), RGB(240, 240, 245));
    }

    std::wstring nextText = (currentPage == 0) ? L"다음 >" : L"시작하기";
    DrawButton(hDC, nextButtonRect, nextText, RGB(255, 205, 80), RGB(35, 28, 20));

    std::wstring pageText = (currentPage == 0) ? L"●  ○" : L"○  ●";
    DrawTextInRect(hDC, pageText, pageDotArea, 30, RGB(255, 220, 110), DT_CENTER | DT_VCENTER | DT_SINGLELINE, FW_BOLD);
}

void MiniGame3TutorialScene::DrawGuideStep(HDC hDC, int index, const std::wstring& text)
{
    RECT stepRect = { 1230, 310 + (index * 120), 1770, 390 + (index * 120) };
    DrawTextInRect(hDC, text, stepRect, 28, RGB(245, 245, 245), DT_LEFT | DT_TOP | DT_WORDBREAK, FW_NORMAL);
}

void MiniGame3TutorialScene::DrawGoalLine(HDC hDC, int index, const std::wstring& text)
{
    RECT lineRect = { 1230, 320 + (index * 120), 1770, 400 + (index * 120) };
    DrawTextInRect(hDC, text, lineRect, 28, RGB(245, 245, 245), DT_LEFT | DT_TOP | DT_WORDBREAK, FW_NORMAL);
}

void MiniGame3TutorialScene::DrawRoundCard(HDC hDC, const RECT& rc, COLORREF fillColor, COLORREF outlineColor)
{
    HBRUSH cardBrush = CreateSolidBrush(fillColor);
    HPEN cardPen = CreatePen(PS_SOLID, 3, outlineColor);

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, cardBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, cardPen));

    RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, 28, 28);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(cardBrush);
    DeleteObject(cardPen);
}

void MiniGame3TutorialScene::DrawButton(HDC hDC, const RECT& rc, const std::wstring& text, COLORREF fillColor, COLORREF textColor)
{
    DrawRoundCard(hDC, rc, fillColor, RGB(240, 220, 150));
    RECT textRect = rc;
    DrawTextInRect(hDC, text, textRect, 28, textColor, DT_CENTER | DT_VCENTER | DT_SINGLELINE, FW_BOLD);
}

void MiniGame3TutorialScene::DrawImageFit(HDC hDC, CImage& image, const RECT& targetRect)
{
    if (image.IsNull())
    {
        return;
    }

    int imageWidth = image.GetWidth();
    int imageHeight = image.GetHeight();
    int targetWidth = targetRect.right - targetRect.left;
    int targetHeight = targetRect.bottom - targetRect.top;

    if (imageWidth <= 0 || imageHeight <= 0 || targetWidth <= 0 || targetHeight <= 0)
    {
        return;
    }

    double scaleX = static_cast<double>(targetWidth) / imageWidth;
    double scaleY = static_cast<double>(targetHeight) / imageHeight;
    double scale = (scaleX < scaleY) ? scaleX : scaleY;

    int drawWidth = static_cast<int>(imageWidth * scale);
    int drawHeight = static_cast<int>(imageHeight * scale);
    int drawX = targetRect.left + (targetWidth - drawWidth) / 2;
    int drawY = targetRect.top + (targetHeight - drawHeight) / 2;

    HBRUSH bgBrush = CreateSolidBrush(RGB(12, 12, 18));
    FillRect(hDC, &targetRect, bgBrush);
    DeleteObject(bgBrush);

    image.Draw(hDC, drawX, drawY, drawWidth, drawHeight);
}

void MiniGame3TutorialScene::DrawTextInRect(HDC hDC, const std::wstring& text, RECT rc, int fontSize, COLORREF color, UINT format, int fontWeight)
{
    HFONT font = CreateFontW(
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
        L"맑은 고딕"
    );

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, font));
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, color);
    ::DrawTextW(hDC, text.c_str(), static_cast<int>(text.length()), &rc, format);
    SelectObject(hDC, oldFont);
    DeleteObject(font);
}

void MiniGame3TutorialScene::GoNext()
{
    if (currentPage == 0)
    {
        currentPage = 1;
        return;
    }

    finished = true;
}

void MiniGame3TutorialScene::GoPrev()
{
    if (currentPage > 0)
    {
        currentPage--;
    }
}
