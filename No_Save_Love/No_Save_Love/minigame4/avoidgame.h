#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <vector>

#pragma comment(lib, "gdiplus.lib")

// 유혹 피하기
class avoidgame
{
public:
    avoidgame();
    ~avoidgame();

    void Initialize(); // 게임 준비
    void Release(); // 리소스 해제
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
        bool moveDown; // 아래 입력
        bool isGrounded; // 착지 상태
        bool faceRight; // 보는 방향
        float animationTimer; // 애니메이션 시간
        int animationFrame; // 애니메이션 프레임
        int jumpCount; // 점프 횟수
        float dropThroughTimer; // 발판 통과 시간
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
    };

    struct FoodPatternState
    {
        float spawnTimer; // 생성 간격
        float patternTimer; // 패턴 지속
    };

    struct DeliverPatternState
    {
        RECT warningRect; // 경고 영역
        RECT riderRect; // 배달 영역
        float warningTimer; // 경고 남은 시간
        float activeTimer; // 활성 남은 시간
        bool isWarning; // 경고 중
        bool isActive; // 활성 중
    };

    struct AttackWarning
    {
        RECT rect; // 공격 영역
        float warningTime; // 경고 시간
        float attackTime; // 공격 시간
        bool isAttackActive; // 공격 활성
        int messageIndex; // 문구 번호
        int attackType; // 공격 종류
        float scrollY; // 릴스 위치
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
    static const int PLAYER_WIDTH = 108; // 플레이어 가로
    static const int PLAYER_HEIGHT = 130; // 플레이어 세로
    static const int PLAYER_HITBOX_MARGIN_X = 18; // [조절값] 좌우 판정 여백
    static const int PLAYER_HITBOX_MARGIN_TOP = 16; // [조절값] 위 판정 여백
    static const int PLAYER_HITBOX_MARGIN_BOTTOM = 8; // [조절값] 아래 판정 여백
    static const int FLOOR_HEIGHT = 60; // 바닥 높이
    static const int MAX_LIFE = 5; // 최대 목숨
    static const int MAX_JUMP_COUNT = 2; // 최대 점프

    static constexpr float UPDATE_DELTA_SECONDS = 0.06f; // 타이머 간격
    static constexpr float GAME_TIME_LIMIT = 40.0f; // 제한 시간
    static constexpr float INVINCIBLE_DURATION = 2.0f; // 무적 지속

    static const int PATTERN_NONE = 0; // 패턴 없음
    static const int PATTERN_KAKAO = 1; // 카톡 패턴
    static const int PATTERN_REELS = 2; // 릴스 패턴
    static const int PATTERN_FOOD = 3; // 음식 패턴
    static const int PATTERN_DELIVER = 4; // 배달 패턴
    static constexpr float PATTERN_DELAY = 1.0f; // 패턴 사이 대기

    static const int ATTACK_TYPE_KAKAO = 0; // 카톡 공격
    static const int ATTACK_TYPE_REELS = 1; // 릴스 공격
    static const int HIT_IMAGE_NONE = -1; // 피격 없음
    static const int HIT_IMAGE_GAME = 0; // 카톡 피격
    static const int HIT_IMAGE_REELS = 1; // 릴스 피격
    static const int HIT_IMAGE_FOOD = 2; // 음식 피격
    static const int HIT_IMAGE_RIDDER = 3; // 배달 피격
    static constexpr float WARNING_DURATION = 1.4f; // 기본 경고
    static constexpr float ATTACK_DURATION = 0.6f; // 기본 공격

    static const int KAKAO_COMBO_COUNT = 5; // 카톡 연타
    static const int KAKAO_POPUP_SIZE = 280; // [조절값] 카톡 팝업 크기
    static constexpr float KAKAO_COMBO_INTERVAL = 0.75f; // 카톡 간격
    static constexpr float REELS_WARNING_DURATION = 1.5f; // 릴스 경고
    static constexpr float REELS_ATTACK_DURATION = 3.2f; // 릴스 공격
    static constexpr float REELS_SCROLL_SPEED = 260.0f; // [조절값] 릴스 속도
    static const int REELS_COLUMN_COUNT = 3; // [조절값] 릴스 개수
    static const int REELS_COLUMN_WIDTH = 450; // [조절값] 릴스 폭

    static constexpr float FOOD_SPAWN_INTERVAL = 0.55f; // 음식 생성
    static constexpr float FOOD_PATTERN_DURATION = 6.0f; // 음식 지속
    static const int FOOD_SIZE = 64; // [조절값] 음식 크기

    static constexpr float DELIVER_WARNING_DURATION = 2.5f; // [조절값] 배달 경고
    static constexpr float DELIVER_ACTIVE_DURATION = 4.0f; // [조절값] 배달 지속
    static constexpr float DELIVER_SPEED = 90.0f; // [조절값] 배달 속도

    static const int PLAYER_RUNNING_FRAME_COUNT = 7; // [조절값] 달리기 프레임 수
    static const int PLAYER_SHEET_FRAME_COUNT = 8; // 달리기 7프레임 + 점프 1프레임
    static const int PLAYER_JUMP_FRAME_INDEX = 7; // 점프 프레임 번호
    static constexpr float PLAYER_ANIMATION_INTERVAL = 0.08f; // [조절값] 달리기 프레임 속도

private:
    PlayerState m_player; // 플레이어 상태
    GameState m_game; // 게임 상태
    PatternState m_pattern; // 패턴 상태
    KakaoPatternState m_kakao; // 카톡 상태
    FoodPatternState m_foodPattern; // 음식 상태
    DeliverPatternState m_deliver; // 배달 상태

    std::vector<Platform> m_platforms; // 발판 목록
    std::vector<AttackWarning> m_attacks; // 장판 공격
    std::vector<FoodDrop> m_foods; // 음식 공격

    ULONG_PTR m_gdiplusToken; // GDI+ 토큰
    bool m_isGdiplusStarted; // GDI+ 시작 여부
    Gdiplus::Image* m_playerStandLeftImage; // 왼쪽 정지
    Gdiplus::Image* m_playerStandRightImage; // 오른쪽 정지
    Gdiplus::Image* m_playerRunningLeftImage; // 왼쪽 달리기
    Gdiplus::Image* m_playerRunningRightImage; // 오른쪽 달리기
    Gdiplus::Image* m_kakaoPopupImage; // 카톡 팝업
    Gdiplus::Image* m_ridderImage; // 배달 기사
    Gdiplus::Image* m_foodImages[3]; // 음식 이미지
    Gdiplus::Image* m_hitImages[4]; // 피격 이미지
    Gdiplus::Image* m_phoneImage; // 릴스 폰 틀
    Gdiplus::Image* m_instarImages[3]; // 릴스 화면
    Gdiplus::Bitmap* m_reelsPhoneCache; // 릴스 폰 캐시
    Gdiplus::Bitmap* m_reelsInstarCaches[3]; // 릴스 화면 캐시
    int m_reelsInstarCacheHeights[3]; // 릴스 화면 높이
    int m_lastHitImageIndex; // 최근 피격 이미지

private:
    void LoadImages(); // 이미지 로드
    void DestroyImages(); // 이미지 해제
    void BuildReelsImageCache(); // 릴스 캐시 생성
    void DestroyReelsImageCache(); // 릴스 캐시 해제
    void DrawGdiImage(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight); // 이미지 출력
    void DrawGdiImageFrame(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight, int frameIndex, int frameCount); // 프레임 출력
    void DrawRedTintedGdiImage(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight); // 붉은 이미지 출력
    void DrawRedTintedGdiImageFrame(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight, int frameIndex, int frameCount); // 붉은 프레임 출력

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

    void StartReelsPattern(); // 릴스 시작
    void SpawnReelsAttack(); // 릴스 생성

    void StartFoodPattern(); // 음식 시작
    void UpdateFoodPattern(); // 음식 패턴 갱신
    void SpawnFoodDrop(); // 음식 생성
    void UpdateFoodDrops(); // 음식 갱신
    void DrawFoodDrops(HDC hDC); // 음식 출력
    RECT GetFoodRect(const FoodDrop& food) const; // 음식 영역

    void StartDeliverPattern(); // 배달 시작
    void UpdateDeliverPattern(); // 배달 갱신
    void DrawDeliver(HDC hDC); // 배달 출력

    void UpdateAttacks(); // 장판 갱신
    void DrawAttacks(HDC hDC); // 장판 출력
    void DrawKakaoPopup(HDC hDC, const AttackWarning& attack); // 카톡 이미지 출력
    void DrawReelsScroll(HDC hDC, const AttackWarning& attack); // 릴스 이미지 출력
    bool IsRectOverlap(const RECT& a, const RECT& b) const; // 충돌 확인
    bool IsRectCircleOverlap(const RECT& rect, const RECT& circleRect) const; // 원 충돌 확인
    void DamagePlayer(int hitImageIndex); // 피격 처리

    void UpdateGameTimer(); // 시간 갱신
    int CalculateScore() const; // 점수 계산
};
