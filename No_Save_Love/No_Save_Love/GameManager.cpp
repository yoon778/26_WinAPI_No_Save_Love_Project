#include "GameManager.h"

void GameManager::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;

    // 게임은 대사 모드에서 시작한다.
    now_game_mode = game_mode_info::Dialogue;

    // 히로인 선택 횟수 초기화
    characters = {
        CharacterInfo{ L"한세아", 0 },
        CharacterInfo{ L"유하린", 0 },
        CharacterInfo{ L"서이린", 0 }
    };

    // 플레이어 스탯 초기화
    player = { 0, 0, 0, 0 };

    // 각 Scene 초기화
    storyScene.Initialize();
    choiceScene.Initialize();
    resultScene.Reset();
}

void GameManager::Shutdown()
{
    // StoryScene 내부 데이터 정리
    storyScene.Shutdown();

    m_hWnd = nullptr;
}

void GameManager::OnMouseClick(int x, int y)
{
    switch (now_game_mode)
    {
    case game_mode_info::Dialogue:
    {
        // 대사 클릭 처리는 StoryScene에게 맡긴다.
        storyScene.OnMouseClick(x, y);

        // 대사가 끝났다면, 지금은 미니게임을 끝냈다고 가정하고 Result로 이동한다.
        if (storyScene.IsFinished())
        {
            // 임시 테스트:
            // 미니게임 0번에서 85점을 받았다고 가정한다.
            EnterResult(currentMiniGameIndex, testMiniGameScore);
        }

        break;
    }

    case game_mode_info::Result:
    {
        // ResultScene에게 클릭 처리를 맡긴다.
        resultScene.OnMouseClick(x, y);

        // Result 화면이 끝났다면 Choice 화면으로 이동한다.
        if (resultScene.IsFinished())
        {
            choiceScene.Reset();
            now_game_mode = game_mode_info::Choice;
        }

        break;
    }

    case game_mode_info::Choice:
    {
        // 선택지 클릭 판정은 ChoiceScene에게 맡긴다.
        choiceScene.HandleChoiceClick(x, y);

        // 실제로 선택지를 클릭했을 때만 선택 횟수를 증가시킨다.
        if (choiceScene.HasSelected())
        {
            int selectedIndex = choiceScene.GetSelectedIndex();

            // 선택한 히로인 횟수 증가
            AddChoiceCount(characters, selectedIndex);

        }

        break;
    }
    }

    InvalidateRect(m_hWnd, nullptr, TRUE);
}

void GameManager::EnterResult(int whichGame, int score)
{
    resultScene.Reset();

    resultScene.SetResult(whichGame, score);

    Player_state plusState = resultScene.GetPlusState();

    ApplyStatGain(plusState);

    resultScene.SetCurrentPlayerState(player);

    now_game_mode = game_mode_info::Result;
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

void GameManager::HandleResultClick()
{
    // 지금 구조에서는 Result 클릭 처리를 OnMouseClick의 Result case에서 직접 처리한다.
    // 나중에 필요하면 이 함수로 분리하면 된다.
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
    }
}