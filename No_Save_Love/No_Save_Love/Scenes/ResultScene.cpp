#include "ResultScene.h"

void ResultScene::Reset()
{
    // 결과 화면 상태를 초기화한다.
    m_whichGame = 0;
    m_score = 0;

    m_hasResult = false;
    m_finished = false;

    m_plusState = {};
    m_currentState = {};
}

void ResultScene::SetResult(int whichGame, int score)
{

    m_whichGame = whichGame;

    m_score = ClampStat(score);

    m_plusState = CalculateStat(m_whichGame, m_score);

    m_hasResult = true;
    m_finished = false;
}

void ResultScene::SetCurrentPlayerState(const Player_state& currentState)
{
    m_currentState = currentState;
}

void ResultScene::SetPlayerName(const std::wstring& playerName)
{
    // 빈 이름이면 기본 이름을 유지한다.
    if (playerName.empty())
    {
        m_playerName = L"윤서";
        return;
    }

    m_playerName = playerName;
}

void ResultScene::OnMouseClick(int x, int y)
{
    if (!m_hasResult)
    {
        return;
    }

    // 확인 버튼을 눌렀을 때만 ResultScene을 끝낸다.
    if (x >= m_confirmButton.left &&
        x <= m_confirmButton.right &&
        y >= m_confirmButton.top &&
        y <= m_confirmButton.bottom)
    {
        m_finished = true;
    }
}

bool ResultScene::IsFinished() const
{
    return m_finished;
}

Player_state ResultScene::GetPlusState() const
{
    return m_plusState;
}

Player_state ResultScene::CalculateStat(int whichGame, int score)
{
    Player_state plusState = {};

    switch (whichGame)
    {
    case 0:
    {
        // 미니게임 1: 라면 가게 / 재력, 대화 스킬
        if (score >= 90)
        {
            plusState.money = 50;
            plusState.speech = 30;
        }
        else if (score >= 80)
        {
            plusState.money = 40;
            plusState.speech = 24;
        }
        else if (score >= 60)
        {
            plusState.money = 30;
            plusState.speech = 18;
        }
        else if (score >= 40)
        {
            plusState.money = 20;
            plusState.speech = 10;
        }
        else if (score >= 20)
        {
            plusState.money = 10;
            plusState.speech = 5;
        }
        else
        {
            plusState.money = 0;
            plusState.speech = 0;
        }

        break;
    }

    case 1:
    {
        // 미니게임 2: 노래 연습 / 매력, 외모
        if (score >= 90)
        {
            plusState.charm = 50;
            plusState.appearance = 30;
        }
        else if (score >= 80)
        {
            plusState.charm = 40;
            plusState.appearance = 24;
        }
        else if (score >= 60)
        {
            plusState.charm = 30;
            plusState.appearance = 18;
        }
        else if (score >= 40)
        {
            plusState.charm = 20;
            plusState.appearance = 10;
        }
        else if (score >= 20)
        {
            plusState.charm = 10;
            plusState.appearance = 5;
        }
        else
        {
            plusState.charm = 0;
            plusState.appearance = 0;
        }

        break;
    }

    case 2:
    {
        // 미니게임 3: 등교길 / 외모, 매력
        if (score >= 90)
        {
            plusState.appearance = 50;
            plusState.charm = 30;
        }
        else if (score >= 80)
        {
            plusState.appearance = 40;
            plusState.charm = 24;
        }
        else if (score >= 60)
        {
            plusState.appearance = 30;
            plusState.charm = 18;
        }
        else if (score >= 40)
        {
            plusState.appearance = 20;
            plusState.charm = 10;
        }
        else if (score >= 20)
        {
            plusState.appearance = 10;
            plusState.charm = 5;
        }
        else
        {
            plusState.appearance = 0;
            plusState.charm = 0;
        }

        break;
    }

    case 3:
    {
        // 미니게임 4: 유혹 피하기 / 대화 스킬, 재력
        if (score >= 90)
        {
            plusState.speech = 50;
            plusState.money = 30;
        }
        else if (score >= 80)
        {
            plusState.speech = 40;
            plusState.money = 24;
        }
        else if (score >= 60)
        {
            plusState.speech = 30;
            plusState.money = 18;
        }
        else if (score >= 40)
        {
            plusState.speech = 20;
            plusState.money = 10;
        }
        else if (score >= 20)
        {
            plusState.speech = 10;
            plusState.money = 5;
        }
        else
        {
            plusState.speech = 0;
            plusState.money = 0;
        }

        break;
    }

    default:
    {
        // 잘못된 미니게임 번호면 상승량 없음
        plusState = {};
        break;
    }
    }

    return plusState;
}

std::wstring ResultScene::GetGameName(int whichGame) const
{
    switch (whichGame)
    {
    case 0:
        return m_playerName + L"의 PC방 알바";

    case 1:
        return m_playerName + L"의 노래 연습";

    case 2:
        return m_playerName + L"의 등교길";

    case 3:
        return L"유혹 피하기";

    default:
        return L"알 수 없는 미니게임";
    }
}

void ResultScene::RenderResult(HDC hDC)
{
    // 결과 데이터가 준비되지 않았으면 아무것도 출력하지 않는다.
    if (!m_hasResult)
    {
        return;
    }

    // =========================
    // 기본 계산
    // =========================

    // 점수에 따라 등급을 정한다.
    std::wstring gradeText = L"F";

    if (m_score >= 90)
    {
        gradeText = L"S";
    }
    else if (m_score >= 80)
    {
        gradeText = L"A";
    }
    else if (m_score >= 60)
    {
        gradeText = L"B";
    }
    else if (m_score >= 40)
    {
        gradeText = L"C";
    }
    else if (m_score >= 20)
    {
        gradeText = L"D";
    }
    else
    {
        gradeText = L"F";
    }

    // =========================
    // 배경 출력
    // =========================

    RECT fullScreen = { 0, 0, 1920, 1080 };

    HBRUSH backgroundBrush = CreateSolidBrush(RGB(9, 18, 45));
    FillRect(hDC, &fullScreen, backgroundBrush);
    DeleteObject(backgroundBrush);

    // 글자 배경을 투명하게 한다.
    SetBkMode(hDC, TRANSPARENT);

    // =========================
    // 큰 패널 출력
    // =========================

    RECT leftPanel = { 70, 70, 760, 910 };
    RECT rightPanel = { 800, 70, 1850, 910 };

    HBRUSH panelBrush = CreateSolidBrush(RGB(18, 28, 62));
    HPEN panelPen = CreatePen(PS_SOLID, 2, RGB(75, 84, 120));

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, panelBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, panelPen));

    RoundRect(hDC, leftPanel.left, leftPanel.top, leftPanel.right, leftPanel.bottom, 28, 28);
    RoundRect(hDC, rightPanel.left, rightPanel.top, rightPanel.right, rightPanel.bottom, 28, 28);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    DeleteObject(panelBrush);
    DeleteObject(panelPen);

    // =========================
    // 폰트 생성
    // =========================

    HFONT titleFont = CreateFontW(
        58, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT bigGradeFont = CreateFontW(
        150, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT largeFont = CreateFontW(
        72, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT normalFont = CreateFontW(
        32, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT boldFont = CreateFontW(
        34, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT smallFont = CreateFontW(
        26, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, normalFont));

    // =========================
    // 왼쪽 RESULT 제목
    // =========================

    SelectObject(hDC, titleFont);
    SetTextColor(hDC, RGB(245, 245, 250));

    RECT resultTitleRect = { 120, 105, 710, 175 };
    DrawTextW(
        hDC,
        L"RESULT",
        -1,
        &resultTitleRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, boldFont);
    SetTextColor(hDC, RGB(245, 90, 150));

    RECT resultSubTitleRect = { 190, 180, 640, 230 };
    DrawTextW(
        hDC,
        L"게임 결과",
        -1,
        &resultSubTitleRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // =========================
    // 왼쪽 등급 리스트
    // =========================

    RECT gradeBox = { 120, 270, 350, 810 };

    HBRUSH gradeBoxBrush = CreateSolidBrush(RGB(20, 30, 65));
    HPEN gradeBoxPen = CreatePen(PS_SOLID, 2, RGB(75, 84, 120));

    oldBrush = static_cast<HBRUSH>(SelectObject(hDC, gradeBoxBrush));
    oldPen = static_cast<HPEN>(SelectObject(hDC, gradeBoxPen));

    RoundRect(hDC, gradeBox.left, gradeBox.top, gradeBox.right, gradeBox.bottom, 20, 20);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    DeleteObject(gradeBoxBrush);
    DeleteObject(gradeBoxPen);

    std::wstring gradeList[6] = { L"S", L"A", L"B", L"C", L"D", L"F" };

    SelectObject(hDC, largeFont);

    for (int i = 0; i < 6; i++)
    {
        RECT itemRect =
        {
            gradeBox.left,
            gradeBox.top + i * 90,
            gradeBox.right,
            gradeBox.top + (i + 1) * 90
        };

        bool isSelected = (gradeText == gradeList[i]);

        // 현재 등급이면 분홍색 배경을 칠한다.
        if (isSelected)
        {
            HBRUSH selectedBrush = CreateSolidBrush(RGB(240, 80, 145));
            FillRect(hDC, &itemRect, selectedBrush);
            DeleteObject(selectedBrush);
        }

        // 등급 칸 구분선을 그린다.
        HPEN linePen = CreatePen(PS_SOLID, 1, RGB(65, 73, 105));
        oldPen = static_cast<HPEN>(SelectObject(hDC, linePen));

        MoveToEx(hDC, itemRect.left, itemRect.bottom, nullptr);
        LineTo(hDC, itemRect.right, itemRect.bottom);

        SelectObject(hDC, oldPen);
        DeleteObject(linePen);

        if (isSelected)
        {
            SetTextColor(hDC, RGB(255, 255, 255));
        }
        else
        {
            SetTextColor(hDC, RGB(165, 170, 190));
        }

        DrawTextW(
            hDC,
            gradeList[i].c_str(),
            -1,
            &itemRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE
        );
    }

    // =========================
    // 왼쪽 큰 등급 출력
    // =========================

    SelectObject(hDC, bigGradeFont);
    SetTextColor(hDC, RGB(245, 90, 150));

    RECT bigGradeRect = { 400, 300, 710, 500 };
    DrawTextW(
        hDC,
        gradeText.c_str(),
        -1,
        &bigGradeRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // =========================
    // 왼쪽 점수 / 등급 요약
    // =========================

    HPEN middleLinePen = CreatePen(PS_SOLID, 2, RGB(85, 92, 125));
    oldPen = static_cast<HPEN>(SelectObject(hDC, middleLinePen));

    MoveToEx(hDC, 390, 525, nullptr);
    LineTo(hDC, 710, 525);

    MoveToEx(hDC, 390, 730, nullptr);
    LineTo(hDC, 710, 730);

    SelectObject(hDC, oldPen);
    DeleteObject(middleLinePen);

    SelectObject(hDC, boldFont);
    SetTextColor(hDC, RGB(245, 90, 150));

    RECT scoreLabelRect = { 420, 555, 580, 610 };
    DrawTextW(
        hDC,
        L"총점",
        -1,
        &scoreLabelRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, largeFont);
    SetTextColor(hDC, RGB(255, 255, 255));

    RECT scoreNumberRect = { 425, 620, 585, 710 };
    std::wstring scoreNumberText = std::to_wstring(m_score);

    DrawTextW(
        hDC,
        scoreNumberText.c_str(),
        -1,
        &scoreNumberRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, boldFont);
    SetTextColor(hDC, RGB(165, 170, 190));

    RECT scoreMaxRect = { 560, 640, 720, 700 };
    DrawTextW(
        hDC,
        L"/ 100",
        -1,
        &scoreMaxRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, boldFont);
    SetTextColor(hDC, RGB(245, 90, 150));

    RECT gainLabelRect = { 415, 765, 600, 820 };
    DrawTextW(
        hDC,
        L"획득 등급",
        -1,
        &gainLabelRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, largeFont);
    SetTextColor(hDC, RGB(245, 90, 150));

    RECT gainGradeRect = { 600, 745, 720, 840 };
    DrawTextW(
        hDC,
        gradeText.c_str(),
        -1,
        &gainGradeRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, smallFont);
    SetTextColor(hDC, RGB(165, 170, 190));

    RECT gameNameRect = { 120, 835, 710, 890 };
    std::wstring gameNameText = GetGameName(m_whichGame);

    DrawTextW(
        hDC,
        gameNameText.c_str(),
        -1,
        &gameNameRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // =========================
    // 오른쪽 STATUS 제목
    // =========================

    // 분홍색 포인트 막대
    RECT pointBarRect = { 855, 120, 870, 175 };
    HBRUSH pointBrush = CreateSolidBrush(RGB(245, 90, 150));
    FillRect(hDC, &pointBarRect, pointBrush);
    DeleteObject(pointBrush);

    SelectObject(hDC, titleFont);
    SetTextColor(hDC, RGB(245, 245, 250));

    RECT statusTitleRect = { 890, 100, 1300, 180 };
    DrawTextW(
        hDC,
        L"STATUS",
        -1,
        &statusTitleRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, smallFont);
    SetTextColor(hDC, RGB(165, 170, 190));

    RECT statusSubRect = { 1320, 120, 1810, 170 };
    DrawTextW(
        hDC,
        L"이번 미니게임으로 상승한 스탯입니다.",
        -1,
        &statusSubRect,
        DT_RIGHT | DT_VCENTER | DT_SINGLELINE
    );

    // =========================
    // 스탯 정보 배열
    // =========================

    std::wstring statNames[4] =
    {
        L"재력",
        L"대화 스킬",
        L"매력",
        L"외모"
    };

    int plusValues[4] =
    {
        m_plusState.money,
        m_plusState.speech,
        m_plusState.charm,
        m_plusState.appearance
    };

    int currentValues[4] =
    {
        m_currentState.money,
        m_currentState.speech,
        m_currentState.charm,
        m_currentState.appearance
    };

    int rowTops[4] =
    {
        215,
        375,
        535,
        695
    };

    // =========================
    // 오른쪽 스탯 줄 출력
    // =========================

    for (int i = 0; i < 4; i++)
    {
        int top = rowTops[i];

        int currentValue = currentValues[i];

        if (currentValue < 0)
        {
            currentValue = 0;
        }

        if (currentValue > 100)
        {
            currentValue = 100;
        }

        int beforeValue = currentValue - plusValues[i];

        if (beforeValue < 0)
        {
            beforeValue = 0;
        }

        if (beforeValue > 100)
        {
            beforeValue = 100;
        }

        // 스탯 줄 배경 카드
        RECT rowRect = { 835, top, 1815, top + 140 };

        HBRUSH rowBrush = CreateSolidBrush(RGB(31, 37, 67));
        HPEN rowPen = CreatePen(PS_SOLID, 1, RGB(31, 37, 67));

        oldBrush = static_cast<HBRUSH>(SelectObject(hDC, rowBrush));
        oldPen = static_cast<HPEN>(SelectObject(hDC, rowPen));

        RoundRect(hDC, rowRect.left, rowRect.top, rowRect.right, rowRect.bottom, 20, 20);

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);

        DeleteObject(rowBrush);
        DeleteObject(rowPen);

        // 스탯 이름
        SelectObject(hDC, boldFont);
        SetTextColor(hDC, RGB(245, 245, 250));

        RECT statNameRect = { 875, top + 35, 1060, top + 100 };
        DrawTextW(
            hDC,
            statNames[i].c_str(),
            -1,
            &statNameRect,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE
        );

        // 증가량
        std::wstring plusText = L"+" + std::to_wstring(plusValues[i]);

        SetTextColor(hDC, RGB(245, 90, 150));

        RECT plusRect = { 1070, top + 35, 1190, top + 100 };
        DrawTextW(
            hDC,
            plusText.c_str(),
            -1,
            &plusRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE
        );

        // 중간 구분선
        HPEN statLinePen = CreatePen(PS_SOLID, 2, RGB(75, 83, 120));
        oldPen = static_cast<HPEN>(SelectObject(hDC, statLinePen));

        MoveToEx(hDC, 1215, top + 35, nullptr);
        LineTo(hDC, 1215, top + 105);

        SelectObject(hDC, oldPen);
        DeleteObject(statLinePen);

        // 이전값 → 현재값
        SelectObject(hDC, boldFont);
        SetTextColor(hDC, RGB(245, 245, 250));

        std::wstring valueText =
            std::to_wstring(beforeValue) +
            L"  →  " +
            std::to_wstring(currentValue);

        RECT valueRect = { 1250, top + 35, 1430, top + 100 };
        DrawTextW(
            hDC,
            valueText.c_str(),
            -1,
            &valueRect,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE
        );

        // 스탯 바 배경
        RECT barBackRect = { 1450, top + 58, 1740, top + 82 };

        HBRUSH barBackBrush = CreateSolidBrush(RGB(210, 215, 225));
        HPEN barBackPen = CreatePen(PS_SOLID, 1, RGB(210, 215, 225));

        oldBrush = static_cast<HBRUSH>(SelectObject(hDC, barBackBrush));
        oldPen = static_cast<HPEN>(SelectObject(hDC, barBackPen));

        RoundRect(
            hDC,
            barBackRect.left,
            barBackRect.top,
            barBackRect.right,
            barBackRect.bottom,
            12,
            12
        );

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);

        DeleteObject(barBackBrush);
        DeleteObject(barBackPen);

        // 스탯 바 채워진 부분
        int barWidth = barBackRect.right - barBackRect.left;
        int fillWidth = barWidth * currentValue / 100;

        if (fillWidth > 0)
        {
            RECT barFillRect = barBackRect;
            barFillRect.right = barFillRect.left + fillWidth;

            HBRUSH barFillBrush = CreateSolidBrush(RGB(240, 80, 145));
            HPEN barFillPen = CreatePen(PS_SOLID, 1, RGB(240, 80, 145));

            oldBrush = static_cast<HBRUSH>(SelectObject(hDC, barFillBrush));
            oldPen = static_cast<HPEN>(SelectObject(hDC, barFillPen));

            RoundRect(
                hDC,
                barFillRect.left,
                barFillRect.top,
                barFillRect.right,
                barFillRect.bottom,
                12,
                12
            );

            SelectObject(hDC, oldBrush);
            SelectObject(hDC, oldPen);

            DeleteObject(barFillBrush);
            DeleteObject(barFillPen);
        }

        // /100 출력
        SelectObject(hDC, normalFont);
        SetTextColor(hDC, RGB(170, 175, 195));

        RECT maxTextRect = { 1750, top + 35, 1810, top + 100 };
        DrawTextW(
            hDC,
            L"/100",
            -1,
            &maxTextRect,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE
        );
    }

    // =========================
    // 하단 확인 버튼
    // =========================

    RECT confirmButtonRect = { 710, 945, 1210, 1030 };

    HBRUSH confirmBrush = CreateSolidBrush(RGB(19, 29, 62));
    HPEN confirmPen = CreatePen(PS_SOLID, 3, RGB(245, 90, 150));

    oldBrush = static_cast<HBRUSH>(SelectObject(hDC, confirmBrush));
    oldPen = static_cast<HPEN>(SelectObject(hDC, confirmPen));

    RoundRect(
        hDC,
        confirmButtonRect.left,
        confirmButtonRect.top,
        confirmButtonRect.right,
        confirmButtonRect.bottom,
        22,
        22
    );

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    DeleteObject(confirmBrush);
    DeleteObject(confirmPen);

    SelectObject(hDC, boldFont);
    SetTextColor(hDC, RGB(250, 250, 255));

    DrawTextW(
        hDC,
        L"확인",
        -1,
        &confirmButtonRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // =========================
    // GDI 폰트 정리
    // =========================

    SelectObject(hDC, oldFont);

    DeleteObject(titleFont);
    DeleteObject(bigGradeFont);
    DeleteObject(largeFont);
    DeleteObject(normalFont);
    DeleteObject(boldFont);
    DeleteObject(smallFont);
}
