#include "GameManager.h"

void GameManager::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    now_game_mode = game_mode_info::Dialogue;
    characters = {
        CharacterInfo{ L"한세아", 0 },
        CharacterInfo{ L"유하린", 0 },
        CharacterInfo{ L"서이린", 0 }
    };
    storyScene.Initialize();
    choiceScene.Initialize();
    return ;
}

void GameManager::Shutdown()
{
    storyScene.Shutdown();

    m_hWnd = nullptr;
}

void GameManager::OnMouseClick(int x, int y)
{
    switch (now_game_mode)
    {
        case game_mode_info::Dialogue:
        {
            storyScene.OnMouseClick(x, y);

            // 대사 끝나면 선택으로 이동
            if (storyScene.IsFinished())
            {
                // 선택지 화면에 들어가기 전에 이전 선택 상태를 초기화한다.
                choiceScene.Reset();

                // 선택지 모드로 이동한다.
                now_game_mode = game_mode_info::Choice;
            }

            break;
        }

        case game_mode_info::Choice:
        {
            // 선택지 클릭 판정은 ChoiceScene에게 맡긴다.
            choiceScene.HandleChoiceClick(x, y);

            // 실제로 선택지를 클릭했을 때만 결과 처리로 넘어간다.
            if (choiceScene.HasSelected())
            {
                int selectedIndex = choiceScene.GetSelectedIndex();

                // 선택한 캐릭터의 호감도를 증가시킨다.
                AddAffection(characters, selectedIndex);
            }

            break;
        }

        case game_mode_info::Result:
        {
            // 결과 모드 클릭 처리
            HandleResultClick();
            break;
        }
    }

    InvalidateRect(m_hWnd, nullptr, TRUE);
}

void GameManager::AddAffection(std::array<CharacterInfo, HEROINE_COUNT>& characters, int selectedcharacter) {
    characters[selectedcharacter].affection += choiceAffectionValue;

    if (characters[selectedcharacter].affection > 100)
    {
        characters[selectedcharacter].affection = 100;
    }

    now_game_mode = game_mode_info::Result;
}

void GameManager::HandleResultClick()
{
    // 다음 스토리로 이동
}

void GameManager::Render(HDC hDC)
{
    switch (now_game_mode)
    {
    case game_mode_info::Dialogue:
    {
        storyScene.Render(hDC);
        break;
    }

    case game_mode_info::Choice:
    {
        choiceScene.RenderChoice(hDC);
        break;
    }

    case game_mode_info::Result:
    {
        RenderResult(hDC);
        break;
    }
    }
}


void GameManager::RenderResult(HDC hDC)
{
    // 선택된 캐릭터가 없으면 출력하지 않는다.
    if (!choiceScene.HasSelected())
    {
        return;
    }

    int selectedIndex = choiceScene.GetSelectedIndex();

    std::wstring resultText =
        characters[selectedIndex].name + L"를 선택했습니다.";

    std::wstring affectionUpText =
        characters[selectedIndex].name +
        L" 호감도 +" +
        std::to_wstring(choiceAffectionValue);

    std::wstring currentAffectionText =
        L"현재 호감도: " +
        std::to_wstring(characters[selectedIndex].affection);

    // 결과 출력용 폰트를 만든다.
    HFONT resultFont = CreateFontW(
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

    // 만든 폰트를 적용하고 기존 폰트를 저장한다.
    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, resultFont));

    // 글자 배경을 투명하게 한다.
    SetBkMode(hDC, TRANSPARENT);

    HPEN boxPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, boxPen));

    // 결과창을 그린다.
    RoundRect(
        hDC,
        dialogueBox.left,
        dialogueBox.top,
        dialogueBox.right,
        dialogueBox.bottom,
        20,
        20
    );

    RECT resultRect1 = { 520, 860, 1600, 910 };
    RECT resultRect2 = { 520, 915, 1600, 965 };
    RECT resultRect3 = { 520, 970, 1600, 1020 };

    SetTextColor(hDC, RGB(0, 0, 0));

    // 선택 결과를 출력한다.
    DrawTextW(
        hDC,
        resultText.c_str(),
        -1,
        &resultRect1,
        DT_LEFT | DT_TOP | DT_SINGLELINE
    );

    // 호감도 증가량을 출력한다.
    DrawTextW(
        hDC,
        affectionUpText.c_str(),
        -1,
        &resultRect2,
        DT_LEFT | DT_TOP | DT_SINGLELINE
    );

    // 현재 호감도를 출력한다.
    DrawTextW(
        hDC,
        currentAffectionText.c_str(),
        -1,
        &resultRect3,
        DT_LEFT | DT_TOP | DT_SINGLELINE
    );
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldFont);
    DeleteObject(boxPen);
    DeleteObject(resultFont);
}