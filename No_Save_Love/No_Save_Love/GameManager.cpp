#include "GameManager.h"
#include "AudioManager.h"
#include <algorithm>

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
    audioManager.RegisterBgm(L"story1", L"resource\\sound\\story_part1\\story1.mp3");
    audioManager.RegisterBgm(L"story2", L"resource\\sound\\story_part1\\story2.mp3");
    audioManager.RegisterBgm(L"story3", L"resource\\sound\\story_part1\\story3.mp3");
    audioManager.RegisterBgm(L"story4", L"resource\\sound\\story_part1\\story4.mp3");
    audioManager.RegisterBgm(L"story5", L"resource\\sound\\story_part1\\story5.mp3");
    audioManager.RegisterBgm(L"story2.1", L"resource\\sound\\story_part2\\story2.1.mp3");
    audioManager.RegisterBgm(L"story3.1", L"resource\\sound\\story_part3\\story3.mp3");
    audioManager.RegisterBgm(L"story4.1", L"resource\\sound\\story_part4\\story4.mp3");
    audioManager.RegisterBgm(L"story5.1", L"resource\\sound\\story_part5\\story5.mp3");
    audioManager.RegisterBgm(L"minigame1", L"resource\\sound\\minigame\\minigame1.mp3");
    audioManager.RegisterBgm(L"minigame3", L"resource\\sound\\minigame\\minigame3.mp3");
    audioManager.RegisterBgm(L"minigame4", L"resource\\sound\\minigame\\minigame4.mp3");
    audioManager.RegisterBgm(L"happyending", L"resource\\sound\\ending\\happy_ending.mp3");
    audioManager.RegisterBgm(L"badending", L"resource\\sound\\ending\\bad_ending.mp3");
    audioManager.RegisterBgm(L"hiddenending", L"resource\\sound\\ending\\hidden_ending.mp3");
    audioManager.RegisterSfx(L"click", L"resource\\sound\\Sfx\\Click.wav");
    audioManager.RegisterSfx(L"minigame1_success", L"resource\\minigame1\\sfx_success.wav");
    audioManager.RegisterSfx(L"minigame1_fail", L"resource\\minigame1\\sfx_fail.wav");
    audioManager.RegisterSfx(L"minigame1_water", L"resource\\minigame1\\sfx_water.wav");
    audioManager.RegisterSfx(L"minigame1_noodle_soup", L"resource\\minigame1\\sfx_noodle_soup.wav");
    audioManager.RegisterSfx(L"minigame1_topping", L"resource\\minigame1\\sfx_topping.wav");
    audioManager.RegisterSfx(L"minigame3_jump", L"resource\\minigame3\\sfx_jump.wav");
    audioManager.RegisterSfx(L"minigame3_slide", L"resource\\minigame3\\sfx_slide.wav");
    audioManager.RegisterSfx(L"minigame3_hit", L"resource\\minigame3\\sfx_hit.wav");
    audioManager.RegisterSfx(L"minigame3_jelly_collect", L"resource\\minigame3\\sfx_jelly_collect.wav");
    audioManager.SetSfxVolume(L"minigame3_jelly_collect", 350);
    audioManager.RegisterSfx(L"minigame4_lolpopup", L"resource\\minigame4\\sound\\lol.wav");
    audioManager.RegisterSfx(L"minigame4_malpa_rock", L"resource\\minigame4\\sound\\rock.wav");
    audioManager.RegisterSfx(L"minigame4_malpa_ult", L"resource\\minigame4\\sound\\ult.wav");
    audioManager.RegisterSfx(L"minigame4_rider", L"resource\\minigame4\\sound\\rider.wav");
    audioManager.RegisterSfx(L"minigame4_reels", L"resource\\minigame4\\sound\\reels.wav");
    audioManager.RegisterSfx(L"minigame4_food", L"resource\\minigame4\\sound\\food.wav");

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
    minigam1_tutorial1.Shutdown();
    minigame2TutorialScene.Shutdown();
    minigame3TutorialScene.Shutdown();
    minigame4TutorialScene.Shutdown();

    ReleasePendingMiniGame();

    if (m_isMiniGame2Initialized)
    {
        ForceSystemCursorVisible();
        minigame2.Release();
        m_isMiniGame2Initialized = false;
    }
    minigame3.Release();
    minigame4.Release();
    promiseScene.Shutdown();
    audioManager.Shutdown();

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
            SetPauseMenuOpen(false);
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
            endingScene.SetPlayerName(playerName);

            // 인트로 대사를 StoryScene에 넣는다.
            storyScene.SetDialogues(storyData.GetIntroStory());

            // 대사 화면으로 이동한다.
            RequestSceneChange(game_mode_info::Story);

        }

        break;
    }
    case game_mode_info::Story:
    case game_mode_info::EndingDialogue:
        storyScene.OnMouseClick(x, y);
        ApplyStoryBgmChange();
        AdvanceFinishedStory();
        break;

    case game_mode_info::MiniGameTutor1:
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
    case game_mode_info::MiniGameTutor2:
    case game_mode_info::MiniGameTutor6:
    {
        minigame2TutorialScene.OnMouseClick(x, y);
        if (minigame2TutorialScene.IsFinished())
        {
            audioManager.PlaySfx(L"click");

            minigame2TutorialScene.Shutdown();
            StartCurrentMiniGame();
        }
        break;
    }
    case game_mode_info::MiniGameTutor5:
    {
        promiseScene.OnMouseDown(x, y);
        if (!promiseScene.IsTutorial()) RequestSceneChange(game_mode_info::MiniGame5);
        break;
    }
    case game_mode_info::MiniGameTutor3:
    {
        minigame3TutorialScene.OnMouseClick(x, y);
        if (minigame3TutorialScene.IsFinished())
        {
            audioManager.PlaySfx(L"click");

            minigame3TutorialScene.Shutdown();
            StartCurrentMiniGame();
        }
        break;
    }
    case game_mode_info::MiniGameTutor4:
    {
        minigame4TutorialScene.OnMouseClick(x, y);
        if (minigame4TutorialScene.IsFinished())
        {
            audioManager.PlaySfx(L"click");

            minigame4TutorialScene.Shutdown();
            StartCurrentMiniGame();
        }
        break;
    }
    case game_mode_info::MiniGame1: 
    case game_mode_info::MiniGame2:
    case game_mode_info::MiniGame6:
    case game_mode_info::MiniGame5:
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

            if (currentMiniGameIndex == 5 || currentMiniGameIndex == 6)
            {
                PrepareFinalChoiceIntro();
                storyScene.SetDialogues(currentMiniGameIndex == 5
                    ? storyData.GetPromiseStory(finalHeroineIndex, true)
                    : storyData.GetPerformanceStory(finalHeroineIndex, true));
                RequestSceneChange(game_mode_info::Story);
                break;
            }

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

            // 분기 스토리 진입에 성공했을 때만 선택 반영 완료 처리한다.
            if (EnterBranchStory(selectedIndex))
            {
                AddChoiceCount(characters, selectedIndex);
                audioManager.PlaySfx(L"click");
                m_choiceApplied = true;
            }
        }

        break;
    }
    case game_mode_info::Ending:
    {
        endingScene.OnMouseClick(x, y);
        if (endingScene.IsReturnRequested())
        {
            audioManager.PlaySfx(L"click");
            ReturnToTitle();
            break;
        }

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

void GameManager::AdvanceFinishedStory()
{
    if (sceneTransition.IsActive() || !storyScene.IsFinished()) return;
    if (now_game_mode == game_mode_info::EndingDialogue)
    {
        endingScene.SetEndingImage(CalculateEndingType(finalHeroineIndex), finalHeroineIndex);
        endingScene.Reset();
        RequestSceneChange(game_mode_info::Ending);
    }
    else if (now_game_mode == game_mode_info::Story)
    {
        if (currentMiniGameIndex < 4)
        {
            // 현재 회차 번호에 맞는 미니게임 튜토리얼로 이동한다.
            EnterCurrentMiniGameTutorial();
        }
        else if (currentMiniGameIndex == 4)
        {
            if (!m_promiseIntroShown)
            {
                PrepareFinalChoiceIntro();
                storyScene.SetDialogues(storyData.GetPromiseStory(finalHeroineIndex, false));
                m_promiseIntroShown = true;
            }
            else EnterCurrentMiniGameTutorial();
        }
        else if (currentMiniGameIndex == 5)
        {
            if (!m_performanceIntroShown)
            {
                PrepareFinalChoiceIntro();
                storyScene.SetDialogues(storyData.GetPerformanceStory(finalHeroineIndex, false));
                m_performanceIntroShown = true;
                ApplyStoryBgmChange();
            }
            else EnterCurrentMiniGameTutorial();
        }
        else if (!m_finalChoiceIntroShown)
        {
            // 마지막 선택 화면으로 가기 전에 노트가 마지막 이름을 보여주는 대사를 출력한다.
            PrepareFinalChoiceIntro();
            storyScene.SetDialogues(storyData.GetFinalChoiceIntroStory(finalChoiceIntroIndex));
            m_finalChoiceIntroShown = true;
        }
        else
        {
            // 최종 도입 대사가 끝나면 엔딩 대사로 들어간다.
            EnterEndingStory();
        }
        ApplyStoryBgmChange();
    }
}

void GameManager::ReturnToTitle()
{
    if (sceneTransition.IsActive()) return;
    pendingSceneSetup = [this]()
    {
        currentStoryRound = currentMiniGameIndex = 0;
        finalHeroineIndex = finalChoiceIntroIndex = 0;
        m_choiceApplied = m_finalChoiceIntroShown = m_performanceIntroShown = m_promiseIntroShown = false;
        m_exitConfirmOpen = false;
        player = {0, 0, 0, 0};
        for (auto& character : characters) character.choice_time = 0;
        playerName = L"윤서";
        storyScene.SetDialogues({});
        storyScene.ClearHistory();
        storyScene.SetPlayerName(playerName);
        endingScene.Reset();
        endingScene.SetPlayerName(playerName);
        resultScene.Reset();
        resultScene.SetPlayerName(playerName);
        choiceScene.Reset();
        choiceScene.SetPlayerState(player);
        nameinputScene.Reset();
        titleScene.Reset();
        ForceSystemCursorVisible();
    };
    RequestSceneChange(game_mode_info::Title);
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
    if (currentMiniGameIndex == 1 || currentMiniGameIndex == 5)
    {
        minigame2TutorialScene.Initialize(currentMiniGameIndex == 5);
    }
    else if (currentMiniGameIndex == 4)
    {
        PrepareFinalChoiceIntro();
        promiseScene.Initialize(finalHeroineIndex, true);
    }
    else if (currentMiniGameIndex == 2)
    {
        minigame3TutorialScene.Initialize();
    }
    else if (currentMiniGameIndex == 3)
    {
        minigame4TutorialScene.Initialize();
    }
    else
    {
        minigam1_tutorial1.Initialize();
    }

    RequestSceneChange(GetTutorialModeByIndex(currentMiniGameIndex));
}

void GameManager::StartCurrentMiniGame()
{
    if (currentMiniGameIndex != 4) promiseScene.Shutdown();
    if (currentMiniGameIndex != 1 && currentMiniGameIndex != 5 && m_isMiniGame2Initialized)
    {
        ForceSystemCursorVisible();
        minigame2.Release();
        m_isMiniGame2Initialized = false;
    }

    if (currentMiniGameIndex == 1 || currentMiniGameIndex == 5)
    {
        // 미니게임 2는 리듬게임을 사용한다.
        // 리듬게임은 자체 BGM을 재생하므로 이전 공용 BGM을 먼저 정지한다.
        audioManager.StopBgm();
        minigame2.Release();
        if (currentMiniGameIndex == 5) PrepareFinalChoiceIntro();
        if (!minigame2.Init(m_hWnd, currentMiniGameIndex == 5, finalHeroineIndex))
        {
            minigame2.Release();
            m_isMiniGame2Initialized = false;
            MessageBoxW(m_hWnd, L"개화 음원을 열 수 없습니다. resource/minigame5/sound/flowering.mp3를 확인해 주세요.", L"음원 확인", MB_OK | MB_ICONERROR);
            EnterCurrentMiniGameTutorial();
            return;
        }
        ForceSystemCursorHidden();
        m_isMiniGame2Initialized = true;
    }
    else if (currentMiniGameIndex == 4)
    {
        PrepareFinalChoiceIntro();
        promiseScene.Initialize(finalHeroineIndex);
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
    if (miniGameIndex < 0 || miniGameIndex >= static_cast<int>(StatBalance::GameWeights.size()))
    {
        return;
    }

    // F1~F6는 앞부분 진행을 건너뛰고 원하는 미니게임 슬롯으로 바로 들어간다.
    currentMiniGameIndex = miniGameIndex;
    currentStoryRound = (std::min)(miniGameIndex, STORY_ROUND_COUNT);
    m_promiseIntroShown = miniGameIndex >= 4;
    m_performanceIntroShown = miniGameIndex == 5;
    m_finalChoiceIntroShown = false;
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
    case 4:
        return game_mode_info::MiniGameTutor5;
    case 5:
        return game_mode_info::MiniGameTutor6;

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
    case 4:
        return game_mode_info::MiniGame5;
    case 5:
        return game_mode_info::MiniGame6;

    default:
        return game_mode_info::MiniGame1;
    }
}

void GameManager::HandleCurrentMiniGameMouse(int x, int y)
{
    if (now_game_mode == game_mode_info::MiniGame5)
    {
        promiseScene.OnMouseDown(x, y);
        if (promiseScene.IsDragging()) SetCapture(m_hWnd);
        return;
    }
    // 미니게임 4는 아직 마우스 입력을 사용하지 않는다.
    // 현재 실행 화면 기준으로 판단해야 Result 전환 페이드 중에도 다른 미니게임으로 잘못 빠지지 않는다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6) {
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
    if (now_game_mode == game_mode_info::MiniGame5) return;
    // 미니게임 4는 방향키를 WM_KEYDOWN/WM_KEYUP에서 따로 처리한다.
    // 현재 실행 화면 기준으로 판단해야 Result 전환 페이드 중에도 다른 미니게임으로 잘못 빠지지 않는다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        return;
    }

    if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6)
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
    if (now_game_mode == game_mode_info::MiniGame5) { promiseScene.Update(); return; }
    // 미니게임 4는 avoidgame의 Update를 사용한다.
    // 현재 실행 화면 기준으로 판단해야 currentMiniGameIndex가 증가한 뒤에도 안전하다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        minigame4.Update();
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6) {
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
    if (now_game_mode == game_mode_info::MiniGame5) { promiseScene.Render(hDC); return; }
    // 미니게임 4는 avoidgame의 Render를 사용한다.
    // avoidgame 종료 직후 currentMiniGameIndex가 4로 증가해도,
    // 페이드가 끝나기 전 화면은 아직 MiniGame4이므로 now_game_mode로 판단한다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        minigame4.Render(hDC);
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6) {
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

void GameManager::FinishCurrentMiniGameIfNeeded(bool forceFinish)
{
    if (sceneTransition.IsActive()) return;
    if (now_game_mode == game_mode_info::MiniGame5)
    {
        if (forceFinish) promiseScene.FinishImmediately();
        if (!promiseScene.IsFinished()) return;
        EnterResult(4, promiseScene.GetScore());
        m_pendingMiniGameRelease = PendingMiniGameRelease5;
        ++currentMiniGameIndex;
        return;
    }
    // 미니게임 4는 avoidgame의 종료 여부와 점수를 사용한다.
    // 결과 처리도 현재 실행 화면 기준으로 판단한다.
    if (now_game_mode == game_mode_info::MiniGame4)
    {
        if (!forceFinish && !minigame4.IsFinished())
        {
            return;
        }

        EnterResult(3, minigame4.GetScore());
        m_pendingMiniGameRelease = PendingMiniGameRelease4;
        currentMiniGameIndex++;
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6)
    {
        if (forceFinish)
        {
            minigame2.FinishImmediately();
        }

        if (!minigame2.IsGameOver())
        {
            return;
        }

        EnterResult(now_game_mode == game_mode_info::MiniGame6 ? 5 : 1, minigame2.GetResultScore100());
        ForceSystemCursorVisible();
        m_pendingMiniGameRelease = PendingMiniGameRelease2;
        currentMiniGameIndex++;
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame3)
    {
        if (!forceFinish && !minigame3.IsFinished())
        {
            return;
        }

        EnterResult(2, minigame3.GetScore());
        m_pendingMiniGameRelease = PendingMiniGameRelease3;
        currentMiniGameIndex++;
        return;
    }
    // 임시 연결된 미니게임 1이 끝나지 않았으면 Result로 이동하지 않는다.
    if (!forceFinish && !minigame1.isfinished())
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
    int convertedScore = rawScore * 100 / 1000;

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

void GameManager::ApplyStoryBgmChange()
{
    if (now_game_mode != game_mode_info::Story &&
        now_game_mode != game_mode_info::EndingDialogue)
    {
        return;
    }

    std::wstring bgmKey = storyScene.ConsumePendingBgmKey();
    if (bgmKey.empty())
    {
        return;
    }

    if (bgmKey == L"stop")
    {
        audioManager.FadeOutBgm(1500);
        return;
    }

    audioManager.PlayBgmFadeIn(bgmKey, 60, 330, 2000);
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

    case PendingMiniGameRelease4:
        minigame4.Release();
        break;

    case PendingMiniGameRelease5:
        promiseScene.Shutdown();
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
    {
        minigam1_tutorial1.Render(hDC);
        break;
    }
    case game_mode_info::MiniGameTutor2:
    case game_mode_info::MiniGameTutor6:
    {
        minigame2TutorialScene.Render(hDC);
        break;
    }
    case game_mode_info::MiniGameTutor5:
        promiseScene.Render(hDC);
        break;
    case game_mode_info::MiniGameTutor3:
    {
        minigame3TutorialScene.Render(hDC);
        break;
    }
    case game_mode_info::MiniGameTutor4:
    {
        minigame4TutorialScene.Render(hDC);
        break;
    }
    case game_mode_info::MiniGame1:
    case game_mode_info::MiniGame2:
    case game_mode_info::MiniGame6:
    case game_mode_info::MiniGame5:
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

    const auto text = [&](const std::wstring& value, RECT bounds, int size, COLORREF color)
    {
        HFONT font = CreateFontW(size, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
            HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        auto old = SelectObject(hDC, font);
        SetBkMode(hDC, TRANSPARENT); SetTextColor(hDC, color);
        DrawTextW(hDC, value.c_str(), -1, &bounds, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
        SelectObject(hDC, old); DeleteObject(font);
    };
    text(L"조작 안내", {500, 215, 1400, 280}, 44, RGB(246, 226, 240));
    text(L"ESC를 다시 누르거나 계속하기를 선택하면 돌아갑니다.",
        {500, 295, 1420, 345}, 26, RGB(183, 191, 215));
    std::vector<std::pair<std::wstring, std::wstring>> commands;
    if (now_game_mode == game_mode_info::Ending)
        commands = {{L"F / Space", L"크레딧 1배 · 16배 전환"},
            {L"Enter", L"크레딧 종료 후 처음으로 돌아가기"}};
    else if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6)
        commands = {{L"마우스 클릭", L"접근 원이 겹치는 박자에 노트 누르기"},
            {L"누른 채 이동", L"슬라이더 공을 끝까지 따라가기"}};
    else if (now_game_mode == game_mode_info::MiniGame5 || now_game_mode == game_mode_info::MiniGameTutor5)
        commands = {{L"클릭 / 끌어 놓기", L"물건의 단서 읽기 · 목적지에 배치"},
            {L"차례로 클릭", L"물건을 고른 뒤 목적지 클릭"}, {L"힌트 보기", L"현재 장의 정답 안내 · 한 장에 5점 차감"}};
    else if (now_game_mode == game_mode_info::MiniGame3)
        commands = {{L"Space", L"점프"}, {L"↓ 누르기", L"슬라이딩"}};
    else if (now_game_mode == game_mode_info::MiniGame4)
        commands = {{L"← / →", L"이동"}, {L"↑ / Space", L"점프"}, {L"↓", L"발판에서 내려가기"}};
    else if (now_game_mode == game_mode_info::MiniGame1)
        commands = {{L"마우스 클릭", L"재료 선택 · 라면 조합 · 좌석으로 배달"}};
    else if (now_game_mode == game_mode_info::Story || now_game_mode == game_mode_info::EndingDialogue)
        commands = {{L"Space / Enter / 클릭", L"문장 전체 표시, 한 번 더 누르면 다음 대사"},
            {L"F", L"스토리 빨리 감기 켜기 · 끄기"},
            {L"L", L"대화 기록 열기 · 닫기"},
            {L"휠 / ↑↓ / Page Up·Down", L"기록 스크롤 · Home/End로 처음/끝"}};
    else if (now_game_mode == game_mode_info::NameInput)
        commands = {{L"문자 입력", L"이름 입력 · 최대 6글자"},
            {L"Backspace", L"마지막 글자 지우기"}, {L"Enter", L"이름 확정"}};
    else
        commands = {{L"마우스 클릭", L"화면의 버튼이나 인물 카드를 선택하기"}};
    for (size_t i = 0; i < commands.size(); ++i)
    {
        const int y = 390 + static_cast<int>(i) * 86;
        text(commands[i].first, {500, y, 885, y + 75}, 26, RGB(233, 200, 218));
        text(commands[i].second, {900, y, 1415, y + 75}, 26, RGB(236, 239, 246));
    }
    DrawExitConfirmButton(hDC, m_exitNoButtonRect, L"계속하기");
    DrawExitConfirmButton(hDC, m_exitYesButtonRect, L"게임 종료");
}

void GameManager::SetPauseMenuOpen(bool open)
{
    if (m_exitConfirmOpen == open) return;
    m_exitConfirmOpen = open;
    if (open && now_game_mode == game_mode_info::MiniGame5)
    {
        promiseScene.CancelDrag();
        if (GetCapture() == m_hWnd) ReleaseCapture();
    }
    const bool rhythm = now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6;
    if (rhythm)
    {
        minigame2.SetPaused(open);
        if (open)
        {
            ForceSystemCursorVisible();
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        else ForceSystemCursorHidden();
    }
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

void GameManager::PrepareFinalChoiceIntro()
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

    // 스탯이 너무 낮으면 히로인 대신 새누 히든 엔딩으로 진입한다.
    if (CalculateEndingType(finalHeroineIndex) == 2)
    {
        finalChoiceIntroIndex = 3;
    }
    else
    {
        finalChoiceIntroIndex = finalHeroineIndex;
    }
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

    // 이제 StoryScene으로 엔딩 대사를 출력한다.
    RequestSceneChange(game_mode_info::EndingDialogue);

}

int GameManager::CalculateEndingType(int heroineIndex) const
{
    return StatBalance::CalculateEndingType(player, heroineIndex);
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
                endingScene.SetPlayerName(playerName);

                storyScene.SetDialogues(storyData.GetIntroStory());

                RequestSceneChange(game_mode_info::Story);

            }

            break;
        }
        
        case game_mode_info::MiniGame1:
        case game_mode_info::MiniGame2:
        case game_mode_info::MiniGame6:
        case game_mode_info::MiniGame5:
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
            SetPauseMenuOpen(false);
            InvalidateRect(m_hWnd, nullptr, FALSE);
        }
        return;
    }

    if ((now_game_mode == game_mode_info::Story || now_game_mode == game_mode_info::EndingDialogue)
        && storyScene.OnKeyDown(wParam))
    {
        ApplyStoryBgmChange();
        AdvanceFinishedStory();
        InvalidateRect(m_hWnd, nullptr, FALSE);
        return;
    }
    if (now_game_mode == game_mode_info::Ending)
    {
        endingScene.OnKeyDown(wParam);
        if (endingScene.IsReturnRequested()) ReturnToTitle();
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

        SetPauseMenuOpen(true);
        InvalidateRect(m_hWnd, nullptr, FALSE);
        return;
    }

    // F1~F6는 테스트용 바로가기다. F5는 노트 밖의 약속, F6는 개화 공연이다.
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

    case VK_F5:
        DebugEnterMiniGameByIndex(4);
        return;
    case VK_F6:
        DebugEnterMiniGameByIndex(5);
        return;

    default:
        break;
    }

    // Q는 테스트 중인 미니게임을 현재 점수로 즉시 종료한다.
    if (wParam == 'Q' &&
        (now_game_mode == game_mode_info::MiniGame1 ||
            now_game_mode == game_mode_info::MiniGame2 ||
            now_game_mode == game_mode_info::MiniGame3 ||
            now_game_mode == game_mode_info::MiniGame4 ||
            now_game_mode == game_mode_info::MiniGame5 ||
            now_game_mode == game_mode_info::MiniGame6))
    {
        FinishCurrentMiniGameIfNeeded(true);
        return;
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
    if (m_exitConfirmOpen || sceneTransition.IsActive()) return;
    if (now_game_mode == game_mode_info::MiniGame5)
    {
        promiseScene.OnMouseUp(x, y);
        if (GetCapture() == m_hWnd) ReleaseCapture();
        return;
    }
    if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6) {
        minigame2.OnMouseUp(x, y);
        return;
    }
}

void GameManager::OnMouseWheel(int delta)
{
    if (!m_exitConfirmOpen && !sceneTransition.IsActive()
        && (now_game_mode == game_mode_info::Story || now_game_mode == game_mode_info::EndingDialogue))
        storyScene.OnMouseWheel(delta);
}

void GameManager::OnCaptureLost()
{
    if (promiseScene.IsDragging()) promiseScene.CancelDrag();
}

void GameManager::OnMouseMove(int x, int y) {
    if (m_exitConfirmOpen || sceneTransition.IsActive()) return;
    if (now_game_mode == game_mode_info::MiniGame5) { promiseScene.OnMouseMove(x, y); return; }
    if (now_game_mode == game_mode_info::MiniGame2 || now_game_mode == game_mode_info::MiniGame6) {
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
        case game_mode_info::EndingDialogue:
            storyScene.Update();
            ApplyStoryBgmChange();
            AdvanceFinishedStory();
            break;

        case game_mode_info::MiniGame1:
        case game_mode_info::MiniGame2:
        case game_mode_info::MiniGame6:
        case game_mode_info::MiniGame5:
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
    else if (now_game_mode == game_mode_info::MiniGame5)
    {
        audioManager.PlayBgmFadeIn(L"story4.1", 60, 250, 1200);
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
    else if (audioManager.GetCurrentBgmKey() == L"story1")
    {
        audioManager.FadeOutBgm(1500);
    }
    else if (now_game_mode != game_mode_info::NameInput && audioManager.GetCurrentBgmKey() == L"title")
    {
        audioManager.StopBgm();
    }

    ApplyStoryBgmChange();

    ReleasePendingMiniGame();

    // 예약 정보 초기화
    hasPendingSceneChange = false;
    pendingSceneSetup = nullptr;

    // 새 화면이 밝아지도록 FadeIn 시작
    sceneTransition.StartFadeIn();
}
