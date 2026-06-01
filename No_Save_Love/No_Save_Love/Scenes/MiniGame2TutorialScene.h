#pragma once

#include <windows.h>
#include <string>
#include <atlimage.h>

class MiniGame2TutorialScene
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
    void DrawPageScoreGuide(HDC hDC);
    void DrawBottomButtons(HDC hDC);

    void DrawGameplayPreview(HDC hDC, const RECT& rc);
    void DrawMissPreview(HDC hDC, const RECT& rc);
    void DrawHitCircle(HDC hDC, int centerX, int centerY, int approachSize);
    void DrawSlider(HDC hDC, int startX, int startY, int endX, int endY);
    void DrawScoreRow(HDC hDC, const RECT& rc, CImage& judgeImage, const std::wstring& label, const std::wstring& scoreText, COLORREF accentColor);
    void DrawPreviewLabel(HDC hDC, const RECT& rc, const std::wstring& text);
    void DrawArrow(HDC hDC, int startX, int startY, int endX, int endY, COLORREF color);

    void DrawRoundCard(HDC hDC, const RECT& rc, COLORREF fillColor, COLORREF outlineColor);
    void DrawButton(HDC hDC, const RECT& rc, const std::wstring& text, COLORREF fillColor, COLORREF textColor);
    void DrawImageFit(HDC hDC, CImage& image, const RECT& targetRect);
    void DrawImageCentered(HDC hDC, CImage& image, int centerX, int centerY, int maxWidth, int maxHeight);
    void DrawTextInRect(HDC hDC, const std::wstring& text, RECT rc, int fontSize, COLORREF color, UINT format, int fontWeight = FW_NORMAL);

    void PremultiplyAlpha(CImage& image);
    void DestroyImage(CImage& image);

    void GoNext();
    void GoPrev();

private:
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    // 0: 조작법 설명, 1: 판정 점수 설명
    int currentPage = 0;
    bool finished = false;

    CImage backgroundImage;
    CImage hitCircleImage;
    CImage hitCircleOverlayImage;
    CImage approachCircleImage;
    CImage sliderStartCircleImage;
    CImage sliderStartCircleOverlayImage;
    CImage sliderBallImage;
    CImage sliderFollowCircleImage;
    CImage hit300Image;
    CImage hit100Image;
    CImage hit50Image;
    CImage hit0Image;
    CImage missReactionImage;

    RECT prevButtonRect = { 1180, 960, 1390, 1025 };
    RECT nextButtonRect = { 1430, 960, 1700, 1025 };
    RECT skipButtonRect = { 1725, 45, 1865, 100 };
    RECT pageDotArea = { 860, 975, 1060, 1020 };
};
