#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <atlimage.h>

class EndingScene
{
public:
    void Initialize(const std::vector<std::wstring>& credits);
    void Shutdown();
    void Reset();
    void SetEndingImage(int endingType, int heroineIndex);
    void SetPlayerName(const std::wstring& playerName);
    void OnMouseClick(int x, int y);
    void OnKeyDown(WPARAM key);
    bool IsExitRequested() const;
    bool IsReturnRequested() const { return returnRequested; }
    void Update(float deltaSeconds = 0.06f);
    void Render(HDC hDC);

private:
    std::wstring GetEndingImagePath(int endingType, int heroineIndex) const;
    std::wstring ReplacePlayerNameToken(const std::wstring& text) const;
    bool IsCreditHeading(const std::wstring& text) const;
    HFONT CreateCreditFont(int fontSize, int fontWeight) const;
    void DrawBackground(HDC hDC);
    void DrawCreditLine(HDC hDC, const std::wstring& text, int y, int index);
    void DrawExitButton(HDC hDC);
    void DrawButton(HDC hDC, RECT rect, const wchar_t* label);
    bool IsCreditFinished() const;

private:
    std::vector<std::wstring> creditLines;
    std::wstring m_playerName = L"윤서";
    CImage endingImage;
    CImage backgroundFrame;

    int scrollOffset = 0;
    double scrollRemainder = 0.0;
    int scrollSpeed = 2;
    int fastScrollSpeed = 32;
    int lineSpacing = 68;
    int startY = 1120;
    int stopOffset = 0;
    bool isFastScroll = false;
    bool exitRequested = false;
    bool returnRequested = false;
    RECT fastButtonRect = { 1690, 24, 1858, 64 };
    RECT returnButtonRect = { 620, 910, 950, 990 };
    RECT exitButtonRect = { 990, 910, 1300, 990 };
};
