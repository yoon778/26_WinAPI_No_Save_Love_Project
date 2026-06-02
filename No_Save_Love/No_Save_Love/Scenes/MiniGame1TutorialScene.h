#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <atlimage.h>

class MiniGame1TutorialScene
{
public:
    void Initialize();
    void Initialize(int miniGameNumber);
    void Shutdown();
    void Reset();
    void Render(HDC hDC);

    void OnMouseClick(int x, int y);

    bool IsFinished() const;

    int GetCurrentPage() const;

private:
    // 클릭 판정 사각형 안에 들어왔는지
    bool IsInsideRect(const RECT& rect, int x, int y) const;

    bool IsValidImage(const CImage& image) const;

    // 공통 배경을 그린다.
    void DrawBackground(HDC hDC);

    // 공통 제목 영역을 그린다.
    void DrawHeader(HDC hDC);

    // 1페이지: 조작법 설명을 그린다.
    void DrawPageControlGuide(HDC hDC);

    // 2페이지: 성공/실패 점수 설명을 그린다.
    void DrawPageScoreGuide(HDC hDC);

    // 2페이지: 목표와 주의점을 그린다.
    void DrawPageGoalGuide(HDC hDC);

    // 하단 버튼을 그린다.
    void DrawBottomButtons(HDC hDC);

    // 둥근 카드 배경을 그린다.
    void DrawRoundCard(HDC hDC, const RECT& rc, COLORREF fillColor, COLORREF outlineColor);

    // 둥근 버튼을 그린다.
    void DrawButton(HDC hDC, const RECT& rc, const std::wstring& text, COLORREF fillColor, COLORREF textColor);

    // 지정한 영역 안에 이미지를 비율 유지해서 그린다.
    void DrawImageFit(HDC hDC, CImage& image, const RECT& targetRect);

    // 텍스트를 그린다.
    void DrawTextInRect(HDC hDC, const std::wstring& text, RECT rc, int fontSize, COLORREF color, UINT format, int fontWeight = FW_NORMAL);

    // 튜토리얼 번호에 맞는 문구와 이미지 경로를 설정한다.
    void ConfigureForMiniGame(int miniGameNumber);

    // 현재 튜토리얼 이미지들을 로드한다.
    void LoadTutorialImages();

    // 설명 목록을 그린다.
    void DrawGuideLines(HDC hDC, const std::vector<std::wstring>& lines, int left, int top, int right, int lineHeight, int fontSize);

    // 다음 페이지로 이동하거나, 마지막 페이지라면 종료 상태로 만든다.
    void GoNext();

    // 이전 페이지로 이동한다.
    void GoPrev();

private:
    // 화면 기준 크기
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    // 현재 설명 페이지
    // 0: 조작법 설명
    // 1: 점수 설명
    int currentPage = 0;

    // 설명 화면 종료 여부
    bool finished = false;

    // 현재 튜토리얼 번호
    int currentMiniGameNumber = 1;

    // 공통 문구
    std::wstring titleText;
    std::wstring subTitleText;
    std::wstring mainImagePath;
    std::wstring highlightText;
    std::wstring goalTitleText;
    std::vector<std::wstring> controlGuideLines;
    std::vector<std::wstring> goalGuideLines;

    
    // 설명 이미지
    CImage mainGuideImage;
    CImage successImage;
    CImage failImage;

    // 버튼 영역
    RECT prevButtonRect = { 1180, 960, 1390, 1025 };
    RECT nextButtonRect = { 1430, 960, 1700, 1025 };
    RECT skipButtonRect = { 1725, 45, 1865, 100 };

    // 페이지 점 표시 영역
    RECT pageDotArea = { 860, 975, 1060, 1020 };
};
