#include "StoryScene.h"

void StoryScene::Initialize()
{
    dialogues.clear();

    DialogueLineInfo line;
    line.speaker = L"한세아";
    line.text = L"안녕!";
    dialogues.push_back(line);

    line.speaker = L"윤서";
    line.text = L"아아아아아아아아아아아아아아아아아아아아아아앙아아!";
    dialogues.push_back(line);

    currentDialogueIndex = 0;

    finished = false;
}

void StoryScene::Shutdown()
{
    dialogues.clear();
    currentDialogueIndex = 0;
    finished = false;
}

void StoryScene::OnMouseClick(int x, int y)
{
    if (finished)
    {
        return;
    }

    if (currentDialogueIndex < static_cast<int>(dialogues.size()) - 1)
    {
        currentDialogueIndex++;
    }
    else
    {
        finished = true;
    }
}

bool StoryScene::IsFinished() const
{
    return finished;
}

StoryScene::SpeakerStyle StoryScene::GetSpeakerStyle(const std::wstring& speaker) const
{
    // 기본 스타일이다.
    SpeakerStyle style = {
        RGB(230, 220, 255),
        RGB(245, 245, 245),
        RGB(210, 190, 255),
        RGB(80, 70, 120)
    };

    // 화자 이름에 따라 스타일을 바꾼다.
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
    // 대사가 없으면 그릴 것이 없다.
    if (dialogues.empty())
    {
        return;
    }

    // 현재 대사 정보를 가져온다.
    const std::wstring& currentSpeaker = dialogues[currentDialogueIndex].speaker;
    const std::wstring& currentText = dialogues[currentDialogueIndex].text;

    // 현재 화자에 맞는 스타일을 가져온다.
    SpeakerStyle style = GetSpeakerStyle(currentSpeaker);

    // 폰트를 만든다.
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

    // 글자 배경 투명 적용
    SetBkMode(hDC, TRANSPARENT);

    HBRUSH boxBrush = CreateSolidBrush(RGB(20, 20, 25));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, boxBrush));

    HPEN boxPen = CreatePen(PS_SOLID, 2, style.outline);
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, boxPen));

    // 대화창을 그린다.
    RoundRect(
        hDC,
        dialogueBox.left,
        dialogueBox.top,
        dialogueBox.right,
        dialogueBox.bottom,
        20,
        20
    );

    
    HPEN linePen = CreatePen(PS_SOLID, 3, style.accent);
    SelectObject(hDC, linePen);
    // 대화창 위 연한 색깔 선 그린다
    MoveToEx(hDC, 0, lineY, nullptr);
    LineTo(hDC, 1920, lineY);

    // 이름 출력
    SetTextColor(hDC, style.nameColor);
    DrawTextW(
        hDC,
        currentSpeaker.c_str(),
        -1,
        &nameBox,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // 대사 출력
    SetTextColor(hDC, style.textColor);
    DrawTextW(
        hDC,
        currentText.c_str(),
        -1,
        &textRect,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );

   
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldFont);
    DeleteObject(linePen);
    DeleteObject(boxPen);
    DeleteObject(boxBrush);
    DeleteObject(dialogueFont);
}