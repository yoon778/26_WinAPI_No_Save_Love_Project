#pragma once

#include <windows.h>
#include <vector>

// 윤서의 등교길 미니게임
class schoolrun
{
public:
    schoolrun();

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
        AirObstacle                 // 슬라이드로 피하는 장애물
    };

    struct Obstacle
    {
        ObstacleType type;          // 장애물 종류
        float x;                    // 장애물 X 위치
        float y;                    // 장애물 Y 위치
        int width;                  // 장애물 너비
        int height;                 // 장애물 높이
    };

private:
    RECT GetPlayerRect() const;     // 현재 플레이어 영역
    RECT GetHitBoxRect() const;     // 현재 충돌 박스
    RECT GetObstacleRect(const Obstacle& obstacle) const; // 장애물 영역
    void UpdateSpeed();             // 속도 증가
    void UpdateObstacles();         // 장애물 이동
    void CheckObstacleCollisions();  // 장애물 충돌 확인
    void SpawnObstacle();           // 장애물 생성
    void RenderScrollGround(HDC hDC) const; // 이동 바닥선 출력
    void RenderObstacles(HDC hDC) const;    // 장애물 출력
    void RenderScore(HDC hDC) const;        // 거리 점수 출력

private:
    float m_playerX;                // 플레이어 X 위치
    float m_playerY;                // 플레이어 Y 위치
    float m_velocityY;              // 세로 속도
    float m_gravity;                // 중력
    float m_jumpPower;              // 점프 힘

    float m_gameSpeed;              // 진행 속도
    float m_scrollOffset;           // 바닥선 이동값
    int m_distanceScore;            // 거리 점수
    int m_nextSpeedUpDistance;      // 다음 속도 증가 거리
    int m_nextObstacleDistance;     // 다음 장애물 생성 거리
    int m_nextObstacleType;         // 다음 장애물 종류
    float m_remainingTime;          // 남은 시간

    int m_score;                    // 결과 점수
    bool m_isFinished;              // 게임 종료 여부
    bool m_isGrounded;              // 바닥 착지 여부
    bool m_isSliding;               // 슬라이드 여부
    int m_jumpCount;                // 점프 횟수

    std::vector<Obstacle> m_obstacles; // 장애물 목록
};
