#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "../ResultGrade.h"
#include "../StatBalance.h"

class ResultScene
{
public:
    // ResultScene 상태를 초기화한다.
    void Reset();

    // GameManager가 미니게임 번호와 점수를 넘겨줄 때 호출한다.
    // 이 함수 안에서 상승 스탯을 계산한다.
    void SetResult(int whichGame, int score);

    // GameManager가 실제 player 스탯에 상승량을 반영한 뒤,
    // ResultScene에게 현재 총 스탯을 다시 알려준다.
    void SetCurrentPlayerState(const Player_state& currentState);

    // GameManager가 입력된 플레이어 이름을 알려줄 때 사용한다.
    void SetPlayerName(const std::wstring& playerName);

    // Result 화면에서 마우스를 클릭했을 때 호출한다.
    void OnMouseClick(int x, int y);

    // Result 화면이 끝났는지 확인한다.
    bool IsFinished() const;

    // ResultScene이 계산한 상승 스탯을 GameManager에게 넘겨준다.
    Player_state GetPlusState() const;

    // 결과 화면을 출력한다.
    void RenderResult(HDC hDC);
    void SetDetails(const std::vector<std::wstring>& details) { m_details = details; }
    void SetPractice(bool practice) { m_practice = practice; }
    void SetDifficulty(int difficulty) { m_difficulty = difficulty; }
    bool IsRetryRequested() const { return m_retryRequested; }

private:
    // 미니게임 번호를 이름으로 바꿔준다.
    std::wstring GetGameName(int whichGame) const;

private:
    int m_whichGame = 0;          // 몇 번째 미니게임인지
    int m_score = 0;              // 0~100으로 환산된 미니게임 점수

    bool m_hasResult = false;     // 결과 데이터가 준비되었는지
    bool m_finished = false;      // Result 화면이 끝났는지
    bool m_retryRequested = false;
    bool m_practice = false;
    int m_difficulty = 1;
    std::vector<std::wstring> m_details;

    Player_state m_plusState{};   // 이번 Result에서 상승한 스탯
    Player_state m_currentState{}; // 상승량 반영 후 현재 총 스탯
    std::wstring m_playerName = L"윤서"; // 입력이 비어 있을 때 사용할 기본 이름

    RECT m_confirmButton = { 710, 945, 1210, 1030 };
    RECT m_retryButton = { 260, 945, 650, 1030 };
};
