#pragma once

#include <windows.h>
#include <string>
#include <atlimage.h>

class MiniGame4TutorialScene
{
public:
    void Initialize();
    void Shutdown();
    void Reset();
    void Render(HDC hDC);
    void OnMouseClick(int x, int y);
    bool IsFinished() const;
    int GetCurrentPage() const;

private:
    bool IsInsideRect(const RECT& rect, int x, int y) const;
    bool IsValidImage(const CImage& image) const;

    void DrawBackground(HDC hDC);
    void DrawHeader(HDC hDC);
    void DrawPageControlGuide(HDC hDC);
    void DrawPageGoalGuide(HDC hDC);
    void DrawBottomButtons(HDC hDC);
    void DrawGuideStep(HDC hDC, int index, const std::wstring& text);
    void DrawGoalLine(HDC hDC, int index, const std::wstring& text);

    void DrawRoundCard(HDC hDC, const RECT& rc, COLORREF fillColor, COLORREF outlineColor);
    void DrawButton(HDC hDC, const RECT& rc, const std::wstring& text, COLORREF fillColor, COLORREF textColor);
    void DrawImageFit(HDC hDC, CImage& image, const RECT& targetRect);
    void DrawTextInRect(HDC hDC, const std::wstring& text, RECT rc, int fontSize, COLORREF color, UINT format, int fontWeight = FW_NORMAL);

    void GoNext();
    void GoPrev();

private:
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    int currentPage = 0;
    bool finished = false;

    CImage guideImage;

    RECT prevButtonRect = { 1180, 960, 1390, 1025 };
    RECT nextButtonRect = { 1430, 960, 1700, 1025 };
    RECT skipButtonRect = { 1725, 45, 1865, 100 };
    RECT pageDotArea = { 860, 975, 1060, 1020 };
};
