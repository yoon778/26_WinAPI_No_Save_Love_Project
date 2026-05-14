#include "RhythmMiniGame.h"

RhythmMiniGame::RhythmMiniGame()
{
    score = 0;
    isGameOver = false;

    mouseX = 0;
    mouseY = 0;
    isMouseDown = false;

    screenWidth = 1920;
    screenHeight = 1080;
}


void RhythmMiniGame::Init()
{
    score = 0;
    isGameOver = false;

    mouseX = 0;
    mouseY = 0;
    isMouseDown = false;
}

void RhythmMiniGame::Update()
{
    // 이후 여기에
    // - 히트서클 갱신
    // - 슬라이더 갱신
    // - Miss 판정
    // 을 넣을 예정
}

void RhythmMiniGame::Render(HDC hDC)
{
    // 임시 화면 확인용 텍스트
    const wchar_t* title = L"Rhythm MiniGame";
    TextOut(hDC, 50, 50, title, lstrlen(title));

    wchar_t scoreText[100];
    wsprintf(scoreText, L"Score : %d", score);
    TextOut(hDC, 50, 90, scoreText, lstrlen(scoreText));
}

void RhythmMiniGame::Release()
{
    // 나중에 동적 메모리나 이미지가 생기면 여기서 해제
}

void RhythmMiniGame::OnMouseDown(int x, int y)
{
    mouseX = x;
    mouseY = y;
    isMouseDown = true;

    // 이후 여기에
    // - 히트서클 클릭 판정
    // - 슬라이더 시작 판정
    // 을 넣을 예정
}

void RhythmMiniGame::OnMouseUp(int x, int y)
{
    mouseX = x;
    mouseY = y;
    isMouseDown = false;

    // 이후 여기에
    // - 슬라이더 중단 처리
    // 를 넣을 예정
}

void RhythmMiniGame::OnMouseMove(int x, int y)
{
    mouseX = x;
    mouseY = y;

    // 이후 여기에
    // - 슬라이더 추적 판정
    // 을 넣을 예정
}

bool RhythmMiniGame::IsGameOver() const
{
    return isGameOver;
}

int RhythmMiniGame::GetScore() const
{
    return score;
}