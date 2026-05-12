#include "StoryScene.h"

void StoryScene::Initialize()
{
    // 처음에는 비워둔다.
    // 실제 대사는 GameManager가 SetDialogues로 넣어준다.
    dialogues.clear();
    currentDialogueIndex = 0;
    visibleTextCount = 0;
    isTypingFinished = false;
    finished = false;
}

void StoryScene::Shutdown()
{
    dialogues.clear();

    currentDialogueIndex = 0;
    visibleTextCount = 0;
    isTypingFinished = false;
    finished = false;
}

void StoryScene::SetDialogues(const std::vector<DialogueLineInfo>& newDialogues)
{
    // 기존 대사를 지우고 새 대사 묶음을 받는다.
    dialogues = newDialogues;

    // 새 대사 묶음은 항상 처음부터 시작한다.
    currentDialogueIndex = 0;

    // 새 대사를 시작했으므로 아직 끝난 상태가 아니다.
    finished = false;

    // 타이핑 상태 초기화
    visibleTextCount = 0;
    isTypingFinished = false;
}

void StoryScene::OnMouseClick(int x, int y)
{
    // 대사가 없으면 바로 끝난 것으로 처리
    if (dialogues.empty())
    {
        finished = true;
        return;
    }
    if (finished)
    {
        return;
    }

    // 현재 대사의 전체 텍스트를 가져온다
    std::wstring displayText = GetCurrentDisplayText();

    // 아직 타이핑 중이라면 전체 문장을 보여준다.
    if (!isTypingFinished)
    {
        visibleTextCount = static_cast<int>(displayText.length());
        isTypingFinished = true;
        return;
    }

    // 타이핑이 끝난 상태에서 클릭하면 다음 대사로 이동
    if (currentDialogueIndex < static_cast<int>(dialogues.size()) - 1)
    {
        currentDialogueIndex++;

        visibleTextCount = 0;
        isTypingFinished = false;
    }
    else
    {
        // 마지막 대사에서 클릭하면 끝난 상태로 만든다.
        finished = true;
    }
}

bool StoryScene::IsFinished() const
{
    return finished;
}

StoryScene::SpeakerStyle StoryScene::GetSpeakerStyle(const std::wstring& speaker) const
{
    // 기본 스타일
    SpeakerStyle style = {
        RGB(230, 220, 255),
        RGB(245, 245, 245),
        RGB(210, 190, 255),
        RGB(80, 70, 120)
    };

    if (speaker == L"한세아")
    {
        return hanseaStyle;
    }
    else if (speaker == L"유하린")
    {
        return yuharinStyle;
    }
    else if (speaker == L"서이린")
    {
        return seoirinStyle;
    }

    return style;
}

void StoryScene::Render(HDC hDC)
{
    // 출력할 대사가 없으면 그리지 않는다.
    if (dialogues.empty())
    {
        return;
    }

    const std::wstring& originalSpeaker = dialogues[currentDialogueIndex].speaker;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꿔서 출력한다.
    std::wstring displaySpeaker = GetCurrentDisplaySpeaker();
    std::wstring displayText = GetCurrentDisplayText();

    SpeakerStyle style = GetSpeakerStyle(originalSpeaker);

    // 현재 보여줄 글자 수가 전체 길이를 넘지 않도록 보정한다.
    int safeVisibleCount = visibleTextCount;
    if (safeVisibleCount > static_cast<int>(displayText.length()))
    {
        safeVisibleCount = static_cast<int>(displayText.length());
    }

    // 현재 보여줄 글자 수만큼 잘라낸다.
    std::wstring visibleText = displayText.substr(0, safeVisibleCount);

    HFONT dialogueFont = CreateFontW(
        42, 
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, dialogueFont));

    SetBkMode(hDC, TRANSPARENT);

    HBRUSH boxBrush = CreateSolidBrush(RGB(20, 20, 25));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, boxBrush));

    HPEN boxPen = CreatePen(PS_SOLID, 2, style.outline);
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, boxPen));

    // 대화창 출력
    RoundRect(
        hDC,
        dialogueBox.left,
        dialogueBox.top,
        dialogueBox.right,
        dialogueBox.bottom,
        20,
        20
    );

    // 포인트 라인 출력
    HPEN linePen = CreatePen(PS_SOLID, 3, style.accent);
    SelectObject(hDC, linePen);

    MoveToEx(hDC, 0, lineY, nullptr);
    LineTo(hDC, 1920, lineY);

    // 이름 출력
    SetTextColor(hDC, style.nameColor);
    DrawTextW(
        hDC,
        displaySpeaker.c_str(),
        -1,
        &nameBox,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // 대사 출력
    SetTextColor(hDC, style.textColor);
    DrawTextW(
        hDC,
        visibleText.c_str(),
        -1,
        &textRect,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );

    // GDI 객체 복구
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldFont);

    // 직접 만든 GDI 객체 삭제
    DeleteObject(linePen);
    DeleteObject(boxPen);
    DeleteObject(boxBrush);
    DeleteObject(dialogueFont);
}

void StoryScene::SetPlayerName(const std::wstring& playerName)
{
    // 빈 이름이 들어오면 기본 이름 윤서를 사용한다.
    if (playerName.empty())
    {
        m_playerName = L"윤서";
        return;
    }

    // 입력받은 이름을 저장한다.
    m_playerName = playerName;
}

std::wstring StoryScene::ReplacePlayerNameToken(const std::wstring& text) const
{
    std::wstring result = text;
    const std::wstring token = L"{PLAYER}";

    size_t position = result.find(token);
    while (position != std::wstring::npos)
    {
        result.replace(position, token.length(), m_playerName);
        position = result.find(token, position + m_playerName.length());
    }

    return result;
}

std::wstring StoryScene::GetCurrentDisplaySpeaker() const
{
    // 대사가 없으면 빈 문자열을 반환한다.
    if (dialogues.empty())
    {
        return L"";
    }

    // 현재 대사의 원래 화자를 가져온다.
    const std::wstring& originalSpeaker = dialogues[currentDialogueIndex].speaker;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꿔 반환한다.
    return ReplacePlayerNameToken(originalSpeaker);
}

std::wstring StoryScene::GetCurrentDisplayText() const
{
    // 대사가 없으면 빈 문자열을 반환한다.
    if (dialogues.empty())
    {
        return L"";
    }

    // 현재 대사의 원래 본문을 가져온다.
    const std::wstring& originalText = dialogues[currentDialogueIndex].text;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꿔 반환한다.
    return ReplacePlayerNameToken(originalText);
}

void StoryScene::UpdateTyping() {
    if (dialogues.empty() || finished) {
        return;
    }

    std::wstring displayText = GetCurrentDisplayText();

    if (visibleTextCount < static_cast<int>(displayText.length()))
    {
        visibleTextCount++;
        isTypingFinished = false;
    }
    else
    {
        isTypingFinished = true;
    }
}