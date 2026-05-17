#pragma once
#include <windows.h>
#include <atlimage.h>

// 화면에 동시에 존재할 수 있는 최대 개수
#define MAX_HIT_CIRCLES 30
#define MAX_SLIDERS 10

enum HitJudge           // 클릭판정
{
    JUDGE_NONE,
    JUDGE_PERFECT,
    JUDGE_GOOD,
    JUDGE_BAD,
    JUDGE_MISS
};

// =========================
// 히트서클 정보
// =========================
struct HitCircle
{
    bool isActive;      // 현재 화면에 존재하는가?
    bool isJudged;      // 이미 클릭 판정이 끝났는가?

    int x;              // 원 중심 x좌표
    int y;              // 원 중심 y좌표

    DWORD spawnTime;    // 화면에 생성된 시간
    DWORD hitTime;      // 정확히 눌러야 하는 시간
};


// =========================
// 슬라이더 정보
// =========================
struct Slider
{
    bool isActive;      // 현재 화면에 존재하는가?
    bool isStarted;     // 시작 원을 클릭해서 슬라이더가 시작되었는가?
    bool isFinished;    // 슬라이더 진행이 끝났는가?
    bool isFailed;      // 도중에 실패했는가?

    int startX;         // 슬라이더 시작점 x좌표
    int startY;         // 슬라이더 시작점 y좌표
    int endX;           // 슬라이더 끝점 x좌표
    int endY;           // 슬라이더 끝점 y좌표

    DWORD spawnTime;    // 슬라이더가 화면에 생성된 시간
    DWORD hitTime;      // 시작 원을 눌러야 하는 정확한 시간
    DWORD duration;     // 슬라이더 볼이 시작점에서 끝점까지 이동하는 시간
};


// =========================
// 리듬 미니게임 클래스
// =========================
class RhythmMiniGame
{
private:
    // =========================
    // 게임 기본 상태
    // =========================
    int score;
    bool isGameOver;

    // =========================
    // 마우스 입력 상태
    // =========================
    int mouseX;
    int mouseY;
    bool isMouseDown;

    // =========================
    // 화면 크기
    // =========================
    int screenWidth;
    int screenHeight;

    // =========================
    // 히트서클 이미지
    // =========================
    CImage hitCircleImg;
    CImage hitCircleOverlayImg;
    CImage approachCircleImg;
    
    // =========================
    // 판정 이미지
    // =========================
    CImage hit300Img;
    CImage hit100Img;
    CImage hit50Img;
    CImage hit0Img;
    
    // =========================
    // 최근 판정 표시용
    // =========================
    HitJudge lastJudge;
    DWORD judgeDisplayStartTime;
    
    int judgeX;
    int judgeY;

    // =========================
    // 히트서클 / 슬라이더 저장 배열
    // =========================
    HitCircle hitCircles[MAX_HIT_CIRCLES];
    Slider sliders[MAX_SLIDERS];

    int hitCircleCount;     // 현재까지 사용 중인 히트서클 개수
    int sliderCount;        // 현재까지 사용 중인 슬라이더 개수

    void CreateHitCircle(int x, int y);     // 히트서클 1개 생성하는 함수
    void PremultiplyAlpha(CImage& image);   // 이미지 알파 처리 함수(이미지에 있는 빛번짐 같은거 제거)

public:
    RhythmMiniGame();

    void Init();
    void Update();
    void Render(HDC hDC);
    void Release();

    void OnMouseDown(int x, int y);
    void OnMouseUp(int x, int y);
    void OnMouseMove(int x, int y);

    bool IsGameOver() const;
    int GetScore() const;
};