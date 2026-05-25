#include "ChoiceScene.h"

void ChoiceScene::Initialize()
{
    // 선택지 화면 배경은 처음 초기화할 때 한 번만 로드한다.
    if (!choice_back_ground.IsNull())
    {
        choice_back_ground.Destroy();
    }
    choice_back_ground.Load(L"resource\\background\\choice.png");

    // 선택지 카드에 사용할 히로인 얼굴 이미지를 미리 로드한다.
    // Render 안에서는 Load를 호출하지 않고, 여기서 준비한 이미지만 Draw한다.
    for (int i = 0; i < HEROINE_COUNT; i++)
    {
        if (!heroineFaceImages[i].IsNull())
        {
            heroineFaceImages[i].Destroy();
        }
    }

    heroineFaceImages[0].Load(L"resource\\heroine\\hansea\\normal.png");
    heroineFaceImages[1].Load(L"resource\\heroine\\yuharin\\normal.png");
    heroineFaceImages[2].Load(L"resource\\heroine\\seoirin\\normal.png");

    Reset();
}

void ChoiceScene::Shutdown()
{
    // 배경 이미지 리소스를 해제한다.
    if (!choice_back_ground.IsNull())
    {
        choice_back_ground.Destroy();
    }

    // 히로인 얼굴 이미지 리소스를 해제한다.
    for (int i = 0; i < HEROINE_COUNT; i++)
    {
        if (!heroineFaceImages[i].IsNull())
        {
            heroineFaceImages[i].Destroy();
        }
    }
}

void ChoiceScene::Reset()
{
    // 선택지 화면에 다시 들어올 때 이전 선택 기록을 지운다.
    m_hasSelected = false;
    selectedCharacter = -1;
}

void ChoiceScene::SetPlayerState(const Player_state& playerState)
{
    // ChoiceScene은 스탯을 계산하지 않고, GameManager가 넘긴 현재 값만 화면에 표시한다.
    currentPlayerState = playerState;
}

void ChoiceScene::HandleChoiceClick(int x, int y)
{
    // 마우스 좌표를 POINT 구조체로 만든다.
    POINT mousePoint = { x, y };

    // 선택지 3개 중 어느 것을 클릭했는지 검사한다.
    // 기존 로직처럼 choiceHitBox 배열을 순회하며 선택 번호만 저장한다.
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
    // 배경 이미지를 화면 전체에 출력한다.
    if (!choice_back_ground.IsNull())
    {
        choice_back_ground.Draw(hDC, 0, 0, 1920, 1080);
    }
    else
    {
        HBRUSH fallbackBrush = CreateSolidBrush(RGB(238, 232, 240));
        RECT fallbackRect = { 0, 0, 1920, 1080 };
        FillRect(hDC, &fallbackRect, fallbackBrush);
        DeleteObject(fallbackBrush);
    }

    // 모든 텍스트는 배경을 투명하게 출력한다.
    SetBkMode(hDC, TRANSPARENT);

    // 첨부 이미지처럼 배경 위를 살짝 어둡게 덮어 카드와 스탯 패널을 읽기 쉽게 만든다.
    HBRUSH overlayBrush = CreateSolidBrush(RGB(0, 0, 0));
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 90;

    HDC overlayDC = CreateCompatibleDC(hDC);
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
    HBITMAP oldOverlayBitmap = static_cast<HBITMAP>(SelectObject(overlayDC, overlayBitmap));
    RECT overlayRect = { 0, 0, 1920, 1080 };
    FillRect(overlayDC, &overlayRect, overlayBrush);
    AlphaBlend(hDC, 0, 0, 1920, 1080, overlayDC, 0, 0, 1920, 1080, blend);
    SelectObject(overlayDC, oldOverlayBitmap);
    DeleteObject(overlayBitmap);
    DeleteObject(overlayBrush);
    DeleteDC(overlayDC);

    // 왼쪽에는 현재 플레이어 스탯 패널을 출력한다.
    DrawStatPanel(hDC);

    // 오른쪽에는 히로인 선택 카드 3개를 세로로 출력한다.
    for (int i = 0; i < HEROINE_COUNT; i++)
    {
        DrawChoiceCard(hDC, i);
    }
}

void ChoiceScene::DrawStatPanel(HDC hDC)
{
    // 첨부 이미지 느낌에 맞춰 왼쪽 전체를 어두운 STATUS 사이드바로 만든다.
    HBRUSH panelBrush = CreateSolidBrush(RGB(30, 38, 92));
    HPEN panelPen = CreatePen(PS_SOLID, 1, RGB(50, 65, 125));

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, panelBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, panelPen));

    Rectangle(
        hDC,
        statPanelRect.left,
        statPanelRect.top,
        statPanelRect.right,
        statPanelRect.bottom
    );

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    DeleteObject(panelBrush);
    DeleteObject(panelPen);

    // 상단에는 현재 화면 역할을 알려주는 STATUS 제목을 출력한다.
    RECT titleRect = { 48, 72, 260, 120 };
    DrawTextInRect(
        hDC,
        L"STATUS",
        titleRect,
        28,
        RGB(245, 245, 255),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    // 제목 왼쪽의 핑크색 포인트 바다.
    HBRUSH accentBrush = CreateSolidBrush(RGB(255, 65, 145));
    RECT accentRect = { 28, 82, 36, 112 };
    FillRect(hDC, &accentRect, accentBrush);
    DeleteObject(accentBrush);

   
    // 플레이어 스탯 4개를 게이지와 함께 출력한다.
    DrawStatRow(hDC, L"재력", currentPlayerState.money, 185);
    DrawStatRow(hDC, L"대화 스킬", currentPlayerState.speech, 285);
    DrawStatRow(hDC, L"매력", currentPlayerState.charm, 385);
    DrawStatRow(hDC, L"외모", currentPlayerState.appearance, 485);
}

void ChoiceScene::DrawStatRow(HDC hDC, const std::wstring& statName, int statValue, int top)
{
    // 스탯 값은 0~100 범위로 보정해서 게이지 길이에 사용한다.
    int safeValue = ClampStat(statValue);

    RECT nameRect = { 42, top, 150, top + 32 };
    DrawTextInRect(
        hDC,
        statName,
        nameRect,
        20,
        RGB(224, 230, 250),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    RECT valueRect = { 160, top, 210, top + 32 };
    DrawTextInRect(
        hDC,
        std::to_wstring(safeValue),
        valueRect,
        20,
        RGB(245, 245, 255),
        DT_RIGHT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    // 게이지 배경을 먼저 출력한다.
    RECT gaugeBack = { 225, top + 8, 330, top + 23 };
    HBRUSH gaugeBackBrush = CreateSolidBrush(RGB(70, 82, 135));
    FillRect(hDC, &gaugeBack, gaugeBackBrush);
    DeleteObject(gaugeBackBrush);

    // 현재 값에 맞춰 채워진 게이지를 출력한다.
    RECT gaugeFill = gaugeBack;
    gaugeFill.right = gaugeFill.left + ((gaugeBack.right - gaugeBack.left) * safeValue / 100);

    HBRUSH gaugeFillBrush = CreateSolidBrush(RGB(255, 78, 158));
    FillRect(hDC, &gaugeFill, gaugeFillBrush);
    DeleteObject(gaugeFillBrush);

    // 게이지 외곽선을 얇게 그린다.
    HPEN gaugePen = CreatePen(PS_SOLID, 1, RGB(180, 190, 230));
    HBRUSH hollowBrush = static_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, gaugePen));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, hollowBrush));

    Rectangle(hDC, gaugeBack.left, gaugeBack.top, gaugeBack.right, gaugeBack.bottom);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(gaugePen);
}

void ChoiceScene::DrawChoiceCard(HDC hDC, int index)
{
    if (index < 0 || index >= HEROINE_COUNT)
    {
        return;
    }

    RECT cardRect = choiceHitBox[index];

    // 선택지 카드는 첨부 이미지처럼 오른쪽이 뾰족한 사선 박스로 출력한다.
    DrawSkewedCard(hDC, cardRect, RGB(52, 55, 68), RGB(24, 35, 90));

    // 카드 왼쪽에는 큰 사각 얼굴 썸네일을 표시해 빈 공간을 줄인다.
    RECT iconRect = { cardRect.left + 22, cardRect.top + 8, cardRect.left + 330, cardRect.bottom - 8 };
    DrawHeroineFace(hDC, index, iconRect);

    // 이름, 설명, 힌트는 사선 카드 내부에 배치한다.
    RECT nameRect = { cardRect.left + 370, cardRect.top + 34, cardRect.right - 150, cardRect.top + 95 };
    RECT descRect = { cardRect.left + 370, cardRect.top + 104, cardRect.right - 150, cardRect.top + 158 };
    RECT hintRect = { cardRect.left + 370, cardRect.top + 174, cardRect.right - 150, cardRect.top + 222 };

    DrawTextInRect(
        hDC,
        choiceTexts[index],
        nameRect,
        46,
        RGB(230, 226, 238),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE,
        FW_BOLD
    );

    DrawTextInRect(
        hDC,
        heroineDescriptions[index],
        descRect,
        32,
        RGB(205, 205, 215),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );

    // 힌트는 작은 파란 라벨처럼 보여주어 스탯 정보임을 구분한다.
    HBRUSH hintBrush = CreateSolidBrush(RGB(48, 108, 205));
    HPEN hintPen = CreatePen(PS_SOLID, 1, RGB(120, 165, 235));
    HBRUSH oldHintBrush = static_cast<HBRUSH>(SelectObject(hDC, hintBrush));
    HPEN oldHintPen = static_cast<HPEN>(SelectObject(hDC, hintPen));
    RoundRect(hDC, hintRect.left - 8, hintRect.top + 3, hintRect.right, hintRect.bottom - 3, 12, 12);
    SelectObject(hDC, oldHintBrush);
    SelectObject(hDC, oldHintPen);
    DeleteObject(hintBrush);
    DeleteObject(hintPen);

    DrawTextInRect(
        hDC,
        heroineHints[index],
        hintRect,
        25,
        RGB(245, 248, 255),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );
}

void ChoiceScene::DrawHeroineFace(HDC hDC, int index, const RECT& iconRect)
{
    if (index < 0 || index >= HEROINE_COUNT)
    {
        return;
    }

    // 얼굴을 더 크게 보이게 하기 위해 단순 사각 썸네일 프레임을 사용한다.
    HBRUSH frameBrush = CreateSolidBrush(RGB(255, 66, 150));
    HPEN framePen = CreatePen(PS_SOLID, 3, RGB(255, 125, 185));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, frameBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, framePen));
    RoundRect(hDC, iconRect.left, iconRect.top, iconRect.right, iconRect.bottom, 12, 12);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(frameBrush);
    DeleteObject(framePen);

    RECT imageRect =
    {
        iconRect.left + 8,
        iconRect.top + 8,
        iconRect.right - 8,
        iconRect.bottom - 8
    };

    HBRUSH imageBackBrush = CreateSolidBrush(RGB(236, 232, 246));
    FillRect(hDC, &imageRect, imageBackBrush);
    DeleteObject(imageBackBrush);

    if (heroineFaceImages[index].IsNull())
    {
        return;
    }

    int sourceWidth = heroineFaceImages[index].GetWidth();
    int sourceHeight = heroineFaceImages[index].GetHeight();

    // 이미지를 늘리거나 줄이지 않고, 원본에서 얼굴 영역만 1:1 크기로 잘라서 가져온다.
    // destination 크기와 source crop 크기를 같게 두면 CImage가 별도 확대/축소를 하지 않는다.
    int cropWidth = imageRect.right - imageRect.left;
    int cropHeight = imageRect.bottom - imageRect.top;
    int cropX = (sourceWidth - cropWidth) / 2;
    int cropY = 0;

    if (index == 0) // 한세아
    {
        cropY = 72;
    }
    else if (index == 1) // 유하린
    {
        cropY = 54;
    }
    else // 서이린
    {
        cropX = sourceWidth * 22 / 100;
        cropY = 50;
    }

    if (cropX < 0)
    {
        cropX = 0;
    }

    if (cropY < 0)
    {
        cropY = 0;
    }

    if (cropX + cropWidth > sourceWidth)
    {
        cropWidth = sourceWidth - cropX;
    }

    if (cropY + cropHeight > sourceHeight)
    {
        cropHeight = sourceHeight - cropY;
    }

    heroineFaceImages[index].Draw(
        hDC,
        imageRect.left,
        imageRect.top,
        imageRect.right - imageRect.left,
        imageRect.bottom - imageRect.top,
        cropX,
        cropY,
        cropWidth,
        cropHeight
    );
}

void ChoiceScene::DrawSkewedCard(HDC hDC, const RECT& rect, COLORREF fillColor, COLORREF lineColor)
{
    POINT shadowPoints[4] =
    {
        { rect.left + 40, rect.top + 16 },
        { rect.right + 18, rect.top + 16 },
        { rect.right - 90, rect.bottom + 16 },
        { rect.left - 18, rect.bottom + 16 }
    };

    HBRUSH shadowBrush = CreateSolidBrush(RGB(12, 18, 45));
    HPEN shadowPen = CreatePen(PS_SOLID, 1, RGB(12, 18, 45));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, shadowBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, shadowPen));
    Polygon(hDC, shadowPoints, 4);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(shadowBrush);
    DeleteObject(shadowPen);

    POINT cardPoints[4] =
    {
        { rect.left + 45, rect.top },
        { rect.right, rect.top },
        { rect.right - 105, rect.bottom },
        { rect.left, rect.bottom }
    };

    HBRUSH cardBrush = CreateSolidBrush(fillColor);
    HPEN cardPen = CreatePen(PS_SOLID, 5, lineColor);
    oldBrush = static_cast<HBRUSH>(SelectObject(hDC, cardBrush));
    oldPen = static_cast<HPEN>(SelectObject(hDC, cardPen));
    Polygon(hDC, cardPoints, 4);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(cardBrush);
    DeleteObject(cardPen);

    HPEN accentPen = CreatePen(PS_SOLID, 4, RGB(255, 70, 150));
    oldPen = static_cast<HPEN>(SelectObject(hDC, accentPen));
    MoveToEx(hDC, rect.left + 45, rect.top, nullptr);
    LineTo(hDC, rect.left, rect.bottom);
    SelectObject(hDC, oldPen);
    DeleteObject(accentPen);
}

void ChoiceScene::DrawTextInRect(HDC hDC, const std::wstring& text, RECT rect, int fontSize, COLORREF color, UINT format, int fontWeight)
{
    HFONT textFont = CreateFontW(
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, textFont));

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, color);
    DrawTextW(
        hDC,
        text.c_str(),
        -1,
        &rect,
        format
    );

    SelectObject(hDC, oldFont);
    DeleteObject(textFont);
}

bool ChoiceScene::HasSelected() const
{
    return m_hasSelected;
}

int ChoiceScene::GetSelectedIndex() const
{
    return selectedCharacter;
}
