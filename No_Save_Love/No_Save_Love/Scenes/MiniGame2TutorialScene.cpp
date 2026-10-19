#include "MiniGame2TutorialScene.h"

#include <cmath>

void MiniGame2TutorialScene::Initialize(bool flowering)
{
    Shutdown();
    Reset();
    floweringMode = flowering;

    backgroundImage.Load(L"resource\\minigame2\\background\\bg1.png");
    hitCircleImage.Load(L"resource\\minigame2\\hitcircle\\hitcircle.png");
    hitCircleOverlayImage.Load(L"resource\\minigame2\\hitcircle\\hitcircleoverlay.png");
    approachCircleImage.Load(L"resource\\minigame2\\hitcircle\\approachcircle.png");
    sliderStartCircleImage.Load(L"resource\\minigame2\\slider\\sliderstartcircle.png");
    sliderStartCircleOverlayImage.Load(L"resource\\minigame2\\slider\\sliderstartcircleoverlay.png");
    sliderBallImage.Load(L"resource\\minigame2\\slider\\sliderb.png");
    sliderFollowCircleImage.Load(L"resource\\minigame2\\slider\\sliderfollowcircle.png");
    hit300Image.Load(L"resource\\minigame2\\judge\\hit300.png");
    hit100Image.Load(L"resource\\minigame2\\judge\\hit100.png");
    hit50Image.Load(L"resource\\minigame2\\judge\\hit50.png");
    hit0Image.Load(L"resource\\minigame2\\judge\\hit0.png");
    missReactionImage.Load(L"resource\\minigame2\\miss\\frame1.png");

    if (floweringMode)
    {
        const wchar_t* names[] = {L"wind", L"memory", L"relay", L"encore", L"return", L"spinner"};
        for (int i = 0; i < 6; ++i)
        {
            const std::wstring path = std::wstring(L"resource\\minigame5\\tutorial\\") + names[i] + L".png";
            floweringExampleImages[i].Load(path.c_str());
            PremultiplyAlpha(floweringExampleImages[i]);
        }
    }

    PremultiplyAlpha(hitCircleImage);
    PremultiplyAlpha(hitCircleOverlayImage);
    PremultiplyAlpha(approachCircleImage);
    PremultiplyAlpha(sliderStartCircleImage);
    PremultiplyAlpha(sliderStartCircleOverlayImage);
    PremultiplyAlpha(sliderBallImage);
    PremultiplyAlpha(sliderFollowCircleImage);
    PremultiplyAlpha(hit300Image);
    PremultiplyAlpha(hit100Image);
    PremultiplyAlpha(hit50Image);
    PremultiplyAlpha(hit0Image);
    PremultiplyAlpha(missReactionImage);
}

void MiniGame2TutorialScene::Shutdown()
{
    DestroyImage(backgroundImage);
    DestroyImage(hitCircleImage);
    DestroyImage(hitCircleOverlayImage);
    DestroyImage(approachCircleImage);
    DestroyImage(sliderStartCircleImage);
    DestroyImage(sliderStartCircleOverlayImage);
    DestroyImage(sliderBallImage);
    DestroyImage(sliderFollowCircleImage);
    DestroyImage(hit300Image);
    DestroyImage(hit100Image);
    DestroyImage(hit50Image);
    DestroyImage(hit0Image);
    DestroyImage(missReactionImage);
    for (CImage& image : floweringExampleImages) DestroyImage(image);
}

void MiniGame2TutorialScene::Reset()
{
    currentPage = 0;
    finished = false;
}

void MiniGame2TutorialScene::Render(HDC hDC)
{
    DrawBackground(hDC);
    DrawHeader(hDC);

    if (currentPage == 0)
    {
        DrawPageControlGuide(hDC);
    }
    else
    {
        if (floweringMode) DrawFloweringGuide(hDC);
        else DrawPageScoreGuide(hDC);
    }

    DrawBottomButtons(hDC);
}

void MiniGame2TutorialScene::OnMouseClick(int x, int y)
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

bool MiniGame2TutorialScene::IsFinished() const
{
    return finished;
}

int MiniGame2TutorialScene::GetCurrentPage() const
{
    return currentPage;
}

int MiniGame2TutorialScene::GetPageCount() const
{
    return floweringMode ? 4 : 2;
}

bool MiniGame2TutorialScene::IsInsideRect(const RECT& rect, int x, int y) const
{
    return x >= rect.left &&
           x <= rect.right &&
           y >= rect.top &&
           y <= rect.bottom;
}

bool MiniGame2TutorialScene::IsValidImage(const CImage& image) const
{
    return !image.IsNull();
}

void MiniGame2TutorialScene::DrawBackground(HDC hDC)
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

void MiniGame2TutorialScene::DrawHeader(HDC hDC)
{
    RECT titleRect = { 100, 35, 1550, 110 };
    DrawTextInRect(
        hDC,
        floweringMode ? L"미니게임 6 - 개화 : 금요일의 작은 무대" : L"미니게임 2 - 노래 연습",
        titleRect,
        46,
        RGB(255, 220, 110),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT subTitleRect = floweringMode ? RECT{100, 91, 1650, 122} : RECT{100, 92, 1650, 135};
    DrawTextInRect(
        hDC,
        floweringMode && currentPage > 0
            ? L"사진 속 신호를 확인하고, 각 카드 아래의 조작을 따라 해 보세요."
            : L"줄어드는 접근 원에 맞춰 노트를 클릭하고, 슬라이더를 끝까지 따라가세요.",
        subTitleRect,
        26,
        RGB(230, 230, 235),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_NORMAL
    );

    DrawButton(
        hDC,
        skipButtonRect,
        floweringMode ? L"바로 시작" : L"SKIP",
        RGB(65, 60, 75),
        RGB(245, 245, 250)
    );
}

void MiniGame2TutorialScene::DrawPageControlGuide(HDC hDC)
{
    RECT imageCard = { 110, 175, 1145, 905 };
    DrawRoundCard(hDC, imageCard, RGB(22, 21, 28), RGB(105, 95, 125));

    RECT previewRect = { 135, 205, 1120, 875 };
    DrawGameplayPreview(hDC, previewRect);

    RECT guideCard = { 1190, 175, 1810, 905 };
    DrawRoundCard(hDC, guideCard, RGB(42, 37, 48), RGB(135, 115, 90));

    RECT guideTitle = { 1230, 215, 1770, 270 };
    DrawTextInRect(
        hDC,
        L"조작 방법",
        guideTitle,
        40,
        RGB(255, 220, 110),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT step1 = { 1230, 310, 1770, 390 };
    DrawTextInRect(
        hDC,
        L"1. 접근 원이 노트와 겹치는 순간 클릭합니다.",
        step1,
        28,
        RGB(245, 245, 245),
        DT_LEFT | DT_TOP | DT_WORDBREAK,
        FW_NORMAL
    );

    RECT step2 = { 1230, 430, 1770, 520 };
    DrawTextInRect(
        hDC,
        L"2. 슬라이더는 시작 원을 누른 채 움직이는 공을 따라갑니다.",
        step2,
        28,
        RGB(245, 245, 245),
        DT_LEFT | DT_TOP | DT_WORDBREAK,
        FW_NORMAL
    );

    RECT step3 = { 1230, 565, 1770, 655 };
    DrawTextInRect(
        hDC,
        floweringMode ? L"3. 꽃과 히로인의 신호를 보며, 새로운 박자에 차근차근 응답하세요." : L"3. 창이 작아지거나 움직여도 리듬을 놓치지 마세요.",
        step3,
        28,
        RGB(245, 245, 245),
        DT_LEFT | DT_TOP | DT_WORDBREAK,
        FW_NORMAL
    );

    RECT warning = { 1230, 720, 1770, 830 };
    DrawTextInRect(
        hDC,
        L"정확한 타이밍과 콤보로 높은 점수를 노리세요!",
        warning,
        30,
        RGB(255, 210, 90),
        DT_CENTER | DT_VCENTER | DT_WORDBREAK,
        FW_BOLD
    );
}

void MiniGame2TutorialScene::DrawPageScoreGuide(HDC hDC)
{
    RECT scoreCard = { 130, 190, 920, 895 };
    DrawRoundCard(hDC, scoreCard, RGB(28, 42, 34), RGB(80, 170, 110));

    RECT scoreTitle = { 170, 225, 880, 280 };
    DrawTextInRect(
        hDC,
        L"판정 점수",
        scoreTitle,
        42,
        RGB(130, 255, 170),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT scorePanel = { 220, 305, 830, 650 };
    DrawRoundCard(hDC, scorePanel, RGB(18, 28, 30), RGB(70, 135, 105));

    RECT perfectRow = { 245, 330, 805, 415 };
    DrawScoreRow(hDC, perfectRow, hit300Image, L"PERFECT", L"+300점", RGB(100, 225, 255));

    RECT goodRow = { 245, 435, 805, 520 };
    DrawScoreRow(hDC, goodRow, hit100Image, L"GOOD", L"+100점", RGB(125, 245, 170));

    RECT badRow = { 245, 540, 805, 625 };
    DrawScoreRow(hDC, badRow, hit50Image, L"BAD", L"+50점", RGB(255, 205, 100));

    RECT scoreDesc = { 180, 690, 870, 750 };
    DrawTextInRect(
        hDC,
        L"타이밍이 정확할수록 높은 점수를 얻습니다.",
        scoreDesc,
        29,
        RGB(245, 245, 245),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_NORMAL
    );

    RECT scoreValue = { 180, 760, 870, 845 };
    DrawTextInRect(
        hDC,
        L"최대 +300점",
        scoreValue,
        48,
        RGB(120, 255, 150),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT missCard = { 1000, 190, 1790, 895 };
    DrawRoundCard(hDC, missCard, RGB(45, 32, 32), RGB(190, 85, 85));

    RECT missTitle = { 1040, 225, 1750, 280 };
    DrawTextInRect(
        hDC,
        L"MISS!",
        missTitle,
        42,
        RGB(255, 140, 140),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT missPreview = { 1090, 305, 1700, 655 };
    DrawMissPreview(hDC, missPreview);

    RECT missDesc = { 1050, 690, 1740, 750 };
    DrawTextInRect(
        hDC,
        L"노트를 놓치거나 슬라이더에서 벗어나면",
        missDesc,
        29,
        RGB(245, 245, 245),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_NORMAL
    );

    RECT missValue = { 1050, 760, 1740, 845 };
    DrawTextInRect(
        hDC,
        L"콤보 초기화",
        missValue,
        48,
        RGB(255, 120, 120),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );
}

void MiniGame2TutorialScene::DrawFloweringGuide(HDC hDC)
{
    const wchar_t* titles[] = {L"01  바람 슬라이더", L"02  기억 꽃", L"03  히로인 릴레이",
        L"04  앵콜 노트", L"05  왕복 슬라이더", L"06  스피너"};
    const wchar_t* actions[] = {L"누른 채 꽃길 따라가기", L"사라진 꽃의 위치 기억하기", L"본 순서대로 누르기",
        L"같은 자리 다시 누르기", L"돌아오는 길까지 따라가기", L"중심 주위로 원 그리기"};
    const wchar_t* descriptions[] = {
        L"시작 원을 누른 채 움직이는 공을 따라가세요.\n휘어진 길에서도 끝까지 버튼을 놓지 않으면 성공!",
        L"보라색 꽃의 위치와 박자를 기억하세요.\n잠깐 숨었던 꽃이 클릭할 순간 다시 나타나요.\n그때 원래 자리를 눌러 주세요.",
        L"빛나는 위치의 순서를 먼저 보세요.\n모든 원이 켜지면 예고된 순서대로 하나씩 눌러요.",
        L"반복 화살표의 원을 같은 자리에서 두 번 눌러요.\n다음 접근 원이 겹치는 순간 다시 클릭하세요.",
        L"공을 누른 채 반전 화살표까지 따라가세요.\n끝에서 잠깐 멈춘 뒤 천천히 돌아옵니다.\n버튼을 놓지 말고 돌아오는 공도 계속 따라가요.",
        L"버튼을 누른 채 중심 주위로 원을 그리세요.\n편한 방향으로 회전하며 게이지를 끝까지 채우면 성공!"
    };
    const wchar_t* pageTitles[] = {L"사진으로 익히기  ·  꽃길과 기억", L"사진으로 익히기  ·  순서와 반복", L"사진으로 익히기  ·  왕복과 회전"};
    const int group = currentPage - 1;
    if (group < 0 || group >= 3) return;
    DrawTextInRect(hDC, pageTitles[group], {130, 162, 1790, 205}, 31,
        RGB(181, 228, 214), DT_LEFT | DT_VCENTER | DT_SINGLELINE, FW_BOLD);

    for (int column = 0; column < 2; ++column)
    {
        const int i = group * 2 + column;
        const int left = 130 + column * 855;
        RECT card = {left, 220, left + 805, 900};
        DrawRoundCard(hDC, card, RGB(24, 49, 54), RGB(94, 157, 143));
        DrawTextInRect(hDC, titles[i], {left + 28, 236, left + 777, 288}, 36,
            RGB(255, 207, 221), DT_LEFT | DT_VCENTER | DT_SINGLELINE, FW_BOLD);

        // 실제 게임에서 캡처한 900x500 예시의 비율을 유지한다.
        const RECT imageRect = {left + 24, 304, left + 780, 724};
        DrawRoundCard(hDC, imageRect, RGB(16, 28, 34), RGB(74, 109, 112));
        if (IsValidImage(floweringExampleImages[i]))
        {
            const int oldStretchMode = SetStretchBltMode(hDC, HALFTONE);
            POINT oldOrigin{}; SetBrushOrgEx(hDC, 0, 0, &oldOrigin);
            DrawImageFit(hDC, floweringExampleImages[i], imageRect);
            SetBrushOrgEx(hDC, oldOrigin.x, oldOrigin.y, nullptr);
            SetStretchBltMode(hDC, oldStretchMode);
        }
        else
        {
            DrawTextInRect(hDC, actions[i], {left + 64, 420, left + 740, 610}, 42,
                RGB(181, 228, 214), DT_CENTER | DT_VCENTER | DT_WORDBREAK, FW_BOLD);
        }
        DrawTextInRect(hDC, descriptions[i], {left + 28, 752, left + 777, 867}, 29,
            RGB(235, 244, 239), DT_LEFT | DT_WORDBREAK);
    }
}

void MiniGame2TutorialScene::DrawBottomButtons(HDC hDC)
{
    if (currentPage > 0)
    {
        DrawButton(
            hDC,
            prevButtonRect,
            L"< 이전",
            RGB(65, 60, 75),
            RGB(240, 240, 245)
        );
    }

    std::wstring nextText = currentPage + 1 < GetPageCount() ? L"다음 >" : L"시작하기";

    DrawButton(
        hDC,
        nextButtonRect,
        nextText,
        RGB(255, 205, 80),
        RGB(35, 28, 20)
    );

    RECT dotRect = pageDotArea;
    std::wstring pageText;
    for (int page = 0; page < GetPageCount(); ++page)
    {
        if (page > 0) pageText += L"  ";
        pageText += page == currentPage ? L"●" : L"○";
    }

    DrawTextInRect(
        hDC,
        pageText,
        dotRect,
        30,
        RGB(255, 220, 110),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );
}

void MiniGame2TutorialScene::DrawGameplayPreview(HDC hDC, const RECT& rc)
{
    HBRUSH bgBrush = CreateSolidBrush(RGB(12, 12, 18));
    FillRect(hDC, &rc, bgBrush);
    DeleteObject(bgBrush);

    if (IsValidImage(backgroundImage))
    {
        backgroundImage.Draw(
            hDC,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top
        );
    }

    RECT scoreTextRect = { rc.left + 30, rc.top + 20, rc.left + 300, rc.top + 65 };
    DrawTextInRect(
        hDC,
        L"Score : 12300",
        scoreTextRect,
        28,
        RGB(255, 255, 255),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT comboTextRect = { rc.left + 30, rc.top + 62, rc.left + 300, rc.top + 107 };
    DrawTextInRect(
        hDC,
        L"24 COMBO",
        comboTextRect,
        27,
        RGB(255, 235, 120),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    const int hitCenterX = rc.left + 245;
    const int hitCenterY = rc.top + 275;
    DrawHitCircle(hDC, hitCenterX, hitCenterY, 230);

    const int sliderStartX = rc.left + 580;
    const int sliderStartY = rc.top + 385;
    const int sliderEndX = rc.left + 835;
    const int sliderEndY = rc.top + 225;
    DrawSlider(hDC, sliderStartX, sliderStartY, sliderEndX, sliderEndY);

    RECT hitLabel = { rc.left + 35, rc.bottom - 105, rc.left + 390, rc.bottom - 55 };
    DrawPreviewLabel(hDC, hitLabel, L"접근 원이 겹칠 때 클릭");
    DrawArrow(hDC, rc.left + 220, rc.bottom - 110, hitCenterX, hitCenterY + 80, RGB(255, 225, 80));

    RECT sliderLabel = { rc.left + 500, rc.bottom - 105, rc.right - 35, rc.bottom - 55 };
    DrawPreviewLabel(hDC, sliderLabel, L"누른 채 슬라이더 볼 따라가기");
    DrawArrow(hDC, rc.left + 715, rc.bottom - 110, rc.left + 710, rc.top + 315, RGB(255, 225, 80));
}

void MiniGame2TutorialScene::DrawMissPreview(HDC hDC, const RECT& rc)
{
    HBRUSH bgBrush = CreateSolidBrush(RGB(12, 12, 18));
    FillRect(hDC, &rc, bgBrush);
    DeleteObject(bgBrush);

    if (IsValidImage(backgroundImage))
    {
        backgroundImage.Draw(
            hDC,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top
        );
    }

    RECT comboTextRect = { rc.left + 25, rc.top + 20, rc.left + 250, rc.top + 60 };
    DrawTextInRect(
        hDC,
        L"COMBO BREAK!",
        comboTextRect,
        25,
        RGB(255, 130, 130),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    DrawHitCircle(hDC, rc.left + 305, rc.top + 165, 245);
    DrawImageCentered(hDC, hit0Image, rc.left + 305, rc.top + 165, 80, 145);

    if (IsValidImage(missReactionImage))
    {
        RECT reactionRect = { rc.left + 80, rc.top + 70, rc.right - 55, rc.bottom - 5 };
        DrawImageFit(hDC, missReactionImage, reactionRect);
    }
}

void MiniGame2TutorialScene::DrawHitCircle(HDC hDC, int centerX, int centerY, int approachSize)
{
    DrawImageCentered(hDC, approachCircleImage, centerX, centerY, approachSize, approachSize);
    DrawImageCentered(hDC, hitCircleImage, centerX, centerY, 128, 128);
    DrawImageCentered(hDC, hitCircleOverlayImage, centerX, centerY, 128, 128);
}

void MiniGame2TutorialScene::DrawSlider(HDC hDC, int startX, int startY, int endX, int endY)
{
    HPEN sliderOutlinePen = CreatePen(PS_SOLID, 74, RGB(255, 255, 255));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, sliderOutlinePen));

    MoveToEx(hDC, startX, startY, nullptr);
    LineTo(hDC, endX, endY);

    SelectObject(hDC, oldPen);
    DeleteObject(sliderOutlinePen);

    HPEN sliderInnerPen = CreatePen(PS_SOLID, 58, RGB(180, 180, 180));
    oldPen = static_cast<HPEN>(SelectObject(hDC, sliderInnerPen));

    MoveToEx(hDC, startX, startY, nullptr);
    LineTo(hDC, endX, endY);

    SelectObject(hDC, oldPen);
    DeleteObject(sliderInnerPen);

    DrawImageCentered(hDC, sliderStartCircleImage, startX, startY, 145, 145);
    DrawImageCentered(hDC, sliderStartCircleOverlayImage, startX, startY, 128, 128);
    DrawImageCentered(hDC, sliderStartCircleImage, endX, endY, 145, 145);
    DrawImageCentered(hDC, sliderStartCircleOverlayImage, endX, endY, 128, 128);

    const int ballX = startX + (endX - startX) * 55 / 100;
    const int ballY = startY + (endY - startY) * 55 / 100;

    DrawImageCentered(hDC, sliderFollowCircleImage, ballX, ballY, 190, 190);
    DrawImageCentered(hDC, sliderBallImage, ballX, ballY, 130, 130);
}

void MiniGame2TutorialScene::DrawScoreRow(
    HDC hDC,
    const RECT& rc,
    CImage& judgeImage,
    const std::wstring& label,
    const std::wstring& scoreText,
    COLORREF accentColor)
{
    DrawRoundCard(hDC, rc, RGB(27, 38, 42), RGB(65, 105, 95));

    DrawImageCentered(
        hDC,
        judgeImage,
        rc.left + 95,
        (rc.top + rc.bottom) / 2,
        120,
        72
    );

    RECT labelRect = { rc.left + 175, rc.top, rc.left + 390, rc.bottom };
    DrawTextInRect(
        hDC,
        label,
        labelRect,
        27,
        accentColor,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT scoreRect = { rc.right - 180, rc.top, rc.right - 25, rc.bottom };
    DrawTextInRect(
        hDC,
        scoreText,
        scoreRect,
        29,
        accentColor,
        DT_RIGHT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );
}

void MiniGame2TutorialScene::DrawPreviewLabel(HDC hDC, const RECT& rc, const std::wstring& text)
{
    DrawRoundCard(hDC, rc, RGB(255, 230, 70), RGB(255, 240, 120));

    RECT textRect = rc;
    DrawTextInRect(
        hDC,
        text,
        textRect,
        24,
        RGB(35, 28, 20),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );
}

void MiniGame2TutorialScene::DrawArrow(HDC hDC, int startX, int startY, int endX, int endY, COLORREF color)
{
    HPEN arrowPen = CreatePen(PS_SOLID, 7, color);
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, arrowPen));

    MoveToEx(hDC, startX, startY, nullptr);
    LineTo(hDC, endX, endY);

    const double angle = std::atan2(
        static_cast<double>(endY - startY),
        static_cast<double>(endX - startX)
    );
    const double arrowAngle = 0.58;
    const int arrowLength = 24;

    POINT points[3] = {
        { endX, endY },
        {
            endX - static_cast<int>(std::cos(angle - arrowAngle) * arrowLength),
            endY - static_cast<int>(std::sin(angle - arrowAngle) * arrowLength)
        },
        {
            endX - static_cast<int>(std::cos(angle + arrowAngle) * arrowLength),
            endY - static_cast<int>(std::sin(angle + arrowAngle) * arrowLength)
        }
    };

    HBRUSH arrowBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, arrowBrush));

    Polygon(hDC, points, 3);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(arrowBrush);
    DeleteObject(arrowPen);
}

void MiniGame2TutorialScene::DrawRoundCard(HDC hDC, const RECT& rc, COLORREF fillColor, COLORREF outlineColor)
{
    HBRUSH cardBrush = CreateSolidBrush(fillColor);
    HPEN cardPen = CreatePen(PS_SOLID, 3, outlineColor);

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, cardBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, cardPen));

    RoundRect(
        hDC,
        rc.left,
        rc.top,
        rc.right,
        rc.bottom,
        28,
        28
    );

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(cardBrush);
    DeleteObject(cardPen);
}

void MiniGame2TutorialScene::DrawButton(HDC hDC, const RECT& rc, const std::wstring& text, COLORREF fillColor, COLORREF textColor)
{
    DrawRoundCard(hDC, rc, fillColor, RGB(240, 220, 150));

    RECT textRect = rc;
    DrawTextInRect(
        hDC,
        text,
        textRect,
        28,
        textColor,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );
}

void MiniGame2TutorialScene::DrawImageFit(HDC hDC, CImage& image, const RECT& targetRect)
{
    if (image.IsNull())
    {
        return;
    }

    const int imageWidth = image.GetWidth();
    const int imageHeight = image.GetHeight();
    const int targetWidth = targetRect.right - targetRect.left;
    const int targetHeight = targetRect.bottom - targetRect.top;

    if (imageWidth <= 0 || imageHeight <= 0 || targetWidth <= 0 || targetHeight <= 0)
    {
        return;
    }

    const double scaleX = static_cast<double>(targetWidth) / imageWidth;
    const double scaleY = static_cast<double>(targetHeight) / imageHeight;
    const double scale = (scaleX < scaleY) ? scaleX : scaleY;

    const int drawWidth = static_cast<int>(imageWidth * scale);
    const int drawHeight = static_cast<int>(imageHeight * scale);
    const int drawX = targetRect.left + (targetWidth - drawWidth) / 2;
    const int drawY = targetRect.top + (targetHeight - drawHeight) / 2;

    image.Draw(hDC, drawX, drawY, drawWidth, drawHeight);
}

void MiniGame2TutorialScene::DrawImageCentered(HDC hDC, CImage& image, int centerX, int centerY, int maxWidth, int maxHeight)
{
    if (image.IsNull())
    {
        return;
    }

    const int imageWidth = image.GetWidth();
    const int imageHeight = image.GetHeight();

    if (imageWidth <= 0 || imageHeight <= 0 || maxWidth <= 0 || maxHeight <= 0)
    {
        return;
    }

    const double scaleX = static_cast<double>(maxWidth) / imageWidth;
    const double scaleY = static_cast<double>(maxHeight) / imageHeight;
    const double scale = (scaleX < scaleY) ? scaleX : scaleY;

    const int drawWidth = static_cast<int>(imageWidth * scale);
    const int drawHeight = static_cast<int>(imageHeight * scale);

    image.Draw(
        hDC,
        centerX - drawWidth / 2,
        centerY - drawHeight / 2,
        drawWidth,
        drawHeight
    );
}

void MiniGame2TutorialScene::DrawTextInRect(
    HDC hDC,
    const std::wstring& text,
    RECT rc,
    int fontSize,
    COLORREF color,
    UINT format,
    int fontWeight)
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

    ::DrawTextW(
        hDC,
        text.c_str(),
        static_cast<int>(text.length()),
        &rc,
        format
    );

    SelectObject(hDC, oldFont);
    DeleteObject(font);
}

void MiniGame2TutorialScene::PremultiplyAlpha(CImage& image)
{
    if (image.IsNull() || image.GetBPP() != 32)
    {
        return;
    }

    BYTE* bits = static_cast<BYTE*>(image.GetBits());
    const int pitch = image.GetPitch();
    const int width = image.GetWidth();
    const int height = image.GetHeight();

    for (int y = 0; y < height; y++)
    {
        BYTE* pixel = bits + y * pitch;

        for (int x = 0; x < width; x++)
        {
            BYTE& blue = pixel[x * 4 + 0];
            BYTE& green = pixel[x * 4 + 1];
            BYTE& red = pixel[x * 4 + 2];
            BYTE& alpha = pixel[x * 4 + 3];

            blue = static_cast<BYTE>((blue * alpha) / 255);
            green = static_cast<BYTE>((green * alpha) / 255);
            red = static_cast<BYTE>((red * alpha) / 255);
        }
    }
}

void MiniGame2TutorialScene::DestroyImage(CImage& image)
{
    if (!image.IsNull())
    {
        image.Destroy();
    }
}

void MiniGame2TutorialScene::GoNext()
{
    if (currentPage + 1 < GetPageCount())
    {
        ++currentPage;
        return;
    }

    finished = true;
}

void MiniGame2TutorialScene::GoPrev()
{
    if (currentPage > 0)
    {
        currentPage--;
    }
}
