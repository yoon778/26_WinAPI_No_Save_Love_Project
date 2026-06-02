#include "MiniGame1TutorialScene.h"


    

void MiniGame1TutorialScene::Initialize()
{
    Initialize(1);
}

void MiniGame1TutorialScene::Initialize(int miniGameNumber)
{
    // 혹시 이미 이미지가 로드되어 있다면 먼저 정리한다.
    Shutdown();

    // 현재 미니게임 번호에 맞게 문구와 이미지 경로를 설정한다.
    ConfigureForMiniGame(miniGameNumber);

    // 설명 상태를 초기화한다.
    Reset();

    // 현재 튜토리얼에서 쓰는 이미지를 로드한다.
    LoadTutorialImages();
}

void MiniGame1TutorialScene::Shutdown()
{
    // CImage는 Destroy로 해제한다.
    if (!mainGuideImage.IsNull())
    {
        mainGuideImage.Destroy();
    }

    if (!successImage.IsNull())
    {
        successImage.Destroy();
    }

    if (!failImage.IsNull())
    {
        failImage.Destroy();
    }
}

void MiniGame1TutorialScene::Reset()
{
    currentPage = 0;
    finished = false;
}



void MiniGame1TutorialScene::Render(HDC hDC)
{
    // 공통 배경을 그린다.
    DrawBackground(hDC);

    // 공통 제목을 그린다.
    DrawHeader(hDC);

    // 현재 페이지에 맞는 내용을 그린다.
    if (currentPage == 0)
    {
        DrawPageControlGuide(hDC);
    }
    else if (currentMiniGameNumber == 1)
    {
        DrawPageScoreGuide(hDC);
    }
    else
    {
        DrawPageGoalGuide(hDC);
    }

    // 공통 하단 버튼을 그린다.
    DrawBottomButtons(hDC);
}

void MiniGame1TutorialScene::OnMouseClick(int x, int y)
{
    // 이미 끝난 상태면 입력을 무시한다.
    if (finished)
    {
        return;
    }

    // 오른쪽 위 SKIP 버튼을 누르면 설명을 끝낸다.
    if (IsInsideRect(skipButtonRect, x, y))
    {
        finished = true;
        return;
    }

    // 이전 버튼은 2페이지에서만 동작한다.
    if (currentPage > 0 && IsInsideRect(prevButtonRect, x, y))
    {
        GoPrev();
        return;
    }

    // 다음/시작 버튼
    if (IsInsideRect(nextButtonRect, x, y))
    {
        GoNext();
        return;
    }
}



bool MiniGame1TutorialScene::IsFinished() const
{
    return finished;
}

int MiniGame1TutorialScene::GetCurrentPage() const
{
    return currentPage;
}

bool MiniGame1TutorialScene::IsInsideRect(const RECT& rect, int x, int y) const
{
    return x >= rect.left &&
           x <= rect.right &&
           y >= rect.top &&
           y <= rect.bottom;
}

bool MiniGame1TutorialScene::IsValidImage(const CImage& image) const
{
    return !image.IsNull();
}

void MiniGame1TutorialScene::ConfigureForMiniGame(int miniGameNumber)
{
    currentMiniGameNumber = miniGameNumber;
    if (currentMiniGameNumber < 1 || currentMiniGameNumber > 4)
    {
        currentMiniGameNumber = 1;
    }

    controlGuideLines.clear();
    goalGuideLines.clear();

    switch (currentMiniGameNumber)
    {
    case 2:
        titleText = L"미니게임 2 - 노래방 리듬게임";
        subTitleText = L"박자에 맞춰 노트를 클릭하고 슬라이더를 따라가세요.";
        mainImagePath = L"resource\\minigame2\\minigame2_tuto_image.PNG";
        highlightText = L"정확한 타이밍과 콤보 유지가 핵심입니다!";
        goalTitleText = L"높은 점수 받기";
        controlGuideLines =
        {
            L"1. 원이 겹치는 타이밍에 노트를 클릭합니다.",
            L"2. 슬라이더는 마우스를 누른 채 끝까지 따라갑니다.",
            L"3. 판정이 좋을수록 점수와 콤보가 올라갑니다."
        };
        goalGuideLines =
        {
            L"300 판정을 많이 받을수록 점수가 크게 오릅니다.",
            L"슬라이더는 시작점부터 끝점까지 놓치지 마세요.",
            L"콤보를 이어가면 최종 점수가 더 좋아집니다."
        };
        break;

    case 3:
        titleText = L"미니게임 3 - 등교길";
        subTitleText = L"장애물을 피하며 제한시간 동안 학교까지 달려가세요.";
        mainImagePath = L"resource\\minigame3\\minigame3_tuto_image.PNG";
        highlightText = L"하트 3개 안에서 최대한 멀리 달리세요!";
        goalTitleText = L"장애물 피하기";
        controlGuideLines =
        {
            L"1. SPACE로 점프합니다. 한 번 더 누르면 2단 점프합니다.",
            L"2. 아래 방향키를 누르고 있으면 슬라이딩합니다.",
            L"3. 장애물에 맞으면 하트가 줄어듭니다."
        };
        goalGuideLines =
        {
            L"공사 표지판과 바닥 장애물은 점프로 넘으세요.",
            L"공중 장애물은 슬라이딩으로 피하세요.",
            L"멀리 달릴수록 점수가 올라갑니다."
        };
        break;

    case 4:
        titleText = L"미니게임 4 - 유혹 피하기";
        subTitleText = L"떨어지는 유혹과 경고 영역을 피하며 버티세요.";
        mainImagePath = L"resource\\minigame4\\minigame4_tuto_image.PNG";
        highlightText = L"경고가 보이면 바로 자리를 비우세요!";
        goalTitleText = L"생존 요령";
        controlGuideLines =
        {
            L"1. 왼쪽/오른쪽 방향키로 이동합니다.",
            L"2. 위 방향키 또는 SPACE로 점프합니다.",
            L"3. 빨간 경고 영역과 날아오는 물체를 피합니다."
        };
        goalGuideLines =
        {
            L"하트가 모두 사라지기 전까지 버티세요.",
            L"말파는 위치가 잠깐 고정된 뒤 돌진합니다.",
            L"발판과 점프를 활용해 위험한 위치를 벗어나세요."
        };
        break;

    case 1:
    default:
        titleText = L"미니게임 1 - PC방 라면 맛집";
        subTitleText = L"손님의 주문에 맞게 라면을 만들고 좌석을 클릭해 배달하세요.";
        mainImagePath = L"resource\\minigame1\\미니1가이드.png";
        highlightText = L"제한시간 안에 최대한 많은 주문을 성공시키세요!";
        goalTitleText = L"성공과 실패";
        controlGuideLines =
        {
            L"1. 물과 재료를 클릭해 라면을 조합합니다.",
            L"2. 손님의 말풍선에 있는 주문을 확인합니다.",
            L"3. 주문에 맞는 라면을 만들었다면 좌석을 클릭해 배달합니다."
        };
        break;
    }
}

void MiniGame1TutorialScene::LoadTutorialImages()
{
    if (!mainImagePath.empty())
    {
        mainGuideImage.Load(mainImagePath.c_str());
    }

    if (currentMiniGameNumber == 1)
    {
        successImage.Load(L"resource\\minigame1\\미니1성공.png");
        failImage.Load(L"resource\\minigame1\\미니1실패.png");
    }
}

void MiniGame1TutorialScene::DrawBackground(HDC hDC)
{
    // 전체 배경
    RECT bgRect = { 0, 0, screenWidth, screenHeight };
    HBRUSH bgBrush = CreateSolidBrush(RGB(18, 16, 22));
    FillRect(hDC, &bgRect, bgBrush);
    DeleteObject(bgBrush);

    // 중앙 어두운 패널
    RECT mainPanel = { 70, 120, 1850, 940 };
    DrawRoundCard(hDC, mainPanel, RGB(32, 29, 40), RGB(90, 78, 110));

    // 상단 장식 라인
    HPEN linePen = CreatePen(PS_SOLID, 4, RGB(255, 205, 80));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, linePen));

    MoveToEx(hDC, 100, 145, nullptr);
    LineTo(hDC, 1820, 145);

    SelectObject(hDC, oldPen);
    DeleteObject(linePen);
}

void MiniGame1TutorialScene::DrawHeader(HDC hDC)
{
    // 제목
    RECT titleRect = { 100, 35, 1550, 110 };
    DrawTextInRect(
        hDC,
        titleText,
        titleRect,
        46,
        RGB(255, 220, 110),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    // 부제
    RECT subTitleRect = { 100, 92, 1600, 135 };
    DrawTextInRect(
        hDC,
        subTitleText,
        subTitleRect,
        26,
        RGB(230, 230, 235),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_NORMAL
    );

    // SKIP 버튼
    DrawButton(
        hDC,
        skipButtonRect,
        L"SKIP",
        RGB(65, 60, 75),
        RGB(245, 245, 250)
    );
}

void MiniGame1TutorialScene::DrawPageControlGuide(HDC hDC)
{
    // 왼쪽 이미지 카드
    RECT imageCard = { 110, 175, 1145, 905 };
    DrawRoundCard(hDC, imageCard, RGB(22, 21, 28), RGB(105, 95, 125));

    RECT imageRect = { 135, 205, 1120, 875 };
    if (IsValidImage(mainGuideImage))
    {
        DrawImageFit(hDC, mainGuideImage, imageRect);
    }
    else
    {
        DrawTextInRect(
            hDC,
            L"튜토리얼 이미지가 없습니다.",
            imageRect,
            30,
            RGB(220, 220, 230),
            DT_CENTER | DT_VCENTER | DT_WORDBREAK,
            FW_BOLD
        );
    }

    // 오른쪽 설명 카드
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

    DrawGuideLines(hDC, controlGuideLines, 1230, 310, 1770, 120, 28);

    RECT warning = { 1230, 710, 1770, 830 };
    DrawTextInRect(
        hDC,
        highlightText,
        warning,
        30,
        RGB(255, 210, 90),
        DT_CENTER | DT_VCENTER | DT_WORDBREAK,
        FW_BOLD
    );
}

void MiniGame1TutorialScene::DrawPageScoreGuide(HDC hDC)
{
    // 왼쪽 성공 카드
    RECT successCard = { 130, 190, 920, 895 };
    DrawRoundCard(hDC, successCard, RGB(28, 42, 34), RGB(80, 170, 110));

    RECT successTitle = { 170, 225, 880, 280 };
    DrawTextInRect(
        hDC,
        L"성공!",
        successTitle,
        42,
        RGB(130, 255, 170),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT successImageRect = { 220, 305, 830, 655 };
    if (IsValidImage(successImage))
    {
        DrawImageFit(hDC, successImage, successImageRect);
    }
    else
    {
        DrawTextInRect(
            hDC,
            L"성공 예시 이미지\n경로를 넣어주세요.",
            successImageRect,
            28,
            RGB(220, 240, 225),
            DT_CENTER | DT_VCENTER | DT_WORDBREAK,
            FW_BOLD
        );
    }

    RECT successDesc = { 180, 690, 870, 750 };
    DrawTextInRect(
        hDC,
        L"주문과 같은 라면을 배달하면",
        successDesc,
        30,
        RGB(245, 245, 245),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_NORMAL
    );

    RECT successScore = { 180, 760, 870, 845 };
    DrawTextInRect(
        hDC,
        L"+100점",
        successScore,
        54,
        RGB(120, 255, 150),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    // 오른쪽 실패 카드
    RECT failCard = { 1000, 190, 1790, 895 };
    DrawRoundCard(hDC, failCard, RGB(45, 32, 32), RGB(190, 85, 85));

    RECT failTitle = { 1040, 225, 1750, 280 };
    DrawTextInRect(
        hDC,
        L"실패!",
        failTitle,
        42,
        RGB(255, 140, 140),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT failImageRect = { 1090, 305, 1700, 655 };
    if (IsValidImage(failImage))
    {
        DrawImageFit(hDC, failImage, failImageRect);
    }
    else
    {
        DrawTextInRect(
            hDC,
            L"실패 예시 이미지\n경로를 넣어주세요.",
            failImageRect,
            28,
            RGB(245, 225, 225),
            DT_CENTER | DT_VCENTER | DT_WORDBREAK,
            FW_BOLD
        );
    }

    RECT failDesc = { 1050, 690, 1740, 750 };
    DrawTextInRect(
        hDC,
        L"주문과 다른 라면을 배달하면",
        failDesc,
        30,
        RGB(245, 245, 245),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_NORMAL
    );

    RECT failScore = { 1050, 760, 1740, 845 };
    DrawTextInRect(
        hDC,
        L"-50점",
        failScore,
        54,
        RGB(255, 120, 120),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );
}

void MiniGame1TutorialScene::DrawPageGoalGuide(HDC hDC)
{
    // 왼쪽 이미지 카드
    RECT imageCard = { 110, 175, 1145, 905 };
    DrawRoundCard(hDC, imageCard, RGB(22, 21, 28), RGB(105, 95, 125));

    RECT imageRect = { 135, 205, 1120, 875 };
    if (IsValidImage(mainGuideImage))
    {
        DrawImageFit(hDC, mainGuideImage, imageRect);
    }
    else
    {
        DrawTextInRect(
            hDC,
            L"튜토리얼 이미지가 없습니다.",
            imageRect,
            30,
            RGB(220, 220, 230),
            DT_CENTER | DT_VCENTER | DT_WORDBREAK,
            FW_BOLD
        );
    }

    // 오른쪽 목표 카드
    RECT guideCard = { 1190, 175, 1810, 905 };
    DrawRoundCard(hDC, guideCard, RGB(42, 37, 48), RGB(135, 115, 90));

    RECT guideTitle = { 1230, 215, 1770, 270 };
    DrawTextInRect(
        hDC,
        goalTitleText,
        guideTitle,
        40,
        RGB(255, 220, 110),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    DrawGuideLines(hDC, goalGuideLines, 1230, 320, 1770, 120, 28);

    RECT warning = { 1230, 710, 1770, 830 };
    DrawTextInRect(
        hDC,
        highlightText,
        warning,
        30,
        RGB(255, 210, 90),
        DT_CENTER | DT_VCENTER | DT_WORDBREAK,
        FW_BOLD
    );
}

void MiniGame1TutorialScene::DrawBottomButtons(HDC hDC)
{
    // 이전 버튼은 2페이지에서만 보여준다.
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

    // 마지막 페이지에서는 시작하기로 표시한다.
    std::wstring nextText = (currentPage == 0) ? L"다음 >" : L"시작하기";

    DrawButton(
        hDC,
        nextButtonRect,
        nextText,
        RGB(255, 205, 80),
        RGB(35, 28, 20)
    );

    // 페이지 점 표시
    RECT dotRect = pageDotArea;

    std::wstring pageText = (currentPage == 0) ? L"●  ○" : L"○  ●";

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

void MiniGame1TutorialScene::DrawRoundCard(HDC hDC, const RECT& rc, COLORREF fillColor, COLORREF outlineColor)
{
    // 카드 내부 색상 브러시를 만든다.
    HBRUSH cardBrush = CreateSolidBrush(fillColor);

    // 카드 외곽선 펜을 만든다.
    HPEN cardPen = CreatePen(PS_SOLID, 3, outlineColor);

    // 기존 GDI 객체를 저장한다.
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, cardBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, cardPen));

    // 둥근 사각형을 그린다.
    RoundRect(
        hDC,
        rc.left,
        rc.top,
        rc.right,
        rc.bottom,
        28,
        28
    );

    // 기존 GDI 객체를 복구한다.
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    // 직접 만든 GDI 객체를 삭제한다.
    DeleteObject(cardBrush);
    DeleteObject(cardPen);
}

void MiniGame1TutorialScene::DrawButton(HDC hDC, const RECT& rc, const std::wstring& text, COLORREF fillColor, COLORREF textColor)
{
    // 버튼 배경을 그린다.
    DrawRoundCard(hDC, rc, fillColor, RGB(240, 220, 150));

    // 버튼 글자를 그린다.
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

void MiniGame1TutorialScene::DrawImageFit(HDC hDC, CImage& image, const RECT& targetRect)
{
    // 이미지가 없으면 그리지 않는다.
    if (image.IsNull())
    {
        return;
    }

    // 원본 이미지 크기를 가져온다.
    int imageWidth = image.GetWidth();
    int imageHeight = image.GetHeight();

    // 목표 영역 크기를 가져온다.
    int targetWidth = targetRect.right - targetRect.left;
    int targetHeight = targetRect.bottom - targetRect.top;

    if (imageWidth <= 0 || imageHeight <= 0 || targetWidth <= 0 || targetHeight <= 0)
    {
        return;
    }

    // 가로/세로 비율을 유지하기 위해 배율을 계산한다.
    double scaleX = static_cast<double>(targetWidth) / imageWidth;
    double scaleY = static_cast<double>(targetHeight) / imageHeight;
    double scale = (scaleX < scaleY) ? scaleX : scaleY;

    // 실제 그릴 크기를 계산한다.
    int drawWidth = static_cast<int>(imageWidth * scale);
    int drawHeight = static_cast<int>(imageHeight * scale);

    // 중앙 정렬 좌표를 계산한다.
    int drawX = targetRect.left + (targetWidth - drawWidth) / 2;
    int drawY = targetRect.top + (targetHeight - drawHeight) / 2;

    // 이미지 외곽에 어두운 배경을 깔아 비율 차이로 남는 공간이 어색하지 않게 한다.
    HBRUSH bgBrush = CreateSolidBrush(RGB(12, 12, 18));
    FillRect(hDC, &targetRect, bgBrush);
    DeleteObject(bgBrush);

    // 이미지를 비율 유지해서 출력한다.
    image.Draw(hDC, drawX, drawY, drawWidth, drawHeight);
}

void MiniGame1TutorialScene::DrawTextInRect(
    HDC hDC,
    const std::wstring& text,
    RECT rc,
    int fontSize,
    COLORREF color,
    UINT format,
    int fontWeight)
{
    // 텍스트용 폰트를 만든다.
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

    // 기존 폰트를 저장하고 새 폰트를 선택한다.
    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, font));

    // 배경을 투명하게 만든다.
    SetBkMode(hDC, TRANSPARENT);

    // 글자 색상을 설정한다.
    SetTextColor(hDC, color);

    // 텍스트를 출력한다.
    ::DrawTextW(
        hDC,
        text.c_str(),
        static_cast<int>(text.length()),
        &rc,
        format
    );

    // 기존 폰트를 복구하고 직접 만든 폰트를 삭제한다.
    SelectObject(hDC, oldFont);
    DeleteObject(font);
}

void MiniGame1TutorialScene::DrawGuideLines(HDC hDC, const std::vector<std::wstring>& lines, int left, int top, int right, int lineHeight, int fontSize)
{
    for (size_t i = 0; i < lines.size(); ++i)
    {
        RECT lineRect =
        {
            left,
            top + static_cast<int>(i) * lineHeight,
            right,
            top + static_cast<int>(i + 1) * lineHeight
        };

        DrawTextInRect(
            hDC,
            lines[i],
            lineRect,
            fontSize,
            RGB(245, 245, 245),
            DT_LEFT | DT_TOP | DT_WORDBREAK,
            FW_NORMAL
        );
    }
}

void MiniGame1TutorialScene::GoNext()
{
    // 1페이지라면 2페이지로 이동한다.
    if (currentPage == 0)
    {
        currentPage = 1;
        return;
    }

    // 마지막 페이지에서 다음을 누르면 설명 화면을 끝낸다.
    finished = true;
}

void MiniGame1TutorialScene::GoPrev()
{
    // 2페이지에서만 이전 페이지로 이동한다.
    if (currentPage > 0)
    {
        currentPage--;
    }
}
