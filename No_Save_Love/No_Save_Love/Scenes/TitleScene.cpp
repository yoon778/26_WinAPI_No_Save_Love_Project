#include "TitleScene.h"

void TitleScene::Initialize() {
    if (!Title_paint.IsNull())
    {
        Title_paint.Destroy();
    }
    Title_paint.Load(L"resource\\background\\Title화면.png");
    startClicked = false;
}

void TitleScene::Shutdown() {
    // CImage가 이미지를 가지고 있다면 해제한다.
    if (!Title_paint.IsNull())
    {
        Title_paint.Destroy();
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
    // 배경 이미지가 정상 로드되었다면 출력한다.
    if (!Title_paint.IsNull())
    {
        // 1920x1080 화면 전체에 배경 이미지를 출력한다.
        Title_paint.Draw(hDC, 0, 0, 1920, 1080);
    }
    else
    {
        // 이미지가 없을 때 임시 배경색을 칠한다.
        HBRUSH bgBrush = CreateSolidBrush(RGB(245, 220, 230));
        RECT bgRect = { 0, 0, 1920, 1080 };
        FillRect(hDC, &bgRect, bgBrush);
        DeleteObject(bgBrush);
    }

    // 텍스트 배경을 투명하게 만든다.
    SetBkMode(hDC, TRANSPARENT);

    // 제목 출력
    SetTextColor(hDC, RGB(230, 80, 130));

    HFONT titleFont = CreateFontW(
        72,
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, titleFont));

    RECT titleRect = { 0, 160, 1920, 280 };

    DrawTextW(
        hDC,
        L"윤서의 고백은 세이브 로드가 없다",
        -1,
        &titleRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // START 버튼 출력
    HBRUSH buttonBrush = CreateSolidBrush(RGB(235, 90, 140));
    HPEN buttonPen = CreatePen(PS_SOLID, 3, RGB(255, 230, 240));

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, buttonBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, buttonPen));

    // 타원형에 가까운 둥근 버튼
    RoundRect(
        hDC,
        startButtonRect.left,
        startButtonRect.top,
        startButtonRect.right,
        startButtonRect.bottom,
        80,
        80
    );

    SetTextColor(hDC, RGB(255, 255, 255));

    DrawTextW(
        hDC,
        L"START",
        -1,
        &startButtonRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // GDI 객체 복구 및 삭제
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldFont);

    DeleteObject(buttonPen);
    DeleteObject(buttonBrush);
    DeleteObject(titleFont);
}