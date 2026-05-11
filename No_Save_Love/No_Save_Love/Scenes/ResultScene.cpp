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
    if (m_hasResult)
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
        // 미니게임 3: 유혹 피하기 / 대화 스킬, 재력
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

    case 3:
    {
        // 미니게임 4: 등교길 / 외모, 매력
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
        return L"유혹 피하기";

    case 3:
        return m_playerName + L"의 등교길";

    default:
        return L"알 수 없는 미니게임";
    }
}

void ResultScene::RenderResult(HDC hDC)
{
    if (!m_hasResult)
    {
        return;
    }

    // 결과 출력용 폰트를 만든다.
    HFONT resultFont = CreateFontW(
        38,
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, resultFont));

    // 글자 배경을 투명하게 한다.
    SetBkMode(hDC, TRANSPARENT);

    // 결과창 배경 브러시
    HBRUSH boxBrush = CreateSolidBrush(RGB(245, 245, 250));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, boxBrush));

    // 결과창 테두리 펜
    HPEN boxPen = CreatePen(PS_SOLID, 3, RGB(80, 90, 140));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, boxPen));

    // 결과창을 그린다.
    RoundRect(
        hDC,
        m_resultBox.left,
        m_resultBox.top,
        m_resultBox.right,
        m_resultBox.bottom,
        24,
        24
    );

    SetTextColor(hDC, RGB(20, 20, 30));

    // 출력할 문자열을 만든다.
    std::wstring titleText =
        L"미니게임 결과 - " + GetGameName(m_whichGame);

    std::wstring scoreText =
        L"점수: " + std::to_wstring(m_score) + L" / 100";

    std::wstring moneyText =
        L"재력 +" + std::to_wstring(m_plusState.money) +
        L"   현재 재력: " + std::to_wstring(m_currentState.money);

    std::wstring speechText =
        L"대화 스킬 +" + std::to_wstring(m_plusState.speech) +
        L"   현재 대화 스킬: " + std::to_wstring(m_currentState.speech);

    std::wstring charmText =
        L"매력 +" + std::to_wstring(m_plusState.charm) +
        L"   현재 매력: " + std::to_wstring(m_currentState.charm);

    std::wstring appearanceText =
        L"외모 +" + std::to_wstring(m_plusState.appearance) +
        L"   현재 외모: " + std::to_wstring(m_currentState.appearance);

    std::wstring nextText =
        L"클릭하면 히로인 선택으로 넘어갑니다.";

    // 출력 위치
    RECT titleRect = { 160, 720, 1760, 770 };
    RECT scoreRect = { 160, 785, 1760, 835 };
    RECT moneyRect = { 160, 850, 1760, 895 };
    RECT speechRect = { 160, 900, 1760, 945 };
    RECT charmRect = { 160, 950, 1760, 995 };
    RECT appearanceRect = { 160, 1000, 1000, 1030 };
    RECT nextRect = { 1120, 980, 1760, 1020 };

    // 텍스트 출력
    DrawTextW(hDC, titleText.c_str(), -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hDC, scoreText.c_str(), -1, &scoreRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hDC, moneyText.c_str(), -1, &moneyRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hDC, speechText.c_str(), -1, &speechRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hDC, charmText.c_str(), -1, &charmRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hDC,appearanceText.c_str(),-1,&appearanceRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hDC, nextText.c_str(), -1, &nextRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);


    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldFont);

    DeleteObject(boxPen);
    DeleteObject(boxBrush);
    DeleteObject(resultFont);
}
