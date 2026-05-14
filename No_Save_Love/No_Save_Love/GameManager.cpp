#include "GameManager.h"

void GameManager::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;

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

    // 처음 시작 모드는 대사 모드다.
    now_game_mode = game_mode_info::Title;
}
void GameManager::Shutdown()
{
    // StoryScene 내부 데이터 정리
    storyScene.Shutdown();
    titleScene.Shutdown();
    m_hWnd = nullptr;
}

void GameManager::OnMouseClick(int x, int y)
{
    switch (now_game_mode)
    {
    case game_mode_info::Title: 
    {
        titleScene.OnMouseClick(x, y);

        if (titleScene.IsStartClicked()) {

            storyScene.SetDialogues(storyData.GetIntroStory());

            now_game_mode = game_mode_info::NameInput;
        }
        break;
    }
    case game_mode_info::NameInput:
    {
        // 이름 입력 화면의 마우스 클릭 처리
        nameinputScene.OnMouseClick(x, y);

        // 확인 버튼을 눌러 이름 입력이 끝났다면
        if (nameinputScene.IsFinished())
        {
            // 입력된 이름을 GameManager에 저장한다.
            playerName = nameinputScene.GetPlayerName();

            // StoryScene에게도 플레이어 이름을 알려준다.
            // 이걸 해야 StoryScene 내부 m_playerName이 "윤서"에서 입력한 이름으로 바뀐다.
            storyScene.SetPlayerName(playerName);
            resultScene.SetPlayerName(playerName);

            // 인트로 대사를 StoryScene에 넣는다.
            storyScene.SetDialogues(storyData.GetIntroStory());

            // 대사 화면으로 이동한다.
            now_game_mode = game_mode_info::Story;
        }

        break;
    }
    case game_mode_info::Story:
    {
        storyScene.OnMouseClick(x, y);

        if (storyScene.IsFinished())
        {
            if (currentMiniGameIndex < 4)
            {
               
                switch (currentMiniGameIndex) {
                    case 0: 
                    {
                        minigame1.Init(); // 미니게임 초기화
                        now_game_mode = game_mode_info::MiniGame1;
                        break;
                    }
                    case 1:
                    {
                        now_game_mode = game_mode_info::MiniGame2;
                        break;
                    }
                    case 2:
                    {
                        now_game_mode = game_mode_info::MiniGame3;
                        break;
                    }
                    case 3:
                    {
                        now_game_mode = game_mode_info::MiniGame4;
                        break;
                    }

                }
                // 다음 미니게임 번호로 증가시킨다.
                currentMiniGameIndex++;
            }
            else
            {
                //최종 선택으로
                EnterFinalChoice();
            }
        }

        break;
    }
    case game_mode_info::MiniGame1: 
    {
        minigame1.MOUSE(x, y);
        if (minigame1.isfinished()) {
            // 미니게임 1 결과
            int rawScore = minigame1.getscore();
            if (rawScore <= 0)
            {
                rawScore = 0;
            }
            // 1500점을 100점 만점 기준으로 환산한다.
            int convertedScore = rawScore * 100 / 2000;

            // 100점을 넘으면 100점으로 고정한다.
            if (convertedScore > 100)
            {
                convertedScore = 100;
            }

            EnterResult(currentMiniGameIndex, convertedScore); // 점수 넘기고 result로 넘어가기

        }
        break;
    }
    case game_mode_info::MiniGame2:
    {
      
            EnterResult(currentMiniGameIndex, testMiniGameScore); // 점수 넘기고 result로 넘어가기

        break;
    }
    case game_mode_info::MiniGame3:
    {

        EnterResult(currentMiniGameIndex, testMiniGameScore); // 점수 넘기고 result로 넘어가기

        break;
    }
    case game_mode_info::MiniGame4:
    {

        EnterResult(currentMiniGameIndex, testMiniGameScore); // 점수 넘기고 result로 넘어가기

        break;
    }


    case game_mode_info::Result:
    {
        resultScene.OnMouseClick(x, y);

        if (resultScene.IsFinished())
        {
            choiceScene.Reset();

            // Choice에 들어갈 때마다 중복 반영 방지 플래그를 초기화한다.
            m_choiceApplied = false;

            now_game_mode = game_mode_info::Choice;
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
        storyScene.OnMouseClick(x, y);

        // 엔딩 대사가 모두 끝나면 THE END 화면으로 이동한다.
        if (storyScene.IsFinished())
        {
            now_game_mode = game_mode_info::Ending;
        }

        break;
    }


    case game_mode_info::Ending:
    {
        // 나중에 EndingScene을 만들 때 처리한다.
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
    case game_mode_info::MiniGame1:
    {
        minigame1.PAINT(hDC);
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
        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, RGB(30, 30, 30));

        RECT endingRect = { 0, 0, 1920, 1080 };

        DrawTextW(
            hDC,
            L"THE END\n\n최종 결과 화면입니다.\n\n[처음으로 돌아가기]\n[종료하기]",
            -1,
            &endingRect,
            DT_CENTER | DT_VCENTER | DT_WORDBREAK
        );

        break;
    }
    }
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
        now_game_mode = game_mode_info::Ending;
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
    now_game_mode = game_mode_info::Story;

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

    // FinalChoiceScene을 처음 상태로 준비한다.
    finalChoiceScene.Reset();

    // 최종 선택 확인 화면으로 이동한다.
    now_game_mode = game_mode_info::FinalChoice;
}

void GameManager::EnterEndingStory()
{

    // 스탯을 보고 해피 / 배드 / 히든 엔딩을 계산한다.
    int endingType = CalculateEndingType(finalHeroineIndex);

    // StoryData에서 해당 히로인, 해당 엔딩 종류의 대사를 가져온다.
    const std::vector<DialogueLineInfo>& endingDialogues =
        storyData.GetEndingStory(endingType, finalHeroineIndex);

    // 엔딩 대사를 StoryScene에 넣는다.
    storyScene.SetDialogues(endingDialogues);

    // 이제 StoryScene으로 엔딩 대사를 출력한다.
    now_game_mode = game_mode_info::EndingDialogue;
}

int GameManager::CalculateEndingType(int heroineIndex) const
{
    // 0 = Happy
    // 1 = Bad
    // 2 = Hidden

    // 모든 스탯이 20 이하이면 히든 엔딩
    if (player.money <= 20 &&
        player.speech <= 20 &&
        player.charm <= 20 &&
        player.appearance <= 20)
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

                now_game_mode = game_mode_info::Story;
            }

            break;
        }
        case game_mode_info::MiniGame1: {
            minigame1.KEYDOWN(inputChar);
        }

        default:
        {
            break;
        }
    }

}

void GameManager::OnTimer(HWND hWnd)
{
    switch (now_game_mode)
    {
    case game_mode_info::Story:
        // StoryScene 내부에서 타이핑과 페이드를 모두 처리한다.
        storyScene.Update();

        // 화면을 다시 그리도록 요청한다.
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case game_mode_info::EndingDialogue:
        // 엔딩 대사도 StoryScene을 재사용하므로 같은 Update를 호출한다.
        storyScene.Update();

        // 화면을 다시 그리도록 요청한다.
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case game_mode_info::MiniGame1: 
    {
        minigame1.Update();
        InvalidateRect(hWnd, NULL, FALSE);
    }
    default:
        break;
    }
}