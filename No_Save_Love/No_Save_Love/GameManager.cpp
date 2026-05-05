#include "GameManager.h"

bool GameManager::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;

    // 임시 대사 데이터
    DialogueLine_info make_dialogue;
    make_dialogue.speaker = L"한세아";
    make_dialogue.text = L"안녕!";
    dialogues.push_back(make_dialogue);
    make_dialogue.speaker = L"윤서";
    make_dialogue.text = L"아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아아앙아아!";
    dialogues.push_back(make_dialogue);

    m_currentDialogueIndex = 0;

    characters = {
    CharacterInfo{ L"한세아", 0 },
    CharacterInfo{ L"유하린", 0 },
    CharacterInfo{ L"서이린", 0 }
    };

    return true;
}

void GameManager::Shutdown()
{
    dialogues.clear();
    m_hWnd = nullptr;
}

void GameManager::HandleDialogueClick() {
    if (m_currentDialogueIndex < static_cast<int>(dialogues.size()) - 1)
    {
        m_currentDialogueIndex++;
    }
    else {
        now_game_mode = game_mode_info::Choice;
    }
}

void GameManager::HandleChoiceClick(int x, int y)
{
    POINT mousePoint = { x, y };

    // 선택지 박스 안을 클릭했는지 확인한다.
    for (int i = 0; i < 3; i++)
    {
        if (PtInRect(&choiceHitBox[i], mousePoint))
        {
            // 선택한 캐릭터 번호 저장
            selectedCharacter = i;

            // 선택한 캐릭터의 호감도 증가
            characters[selectedCharacter].affection += choiceAffectionValue;

            // 호감도가 100을 넘지 않도록 제한
            if (characters[selectedCharacter].affection > 100)
            {
                characters[selectedCharacter].affection = 100;
            }

            // 결과 화면으로 이동
            now_game_mode = game_mode_info::Result;
            break;
        }
    }
}

void GameManager::HandleResultClick() {
    
}

void GameManager::OnMouseClick(int x, int y)
{
    switch (now_game_mode) {
        case game_mode_info::Dialogue: {
            HandleDialogueClick();
            break;
        }
        case game_mode_info::Choice: {
            HandleChoiceClick(x, y);
            break;
        }
        case game_mode_info::Result: {
            HandleResultClick();
            break;
        }
    }
    InvalidateRect(m_hWnd, nullptr, TRUE);
}

GameManager::SpeakerStyle GameManager::GetSpeakerStyle(const std::wstring& speaker) const
{
    // 기본 스타일을 먼저 설정한다.
    SpeakerStyle font_style = {
        RGB(230, 220, 255),
        RGB(245, 245, 245),
        RGB(210, 190, 255),
        RGB(80, 70, 120)
    };

    // 화자 이름에 따라 캐릭터별 스타일을 반환한다.
    if (speaker == L"한세아")
    {
        return hansea_font_style;
    }
    else if (speaker == L"유하린")
    {
        return yuharin_font_style;
    }
    else if (speaker == L"서이린")
    {
        return seoirin_font_style;
    }

    return font_style;
}

void GameManager::RenderDialogue(HDC hDC) {
  
   

    const std::wstring& current_speaker = dialogues[m_currentDialogueIndex].speaker;
    const std::wstring& current_text = dialogues[m_currentDialogueIndex].text;

    SpeakerStyle font_style = GetSpeakerStyle(current_speaker);

    // 폰트 생성
    HFONT dialogueFont = CreateFontW(42, 0, 0, 0, 600, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");

    HFONT oldFont = (HFONT)SelectObject(hDC, dialogueFont);

    // 글자 배경 투명
    SetBkMode(hDC, TRANSPARENT);

    // 대화창 배경용 브러시
    HBRUSH boxBrush = CreateSolidBrush(RGB(20, 20, 25));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, boxBrush);

    // 대화창 테두리용 펜
    HPEN boxPen = CreatePen(PS_SOLID, 2, font_style.outline);
    HPEN oldPen = (HPEN)SelectObject(hDC, boxPen);

    // 대화창
    RoundRect(
        hDC,
        dialogue_box.left,
        dialogue_box.top,
        dialogue_box.right,
        dialogue_box.bottom,
        20,
        20
    );

    // 위쪽 포인트 라인
    HPEN linePen = CreatePen(PS_SOLID, 3, font_style.accent);
    SelectObject(hDC, linePen);

    MoveToEx(hDC, 0, lineY, nullptr);
    LineTo(hDC, 1920, lineY);

    // 이름 출력
    SetTextColor(hDC, font_style.name_color);
    DrawTextW(
        hDC,
        current_speaker.c_str(),
        -1,
        &name_box,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // 대사 출력
    SetTextColor(hDC, font_style.text_color);
    DrawTextW(
        hDC,
        current_text.c_str(),
        -1,
        &text_rect,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );

    // GDI 객체 정리
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldFont);

    DeleteObject(linePen);
    DeleteObject(boxPen);
    DeleteObject(boxBrush);
    DeleteObject(dialogueFont);
}
void GameManager::RenderChoice(HDC hDC) {
    HBRUSH whiteBrush = CreateSolidBrush(RGB(245, 245, 245));
    HPEN bluePen = CreatePen(PS_SOLID, 5, RGB(70, 110, 230));

    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, whiteBrush);
    HPEN oldPen = (HPEN)SelectObject(hDC, bluePen);

    Polygon(hDC, choiceBox[0], 4);
    Polygon(hDC, choiceBox[1], 4);
    Polygon(hDC, choiceBox[2], 4);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    DeleteObject(whiteBrush);
    DeleteObject(bluePen);

    TextOut(hDC, 760, 245, L"한세아", lstrlen(L"한세아"));
    TextOut(hDC, 760, 445, L"유하린", lstrlen(L"유하린"));
    TextOut(hDC, 760, 690, L"서이린", lstrlen(L"서이린"));
}

void GameManager::RenderResult(HDC hDC)
{
   //결과 문구
    std::wstring resultText = heroineNames[selectedCharacter] + L"를 선택했습니다";

    std::wstring affectionUpText =
        characters[selectedCharacter].name + L" 호감도 +" + std::to_wstring(choiceAffectionValue);

    std::wstring currentAffectionText =
        L"현재 호감도: " + std::to_wstring(characters[selectedCharacter].affection);

    // 폰트 생성
    HFONT dialogueFont = CreateFontW(
        42, 0, 0, 0, 600,
        FALSE, FALSE, FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT oldFont = (HFONT)SelectObject(hDC, dialogueFont);

    // 글자 배경을 투명하게 한다.
    SetBkMode(hDC, TRANSPARENT);

    // 대화창 테두리 펜 생성
    HPEN boxPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(hDC, boxPen);

    // 대화창을 그린다.
    RoundRect(
        hDC,
        dialogue_box.left,
        dialogue_box.top,
        dialogue_box.right,
        dialogue_box.bottom,
        20,
        20
    );

    // 결과 문구를 출력한다.
    SetTextColor(hDC, RGB(0, 0, 0));
    DrawTextW(
        hDC,
        resultText.c_str(),
        -1,
        &resultRect1,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );
    SetTextColor(hDC, RGB(0, 0, 0));
    DrawTextW(
        hDC,
        affectionUpText.c_str(),
        -1,
        &resultRect2,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );
    SetTextColor(hDC, RGB(0, 0, 0));
    DrawTextW(
        hDC,
        currentAffectionText.c_str(),
        -1,
        &resultRect3,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );

    // GDI 객체 복구
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldFont);

    // GDI 객체 삭제
    DeleteObject(boxPen);
    DeleteObject(dialogueFont);
}

void GameManager::Render(HDC hDC)
{
    if (dialogues.empty())
    {
        return;
    }
    switch (now_game_mode) {
        case game_mode_info::Dialogue: {
            RenderDialogue(hDC);
            break;
        }
        case game_mode_info::Choice: {
            RenderChoice(hDC);
            break;
        }
        case game_mode_info::Result: {
            RenderResult(hDC);
            break;
        }
    }
}

