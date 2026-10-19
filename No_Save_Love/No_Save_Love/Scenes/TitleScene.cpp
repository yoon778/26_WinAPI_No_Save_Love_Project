#include "TitleScene.h"
#include "../MenuUI.h"

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
        HBRUSH bgBrush = CreateSolidBrush(RGB(250, 247, 242));
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

    MenuUI::Text(hDC, L"다섯 번의 만남, 다시 돌아갈 수 없는 마음", {520, 580, 1400, 625},
        28, MenuUI::Ink, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // 시작 버튼의 클릭 영역은 그대로 두고 공통 메뉴의 색과 글자 계층을 사용한다.
    MenuUI::Button(hDC, startButtonRect, L"", true);
    MenuUI::Text(hDC, L"이야기 시작", startButtonRect, 32, RGB(255, 255, 255),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX, FW_SEMIBOLD);

    // 다른 Scene 출력에 영향을 주지 않도록 StretchBlt 모드를 원래대로 되돌린다.
    SetStretchBltMode(hDC, oldStretchMode);
}

void TitleScene::DrawSoftOverlay(HDC hDC)
{
    // 단색 소스를 확대해 큰 임시 비트맵을 매 프레임 만들지 않는다.
    HDC overlayDC = CreateCompatibleDC(hDC);
    if (!overlayDC) return;
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, 1, 1);
    if (!overlayBitmap) { DeleteDC(overlayDC); return; }
    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(overlayDC, overlayBitmap));
    SetPixelV(overlayDC, 0, 0, RGB(250, 247, 242));
    BLENDFUNCTION blend{};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 76;
    AlphaBlend(hDC, 0, 0, 1920, 1080, overlayDC, 0, 0, 1, 1, blend);

    // 하단은 밝은 종이색을 조금 더 얹어 시작 버튼과 보조 메뉴의 대비를 확보한다.
    for (int band = 0; band < 12; ++band)
    {
        blend.SourceConstantAlpha = static_cast<BYTE>(12 + band * 7);
        AlphaBlend(hDC, 0, 600 + band * 40, 1920, 40, overlayDC, 0, 0, 1, 1, blend);
    }
    SelectObject(overlayDC, oldBitmap);
    DeleteObject(overlayBitmap);
    DeleteDC(overlayDC);
}
