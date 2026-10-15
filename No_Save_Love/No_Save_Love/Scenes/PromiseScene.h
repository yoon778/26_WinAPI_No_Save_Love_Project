#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <array>
#include <string>

// 다섯 번째 미니게임: 물건의 짝, 사건의 순서, 작은 상자의 공간을 함께 정리한다.
class PromiseScene
{
public:
    void Initialize(int heroine, bool tutorial = false);
    void Shutdown();
    void Update();
    void Render(HDC dc);
    void OnMouseDown(int x, int y);
    void OnMouseMove(int x, int y);
    void OnMouseUp(int x, int y);
    void CancelDrag();
    bool IsDragging() const { return dragging; }
    bool IsTutorial() const { return tutorial; }
    bool IsFinished() const { return finished; }
    int GetScore() const;
    void FinishImmediately() { CancelDrag(); finished = true; }

private:
    struct Item { const wchar_t* name; const wchar_t* clue; const wchar_t* destination; const wchar_t* reply; };
    static const Item items[3][3][6];
    static constexpr int widths[6] = {2, 1, 1, 2, 1, 1};
    static constexpr int heights[6] = {1, 2, 2, 2, 1, 1};
    static constexpr int packingSolution[6] = {8, 2, 3, 0, 10, 11};
    static constexpr int trayOrder[6] = {3, 0, 5, 1, 4, 2};
    static constexpr int pairOrder[6] = {2, 5, 1, 4, 0, 3};
    int heroine = 0, stage = 0, selected = -1, completed = 0;
    std::array<int, 6> positions{};
    std::array<bool, 3> hints{};
    bool tutorial = false, finished = false, dragging = false, moved = false;
    POINT mouse{}, dragStart{};
    std::wstring feedback;
    double celebration = 0;
    ULONGLONG lastTick = 0;
    ULONG_PTR graphicsToken = 0;
    Gdiplus::Image* background = nullptr;
    Gdiplus::Image* portrait = nullptr;

    int PlacedCount() const;
    int TargetFor(int item) const;
    RECT SlotRect(int slot) const;
    RECT ItemRect(int item) const;
    bool Fits(int item, int cell) const;
    bool TryPlace(int item, int x, int y);
    void NextStage();
    void DrawItem(Gdiplus::Graphics& g, int item, RECT rect);
};
