#pragma once

#include <windows.h>
#include <atlimage.h>
#include <vector>


// 윤서의 등교길 미니게임
class schoolrun
{
public:
    schoolrun();

    ~schoolrun();

    void Initialize();              // 게임 준비
    void Release();                 // 리소스 해제
    void Reset();                   // 상태 초기화
    void Update();                  // 상태 갱신
    void Render(HDC hDC);           // 화면 출력
    void OnKeyDown(WPARAM wParam);  // 키 입력
    void OnKeyUp(WPARAM wParam);    // 키 해제
    bool IsFinished() const;        // 종료 확인
    int GetScore() const;           // 점수 반환

private:
    enum ObstacleType
    {
        GroundObstacle,             // 점프로 피하는 장애물
        AirObstacle,                // 슬라이드로 피하는 장애물
        PaperObstacle,              // 종이 장애물
        BirdObstacle,               // 새 장애물
        BoxObstacle,                // 상자 장애물
        ConstructionObstacle,       // 공사 장애물
        WaterObstacle,              // 물웅덩이 장애물
        SignboardObstacle           // 간판 장애물
    };

    struct Obstacle
    {
        ObstacleType type;          // 장애물 종류
        float x;                    // 장애물 X 위치
        float y;                    // 장애물 Y 위치
        int width;                  // 장애물 너비
        int height;                 // 장애물 높이
    };

    struct PatternObstacle
    {
        ObstacleType type;          // 패턴 안 장애물 종류
        float offsetX;              // 패턴 시작점 기준 X 간격
    };

    struct ObstaclePattern
    {
        std::vector<PatternObstacle> obstacles; // 패턴 장애물 목록
    };

    struct Cloud
    {
        float x;                    // 구름 X 위치
        float y;                    // 구름 Y 위치
        int width;                  // 구름 너비
        int height;                 // 구름 높이
    };

private:
    RECT GetPlayerRect() const;     // 현재 플레이어 영역
    RECT GetHitBoxRect() const;     // 현재 충돌 박스
    RECT GetObstacleRect(const Obstacle& obstacle) const; // 장애물 영역
    bool TryJump();                 // 점프 가능하면 실행
    void UpdateBackground();        // 배경 이동
    void ResetClouds();             // 구름 상태 초기화
    void UpdateClouds();            // 구름 이동
    void UpdateObstacles();         // 장애물 이동
    void UpdatePlayerAnimation();    // 플레이어 애니메이션 갱신
    void CheckObstacleCollisions();  // 장애물 충돌 확인
    void SpawnObstacle();           // 장애물 생성
    void CreateObstaclePatterns();   // 장애물 패턴 생성
    void SetObstacleSpec(Obstacle& obstacle, ObstacleType type) const; // 장애물 크기와 위치 설정
    CImage* GetObstacleImage(ObstacleType type); // 장애물 이미지 반환
    void RenderBackground(HDC hDC); // 원경 배경 출력
    void RenderClouds(HDC hDC);     // 구름 출력
    void RenderGround(HDC hDC);     // 이동 바닥 출력
    void RenderObstacles(HDC hDC);          // 장애물 출력
    void RenderPlayer(HDC hDC);             // 플레이어 출력
    void RenderHitBoxes(HDC hDC);   // 충돌 박스 출력
    void RenderScore(HDC hDC) const;        // 거리 점수 출력
    void RenderLife(HDC hDC);       // 목숨 출력
    void RenderDamageOverlay(HDC hDC); // 피격 화면 출력
    void LoadImages();              // 이미지 로드
    void DestroyImages();           // 이미지 해제
    void DrawImage(HDC hDC, CImage& image, int drawX, int drawY, int drawWidth, int drawHeight); // 이미지 출력
    void DrawImageFrame(HDC hDC, CImage& image, int drawX, int drawY, int drawWidth, int drawHeight, int frameIndex, int sheetFrameCount); // 프레임 출력

private:
    float m_playerX;                // 플레이어 X 위치
    float m_playerY;                // 플레이어 Y 위치
    float m_velocityY;              // 세로 속도
    float m_gravity;                // 중력
    float m_jumpPower;              // 점프 힘

    float m_gameSpeed;              // 진행 속도
    float m_backgroundOffset;       // 배경 이동값
    float m_scrollOffset;           // 바닥선 이동값
    int m_distanceScore;            // 거리 점수
    int m_nextObstacleDistance;     // 다음 장애물 생성 거리
    float m_remainingTime;          // 남은 시간

    int m_score;                    // 결과 점수
    int m_life;                     // 남은 목숨
    float m_invincibleTime;         // 무적 남은 시간
    bool m_isFinished;              // 게임 종료 여부
    bool m_isGrounded;              // 바닥 착지 여부
    bool m_isSliding;               // 슬라이드 여부
    bool m_isDownKeyPressed;        // 아래키 눌림 여부
    int m_jumpCount;                // 점프 횟수
    float m_jumpBufferTime;         // 예약된 점프 남은 시간
    int m_playerFrameIndex;         // 현재 프레임
    float m_playerAnimationTime;     // 프레임 타이머

    std::vector<Obstacle> m_obstacles; // 장애물 목록
    std::vector<ObstaclePattern> m_obstaclePatterns; // 장애물 패턴 목록
    std::vector<Cloud> m_clouds;    // 배경 구름 목록
    int m_nextObstaclePatternIndex; // 다음 패턴 번호

    CImage m_playerRunImage;        // 달리기/점프 이미지
    CImage m_playerSlideImage;      // 슬라이드 이미지
    CImage m_backgroundImage;       // 등굣길 배경 이미지
    CImage m_backgroundImage2;      // 등굣길 배경 연결 이미지
    CImage m_heartImage;            // 목숨 이미지
    CImage m_groundImage;           // 바닥 이미지
    CImage m_groundObstacleImage;   // 지상 장애물 이미지
    CImage m_airObstacleImage;      // 공중 장애물 이미지
    CImage m_paperObstacleImage;    // 종이 이미지
    CImage m_boxObstacleImage;      // 상자 이미지
    CImage m_waterObstacleImage;    // 물웅덩이 이미지
    CImage m_signboardObstacleImage; // 간판 이미지
    CImage m_cloudImage;            // 배경 구름 이미지
};
