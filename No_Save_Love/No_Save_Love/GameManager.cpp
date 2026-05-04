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

    return true;
}

void GameManager::Shutdown()
{
    
    dialogues.clear();
    m_hWnd = nullptr;
}

void GameManager::OnMouseClick(int x, int y)
{
    switch (now_game_mode) {
        case game_mode_info::Dialogue: {
            if (m_currentDialogueIndex < static_cast<int>(dialogues.size()) - 1)
            {
                m_currentDialogueIndex++;
            }
            else {
                now_game_mode = game_mode_info::Choice;
            }
            break;
        }
        case game_mode_info::Choice: {

            POINT mousePoint = { x, y };
            if (PtInRect(&choiceHitBox[0], mousePoint)) // 한세아 선택
            {
                selectedCharacter = 0;
            }
            else if (PtInRect(&choiceHitBox[1], mousePoint)) // 유하린
            {
                selectedCharacter = 1;
            }
            else if (PtInRect(&choiceHitBox[2], mousePoint)) // 서이린
            {
                selectedCharacter = 2;
                
            }
            now_game_mode = game_mode_info::Result;
            break;
        }
        
    }
    
    

    InvalidateRect(m_hWnd, nullptr, TRUE);
}

void GameManager::Render(HDC hDC)
{
    if (dialogues.empty())
    {
        return;
    }
    switch (now_game_mode) {
        case game_mode_info::Dialogue: {
            SpeakerStyle font_style = {
            RGB(230, 220, 255),
            RGB(245, 245, 245),
            RGB(210, 190, 255),
            RGB(80, 70, 120)
            };

            if (dialogues[m_currentDialogueIndex].speaker == L"한세아")
            {
                font_style = hansea_font_style;
            }
            else if (dialogues[m_currentDialogueIndex].speaker == L"유하린")
            {
                font_style = yuharin_font_style;
            }
            else if (dialogues[m_currentDialogueIndex].speaker == L"서이린")
            {
                font_style = seoirin_font_style;
            }

            const std::wstring& current_speaker = dialogues[m_currentDialogueIndex].speaker;
            const std::wstring& current_text = dialogues[m_currentDialogueIndex].text;

            // 폰트 생성
            HFONT dialogueFont = CreateFontW(42,0, 0, 0, 600,FALSE,FALSE,FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_NATURAL_QUALITY,DEFAULT_PITCH | FF_DONTCARE,L"맑은 고딕");

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
            break;
        }
        case game_mode_info::Choice: {
            HBRUSH whiteBrush = CreateSolidBrush(RGB(245, 245, 245));
            HPEN bluePen = CreatePen(PS_SOLID, 5, RGB(70, 110, 230));

            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, whiteBrush);
            HPEN oldPen = (HPEN)SelectObject(hDC, bluePen);

            Polygon(hDC, shoiceBox[0], 4);
            Polygon(hDC, shoiceBox[1], 4);
            Polygon(hDC, shoiceBox[2], 4);

            SelectObject(hDC, oldBrush);
            SelectObject(hDC, oldPen);

            DeleteObject(whiteBrush);
            DeleteObject(bluePen);

            TextOut(hDC, 760, 245, L"한세아", lstrlen(L"한세아"));
            TextOut(hDC, 760, 445, L"유하린", lstrlen(L"유하린"));
            TextOut(hDC, 760, 690, L"서이린", lstrlen(L"서이린"));
            break;
        }
        case game_mode_info::Result: {

            switch (selectedCharacter) {
                case 0: {
                    HFONT dialogueFont = CreateFontW(42, 0, 0, 0, 600, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");
                    HFONT oldFont = (HFONT)SelectObject(hDC, dialogueFont);
                    HPEN boxPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
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
                    SetTextColor(hDC, RGB(0, 0, 0));
                    DrawTextW(
                        hDC,
                        L"한세아를 선택했습니다",
                        -1,
                        &text_rect,
                        DT_LEFT | DT_TOP | DT_WORDBREAK
                    );
                    DeleteObject(boxPen);
                    DeleteObject(dialogueFont);
                    break;
                }
                case 1: {
                    HFONT dialogueFont = CreateFontW(42, 0, 0, 0, 600, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");
                    HFONT oldFont = (HFONT)SelectObject(hDC, dialogueFont);
                    HPEN boxPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
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
                    SetTextColor(hDC, RGB(0, 0, 0));
                    DrawTextW(
                        hDC,
                        L"유하린을 선택했습니다",
                        -1,
                        &text_rect,
                        DT_LEFT | DT_TOP | DT_WORDBREAK
                    );
                    DeleteObject(boxPen);
                    DeleteObject(dialogueFont);
                    break;
                }
                case 2: {
                    HFONT dialogueFont = CreateFontW(42, 0, 0, 0, 600, FALSE, FALSE, FALSE, HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");
                    HFONT oldFont = (HFONT)SelectObject(hDC, dialogueFont);
                    HPEN boxPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
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
                    SetTextColor(hDC, RGB(0, 0, 0));
                    DrawTextW(
                        hDC,
                        L"서이린를 선택했습니다",
                        -1,
                        &text_rect,
                        DT_LEFT | DT_TOP | DT_WORDBREAK
                    );
                    DeleteObject(boxPen);
                    DeleteObject(dialogueFont);
                    break;
                }
            }
            break;
        }
    }
    
}

