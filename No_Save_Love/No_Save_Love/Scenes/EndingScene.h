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
    void Update();
    void Render(HDC hDC);

private:
    std::wstring GetEndingImagePath(int endingType, int heroineIndex) const;
    void DrawBackground(HDC hDC);
    void DrawCreditLine(HDC hDC, const std::wstring& text, int y, int index);

private:
    std::vector<std::wstring> creditLines;
    CImage endingImage;

    int scrollOffset = 0;
    int scrollSpeed = 2;
    int lineSpacing = 68;
    int startY = 1120;
    int stopOffset = 0;
};
