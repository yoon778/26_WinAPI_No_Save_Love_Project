#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <atlimage.h>
#include <gdiplus.h>
#include <array>
#include <vector>
#include <span>
#include <string>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <random>
#include <cassert>
#include <iostream>
#define private public
#include "../No_Save_Love/GameManager.h"
#undef private

HWND invalidationWindow = nullptr;
int invalidationCount = 0;
BOOL WINAPI RecordedInvalidateRect(HWND window, const RECT* rect, BOOL erase)
{
    if (window == invalidationWindow) ++invalidationCount;
    return InvalidateRect(window, rect, erase);
}
#define InvalidateRect RecordedInvalidateRect
#include "../No_Save_Love/GameManager.cpp"
#include "../No_Save_Love/GameManagerMenus.cpp"
#undef InvalidateRect

struct PaintOperation { bool clear; RECT bounds; HDC source; };
std::vector<PaintOperation> paintOperations;
bool recordPaint = false;
HDC paintOutput = nullptr;

HDC WINAPI RecordedBeginPaint(HWND window, LPPAINTSTRUCT paint)
{
    HDC dc = BeginPaint(window, paint);
    if (recordPaint) paintOutput = dc;
    return dc;
}
int WINAPI RecordedFillRect(HDC dc, const RECT* rect, HBRUSH brush)
{
    if (recordPaint && dc == paintOutput) paintOperations.push_back({true, *rect, nullptr});
    return FillRect(dc, rect, brush);
}
BOOL WINAPI RecordedBitBlt(HDC dc, int x, int y, int width, int height,
    HDC source, int sourceX, int sourceY, DWORD operation)
{
    if (recordPaint && dc == paintOutput) paintOperations.push_back({false, {x, y, x + width, y + height}, source});
    return BitBlt(dc, x, y, width, height, source, sourceX, sourceY, operation);
}
BOOL WINAPI RecordedStretchBlt(HDC dc, int x, int y, int width, int height,
    HDC source, int sourceX, int sourceY, int sourceWidth, int sourceHeight, DWORD operation)
{
    if (recordPaint && dc == paintOutput) paintOperations.push_back({false, {x, y, x + width, y + height}, source});
    return StretchBlt(dc, x, y, width, height, source, sourceX, sourceY, sourceWidth, sourceHeight, operation);
}
#define BeginPaint RecordedBeginPaint
#define FillRect RecordedFillRect
#define BitBlt RecordedBitBlt
#define StretchBlt RecordedStretchBlt
#include "../No_Save_Love/main.cpp"
#undef BeginPaint
#undef FillRect
#undef BitBlt
#undef StretchBlt

bool IsFlickerFreePaint(const std::vector<PaintOperation>& operations, RECT client, RECT viewport)
{
    // Check the intermediate GDI calls, not just the final (already repaired) pixels.
    int copies = 0;
    HRGN margins = CreateRectRgnIndirect(&client);
    HRGN view = CreateRectRgnIndirect(&viewport);
    CombineRgn(margins, margins, view, RGN_DIFF);
    DeleteObject(view);
    bool valid = true;
    for (const auto& operation : operations)
    {
        if (operation.clear)
        {
            RECT overlap{};
            if (IntersectRect(&overlap, &operation.bounds, &viewport)) valid = false;
            HRGN cleared = CreateRectRgnIndirect(&operation.bounds);
            CombineRgn(margins, margins, cleared, RGN_DIFF);
            DeleteObject(cleared);
        }
        else
        {
            ++copies;
            if (!EqualRect(&operation.bounds, &viewport) || operation.source != g_backBufferDC) valid = false;
        }
    }
    RECT remaining{};
    valid = valid && copies == 1 && GetRgnBox(margins, &remaining) == NULLREGION;
    DeleteObject(margins);
    return valid;
}

void CheckPaintSequence(HWND window, const char* mode)
{
    RECT client{};
    GetClientRect(window, &client);
    const RECT viewport = DisplayGeometry::Fit(client.right, client.bottom);
    paintOperations.clear();
    paintOutput = nullptr;
    recordPaint = true;
    InvalidateRect(window, nullptr, FALSE);
    SendMessageW(window, WM_PAINT, 0, 0);
    recordPaint = false;
    assert(paintOutput && IsFlickerFreePaint(paintOperations, client, viewport));
    // The old full-client black clear must fail even though the final blit is identical.
    auto oldSequence = paintOperations;
    oldSequence.insert(oldSequence.begin(), {true, client, nullptr});
    assert(!IsFlickerFreePaint(oldSequence, client, viewport));
    std::cout << "PASS WM_PAINT " << mode << ": one complete frame, only letterbox clears; old full-client clear rejected\n";
}

// Real production WndProc, sent only to this test's hidden window.
POINT ClientPoint(HWND window, int logicalX, int logicalY)
{
    RECT client{};
    GetClientRect(window, &client);
    const RECT view = DisplayGeometry::Fit(client.right, client.bottom);
    return {view.left + MulDiv(logicalX, view.right - view.left, 1920),
        view.top + MulDiv(logicalY, view.bottom - view.top, 1080)};
}

void Click(HWND window, int x, int y, UINT message = WM_LBUTTONDOWN)
{
    const POINT point = ClientPoint(window, x, y);
    SendMessageW(window, message, MK_LBUTTON, MAKELPARAM(point.x, point.y));
    SendMessageW(window, WM_LBUTTONUP, 0, MAKELPARAM(point.x, point.y));
}

void CompleteFade(HWND window)
{
    for (int tick = 0; tick < 160 && g_gameManager.sceneTransition.IsActive(); ++tick)
    {
        // Supply elapsed time without making this integration check wait on a timer.
        g_gameManager.m_lastUpdateTick = GetTickCount64() - 16;
        SendMessageW(window, WM_TIMER, 1, 0);
    }
    assert(!g_gameManager.sceneTransition.IsActive());
}

void CheckSettings(HWND window)
{
    Click(window, 650, 860);
    assert(g_gameManager.m_panel == GameManager::Panel::Settings);
    assert(g_gameManager.m_exitConfirmOpen);
    const int effectsBefore = GameOptions::effectsVolume;
    Click(window, 1590, 520);
    assert(GameOptions::effectsVolume == (std::min)(100, effectsBefore + 10));
    assert(GetPrivateProfileIntW(L"settings", L"effects", -1, g_gameManager.m_profile.storagePath.c_str()) == GameOptions::effectsVolume);
    Click(window, 1620, 145);
    assert(g_gameManager.m_panel == GameManager::Panel::None && !g_gameManager.m_exitConfirmOpen);
}

int wmain(int argc, wchar_t** argv)
{
    std::cout << std::unitbuf;
    assert(argc == 2);
    const std::filesystem::path profile = std::filesystem::absolute(argv[1]);
    assert(SetEnvironmentVariableW(L"NO_SAVE_LOVE_PROFILE", profile.c_str()));
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_hinst = GetModuleHandleW(nullptr);
    const wchar_t* className = L"NoSaveLoveWindowInputCheck";
    WNDCLASSW windowClass{};
    windowClass.hInstance = g_hinst;
    windowClass.lpfnWndProc = WndProc;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    assert(RegisterClassW(&windowClass));
    HWND window = CreateWindowW(className, L"Hidden input integration check", WS_POPUP,
        40, 60, 1280, 1024, nullptr, nullptr, g_hinst, nullptr);
    assert(window && !IsWindowVisible(window));
    assert(g_gameManager.m_profile.storagePath == profile.wstring());
    assert(GameOptions::musicVolume == 0); // The test profile keeps playback silent.
    KillTimer(window, 1);
    CompleteFade(window);
    assert(g_gameManager.now_game_mode == GameManager::game_mode_info::Title);
    CheckPaintSequence(window, "5:4 letterbox");

    // Hidden windows may discard update regions, so observe actual invalidation calls.
    // Hover changes redraw once; motion inside the same button does not redraw continuously.
    const RECT startButton = {760, 680, 1160, 770};
    assert(std::any_of(MenuUI::HitAreas.begin(), MenuUI::HitAreas.end(),
        [&](const RECT& area) { return EqualRect(&area, &startButton); }));
    assert(MenuUI::Pointer.x == -1 && MenuUI::Pointer.y == -1);
    invalidationWindow = window;
    invalidationCount = 0;
    POINT hoverStart = ClientPoint(window, 900, 720);
    SendMessageW(window, WM_MOUSEMOVE, 0, MAKELPARAM(hoverStart.x, hoverStart.y));
    assert(MenuUI::Pointer.x == 900 && MenuUI::Pointer.y == 720);
    assert(MenuUI::Hit(startButton, MenuUI::Pointer.x, MenuUI::Pointer.y));
    assert(invalidationCount == 1);
    hoverStart = ClientPoint(window, 920, 720);
    SendMessageW(window, WM_MOUSEMOVE, 0, MAKELPARAM(hoverStart.x, hoverStart.y));
    assert(MenuUI::Hit(startButton, MenuUI::Pointer.x, MenuUI::Pointer.y));
    assert(invalidationCount == 1);
    SendMessageW(window, WM_MOUSELEAVE, 0, 0);
    assert(MenuUI::Pointer.x == -1 && MenuUI::Pointer.y == -1 && invalidationCount == 2);
    invalidationWindow = nullptr;
    std::cout << "PASS menu hover: redraw on enter/leave, no continuous redraw inside a button\n";

    CheckSettings(window);
    RECT client{};
    GetClientRect(window, &client);
    RECT view = DisplayGeometry::Fit(client.right, client.bottom);
    assert(view.top > 0);
    SendMessageW(window, WM_LBUTTONDOWN, 0, MAKELPARAM(640, 20));
    SendMessageW(window, WM_LBUTTONDOWN, 0, MAKELPARAM(-30, -30));
    assert(g_gameManager.m_panel == GameManager::Panel::None);
    std::cout << "PASS WndProc: scaled settings and volume clicks, saved test profile, letterbox and negative click rejection\n";

    // A message-only window has no desktop surface, even when its style contains WS_VISIBLE.
    // Exercise the real style switch without showing a test window.
    HWND stateWindow = CreateWindowW(L"STATIC", L"Style-only check", 0,
        0, 0, 0, 0, HWND_MESSAGE, nullptr, g_hinst, nullptr);
    assert(stateWindow);
    SetWindowLongPtrW(stateWindow, GWL_STYLE, WS_VISIBLE | WS_DISABLED);
    ApplyWindowMode(stateWindow);
    assert((GetWindowLongPtrW(stateWindow, GWL_STYLE) & (WS_VISIBLE | WS_DISABLED)) == (WS_VISIBLE | WS_DISABLED));
    GameOptions::fullscreen = false;
    ApplyWindowMode(stateWindow);
    assert((GetWindowLongPtrW(stateWindow, GWL_STYLE) & (WS_VISIBLE | WS_DISABLED)) == (WS_VISIBLE | WS_DISABLED));
    GameOptions::fullscreen = true;
    DestroyWindow(stateWindow);
    std::cout << "PASS window mode: ApplyWindowMode preserves visibility and disabled flags on a message-only window\n";

    const bool previousFullscreen = GameOptions::fullscreen;
    SendMessageW(window, WM_KEYDOWN, VK_F11, 0);
    assert(GameOptions::fullscreen != previousFullscreen);
    assert(!GameOptions::fullscreen);
    assert((GetWindowLongPtrW(window, GWL_STYLE) & WS_CAPTION) == WS_CAPTION);
    assert(GetPrivateProfileIntW(L"settings", L"fullscreen", -1, profile.c_str()) == 0);
    CheckPaintSequence(window, "windowed");
    CheckSettings(window);
    SendMessageW(window, WM_KEYDOWN, VK_F11, 0);
    assert(GameOptions::fullscreen);
    assert((GetWindowLongPtrW(window, GWL_STYLE) & WS_CAPTION) == 0);
    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &monitor);
    RECT windowBounds{};
    GetWindowRect(window, &windowBounds);
    assert(EqualRect(&windowBounds, &monitor.rcMonitor));
    CheckPaintSequence(window, "fullscreen");
    SendMessageW(window, WM_KEYDOWN, VK_F11, 0);
    assert(!GameOptions::fullscreen && !IsWindowVisible(window));
    std::cout << "PASS WndProc: F11 switches window styles, fits the monitor and persists settings without showing the window\n";

    RECT suggested{-1200, 80, -200, 780};
    SendMessageW(window, WM_DPICHANGED, MAKELONG(144, 144), reinterpret_cast<LPARAM>(&suggested));
    GetWindowRect(window, &windowBounds);
    assert(EqualRect(&windowBounds, &suggested));
    CheckSettings(window);
    SetWindowPos(window, nullptr, -1100, 80, 1400, 600, SWP_NOZORDER | SWP_NOACTIVATE);
    CheckPaintSequence(window, "wide letterbox");
    SetWindowPos(window, nullptr, -1100, 80, 800, 1000, SWP_NOZORDER | SWP_NOACTIVATE);
    CheckSettings(window);
    CheckPaintSequence(window, "portrait letterbox");
    assert(g_backBufferDC && g_backBufferWidth == 1920 && g_backBufferHeight == 1080);
    std::cout << "PASS WndProc: DPI suggested rectangle and portrait resize keep click targets aligned; paint keeps a 1920x1080 logical buffer\n";

    // Verify WM_MOUSEWHEEL's screen coordinates, including a negative desktop X.
    g_gameManager.now_game_mode = GameManager::game_mode_info::Story;
    g_gameManager.storyScene.history.clear();
    for (int i = 0; i < 10; ++i)
        g_gameManager.storyScene.history.push_back({L"윤서", L"검사용 대사", StoryPresentation::Dialogue});
    g_gameManager.storyScene.historyOpen = true;
    g_gameManager.storyScene.historyEnd = 10;
    POINT wheel = ClientPoint(window, 960, 540);
    ClientToScreen(window, &wheel);
    assert(wheel.x < 0);
    SendMessageW(window, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA), MAKELPARAM(wheel.x, wheel.y));
    assert(g_gameManager.storyScene.historyEnd == 9);
    wheel = {20, 20}; ClientToScreen(window, &wheel);
    SendMessageW(window, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA), MAKELPARAM(wheel.x, wheel.y));
    assert(g_gameManager.storyScene.historyEnd == 9);
    g_gameManager.storyScene.historyOpen = false;
    g_gameManager.now_game_mode = GameManager::game_mode_info::Title;
    std::cout << "PASS WndProc: mouse-wheel screen-to-client routing works at negative desktop coordinates and ignores letterbox margins\n";

    SendMessageW(window, WM_KEYDOWN, VK_F5, 0);
    CompleteFade(window);
    assert(g_gameManager.now_game_mode == GameManager::game_mode_info::MiniGame5);
    Click(window, 250, 920);
    assert(g_gameManager.promiseScene.selected == 0);
    Click(window, 310, 295);
    assert(g_gameManager.promiseScene.units[0].type == 0);
    const POINT hover = ClientPoint(window, 700, 450);
    SendMessageW(window, WM_MOUSEMOVE, 0, MAKELPARAM(hover.x, hover.y));
    assert(std::abs(g_gameManager.promiseScene.mouse.x - 700) <= 3);
    assert(std::abs(g_gameManager.promiseScene.mouse.y - 450) <= 3);
    SendMessageW(window, WM_MOUSEMOVE, 0, MAKELPARAM(-20, -10));
    assert(g_gameManager.promiseScene.mouse.x == -1 && g_gameManager.promiseScene.mouse.y == -1);
    SendMessageW(window, WM_KILLFOCUS, 0, 0);
    assert(g_gameManager.m_exitConfirmOpen);
    const float practiceTime = g_gameManager.promiseScene.practiceTime;
    for (int i = 0; i < 3; ++i)
    {
        Sleep(16);
        SendMessageW(window, WM_TIMER, 1, 0);
    }
    assert(g_gameManager.promiseScene.practiceTime == practiceTime);
    SendMessageW(window, WM_KEYDOWN, VK_ESCAPE, 0);
    assert(!g_gameManager.m_exitConfirmOpen);
    Sleep(35);
    SendMessageW(window, WM_TIMER, 1, 0);
    assert(g_gameManager.promiseScene.practiceTime > practiceTime);
    std::cout << "PASS WndProc: scaled defense placement/hover, focus-loss pause, frozen timer and ESC resume\n";

    assert(DestroyWindow(window));
    assert(g_gameManager.m_hWnd == nullptr && g_backBufferDC == nullptr);
    assert(!IsWindow(window));
    UnregisterClassW(className, g_hinst);
    std::cout << "ALL HIDDEN WINDOW INPUT INTEGRATION CHECKS PASSED\n";
}
