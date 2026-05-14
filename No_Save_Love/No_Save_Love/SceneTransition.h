#pragma once

#include <windows.h>

#pragma comment(lib, "Msimg32.lib")


enum class SceneTransitionState
{
    None,      
    FadeOut,   
    Black,     
    FadeIn      
};

class SceneTransition
{
public:
    void Start();
    //timer
    void Update();

    void Render(HDC hDC, int screenWidth, int screenHeight);

    // 현재 전환 중인지 확인
    bool IsActive() const;

    // 완전히 검은 화면이 됐는지 확인.
    bool IsBlack() const;

    // Scene 변경이 끝난 뒤 FadeIn으로 넘어간다.
    void StartFadeIn();

private:
    // 현재 전환 상태
    SceneTransitionState state = SceneTransitionState::None;

    // 검은 화면 투명도
    // 0이면 안 보이고, 255면 완전 검정이다.
    int alpha = 0;

    // 페이드 속도
    // 숫자가 클수록 빠르다.
    int speed = 25;
};