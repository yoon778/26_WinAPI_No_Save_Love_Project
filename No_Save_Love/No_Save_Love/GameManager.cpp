#include "GameManager.h"

void GameManager::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;

    now_game_mode = game_mode_info::Dialogue;

    // 스토리 데이터를 초기화한다.
    storyData.Initialize();

    // 다른 Scene들도 초기화한다.
    storyScene.Initialize();
    choiceScene.Initialize();
    resultScene.Reset();

    // 임시 인트로 대사를 넣는다.
    std::vector<DialogueLineInfo> introDialogues =
    {
        { L"윤서", L"이상한 노트를 발견한 뒤로, 내 봄은 조금씩 반복되고 있었다." },
        { L"윤서", L"그리고 그 반복의 중심에는 세 명의 소녀가 있었다." },
        { L"시스템", L"미니게임 성적에 따라 스탯이 상승합니다." },
        { L"시스템", L"스탯과 선택 기록에 따라 마지막 엔딩이 달라집니다." }
    };

    storyScene.SetDialogues(introDialogues);
}

void GameManager::InitializeStoryScripts()
{
    // =========================
    // 1회차 선택 이후 스토리
    // =========================

    storyScripts[0][0] =
    {
        { L"한세아", L"네가 도와준 덕분에 일이 조금 정리됐어." },
        { L"윤서", L"그냥... 내가 할 수 있는 걸 한 것뿐이야." },
        { L"한세아", L"그런 말을 아무렇지 않게 하는 게 더 이상하네." }
    };

    storyScripts[0][1] =
    {
        { L"유하린", L"오, 윤서! 방금 선택 나였지?" },
        { L"윤서", L"그걸 그렇게 바로 물어봐?" },
        { L"유하린", L"당연하지. 이런 건 확인이 중요하거든." }
    };

    storyScripts[0][2] =
    {
        { L"서이린", L"나를 선택했구나." },
        { L"윤서", L"응. 뭔가 네가 신경 쓰여서." },
        { L"서이린", L"그 말... 기억해둘게." }
    };

    // =========================
    // 2회차 선택 이후 스토리
    // =========================

    storyScripts[1][0] =
    {
        { L"한세아", L"또 나를 도와주는 거야?" },
        { L"윤서", L"싫으면 안 할게." },
        { L"한세아", L"싫다고는 안 했어." }
    };

    storyScripts[1][1] =
    {
        { L"유하린", L"윤서, 너 은근히 나랑 잘 맞는 거 알아?" },
        { L"윤서", L"갑자기 그런 말을 하면 반응하기 어렵잖아." },
        { L"유하린", L"그 반응 보려고 한 건데?" }
    };

    storyScripts[1][2] =
    {
        { L"서이린", L"오늘도 왔네." },
        { L"윤서", L"오면 안 되는 거였어?" },
        { L"서이린", L"아니. 기다렸어." }
    };

    // =========================
    // 3회차 선택 이후 스토리
    // =========================

    storyScripts[2][0] =
    {
        { L"한세아", L"나는 항상 실수하면 안 된다고 생각했어." },
        { L"윤서", L"그래도 사람은 실수할 수 있잖아." },
        { L"한세아", L"그 말을 듣고 싶었던 걸지도 몰라." }
    };

    storyScripts[2][1] =
    {
        { L"유하린", L"나는 웃고 있으면 괜찮아 보이잖아." },
        { L"윤서", L"괜찮아 보이는 거랑 진짜 괜찮은 건 다르지." },
        { L"유하린", L"...오늘은 장난으로 못 넘기겠네." }
    };

    storyScripts[2][2] =
    {
        { L"서이린", L"윤서야, 넌 반복이 무섭지 않아?" },
        { L"윤서", L"무섭지. 그런데 혼자 있는 게 더 무서워." },
        { L"서이린", L"그럼 내 옆에 있어." }
    };

    // =========================
    // 4회차 선택 이후 스토리
    // =========================

    storyScripts[3][0] =
    {
        { L"한세아", L"이번 봄이 끝나면, 나는 조금 달라질 수 있을까?" },
        { L"윤서", L"혼자서 다 버티지 않아도 된다면 가능하지 않을까." },
        { L"한세아", L"그럼... 조금만 더 옆에 있어줘." }
    };

    storyScripts[3][1] =
    {
        { L"유하린", L"나 사실 겁났어. 진지해지면 망가질까 봐." },
        { L"윤서", L"그래도 말해줘서 고마워." },
        { L"유하린", L"이번엔 웃으면서 도망치지 않을게." }
    };

    storyScripts[3][2] =
    {
        { L"서이린", L"반복이 끝나도, 넌 나를 기억할까?" },
        { L"윤서", L"잊지 않을게." },
        { L"서이린", L"그럼 됐어. 나는 그 말이면 충분해." }
    };
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
        storyScene.OnMouseClick(x, y);

        if (storyScene.IsFinished())
        {
            // 현재는 미니게임이 없으므로,
            // 대사가 끝나면 미니게임을 완료했다고 가정하고 Result로 이동한다.
            if (currentMiniGameIndex < 4)
            {
                EnterResult(currentMiniGameIndex, testMiniGameScore);

                // 다음 미니게임 번호로 증가시킨다.
                currentMiniGameIndex++;
            }
            else
            {
                // 나중에 EndingScene으로 이동시킬 부분이다.
                now_game_mode = game_mode_info::Ending;
            }
        }

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
        choiceScene.HandleChoiceClick(x, y);

        if (!m_choiceApplied && choiceScene.HasSelected())
        {
            int selectedIndex = choiceScene.GetSelectedIndex();

            // 선택 횟수 증가
            AddChoiceCount(characters, selectedIndex);

            // 같은 선택이 여러 번 반영되지 않도록 잠근다.
            m_choiceApplied = true;

            // 선택한 히로인에 맞는 다음 스토리로 이동한다.
            EnterBranchStory(selectedIndex);
        }

        break;
    }

    case game_mode_info::Ending:
    {
        // 나중에 EndingScene을 만들 때 처리한다.
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

void GameManager::EnterBranchStory(int selectedHeroineIndex)
{
    // 잘못된 히로인 번호면 아무것도 하지 않는다.
    if (selectedHeroineIndex < 0 || selectedHeroineIndex >= HEROINE_COUNT)
    {
        return;
    }

    // 4번의 선택 이후에는 엔딩으로 넘어간다.
    if (currentStoryRound >= STORY_ROUND_COUNT)
    {
        now_game_mode = game_mode_info::Ending;
        return;
    }

    // StoryData에서 현재 회차와 선택 히로인에 맞는 대사 묶음을 가져온다.
    const std::vector<DialogueLineInfo>& nextDialogues =
        storyData.GetBranchStory(currentStoryRound, selectedHeroineIndex);

    // 가져온 대사 묶음을 StoryScene에 넣는다.
    storyScene.SetDialogues(nextDialogues);

    // 이번 회차 스토리를 사용했으므로 다음 회차로 넘긴다.
    currentStoryRound++;

    // 다시 대사 화면으로 이동한다.
    now_game_mode = game_mode_info::Dialogue;
}