#pragma once

#include <windows.h>
#include <string>
#include <vector>

class EndingScene
{
public:
    void Initialize(const std::vector<std::wstring>& credits);
    void Reset();
    void Update();
    void Render(HDC hDC);

private:
    void DrawCreditLine(HDC hDC, const std::wstring& text, int y, int index);

private:
    std::vector<std::wstring> creditLines;

    int scrollOffset = 0;
    int scrollSpeed = 2;
    int lineSpacing = 68;
    int startY = 1120;
    int stopOffset = 0;
};
