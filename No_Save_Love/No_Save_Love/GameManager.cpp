#include "GameManager.h"
#include "AudioManager.h"

#pragma comment(lib, "Msimg32.lib")

AudioManager audioManager;

namespace
{
    // ShowCursor는 내부 카운터 방식이라 여러 번 숨기면 한 번 보이기로 복구되지 않는다.
    void ForceSystemCursorVisible()
    {
        while (ShowCursor(TRUE) < 0)
        {
        }
    }

    void ForceSystemCursorHidden()
    {
        ForceSystemCursorVisible();
        while (ShowCursor(FALSE) >= 0)
        {
        }
    }
}

void GameManager::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;

    audioManager.Initialize();
    audioManager.RegisterBgm(L"title", L"resource\\sound\\title.mp3");
    audioManager.RegisterBgm(L"minigame1", L"resource\\sound\\minigame1.mp3");
    audioManager.RegisterBgm(L"minigame3", L"resource\\sound\\minigame3.mp3");
    audioManager.RegisterBgm(L"minigame4", L"resource\\sound\\minigame4.mp3");
    audioManager.RegisterBgm(L"happyending", L"resource\\sound\\ending\\happy_ending.mp3");
    audioManager.RegisterBgm(L"badending", L"resource\\sound\\ending\\bad_ending.mp3");
    audioManager.RegisterBgm(L"hiddenending", L"resource\\sound\\ending\\hidden_ending.mp3");
    audioManager.RegisterSfx(L"click", L"resource\\sound\\Click.wav");

    // 히로인 선택 횟수를 초기화한다.
    characters = {
        CharacterInfo{ L"한세아", 0 },
        CharacterInfo{ L"유하린", 0 },
        CharacterInfo{ L"서이린", 0 }
    };

    // 플레이어 스탯을 초기화한다.
    player = { 0, 0, 0, 0 };

    // 스토리 데이터를 먼저 준비한다.
    storyData.Initialize();

    // 각 Scene을 초기화한다.
    nameinputScene.Initialize();
    storyScene.Initialize();
    choiceScene.Initialize();
    resultScene.Reset();
    titleScene.Initialize();
    endingScene.Initialize(storyData.GetEndingCredits());

    // 처음 시작 모드는 대사 모드다.
    RequestSceneChange(game_mode_info::Title);


}
void GameManager::Shutdown()
{
    audioManager.Shutdown();

    ReleasePendingMiniGame();

    if (m_isMiniGame2Initialized)
    {
        ForceSystemCursorVisible();
        minigame2.Release();
        m_isMiniGame2Initialized = false;
    }
    minigame3.Release();

    // StoryScene 내부 데이터 정리
    storyScene.Shutdown();
    titleScene.Shutdown();
    choiceScene.Shutdown();
    endingScene.Shutdown();
    m_hWnd = nullptr;
}

void GameManager::OnMouseClick(int x, int y)
{
    if (m_exitConfirmOpen)
    {
        if (IsPointInsideRect(m_exitYesButtonRect, x, y))
        {
            audioManager.PlaySfx(L"click");
            PostMessage(m_hWnd, WM_CLOSE, 0, 0);
            return;
        }

        if (IsPointInsideRect(m_exitNoButtonRect, x, y))
        {
            audioManager.PlaySfx(L"click");
            m_exitConfirmOpen = false;
            InvalidateRect(m_hWnd, nullptr, FALSE);
            return;
        }

        return;
    }

    if (sceneTransition.IsActive()) // 페이드 인 아웃시 클릭 금지
    {
        return;
    }

    switch (now_game_mode)
    {
    case game_mode_info::Title: 
    {
        titleScene.OnMouseClick(x, y);

        if (titleScene.IsStartClicked()) {
            audioManager.PlaySfx(L"click");

            storyScene.SetDialogues(storyData.GetIntroStory());

            RequestSceneChange(game_mode_info::NameInput);
        }
        break;
    }
    case game_mode_info::NameInput:
    {
        nameinputScene.OnMouseClick(x, y);

        // 확인 버튼을 눌러 이름 입력이 끝났다면
        if (nameinputScene.IsFinished())
        {
            audioManager.PlaySfx(L"click");

            // 입력된 이름을 GameManager에 저장한다.
            playerName = nameinputScene.GetPlayerName();

            // StoryScene에게도 플레이어 이름을 알려준다.
            // 이걸 해야 StoryScene 내부 m_playerName이 "윤서"에서 입력한 이름으로 바뀐다.
            storyScene.SetPlayerName(playerName);
            resultScene.SetPlayerName(playerName);

            // 인트로 대사를 StoryScene에 넣는다.
            storyScene.SetDialogues(storyData.GetIntroStory());

            // 대사 화면으로 이동한다.
            RequestSceneChange(game_mode_info::Story);

        }

        break;
    }
    case game_mode_info::Story:
    {
        bool isSkipClicked = storyScene.IsClickSkipButton(x, y);

        storyScene.OnMouseClick(x, y);

        if (storyScene.IsFinished())
        {
            if (isSkipClicked)
            {
                audioManager.PlaySfx(L"click");
            }

            if (currentMiniGameIndex < 4)
            {
                // 현재 회차 번호에 맞는 미니게임 튜토리얼로 이동한다.
                // 지금은 1~4번 슬롯 모두 미니게임 1 튜토리얼을 임시로 사용한다.
                EnterCurrentMiniGameTutorial();
            }
            else if (!m_finalChoiceIntroShown)
            {
                // 마지막 선택 화면으로 가기 전에 노트가 마지막 이름을 보여주는 대사를 출력한다.
                EnterFinalChoice();
                storyScene.SetDialogues(storyData.GetFinalChoiceIntroStory(finalChoiceIntroIndex));
                m_finalChoiceIntroShown = true;
            }
            else
            {
                // 최종 확인 화면 없이 바로 엔딩 대사로 들어간다.
                EnterEndingStory();
            }
        }

        break;
    }

    case game_mode_info::MiniGameTutor1: 
    case game_mode_info::MiniGameTutor2:
    case game_mode_info::MiniGameTutor3:
    case game_mode_info::MiniGameTutor4:
    {
        minigam1_tutorial1.OnMouseClick(x, y);
        if (minigam1_tutorial1.IsFinished())
        {
            audioManager.PlaySfx(L"click");

            minigam1_tutorial1.Shutdown();
            StartCurrentMiniGame();
        }
        break;
    }
    case game_mode_info::MiniGame1: 
    case game_mode_info::MiniGame2:
    case game_mode_info::MiniGame3:
    case game_mode_info::MiniGame4:
    {
        HandleCurrentMiniGameMouse(x, y);
        FinishCurrentMiniGameIfNeeded();
        break;
    }
    case game_mode_info::Result:
    {
        resultScene.OnMouseClick(x, y);

        if (resultScene.IsFinished())
        {
            audioManager.PlaySfx(L"click");

            // ChoiceScene은 화면 표시용으로 현재 플레이어 스탯만 전달받는다.
            choiceScene.SetPlayerState(player);
            choiceScene.Reset();

            // Choice에 들어갈 때마다 중복 반영 방지 플래그를 초기화한다.
            m_choiceApplied = false;

            RequestSceneChange(game_mode_info::Choice);

        }

        break;
    }

    case game_mode_info::Choice:
    {
        // 선택지 클릭 판정을 ChoiceScene에게 맡긴다.
        choiceScene.HandleChoiceClick(x, y);

        // 아직 선택 결과를 반영하지 않았고, 실제 선택이 발생했을 때만 처리한다.
        if (!m_choiceApplied && choiceScene.HasSelected())
        {
            int selectedIndex = choiceScene.GetSelectedIndex();

            // 선택한 히로인의 선택 횟수를 증가시킨다.
            AddChoiceCount(characters, selectedIndex);

            // 분기 스토리 진입에 성공했을 때만 선택 반영 완료 처리한다.
            if (EnterBranchStory(selectedIndex))
            {
                audioManager.PlaySfx(L"click");
                m_choiceApplied = true;
            }
        }

        break;
    }
    case game_mode_info::FinalChoice:
    {
        // FinalChoiceScene에게 클릭 처리를 맡긴다.
        finalChoiceScene.OnMouseClick(x, y);

        // 최종 히로인 확인 화면이 끝났다면,
        // 바로 THE END 화면으로 가지 않고 엔딩 대사로 들어간다.
        if (finalChoiceScene.IsFinished())
        {
            EnterEndingStory();
        }

        break;
    }

    case game_mode_info::EndingDialogue:
    {
        // 엔딩 대사도 StoryScene으로 출력하므로 클릭 처리는 StoryScene에게 맡긴다.
        bool isSkipClicked = storyScene.IsClickSkipButton(x, y);

        storyScene.OnMouseClick(x, y);

        // 엔딩 대사가 모두 끝나면 THE END 화면으로 이동한다.
        if (storyScene.IsFinished())
        {
            if (isSkipClicked)
            {
                audioManager.PlaySfx(L"click");
            }

            endingScene.SetEndingImage(CalculateEndingType(finalHeroineIndex), finalHeroineIndex);
            endingScene.Reset();
            RequestSceneChange(game_mode_info::Ending);

        }

        break;
    }


    case game_mode_info::Ending:
    {
        endingScene.OnMouseClick(x, y);

        if (endingScene.IsExitRequested())
        {
            audioManager.PlaySfx(L"click");
            PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        }

        break;
    }
    }

    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void GameManager::EnterResult(int whichGame, int score)
{
    resultScene.Reset();

    resultScene.SetResult(whichGame, score);

    Player_state plusState = resultScene.GetPlusState();

    ApplyStatGain(plusState);

    resultScene.SetCurrentPlayerState(player);

    RequestSceneChange(game_mode_info::Result);

}

void GameManager::EnterCurrentMiniGameTutorial()
{
    // 지금은 1~4번 미니게임 튜토리얼 모두 미니게임 1 튜토리얼 화면을 임시로 사용한다.
    // 나중에 MiniGame2TutorialScene 같은 클래스가 생기면 여기에서 번호별로 바꾸면 된다.
    minigam1_tutorial1.Initialize();
    RequestSceneChange(GetTutorialModeByIndex(currentMiniGameIndex));
}

void GameManager::StartCurrentMiniGame()
{
    if (currentMiniGameIndex != 1 && m_isMiniGame2Initialized)
    {
        ForceSystemCursorVisible();
        minigame2.Release();
        m_isMiniGame2Initialized = false;
    }

    if (currentMiniGameIndex == 1)
    {
        // 미니게임 2는 리듬게임을 사용한다.
        minigame2.Release();
        minigame2.Init(m_hWnd);
        ForceSystemCursorHidden();
        m_isMiniGame2Initialized = true;
    }
    else if (currentMiniGameIndex == 3)
    {
        // 미니게임 4는 avoidgame을 사용한다.
        minigame4.Initialize();
    }
    else if (currentMiniGameIndex == 2)
    {
        // 미니게임 3은 윤서의 등교길을 사용한다.
        minigame3.Release();
        minigame3.Initialize();
    }
    else
    {
        // 미니게임 1은 PCroomgame을 사용한다.
        minigame1.Release();
        minigame1.Init();
    }

    RequestSceneChange(GetMiniGameModeByIndex(currentMiniGameIndex));
}

void GameManager::DebugEnterMiniGameByIndex(int miniGameIndex)
{
    // 잘못된 번호가 들어오면 아무것도 하지 않는다.
    if (miniGameIndex < 0 || miniGameIndex >= 4)
    {
        return;
    }

    // F1~F4는 앞부분 진행을 건너뛰고 원하는 미니게임 슬롯으로 바로 들어간다.
    currentMiniGameIndex = miniGameIndex;
    StartCurrentMiniGame();
}

GameManager::game_mode_info GameManager::GetTutorialModeByIndex(int miniGameIndex) const
{
    // 미니게임 번호를 튜토리얼 Scene 모드로 변환한다.
    switch (miniGameIndex)
    {
    case 0:
        return game_mode_info::MiniGameTutor1;

    case 1:
        return game_mode_info::MiniGameTutor2;

    case 2:
        return game_mode_info::MiniGameTutor3;

    case 3:
        return game_mode_info::MiniGameTutor4;

    default:
        return game_mode_info::MiniGameTutor1;
    }
}

GameManager::game_mode_info GameManager::GetMiniGameModeByIndex(int miniGameIndex) const
{
    // 미니게임 번호를 실제 게임 Scene 모드로 변환한다.
    switch (miniGameIndex)
    {
    case 0:
        return game_mode_info::MiniGame1;

    case 1:
        return game_mode_info::MiniGame2;

    case 2:
        return game_mode_info::MiniGame3;

    case 3:
        return game_mode_info::MiniGame4;

    default:
        return game_mode_info::MiniGame1;
    }
}

void GameManager::HandleCurrentMiniGameMouse(int x, int y)
{
    // 미니게임 4는 아직 마우스 입력을 사용하지 않는다.
    // 현재 실행 화면 기준으로 판단해야 Result 전환 페이드 중에도 다른 미니게임으로 잘못 빠지지 않는다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2) {
        minigame2.OnMouseDown(x, y);
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame3)
    {
        return;
    }
    // 미니게임 1의 마우스 처리를 사용한다.
    minigame1.MOUSE(x, y);
}

void GameManager::HandleCurrentMiniGameKey(wchar_t inputChar)
{
    // 미니게임 4는 방향키를 WM_KEYDOWN/WM_KEYUP에서 따로 처리한다.
    // 현재 실행 화면 기준으로 판단해야 Result 전환 페이드 중에도 다른 미니게임으로 잘못 빠지지 않는다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        return;
    }

    if (now_game_mode == game_mode_info::MiniGame2)
    {
        return;
    }

    if (now_game_mode == game_mode_info::MiniGame3)
    {
        return;
    }

    // 미니게임 1의 키 입력 처리를 사용한다.
    minigame1.KEYDOWN(inputChar);
}

void GameManager::UpdateCurrentMiniGame()
{
    // 미니게임 4는 avoidgame의 Update를 사용한다.
    // 현재 실행 화면 기준으로 판단해야 currentMiniGameIndex가 증가한 뒤에도 안전하다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        minigame4.Update();
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2) {
        minigame2.Update();
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame3)
    {
        minigame3.Update();
        return;
    }

    // 미니게임 1의 Update를 사용한다.
    minigame1.Update();
}

void GameManager::RenderCurrentMiniGame(HDC hDC)
{
    // 미니게임 4는 avoidgame의 Render를 사용한다.
    // avoidgame 종료 직후 currentMiniGameIndex가 4로 증가해도,
    // 페이드가 끝나기 전 화면은 아직 MiniGame4이므로 now_game_mode로 판단한다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        minigame4.Render(hDC);
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2) {
        minigame2.Render(hDC);
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame3)
    {
        minigame3.Render(hDC);
        return;
    }
    // 미니게임 1의 화면 출력을 사용한다.
    minigame1.PAINT(hDC);
}

void GameManager::FinishCurrentMiniGameIfNeeded()
{
    // 미니게임 4는 avoidgame의 종료 여부와 점수를 사용한다.
    // 결과 처리도 현재 실행 화면 기준으로 판단한다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        if (!minigame4.IsFinished())
        {
            return;
        }

        EnterResult(3, minigame4.GetScore());
        currentMiniGameIndex++;
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2)
    {
        if (!minigame2.IsGameOver())
        {
            return;
        }

        EnterResult(1, minigame2.GetResultScore100());
        ForceSystemCursorVisible();
        m_pendingMiniGameRelease = PendingMiniGameRelease2;
        currentMiniGameIndex++;
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame3)
    {
        if (!minigame3.IsFinished())
        {
            return;
        }

        EnterResult(2, minigame3.GetScore());
        m_pendingMiniGameRelease = PendingMiniGameRelease3;
        currentMiniGameIndex++;
        return;
    }
    // 임시 연결된 미니게임 1이 끝나지 않았으면 Result로 이동하지 않는다.
    if (!minigame1.isfinished())
    {
        return;
    }

    int rawScore = minigame1.getscore();
    if (rawScore <= 0)
    {
        rawScore = 0;
    }

    // 현재 PCroomgame 점수를 100점 만점으로 환산한다.
    // 실제 미니게임 2, 3, 4는 각 게임의 점수 기준에 맞게 여기에서 분기하면 된다.
    int convertedScore = rawScore * 100 / 2000;

    if (convertedScore > 100)
    {
        convertedScore = 100;
    }

    // currentMiniGameIndex는 ResultScene에 몇 번째 미니게임인지 알려주는 값이다.
    EnterResult(currentMiniGameIndex, convertedScore);
    m_pendingMiniGameRelease = PendingMiniGameRelease1;

    // Result 이후 Choice와 Story를 거쳐 다음 미니게임 슬롯으로 넘어가게 한다.
    currentMiniGameIndex++;
}

void GameManager::ReleasePendingMiniGame()
{
    switch (m_pendingMiniGameRelease)
    {
    case PendingMiniGameRelease1:
        minigame1.Release();
        break;

    case PendingMiniGameRelease2:
        if (m_isMiniGame2Initialized)
        {
            ForceSystemCursorVisible();
            minigame2.Release();
            m_isMiniGame2Initialized = false;
        }
        break;

    case PendingMiniGameRelease3:
        minigame3.Release();
        break;

    case PendingMiniGameReleaseNone:
    default:
        break;
    }

    m_pendingMiniGameRelease = PendingMiniGameReleaseNone;
}

void GameManager::ApplyStatGain(const Player_state& plusState)
{
    // ResultScene이 계산한 상승량을 실제 player 스탯에 더한다.
    player.money = ClampStat(player.money + plusState.money);
    player.speech = ClampStat(player.speech + plusState.speech);
    player.charm = ClampStat(player.charm + plusState.charm);
    player.appearance = ClampStat(player.appearance + plusState.appearance);
}

void GameManager::AddChoiceCount(std::array<CharacterInfo, HEROINE_COUNT>& characters, int selectedCharacter)
{
    // 잘못된 인덱스가 들어오면 아무것도 하지 않는다.
    if (selectedCharacter < 0 || selectedCharacter >= HEROINE_COUNT)
    {
        return;
    }

    characters[selectedCharacter].choice_time += 1;
}



void GameManager::Render(HDC hDC)
{
    switch (now_game_mode)
    {
    case game_mode_info::Title:
    {
        titleScene.Render(hDC);
        break;
    }
    case game_mode_info::NameInput: {
        nameinputScene.Render(hDC);
        break;
    }
    case game_mode_info::Story:
    {
        storyScene.Render(hDC);
        break;
    }
    case game_mode_info::MiniGameTutor1:
    case game_mode_info::MiniGameTutor2:
    case game_mode_info::MiniGameTutor3:
    case game_mode_info::MiniGameTutor4:
    {
        minigam1_tutorial1.Render(hDC);
        break;
    }
    case game_mode_info::MiniGame1:
    case game_mode_info::MiniGame2:
    case game_mode_info::MiniGame3:
    case game_mode_info::MiniGame4:
    {
        RenderCurrentMiniGame(hDC);
        break;
    }

    case game_mode_info::Result:
    {
        resultScene.RenderResult(hDC);
        break;
    }

    case game_mode_info::Choice:
    {
        choiceScene.RenderChoice(hDC);
        break;
    }
    case game_mode_info::FinalChoice:
    {
        // 최종 히로인 확인 화면을 출력한다.
        finalChoiceScene.Render(hDC);
        break;
    }
    case game_mode_info::EndingDialogue:
    {
        // 엔딩 대사도 StoryScene을 재사용해서 출력한다.
        storyScene.Render(hDC);
        break;
    }

    case game_mode_info::Ending:
    {
        endingScene.Render(hDC);
        break;
    }
    }
    sceneTransition.Render(hDC, 1920, 1080); // 페이드 인 아웃 덮기

    if (m_exitConfirmOpen)
    {
        RenderExitConfirmPopup(hDC);
    }
}

void GameManager::RenderExitConfirmPopup(HDC hDC)
{
    // 어두운 덮개
    HDC overlayDC = CreateCompatibleDC(hDC);
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
    HBITMAP oldOverlayBitmap = static_cast<HBITMAP>(SelectObject(overlayDC, overlayBitmap));

    RECT screenRect = { 0, 0, 1920, 1080 };
    HBRUSH overlayBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(overlayDC, &screenRect, overlayBrush);

    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 120;
    AlphaBlend(hDC, 0, 0, 1920, 1080, overlayDC, 0, 0, 1920, 1080, blend);

    DeleteObject(overlayBrush);
    SelectObject(overlayDC, oldOverlayBitmap);
    DeleteObject(overlayBitmap);
    DeleteDC(overlayDC);

    // 팝업 본체
    HBRUSH popupBrush = CreateSolidBrush(RGB(22, 22, 30));
    HPEN popupPen = CreatePen(PS_SOLID, 3, RGB(230, 210, 255));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, popupBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, popupPen));

    RoundRect(
        hDC,
        m_exitPopupRect.left,
        m_exitPopupRect.top,
        m_exitPopupRect.right,
        m_exitPopupRect.bottom,
        28,
        28
    );

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(popupBrush);
    DeleteObject(popupPen);

    HFONT titleFont = CreateFontW(
        42,
        0,
        0,
        0,
        FW_BOLD,
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, titleFont));
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(245, 245, 250));

    RECT titleRect = { m_exitPopupRect.left, m_exitPopupRect.top + 70, m_exitPopupRect.right, m_exitPopupRect.top + 135 };
    DrawTextW(hDC, L"프로그램을 종료하시겠습니까?", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hDC, oldFont);
    DeleteObject(titleFont);

    HFONT subFont = CreateFontW(
        26,
        0,
        0,
        0,
        FW_NORMAL,
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

    oldFont = static_cast<HFONT>(SelectObject(hDC, subFont));
    SetTextColor(hDC, RGB(205, 195, 220));

    RECT subRect = { m_exitPopupRect.left, m_exitPopupRect.top + 145, m_exitPopupRect.right, m_exitPopupRect.top + 195 };
    DrawTextW(hDC, L"No를 누르면 현재 화면으로 돌아갑니다.", -1, &subRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hDC, oldFont);
    DeleteObject(subFont);

    DrawExitConfirmButton(hDC, m_exitYesButtonRect, L"YES");
    DrawExitConfirmButton(hDC, m_exitNoButtonRect, L"NO");
}

void GameManager::DrawExitConfirmButton(HDC hDC, const RECT& rect, const wchar_t* text)
{
    // StoryScene SKIP 버튼 스타일
    HBRUSH buttonBrush = CreateSolidBrush(RGB(35, 35, 45));
    HPEN buttonPen = CreatePen(PS_SOLID, 2, RGB(230, 210, 255));

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, buttonBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, buttonPen));

    RoundRect(hDC, rect.left, rect.top, rect.right, rect.bottom, 18, 18);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(buttonBrush);
    DeleteObject(buttonPen);

    HFONT buttonFont = CreateFontW(
        28,
        0,
        0,
        0,
        FW_BOLD,
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, buttonFont));
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(245, 245, 250));
    DrawTextW(hDC, text, -1, const_cast<RECT*>(&rect), DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hDC, oldFont);
    DeleteObject(buttonFont);
}

bool GameManager::IsPointInsideRect(const RECT& rect, int x, int y) const
{
    POINT point = { x, y };
    return PtInRect(&rect, point);
}

bool GameManager::EnterBranchStory(int selectedHeroineIndex)
{
    // 히로인 번호가 잘못되면 실패 처리한다.
    if (selectedHeroineIndex < 0 || selectedHeroineIndex >= HEROINE_COUNT)
    {
        return false;
    }

    // 4회차를 모두 끝냈다면 엔딩으로 이동한다.
    if (currentStoryRound >= STORY_ROUND_COUNT)
    {
        endingScene.SetEndingImage(CalculateEndingType(finalHeroineIndex), finalHeroineIndex);
        endingScene.Reset();
        RequestSceneChange(game_mode_info::Ending);

        return true;
    }

    // 현재 회차와 선택 히로인에 맞는 대사 묶음을 가져온다.
    const std::vector<DialogueLineInfo>& nextDialogues =
        storyData.GetBranchStory(currentStoryRound, selectedHeroineIndex);

    // 대사가 비어 있으면 실패 처리한다.
    if (nextDialogues.empty())
    {
        return false;
    }

    // StoryScene에 새 대사 묶음을 넣는다.
    storyScene.SetDialogues(nextDialogues);

    // 다음 회차로 이동한다.
    currentStoryRound++;

    // 대사 모드로 전환한다.
    RequestSceneChange(game_mode_info::Story);


    return true;
}

void GameManager::EnterFinalChoice()
{
    // 현재 가장 높은 선택 횟수를 저장한다.
    int maxChoiceCount = characters[0].choice_time;

    // 가장 많이 선택된 히로인 번호를 저장한다.
    finalHeroineIndex = 0;

    // 현재 가장 높은 선택 횟수를 저장한다.
    maxChoiceCount = characters[0].choice_time;

    // 1번 유하린, 2번 서이린과 비교한다.
    for (int i = 1; i < HEROINE_COUNT; i++)
    {
        // 현재 히로인의 선택 횟수가 더 많으면 최종 히로인을 갱신한다.
        if (characters[i].choice_time > maxChoiceCount)
        {
            maxChoiceCount = characters[i].choice_time;
            finalHeroineIndex = i;
        }
    }

    // 최종 히로인 정보를 FinalChoiceScene에게 넘긴다.
    finalChoiceScene.SetFinalHeroine(
        finalHeroineIndex,
        characters[finalHeroineIndex].name
    );

    // 스탯이 너무 낮으면 히로인 대신 새누 히든 엔딩으로 진입한다.
    if (CalculateEndingType(finalHeroineIndex) == 2)
    {
        finalChoiceIntroIndex = 3;
        finalChoiceScene.SetFinalHeroine(-1, L"새누");
    }
    else
    {
        finalChoiceIntroIndex = finalHeroineIndex;
    }

    // FinalChoiceScene을 처음 상태로 준비한다.
    finalChoiceScene.Reset();

}

void GameManager::EnterEndingStory()
{

    // 스탯을 보고 해피 / 배드 / 히든 엔딩을 계산한다.
    int endingType = CalculateEndingType(finalHeroineIndex);

    // 히든 엔딩은 새누 전용 대사로 고정한다.
    int endingHeroineIndex = finalHeroineIndex;
    if (endingType == 2)
    {
        endingHeroineIndex = 0;
    }

    // StoryData에서 해당 히로인, 해당 엔딩 종류의 대사를 가져온다.
    const std::vector<DialogueLineInfo>& endingDialogues =
        storyData.GetEndingStory(endingType, endingHeroineIndex);

    endingScene.SetEndingImage(endingType, finalHeroineIndex);

    // StoryScene에서는 실제 대사만 출력하고, 엔딩 CG 설명 줄은 숨긴다.
    std::vector<DialogueLineInfo> endingDialoguesForStory;
    for (int i = 0; i < static_cast<int>(endingDialogues.size()); i++)
    {
        if (endingDialogues[i].speaker == L"엔딩 CG")
        {
            continue;
        }

        endingDialoguesForStory.push_back(endingDialogues[i]);
    }

    // 엔딩 대사를 StoryScene에 넣는다.
    storyScene.SetDialogues(endingDialoguesForStory);
    storyScene.SetEndingIllustration(endingType, finalHeroineIndex);

    // 이제 StoryScene으로 엔딩 대사를 출력한다.
    RequestSceneChange(game_mode_info::EndingDialogue);

}

int GameManager::CalculateEndingType(int heroineIndex) const
{
    // 0 = Happy
    // 1 = Bad
    // 2 = Hidden

    // 전체 스탯이 낮으면 새누 히든 엔딩으로 들어간다.
    int totalStat =
        player.money +
        player.speech +
        player.charm +
        player.appearance;

    if (totalStat <= 80)
    {
        return 2;
    }

    switch (heroineIndex)
    {
    case 0:
    {
        // 한세아: 재력 + 대화스킬 중심
        if (player.money + player.speech >= 120)
        {
            return 0;
        }

        return 1;
    }

    case 1:
    {
        // 유하린: 대화스킬 + 매력 중심
        if (player.speech + player.charm >= 120)
        {
            return 0;
        }

        return 1;
    }

    case 2:
    {
        // 서이린: 매력 + 외모 중심
        if (player.charm + player.appearance >= 120)
        {
            return 0;
        }

        return 1;
    }

    default:
    {
        // 잘못된 히로인 번호면 배드 엔딩으로 처리한다.
        return 1;
    }
    }

}
void GameManager::OnChar(wchar_t inputChar)
{
    if (m_exitConfirmOpen)
    {
        return;
    }

    switch (now_game_mode)
    {
        case game_mode_info::NameInput:
        {
            // 이름 입력 화면일 때만 문자 입력을 전달한다.
            nameinputScene.OnChar(inputChar);

            // 엔터로 입력이 끝났다면 다음 장면으로 이동한다.
            if (nameinputScene.IsFinished())
            {
                playerName = nameinputScene.GetPlayerName();

                storyScene.SetPlayerName(playerName);
                resultScene.SetPlayerName(playerName);

                storyScene.SetDialogues(storyData.GetIntroStory());

                RequestSceneChange(game_mode_info::Story);

            }

            break;
        }
        
        case game_mode_info::MiniGame1:
        case game_mode_info::MiniGame2:
        case game_mode_info::MiniGame3:
        case game_mode_info::MiniGame4:
        {
            HandleCurrentMiniGameKey(inputChar);
            break;
        }

        default:
        {
            break;
        }
    }

}

void GameManager::OnKeyDown(WPARAM wParam)
{
    // 페이드 전환 중에는 중복 Scene 이동을 막는다.
    if (sceneTransition.IsActive())
    {
        return;
    }

    if (m_exitConfirmOpen)
    {
        if (wParam == VK_ESCAPE)
        {
            m_exitConfirmOpen = false;
            InvalidateRect(m_hWnd, nullptr, FALSE);
        }
        return;
    }

    if (wParam == VK_ESCAPE)
    {
        if (now_game_mode == game_mode_info::MiniGame4)
        {
            minigame4.OnKeyUp(VK_LEFT);
            minigame4.OnKeyUp(VK_RIGHT);
            minigame4.OnKeyUp(VK_DOWN);
            minigame4.OnKeyUp(VK_UP);
            minigame4.OnKeyUp(VK_SPACE);
        }
        else if (now_game_mode == game_mode_info::MiniGame3)
        {
            minigame3.OnKeyUp(VK_DOWN);
            minigame3.OnKeyUp(VK_SPACE);
        }

        m_exitConfirmOpen = true;
        InvalidateRect(m_hWnd, nullptr, FALSE);
        return;
    }

    // F1~F4는 테스트용 바로가기다.
    // F1 = 미니게임 1, F2 = 미니게임 2, F3 = 미니게임 3, F4 = 미니게임 4.
    switch (wParam)
    {
    case VK_F1:
        DebugEnterMiniGameByIndex(0);
        return;

    case VK_F2:
        DebugEnterMiniGameByIndex(1);
        return;

    case VK_F3:
        DebugEnterMiniGameByIndex(2);
        return;

    case VK_F4:
        DebugEnterMiniGameByIndex(3);
        return;

    default:
        break;
    }

    // 미니게임 4는 방향키를 사용하므로 특수 키 입력을 직접 전달한다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        minigame4.OnKeyDown(wParam);
    }
    else if (now_game_mode == game_mode_info::MiniGame3)
    {
        minigame3.OnKeyDown(wParam);
    }
}

void GameManager::OnKeyUp(WPARAM wParam)
{
    if (m_exitConfirmOpen)
    {
        return;
    }

    // 페이드 전환 중에는 입력 상태를 바꾸지 않는다.
    if (sceneTransition.IsActive())
    {
        return;
    }

    // 미니게임 4는 키를 누르는 동안 계속 이동하므로, 키를 뗄 때도 전달해야 한다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        minigame4.OnKeyUp(wParam);

    }
    else if (now_game_mode == game_mode_info::MiniGame3)
    {
        minigame3.OnKeyUp(wParam);
    }
}

void GameManager::OnMouseUp(int x, int y) {
    if (now_game_mode == game_mode_info::MiniGame2) {
        minigame2.OnMouseUp(x, y);
        return;
    }
}

void GameManager::OnMouseMove(int x, int y) {
    if (now_game_mode == game_mode_info::MiniGame2) {
        minigame2.OnMouseMove(x, y);
        return;
    }
}

void GameManager::OnTimer(HWND hWnd)
{
    audioManager.Update();

    if (m_exitConfirmOpen)
    {
        InvalidateRect(hWnd, NULL, FALSE);
        return;
    }

    // 전역 Scene 전환 중이 아닐 때만 현재 Scene 내부 업데이트를 진행한다.
    if (!sceneTransition.IsActive())
    {
        switch (now_game_mode)
        {
        case game_mode_info::Story:
            storyScene.Update();
            break;

        case game_mode_info::EndingDialogue:
            storyScene.Update();
            break;

        case game_mode_info::MiniGame1:
        case game_mode_info::MiniGame2:
        case game_mode_info::MiniGame3:
        case game_mode_info::MiniGame4:
            UpdateCurrentMiniGame();
            FinishCurrentMiniGameIfNeeded();
            break;
        case game_mode_info::Ending:
            endingScene.Update();
            break;
        default:
            break;
        }
    }

    // 전역 페이드는 항상 업데이트한다.
    sceneTransition.Update();

    // 완전히 검은 화면이 되었으면 이 순간 실제 Scene을 변경한다.
    if (sceneTransition.IsBlack())
    {
        ApplyPendingSceneChange();
    }

    // 다시 그리기 요청
    InvalidateRect(hWnd, NULL, FALSE);
}

void GameManager::RequestSceneChange(game_mode_info nextMode )
{
    // 이미 전환 중이면 중복 요청을 막는다.
    if (sceneTransition.IsActive())
    {
        return;
    }

    // 다음에 이동할 모드를 저장한다.
    pendingGameMode = nextMode;



    // 예약된 Scene 변경이 있다고 표시한다.
    hasPendingSceneChange = true;

    // 전역 페이드 아웃 시작
    sceneTransition.Start();
}

void GameManager::ApplyPendingSceneChange()
{
    // 예약된 Scene 변경이 없으면 아무것도 하지 않는다.
    if (!hasPendingSceneChange)
    {
        return;
    }

    // 검은 화면이 된 순간 다음 Scene 준비 작업을 실행한다.
    if (pendingSceneSetup != nullptr)
    {
        pendingSceneSetup();
    }

    // 실제 모드를 변경한다.
    now_game_mode = pendingGameMode;

    if (now_game_mode == game_mode_info::Title)
    {
        audioManager.PlayBgmFadeIn(L"title", 50, 450, 5000);
    }
    else if (now_game_mode == game_mode_info::MiniGame1)
    {
        audioManager.PlayBgmFadeIn(L"minigame1", 80, 350, 2000);
    }
    else if (now_game_mode == game_mode_info::MiniGame3)
    {
        audioManager.PlayBgmFadeIn(L"minigame3", 80, 350, 2000);
    }
    else if (now_game_mode == game_mode_info::MiniGame4)
    {
        audioManager.PlayBgmFadeIn(L"minigame4", 80, 350, 2000);
    }
    else if (now_game_mode == game_mode_info::EndingDialogue ||
        now_game_mode == game_mode_info::Ending)
    {
        int endingType = CalculateEndingType(finalHeroineIndex);

        if (endingType == 0)
        {
            audioManager.PlayBgmFadeIn(L"happyending", 60, 330, 3000);
        }
        else if (endingType == 1)
        {
            audioManager.PlayBgmFadeIn(L"badending", 60, 330, 3000);
        }
        else
        {
            audioManager.PlayBgmFadeIn(L"hiddenending", 60, 330, 3000);
        }
    }
    else if (now_game_mode == game_mode_info::NameInput) {
        audioManager.FadeOutBgm(10000);
    }
    else if (audioManager.GetCurrentBgmKey() == L"minigame1")
    {
        audioManager.FadeOutBgm(1500);
    }
    else if (audioManager.GetCurrentBgmKey() == L"minigame3")
    {
        audioManager.FadeOutBgm(1500);
    }
    else if (audioManager.GetCurrentBgmKey() == L"minigame4")
    {
        audioManager.FadeOutBgm(1500);
    }
    else if (now_game_mode != game_mode_info::NameInput && audioManager.GetCurrentBgmKey() == L"title")
    {
        audioManager.StopBgm();
    }

    ReleasePendingMiniGame();

    // 예약 정보 초기화
    hasPendingSceneChange = false;
    pendingSceneSetup = nullptr;

    // 새 화면이 밝아지도록 FadeIn 시작
    sceneTransition.StartFadeIn();
}
