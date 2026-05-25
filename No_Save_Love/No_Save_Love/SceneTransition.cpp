#include "SceneTransition.h"

void SceneTransition::Start()
{
    // 이미 전환 중이면 중복 실행하지 않는다.
    if (state != SceneTransitionState::None)
    {
        return;
    }

    // 현재 화면이 점점 어두워지는 상태로 시작한다.
    state = SceneTransitionState::FadeOut;
    alpha = 0;
}

void SceneTransition::Update()
{
    // 전환 중이 아니면 아무것도 하지 않는다.
    if (state == SceneTransitionState::None)
    {
        return;
    }

    // FadeOut: 현재 화면을 점점 어둡게 만든다.
    if (state == SceneTransitionState::FadeOut)
    {
        alpha += speed;

        // 완전히 어두워졌으면 검은 화면 상태로 멈춘다.
        if (alpha >= 255)
        {
            alpha = 255;
            state = SceneTransitionState::Black;
        }
    }
    // FadeIn: 새 화면을 점점 밝게 만든다.
    else if (state == SceneTransitionState::FadeIn)
    {
        alpha -= speed;

        // 완전히 밝아졌으면 전환 종료
        if (alpha <= 0)
        {
            alpha = 0;
            state = SceneTransitionState::None;
        }
    }
}

void SceneTransition::Render(HDC hDC, int screenWidth, int screenHeight)
{
    // 투명도가 0이면 그릴 필요가 없다.
    if (alpha <= 0)
    {
        return;
    }

    // 검은색 레이어를 그릴 메모리 DC를 만든다.
    HDC memDC = CreateCompatibleDC(hDC);

    // 화면 크기와 같은 비트맵을 만든다.
    HBITMAP memBitmap = CreateCompatibleBitmap(hDC, screenWidth, screenHeight);

    // 메모리 DC에 비트맵을 연결한다.
    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memDC, memBitmap));

    // 메모리 DC 전체를 검은색으로 칠한다.
    RECT rect = { 0, 0, screenWidth, screenHeight };
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(memDC, &rect, blackBrush);

    // AlphaBlend 설정
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = static_cast<BYTE>(alpha); // 반투명 정도
    blend.AlphaFormat = 0;

    // 현재 화면 위에 검은 레이어를 반투명으로 덮는다.
    AlphaBlend(
        hDC,
        0,
        0,
        screenWidth,
        screenHeight,
        memDC,
        0,
        0,
        screenWidth,
        screenHeight,
        blend
    );

    // GDI 객체 정리
    SelectObject(memDC, oldBitmap);
    DeleteObject(blackBrush);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

bool SceneTransition::IsActive() const
{
    return state != SceneTransitionState::None;
}

bool SceneTransition::IsBlack() const
{
    return state == SceneTransitionState::Black;
}

void SceneTransition::StartFadeIn()
{
    // 완전히 검은 상태일 때만 FadeIn으로 넘어간다.
    if (state == SceneTransitionState::Black)
    {
        state = SceneTransitionState::FadeIn;
        alpha = 255;
    }
}