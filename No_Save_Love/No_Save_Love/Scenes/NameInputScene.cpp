#include "NameInputScene.h"

void NameInputScene::Initialize()
{
    // 처음 들어올 때 상태를 초기화한다.
    Reset();
}

void NameInputScene::Shutdown()
{
    // 현재는 따로 해제할 이미지나 리소스가 없으므로 비워둔다.
}

void NameInputScene::Reset()
{
    // 입력된 이름을 비운다.
    m_inputName.clear();

    // 입력 완료 상태를 false로 돌린다.
    m_isFinished = false;

    // 이름 입력 화면에 들어오면 바로 입력 가능하게 한다.
    m_isInputActive = true;
}

void NameInputScene::OnMouseClick(int x, int y)
{
    // 입력 박스를 클릭하면 입력 활성화 상태로 만든다.
    if (IsClickInputBox(x, y))
    {
        m_isInputActive = true;
        return;
    }

    // 확인 버튼을 클릭하면 이름을 확정한다.
    if (IsClickConfirmButton(x, y))
    {
        ConfirmName();
        return;
    }
}

void NameInputScene::OnChar(wchar_t inputChar)
{
    // 이미 입력이 끝났다면 더 이상 입력받지 않는다.
    if (m_isFinished)
    {
        return;
    }

    // 입력 박스가 활성화되지 않았다면 입력받지 않는다.
    if (!m_isInputActive)
    {
        return;
    }

    // 백스페이스 처리
    if (inputChar == VK_BACK)
    {
        // 입력된 글자가 있을 때만 마지막 글자를 지운다.
        if (!m_inputName.empty())
        {
            m_inputName.pop_back();
        }

        return;
    }

    // 엔터키 처리
    if (inputChar == VK_RETURN)
    {
        // 엔터를 누르면 이름을 확정한다.
        ConfirmName();
        return;
    }

    // ESC는 현재 이름 입력 화면에서는 무시한다.
    if (inputChar == VK_ESCAPE)
    {
        return;
    }

    // 탭, 줄바꿈 등 제어 문자는 이름으로 받지 않는다.
    if (inputChar < 32)
    {
        return;
    }

    // 이름은 최대 6글자까지만 입력받는다.
    if (static_cast<int>(m_inputName.length()) >= MAX_NAME_LENGTH)
    {
        return;
    }

    // 공백은 이름에 넣지 않도록 막는다.
    if (inputChar == L' ')
    {
        return;
    }

    // 정상 문자라면 이름 뒤에 추가한다.
    m_inputName.push_back(inputChar);
}

void NameInputScene::Render(HDC hDC)
{
    // 전체 배경을 검은색으로 칠한다.
    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT bgRect = { 0, 0, 1920, 1080 };
    FillRect(hDC, &bgRect, bgBrush);
    DeleteObject(bgBrush);

    // 텍스트 배경을 투명하게 만든다.
    SetBkMode(hDC, TRANSPARENT);

    // =========================
    // 1. 제목 출력
    // =========================

    HFONT titleFont = CreateSceneFont(58, FW_BOLD);
    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, titleFont));

    SetTextColor(hDC, RGB(255, 255, 255));

    RECT titleRect = { 0, 220, 1920, 320 };

    DrawTextW(
        hDC,
        L"너의 이름은?",
        -1,
        &titleRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, oldFont);
    DeleteObject(titleFont);

    // =========================
    // 2. 안내 문구 출력
    // =========================

    HFONT guideFont = CreateSceneFont(28, FW_NORMAL);
    oldFont = static_cast<HFONT>(SelectObject(hDC, guideFont));

    SetTextColor(hDC, RGB(220, 220, 220));

    RECT guideRect = { 0, 315, 1920, 370 };

    DrawTextW(
        hDC,
        L"[이름은 최대 6자 까지 가능합니다]",
        -1,
        &guideRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, oldFont);
    DeleteObject(guideFont);

    // =========================
    // 3. 입력 박스 출력
    // =========================

    HBRUSH inputBrush = CreateSolidBrush(RGB(245, 245, 245));

    // 입력 중일 때는 테두리를 파란색으로, 아니면 회색으로 그린다.
    HPEN inputPen = nullptr;

    if (m_isInputActive)
    {
        inputPen = CreatePen(PS_SOLID, 4, RGB(90, 140, 255));
    }
    else
    {
        inputPen = CreatePen(PS_SOLID, 3, RGB(180, 180, 180));
    }

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, inputBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, inputPen));

    // 입력 박스는 살짝 둥근 사각형으로 출력한다.
    RoundRect(
        hDC,
        m_inputBoxRect.left,
        m_inputBoxRect.top,
        m_inputBoxRect.right,
        m_inputBoxRect.bottom,
        18,
        18
    );

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);

    DeleteObject(inputPen);
    DeleteObject(inputBrush);

    // =========================
    // 4. 입력된 이름 출력
    // =========================

    HFONT inputFont = CreateSceneFont(50, FW_BOLD);
    oldFont = static_cast<HFONT>(SelectObject(hDC, inputFont));

    SetTextColor(hDC, RGB(35, 35, 35));

    std::wstring displayName = m_inputName;

    // 아무것도 입력하지 않았으면 안내용 placeholder를 보여준다.
    if (displayName.empty())
    {
        SetTextColor(hDC, RGB(160, 160, 160));
        displayName = L"이름 입력";
    }

    DrawTextW(
        hDC,
        displayName.c_str(),
        -1,
        &m_inputTextRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, oldFont);
    DeleteObject(inputFont);

    // =========================
    // 5. 확인 버튼 출력
    // =========================

    HBRUSH buttonBrush = CreateSolidBrush(RGB(80, 130, 255));
    HPEN buttonPen = CreatePen(PS_SOLID, 3, RGB(120, 160, 255));

    oldBrush = static_cast<HBRUSH>(SelectObject(hDC, buttonBrush));
    oldPen = static_cast<HPEN>(SelectObject(hDC, buttonPen));

    RoundRect(
        hDC,
        m_confirmButtonRect.left,
        m_confirmButtonRect.top,
        m_confirmButtonRect.right,
        m_confirmButtonRect.bottom,
        35,
        35
    );

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);

    DeleteObject(buttonPen);
    DeleteObject(buttonBrush);

    // =========================
    // 6. 확인 버튼 글자 출력
    // =========================

    HFONT buttonFont = CreateSceneFont(38, FW_BOLD);
    oldFont = static_cast<HFONT>(SelectObject(hDC, buttonFont));

    SetTextColor(hDC, RGB(255, 255, 255));

    DrawTextW(
        hDC,
        L"확 인",
        -1,
        &m_confirmButtonRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hDC, oldFont);
    DeleteObject(buttonFont);
}

bool NameInputScene::IsFinished() const
{
    // 이름 입력이 완료되었는지 GameManager가 확인할 때 사용한다.
    return m_isFinished;
}

std::wstring NameInputScene::GetPlayerName() const
{
    // 빈 이름으로 확정된 경우 기본 이름을 반환한다.
    if (m_inputName.empty())
    {
        return L"윤서";
    }

    return m_inputName;
}

bool NameInputScene::IsClickConfirmButton(int x, int y) const
{
    // 마우스 좌표를 POINT 구조체로 변환한다.
    POINT mousePoint = { x, y };

    // 확인 버튼 영역 안에 있는지 검사한다.
    return PtInRect(&m_confirmButtonRect, mousePoint);
}

bool NameInputScene::IsClickInputBox(int x, int y) const
{
    // 마우스 좌표를 POINT 구조체로 변환한다.
    POINT mousePoint = { x, y };

    // 입력 박스 영역 안에 있는지 검사한다.
    return PtInRect(&m_inputBoxRect, mousePoint);
}

void NameInputScene::ConfirmName()
{
    // 이름이 비어 있으면 GetPlayerName()에서 기본값 윤서를 반환한다.
    // 여기서는 입력 완료 상태만 true로 바꾼다.
    m_isFinished = true;
}

HFONT NameInputScene::CreateSceneFont(int fontSize, int fontWeight) const
{
    // 장면에서 사용할 폰트를 생성한다.
    return CreateFontW(
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
}