#include "StoryScene.h"

void StoryScene::Initialize()
{
    // 처음에는 비워둔다.
    // 실제 대사는 GameManager가 SetDialogues로 넣어준다.
    dialogues.clear();
    currentDialogueIndex = 0;
    finished = false;
}

void StoryScene::Shutdown()
{
    // 대사 목록을 비운다.
    dialogues.clear();

    // 상태값 초기화
    currentDialogueIndex = 0;
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
}

void StoryScene::OnMouseClick(int x, int y)
{
    // 대사가 없으면 바로 끝난 것으로 처리한다.
    if (dialogues.empty())
    {
        finished = true;
        return;
    }

    // 이미 끝났으면 더 이상 처리하지 않는다.
    if (finished)
    {
        return;
    }

    // 다음 대사가 남아 있으면 다음 대사로 이동한다.
    if (currentDialogueIndex < static_cast<int>(dialogues.size()) - 1)
    {
        currentDialogueIndex++;
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
    const std::wstring& originalText = dialogues[currentDialogueIndex].text;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꿔서 출력한다.
    std::wstring displaySpeaker = ReplacePlayerNameToken(originalSpeaker);
    std::wstring displayText = ReplacePlayerNameToken(originalText);

    SpeakerStyle style = GetSpeakerStyle(originalSpeaker);

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
        displayText.c_str(),
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
