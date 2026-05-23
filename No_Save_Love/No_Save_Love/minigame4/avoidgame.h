#pragma once

#include <windows.h>
#include <vector>

// 유혹 피하기
class avoidgame
{
public:
    avoidgame();

    void Initialize(); // 게임 준비
    void Reset(); // 처음 상태
    void Update(); // 상태 갱신
    void Render(HDC hDC); // 화면 출력
    void OnKeyDown(WPARAM wParam); // 키 입력
    void OnKeyUp(WPARAM wParam); // 키 해제
    bool IsFinished() const; // 종료 확인
    int GetScore() const; // 점수 반환

private:
    struct Platform
    {
        RECT rect; // 발판 영역
    };

    struct PlayerState
    {
        float x; // 위치
        float y;
        float velocityX; // 속도
        float velocityY;
        float gravity; // 중력
        float jumpPower; // 점프 힘
        float moveSpeed; // 이동 속도
        bool moveLeft; // 왼쪽 입력
        bool moveRight; // 오른쪽 입력
        bool isGrounded; // 착지 상태
        int jumpCount; // 점프 횟수
    };

    struct GameState
    {
        int life; // 목숨
        float remainingTime; // 남은 시간
        float invincibleTimer; // 무적 시간
        bool isFinished; // 종료 여부
        bool isSuccess; // 성공 여부
    };

    struct PatternState
    {
        int currentPattern; // 현재 패턴
        int nextPattern; // 다음 패턴 순서
        float waitTimer; // 다음 패턴 대기
    };

    struct KakaoPatternState
    {
        float comboTimer; // 연타 간격
        int comboCount; // 남은 연타
        RECT moleWarningRect; // 두더지 경고 영역
        float moleWarningTimer; // 두더지 경고 시간
        float moleX; // 두더지 X 위치
        float moleY; // 두더지 현재 위치
        bool isMoleWarning; // 두더지 경고 중
        bool isMoleActive; // 두더지 활성 중
        bool isMoleGoingUp; // 상승 중
    };

    struct FoodPatternState
    {
        float spawnTimer; // 생성 간격
        float patternTimer; // 패턴 지속
    };

    struct BlackholePatternState
    {
        RECT warningRect; // 경고 영역
        RECT coreRect; // 블랙홀 중심
        float warningTimer; // 경고 남은 시간
        float activeTimer; // 활성 남은 시간
        float spawnTimer; // 텍스트 생성 시간
        int spawnedCount; // 생성한 텍스트 수
        bool isWarning; // 경고 중
        bool isActive; // 활성 중
    };

    struct BlackholeText
    {
        float x; // 현재 위치
        float y;
        int messageIndex; // 문구 번호
    };

    struct AttackWarning
    {
        RECT rect; // 공격 영역
        float warningTime; // 경고 시간
        float attackTime; // 공격 시간
        bool isAttackActive; // 공격 활성
        int messageIndex; // 문구 번호
        int attackType; // 공격 종류
    };

    struct FoodDrop
    {
        float x; // 현재 위치
        float y;
        float baseX; // 흔들림 기준
        float wave; // 흔들림 값
        float speed; // 낙하 속도
        int foodType; // 음식 종류
    };

private:
    static const int SCREEN_WIDTH = 1920; // 화면 가로
    static const int SCREEN_HEIGHT = 1080; // 화면 세로
    static const int PLAYER_WIDTH = 60; // 플레이어 가로
    static const int PLAYER_HEIGHT = 80; // 플레이어 세로
    static const int FLOOR_HEIGHT = 60; // 바닥 높이
    static const int MAX_LIFE = 5; // 최대 목숨
    static const int MAX_JUMP_COUNT = 2; // 최대 점프

    static constexpr float UPDATE_DELTA_SECONDS = 0.06f; // 타이머 간격
    static constexpr float GAME_TIME_LIMIT = 60.0f; // 제한 시간
    static constexpr float INVINCIBLE_DURATION = 1.0f; // 무적 지속

    static const int PATTERN_NONE = 0; // 패턴 없음
    static const int PATTERN_KAKAO = 1; // 카톡 패턴
    static const int PATTERN_REELS = 2; // 릴스 패턴
    static const int PATTERN_FOOD = 3; // 음식 패턴
    static const int PATTERN_BLACKHOLE = 4; // 블랙홀 패턴
    static constexpr float PATTERN_DELAY = 1.0f; // 패턴 사이 대기

    static const int ATTACK_TYPE_KAKAO = 0; // 카톡 공격
    static const int ATTACK_TYPE_REELS = 1; // 릴스 공격
    static constexpr float WARNING_DURATION = 1.0f; // 기본 경고
    static constexpr float ATTACK_DURATION = 0.6f; // 기본 공격

    static const int KAKAO_COMBO_COUNT = 5; // 카톡 연타
    static constexpr float KAKAO_COMBO_INTERVAL = 0.45f; // 카톡 간격
    static constexpr float KAKAO_MOLE_WARNING_DURATION = 2.2f; // [조절값] 두더지 경고
    static constexpr float KAKAO_MOLE_SPEED = 42.0f; // [조절값] 두더지 속도
    static const int KAKAO_MOLE_WIDTH = 520; // [조절값] 두더지 가로
    static const int KAKAO_MOLE_HEIGHT = SCREEN_HEIGHT / 2; // [조절값] 두더지 높이

    static constexpr float REELS_WARNING_DURATION = 1.8f; // 릴스 경고
    static constexpr float REELS_ATTACK_DURATION = 3.2f; // 릴스 공격

    static constexpr float FOOD_SPAWN_INTERVAL = 0.55f; // 음식 생성
    static constexpr float FOOD_PATTERN_DURATION = 6.0f; // 음식 지속

    static constexpr float BLACKHOLE_WARNING_DURATION = 1.6f; // [조절값] 블랙홀 경고
    static constexpr float BLACKHOLE_ACTIVE_DURATION = 8.8f; // [조절값] 블랙홀 지속
    static constexpr float BLACKHOLE_PULL_FORCE = 12.0f; // [조절값] 기본 끌림
    static constexpr float BLACKHOLE_RESIST_PULL_FORCE = 10.0f; // [조절값] 반대 입력 끌림
    static constexpr float BLACKHOLE_VERTICAL_PULL_FORCE = 4.5f; // [조절값] 세로 끌림
    static constexpr float BLACKHOLE_TEXT_SPEED = 34.0f; // [조절값] 텍스트 속도
    static constexpr float BLACKHOLE_TEXT_SPAWN_INTERVAL = 0.9f; // [조절값] 텍스트 간격
    static const int BLACKHOLE_TEXT_COUNT = 7; // [조절값] 텍스트 생성 수
    static const int BLACKHOLE_TEXT_WIDTH = 70; // [조절값] 텍스트 가로
    static const int BLACKHOLE_TEXT_HEIGHT = PLAYER_HEIGHT ; // [조절값] 캐릭터 높이 1.5배
    static const int BLACKHOLE_TEXT_MIN_Y = 140; // [조절값] 생성 최소 Y
    static const int BLACKHOLE_TEXT_MAX_Y = SCREEN_HEIGHT - BLACKHOLE_TEXT_HEIGHT; // [조절값] 생성 최대 Y
    static const int BLACKHOLE_SIZE = 130; // [조절값] 중심 크기
    static const int BLACKHOLE_RIGHT_MARGIN = 160; // [조절값] 오른쪽 여백
    static const int BLACKHOLE_WARNING_WIDTH = 420; // [조절값] 왼쪽 경고 폭

private:
    PlayerState m_player; // 플레이어 상태
    GameState m_game; // 게임 상태
    PatternState m_pattern; // 패턴 상태
    KakaoPatternState m_kakao; // 카톡 상태
    FoodPatternState m_foodPattern; // 음식 상태
    BlackholePatternState m_blackhole; // 블랙홀 상태

    std::vector<Platform> m_platforms; // 발판 목록
    std::vector<AttackWarning> m_attacks; // 장판 공격
    std::vector<FoodDrop> m_foods; // 음식 공격
    std::vector<BlackholeText> m_blackholeTexts; // 블랙홀 텍스트

private:
    void ResetPlayer(); // 플레이어 초기화
    void UpdatePlayer(); // 플레이어 갱신
    void RenderPlayer(HDC hDC); // 플레이어 출력
    void Jump(); // 점프
    void HandleLanding(float previousY); // 착지 처리
    void ClampPlayerPosition(); // 화면 제한
    RECT GetPlayerRect() const; // 플레이어 영역

    void CreatePlatforms(); // 발판 생성
    void RenderMap(HDC hDC); // 맵 출력
    void RenderPlatforms(HDC hDC); // 발판 출력
    void RenderHud(HDC hDC); // UI 출력

    void ResetPatterns(); // 패턴 초기화
    void UpdatePatternScheduler(); // 패턴 선택
    void FinishPatternIfEmpty(); // 패턴 종료

    void StartKakaoCombo(); // 카톡 시작
    void UpdateKakaoCombo(); // 카톡 갱신
    void SpawnKakaoAttack(); // 카톡 생성
    void StartKakaoMole(); // 두더지 시작
    void UpdateKakaoMole(); // 두더지 갱신
    void DrawKakaoMole(HDC hDC); // 두더지 출력
    RECT GetKakaoMoleRect() const; // 두더지 영역

    void StartReelsPattern(); // 릴스 시작
    void SpawnReelsAttack(); // 릴스 생성

    void StartFoodPattern(); // 음식 시작
    void UpdateFoodPattern(); // 음식 패턴 갱신
    void SpawnFoodDrop(); // 음식 생성
    void UpdateFoodDrops(); // 음식 갱신
    void DrawFoodDrops(HDC hDC); // 음식 출력
    RECT GetFoodRect(const FoodDrop& food) const; // 음식 영역

    void StartBlackholePattern(); // 블랙홀 시작
    void UpdateBlackholePattern(); // 블랙홀 갱신
    void ApplyBlackholePull(); // 캐릭터 끌림
    void SpawnBlackholeText(); // 텍스트 생성
    void UpdateBlackholeTexts(); // 텍스트 이동
    void DrawBlackhole(HDC hDC); // 블랙홀 출력
    RECT GetBlackholeTextRect(const BlackholeText& text) const; // 텍스트 영역

    void UpdateAttacks(); // 장판 갱신
    void DrawAttacks(HDC hDC); // 장판 출력
    bool IsRectOverlap(const RECT& a, const RECT& b) const; // 충돌 확인
    void DamagePlayer(); // 피격 처리

    void UpdateGameTimer(); // 시간 갱신
    int CalculateScore() const; // 점수 계산
};
