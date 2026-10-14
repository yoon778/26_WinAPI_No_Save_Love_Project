#pragma once

#include <array>
#include <span>

struct Player_state
{
    int money = 0;      // 재력
    int speech = 0;     // 대화 스킬
    int charm = 0;      // 매력
    int appearance = 0; // 외모
};

inline int ClampStat(int value)
{
    if (value < 0) return 0;
    if (value > 100) return 100;
    return value;
}

namespace StatBalance
{
    // 한 회차에서 각 미니게임을 한 번씩 플레이할 때의 스탯별 최대 획득량.
    inline constexpr int RunStatBudget = 80;

    // 보상 수치가 아닌 배분 비중이다. 실제로 회차에 연결된 게임만 등록한다.
    // 새 게임을 추가하면 해당 스탯의 기존 보상도 이 표를 기준으로 재배분된다.
    inline constexpr std::array<Player_state, 5> GameWeights = {{
        {5, 3, 0, 0}, // PC방 알바: 재력 / 대화 스킬
        {0, 0, 5, 3}, // 노래 연습: 매력 / 외모
        {0, 0, 3, 5}, // 등교길: 외모 / 매력
        {3, 5, 0, 0}, // 유혹 피하기: 대화 스킬 / 재력
        {0, 5, 3, 0}, // 개화 공연: 대화 스킬 / 매력
    }};

    // 10000 = 100%. 40~60점 구간은 학습 보상을, 60점 이후는 안정적인 성장을 준다.
    inline int GetRewardRate(int score)
    {
        score = ClampStat(score);
        if (score <= 40) return score * 100;
        if (score <= 60) return 4000 + (score - 40) * 150;
        return 7000 + (score - 60) * 75;
    }

    inline int CalculateStatGain(int gameIndex, int rate, int Player_state::*stat,
        std::span<const Player_state> weights)
    {
        int totalWeight = 0;
        int precedingWeight = 0;
        for (int i = 0; i < static_cast<int>(weights.size()); ++i)
        {
            totalWeight += weights[i].*stat;
            if (i < gameIndex) precedingWeight += weights[i].*stat;
        }
        if (totalWeight <= 0) return 0;

        // 누적 비중의 차이로 정수 몫을 나눠, 5~6개 구성에서도 만점 합계는 정확히 80이다.
        const int throughWeight = precedingWeight + weights[gameIndex].*stat;
        const int maxGain = RunStatBudget * throughWeight / totalWeight
            - RunStatBudget * precedingWeight / totalWeight;
        return (maxGain * rate + 5000) / 10000;
    }

    inline Player_state CalculateGain(int gameIndex, int score,
        std::span<const Player_state> weights = GameWeights)
    {
        if (gameIndex < 0 || gameIndex >= static_cast<int>(weights.size())) return {};
        const int rate = GetRewardRate(score);
        return {
            CalculateStatGain(gameIndex, rate, &Player_state::money, weights),
            CalculateStatGain(gameIndex, rate, &Player_state::speech, weights),
            CalculateStatGain(gameIndex, rate, &Player_state::charm, weights),
            CalculateStatGain(gameIndex, rate, &Player_state::appearance, weights)
        };
    }

    // 기존 엔딩 조건 유지: 0 = Happy, 1 = Bad, 2 = Hidden.
    inline int CalculateEndingType(const Player_state& player, int heroineIndex)
    {
        if (player.money + player.speech + player.charm + player.appearance <= 80)
            return 2;

        switch (heroineIndex)
        {
        case 0: return player.money + player.speech >= 120 ? 0 : 1;      // 한세아
        case 1: return player.speech + player.charm >= 120 ? 0 : 1;      // 유하린
        case 2: return player.charm + player.appearance >= 120 ? 0 : 1;  // 서이린
        default: return 1;
        }
    }
}
