#include <cassert>
#include <climits>
#include <iostream>
#include <vector>
#include "../No_Save_Love/Scenes/ResultScene.h"

namespace
{
    constexpr int Player_state::* Fields[] = {
        &Player_state::money, &Player_state::speech,
        &Player_state::charm, &Player_state::appearance
    };

    void ExpectState(const Player_state& actual, const Player_state& expected)
    {
        for (auto field : Fields) assert(actual.*field == expected.*field);
    }

    Player_state PlayRun(std::span<const Player_state> weights, std::span<const int> scores)
    {
        assert(weights.size() == scores.size());
        Player_state player;
        for (int game = 0; game < static_cast<int>(weights.size()); ++game)
        {
            const auto gain = StatBalance::CalculateGain(game, scores[game], weights);
            for (auto field : Fields) player.*field += gain.*field;
        }
        return player;
    }

    void CheckRoster(std::span<const Player_state> weights)
    {
        using namespace StatBalance;
        for (int game = 0; game < static_cast<int>(weights.size()); ++game)
        {
            Player_state previous;
            for (int score = 0; score <= 100; ++score)
            {
                const auto gain = CalculateGain(game, score, weights);
                for (auto field : Fields)
                {
                    assert(gain.*field >= previous.*field);
                    assert(gain.*field <= RunStatBudget);
                    if (weights[game].*field == 0) assert(gain.*field == 0);
                }
                previous = gain;
            }
            ExpectState(CalculateGain(game, INT_MIN, weights), {});
            ExpectState(CalculateGain(game, INT_MAX, weights), previous);
        }

        for (const int score : {0, 20, 40, 60, 70, 80, 90, 100})
        {
            const std::vector<int> scores(weights.size(), score);
            const auto player = PlayRun(weights, scores);
            for (auto field : Fields)
            {
                assert(player.*field <= RunStatBudget);
                if (score == 100) assert(player.*field == RunStatBudget);
            }
            const int expectedEnding = score <= 20 ? 2 : (score < 70 ? 1 : 0);
            for (int heroine = 0; heroine < 3; ++heroine)
                assert(CalculateEndingType(player, heroine) == expectedEnding);
            std::cout << weights.size() << " games, score " << score << ": "
                << player.money << ',' << player.speech << ','
                << player.charm << ',' << player.appearance << '\n';
        }
    }
}

int main()
{
    using namespace StatBalance;
    const std::span<const Player_state> originalFour(GameWeights.data(), 4);
    CheckRoster(originalFour);
    const std::array<Player_state, 5> originalFive = {{{5,3,0,0}, {0,0,5,3}, {0,0,3,5}, {3,5,0,0}, {0,5,3,0}}};
    CheckRoster(originalFive);
    CheckRoster(GameWeights);

    // 실제 여섯 게임에 일곱 번째 게임이 추가되어도 총량을 유지한다.
    std::vector<Player_state> expanded(GameWeights.begin(), GameWeights.end());
    expanded.push_back({3, 0, 0, 5});
    CheckRoster(expanded);

    // 7번째 게임이 한 스탯에 치우쳐도 총량이 늘지 않아야 한다.
    expanded.assign(GameWeights.begin(), GameWeights.end());
    expanded.push_back({8, 0, 0, 0});
    CheckRoster(expanded);

    ExpectState(CalculateGain(-1, 100), {});
    ExpectState(CalculateGain(static_cast<int>(GameWeights.size()), 100), {});
    ExpectState(CalculateGain(0, 100, {}), {});
    const std::array<Player_state, 1> noWeights = {};
    ExpectState(CalculateGain(0, 100, noWeights), {});

    ExpectState(CalculateGain(0, 60, originalFour), {35, 21, 0, 0});
    ExpectState(CalculateGain(0, 70, originalFour), {39, 23, 0, 0});
    ExpectState(CalculateGain(0, 80, originalFour), {43, 26, 0, 0});
    ExpectState(CalculateGain(0, 90, originalFour), {46, 28, 0, 0});
    ExpectState(CalculateGain(0, 100, originalFour), {50, 30, 0, 0});

    const std::array<int, 4> recoveredScores = {60, 80, 80, 80};
    const auto recovered = PlayRun(originalFour, recoveredScores);
    ExpectState(recovered, {61, 64, 69, 69});
    for (int heroine = 0; heroine < 3; ++heroine)
        assert(CalculateEndingType(recovered, heroine) == 0);

    // 회피 게임에서 실제로 가능한 하트 점수: 0, 20, 40, 60, 80, 100.
    const std::array<int, 4> withLowAvoidScore = {80, 80, 80, 40};
    const auto lowAvoid = PlayRun(originalFour, withLowAvoidScore);
    assert(CalculateEndingType(lowAvoid, 0) == 1);
    assert(CalculateEndingType(lowAvoid, 2) == 0);

    const std::array<int, 5> fiveGameScores = {60, 80, 80, 80, 80};
    const auto fiveGameRecovery = PlayRun(originalFive, fiveGameScores);
    ExpectState(fiveGameRecovery, {61, 65, 69, 69});
    ExpectState(CalculateGain(4, 70, originalFive), {0, 24, 17, 0});
    for (int heroine = 0; heroine < 3; ++heroine)
        assert(CalculateEndingType(fiveGameRecovery, heroine) == 0);

    const std::array<int, 6> sixGameScores = {60, 80, 80, 80, 100, 80};
    const auto sixGameRecovery = PlayRun(GameWeights, sixGameScores);
    ExpectState(sixGameRecovery, {64, 68, 70, 70});
    ExpectState(CalculateGain(4, 100), {16, 15, 17, 16});
    ExpectState(CalculateGain(5, 100), {0, 25, 18, 0});
    for (int heroine = 0; heroine < 3; ++heroine)
        assert(CalculateEndingType(sixGameRecovery, heroine) == 0);

    // 80/81 히든 경계, 히로인별 119/120 해피 경계와 잘못된 인덱스.
    assert(CalculateEndingType({20, 20, 20, 20}, 0) == 2);
    assert(CalculateEndingType({21, 20, 20, 20}, 0) == 1);
    for (int heroine = 0; heroine < 3; ++heroine)
    {
        Player_state below = {60, 59, 60, 59};
        if (heroine == 1) below = {59, 60, 59, 60};
        assert(CalculateEndingType(below, heroine) == 1);
        assert(CalculateEndingType({60, 60, 60, 60}, heroine) == 0);
    }
    assert(CalculateEndingType({80, 80, 80, 80}, -1) == 1);
    assert(CalculateEndingType({80, 80, 80, 80}, 3) == 1);

    // 결과 화면이 새 계산을 실제로 사용하며 리셋 후 이전 보상이 남지 않는지 확인한다.
    ResultScene result;
    for (int game = 0; game < static_cast<int>(GameWeights.size()); ++game)
    {
        Player_state previous;
        for (int score = 0; score <= 100; ++score)
        {
            result.SetResult(game, score);
            const auto gain = result.GetPlusState();
            ExpectState(gain, CalculateGain(game, score));
            for (auto field : Fields) assert(gain.*field - previous.*field <= 1);
            previous = gain;
        }
    }
    result.SetResult(0, INT_MAX);
    ExpectState(result.GetPlusState(), {40, 15, 0, 0});
    result.SetResult(0, INT_MIN);
    ExpectState(result.GetPlusState(), {});
    result.SetResult(static_cast<int>(GameWeights.size()), 100);
    ExpectState(result.GetPlusState(), {});
    result.Reset();
    ExpectState(result.GetPlusState(), {});
    assert(!result.IsFinished());
    std::cout << "PASS: reward curve, ResultScene, ending boundaries, 4/5/6/7-game budgets\n";
}
