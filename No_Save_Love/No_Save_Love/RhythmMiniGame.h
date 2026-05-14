#pragma once
#include <windows.h>

class RhythmMiniGame
{
private:
    // =========================
    // 게임 기본 상태
    // =========================
    int score;              // 점수
    bool isGameOver;         // 게임 종료 여부

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

public:
    RhythmMiniGame();


    // =========================
    // 게임 기본 함수
    // =========================
    void Init();            // 초기화
    void Update();          // 매 프레임 게임 갱신
    void Render(HDC hDC);   // 화면 출력
    void Release();         // 종료 시 정리

    // =========================
    // 입력 처리 함수
    // =========================
    void OnMouseDown(int x, int y);
    void OnMouseUp(int x, int y);
    void OnMouseMove(int x, int y);

    // =========================
    // 외부에서 확인할 정보
    // =========================
    bool IsGameOver() const;
    int GetScore() const;
};