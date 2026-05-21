#include "TitleScene.h"

void TitleScene::Initialize() {
    // 이전에 로드된 배경 이미지가 있다면 새로 로드하기 전에 해제한다.
    if (!titleBackgroundImage.IsNull())
    {
        titleBackgroundImage.Destroy();
    }

    // 이전에 로드된 로고 이미지가 있다면 새로 로드하기 전에 해제한다.
    if (!titleLogoImage.IsNull())
    {
        titleLogoImage.Destroy();
    }

    // 실시간 블러를 만들지 않고, 미리 블러 처리된 1920x1080 PNG를 그대로 사용한다.
    // 파일이 아직 준비되지 않은 경우 기존 배경으로 fallback해서 화면이 비지 않게 한다.
    HRESULT backgroundResult = titleBackgroundImage.Load(L"resource\\background\\title_background_blur.png");
    if (FAILED(backgroundResult))
    {
        titleBackgroundImage.Load(L"resource\\background\\Title화면.png");
    }

    // 새 로고 파일명을 우선 사용하고, 없으면 기존 logo.png를 사용한다.
    HRESULT logoResult = titleLogoImage.Load(L"resource\\background\\title_logo.png");
    if (FAILED(logoResult))
    {
        titleLogoImage.Load(L"resource\\background\\logo.png");
    }

    startClicked = false;
}

void TitleScene::Shutdown() {
    // CImage가 배경 이미지를 가지고 있다면 해제한다.
    if (!titleBackgroundImage.IsNull())
    {
        titleBackgroundImage.Destroy();
    }

    // CImage가 로고 이미지를 가지고 있다면 해제한다.
    if (!titleLogoImage.IsNull())
    {
        titleLogoImage.Destroy();
    }

}

void TitleScene::OnMouseClick(int x, int y) {
    // START 버튼을 눌렀다면 시작 상태로 바꾼다.
    if (IsClickStart(x, y))
    {
        startClicked = true;
    }
}


bool TitleScene::IsStartClicked() const
{
    // GameManager가 START 클릭 여부를 확인할 때 사용한다.
    return startClicked;
}

bool TitleScene::IsClickStart(int x, int y) const
{
    // 마우스 좌표를 POINT 구조체로 만든다.
    POINT mousePoint = { x, y };

    // 마우스 좌표가 START 버튼 영역 안에 있는지 확인한다.
    return PtInRect(&startButtonRect, mousePoint);
}

void TitleScene::Render(HDC hDC)
{
    // 이미지 확대/축소 시 깨짐을 줄이기 위해 HALFTONE 모드를 사용한다.
    int oldStretchMode = SetStretchBltMode(hDC, HALFTONE);
    SetBrushOrgEx(hDC, 0, 0, nullptr);

    // 1. 미리 블러 처리된 배경 이미지를 화면 전체에 출력한다.
    if (!titleBackgroundImage.IsNull())
    {
        titleBackgroundImage.Draw(hDC, 0, 0, 1920, 1080);
    }
    else
    {
        // 이미지가 없을 때 임시 배경색을 칠한다.
        HBRUSH bgBrush = CreateSolidBrush(RGB(245, 220, 230));
        RECT bgRect = { 0, 0, 1920, 1080 };
        FillRect(hDC, &bgRect, bgBrush);
        DeleteObject(bgBrush);
    }

    // 2. 배경 위에 아주 약한 밝은 오버레이를 얹어 미연시 타이틀 느낌을 만든다.
    DrawSoftOverlay(hDC);

    // 텍스트 배경을 투명하게 만든다.
    SetBkMode(hDC, TRANSPARENT);

    // 3. 타이틀 로고 PNG를 출력한다. 로고는 배경 위에 선명하게 출력한다.
    if (!titleLogoImage.IsNull())
    {
        // 기존 1080x260은 원본 로고 비율보다 납작하게 보여서,
        // 원본 비율에 가까운 1260x420 크기로 더 크게 출력한다.
        titleLogoImage.Draw(hDC, 330, 135, 1260, 420);
    }

    // 4. START 버튼 출력
    HBRUSH buttonBrush = CreateSolidBrush(RGB(235, 90, 140));
    HPEN buttonPen = CreatePen(PS_SOLID, 3, RGB(255, 230, 240));

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, buttonBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, buttonPen));

    // 타원형에 가까운 둥근 버튼을 코드로 그린다.
    RoundRect(
        hDC,
        startButtonRect.left,
        startButtonRect.top,
        startButtonRect.right,
        startButtonRect.bottom,
        80,
        80
    );

    HFONT startFont = CreateFontW(
        44,
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, startFont));

    SetTextColor(hDC, RGB(255, 255, 255));

    // 5. START 텍스트 출력
    DrawTextW(
        hDC,
        L"START",
        -1,
        &startButtonRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // GDI 객체 복구 및 삭제
    SelectObject(hDC, oldFont);
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);

    DeleteObject(buttonPen);
    DeleteObject(buttonBrush);
    DeleteObject(startFont);

    // 다른 Scene 출력에 영향을 주지 않도록 StretchBlt 모드를 원래대로 되돌린다.
    SetStretchBltMode(hDC, oldStretchMode);
}

void TitleScene::DrawSoftOverlay(HDC hDC)
{
    RECT overlayRect = { 0, 0, 1920, 1080 };

    // 반투명 출력용 메모리 DC를 만든다.
    HDC overlayDC = CreateCompatibleDC(hDC);
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, 1920, 1080);
    HBITMAP oldOverlayBitmap = static_cast<HBITMAP>(SelectObject(overlayDC, overlayBitmap));

    // 흰색보다 약간 연핑크가 도는 색을 사용한다.
    HBRUSH overlayBrush = CreateSolidBrush(RGB(255, 245, 250));
    FillRect(overlayDC, &overlayRect, overlayBrush);

    // alpha는 40~70 사이에서 조금 더 부드러워 보이는 68로 둔다.
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 90;

    AlphaBlend(hDC, 0, 0, 1920, 1080, overlayDC, 0, 0, 1920, 1080, blend);

    SelectObject(overlayDC, oldOverlayBitmap);
    DeleteObject(overlayBrush);
    DeleteObject(overlayBitmap);
    DeleteDC(overlayDC);
}
