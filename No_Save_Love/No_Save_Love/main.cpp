#include <windows.h>
#include <tchar.h>
#include <random>
#include <windowsx.h>
#include "GameOptions.h"
#include "DisplayGeometry.h"

// 기본 창 설정
int location_x = 0; // 창 위치
int location_y = 0;
int size_w = 1920; //창 크기
int size_h = 1080;

HINSTANCE g_hinst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"NO_SAVE_LOVE";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM Wparam, LPARAM IPARAM);
void ApplyWindowMode(HWND hWnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevinstance, LPSTR lpszCmdParam, int nCmdShow) {
    // Request native pixels before creating any HWND; older Windows keeps the fallback.
    auto setDpiAwareness = reinterpret_cast<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetProcessDpiAwarenessContext"));
    if (setDpiAwareness) setDpiAwareness(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    else SetProcessDPIAware();
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;
    g_hinst = hInstance;
    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExW(&WndClass);

    hWnd = CreateWindow(
        lpszClass, lpszWindowName, WS_POPUP,
        location_x, location_y, size_w, size_h,
        NULL, (HMENU)NULL, hInstance, NULL);

    if (!hWnd) return 1;
    ApplyWindowMode(hWnd);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return static_cast<int>(Message.wParam);
}

#include "GameManager.h"

GameManager g_gameManager;

namespace
{
    HDC g_backBufferDC = nullptr;
    HBITMAP g_backBufferBitmap = nullptr;
    HBITMAP g_backBufferOldBitmap = nullptr;
    int g_backBufferWidth = 0;
    int g_backBufferHeight = 0;
    UINT g_updateInterval = 16;

    POINT LogicalMouse(HWND hWnd, LPARAM lParam, bool screenCoordinates = false)
    {
        POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (screenCoordinates) ScreenToClient(hWnd, &point);
        RECT client = {};
        GetClientRect(hWnd, &client);
        return DisplayGeometry::ToLogical(point, DisplayGeometry::Fit(client.right, client.bottom));
    }

    void ReleaseBackBuffer()
    {
        if (g_backBufferDC != nullptr)
        {
            if (g_backBufferOldBitmap != nullptr)
            {
                SelectObject(g_backBufferDC, g_backBufferOldBitmap);
                g_backBufferOldBitmap = nullptr;
            }

            if (g_backBufferBitmap != nullptr)
            {
                DeleteObject(g_backBufferBitmap);
                g_backBufferBitmap = nullptr;
            }

            DeleteDC(g_backBufferDC);
            g_backBufferDC = nullptr;
        }

        g_backBufferWidth = 0;
        g_backBufferHeight = 0;
    }

    bool EnsureBackBuffer(HDC hDC, int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return false;
        }

        if (g_backBufferDC != nullptr &&
            g_backBufferBitmap != nullptr &&
            g_backBufferWidth == width &&
            g_backBufferHeight == height)
        {
            return true;
        }

        ReleaseBackBuffer();

        g_backBufferDC = CreateCompatibleDC(hDC);
        if (g_backBufferDC == nullptr)
        {
            return false;
        }

        g_backBufferBitmap = CreateCompatibleBitmap(hDC, width, height);
        if (g_backBufferBitmap == nullptr)
        {
            ReleaseBackBuffer();
            return false;
        }

        g_backBufferOldBitmap = static_cast<HBITMAP>(SelectObject(g_backBufferDC, g_backBufferBitmap));
        if (g_backBufferOldBitmap == nullptr)
        {
            ReleaseBackBuffer();
            return false;
        }

        g_backBufferWidth = width;
        g_backBufferHeight = height;
        return true;
    }
}

void ApplyWindowMode(HWND hWnd)
{
    MONITORINFO monitor = { sizeof(monitor) };
    GetMonitorInfoW(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &monitor);
    const DWORD state = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)) &
        (WS_VISIBLE | WS_DISABLED | WS_MINIMIZE);
    const DWORD style = (GameOptions::fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW) | state;
    SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    RECT window = monitor.rcMonitor;
    if (!GameOptions::fullscreen)
    {
        const int availableWidth = monitor.rcWork.right - monitor.rcWork.left;
        const int availableHeight = monitor.rcWork.bottom - monitor.rcWork.top;
        const RECT fit = DisplayGeometry::Fit(availableWidth * 4 / 5, availableHeight * 4 / 5);
        window = { 0, 0, fit.right - fit.left, fit.bottom - fit.top };
        AdjustWindowRectExForDpi(&window, style, FALSE, 0, GetDpiForWindow(hWnd));
        const int width = window.right - window.left;
        const int height = window.bottom - window.top;
        window = { monitor.rcWork.left + (availableWidth - width) / 2,
            monitor.rcWork.top + (availableHeight - height) / 2, 0, 0 };
        window.right = window.left + width;
        window.bottom = window.top + height;
    }
    SetWindowPos(hWnd, nullptr, window.left, window.top, window.right - window.left,
        window.bottom - window.top, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(hWnd, nullptr, FALSE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
    HDC hDC;
    RECT rt;
    switch (uMsg) {
    case WM_CREATE: {
        g_gameManager.Initialize(hWnd);
        SetTimer(hWnd, 1, g_updateInterval, nullptr);
        
        return 0;
    }
    case WM_CHAR:
    {
        // 현재 입력된 문자를 GameManager로 넘긴다.
        g_gameManager.OnChar(static_cast<wchar_t>(wParam));

        // 화면을 다시 그리게 한다.
        InvalidateRect(hWnd, NULL, FALSE);

        return 0;
    }
    case WM_KEYDOWN:
    {
        const bool isRepeatedKey = (lParam & (1 << 30)) != 0;
        if (isRepeatedKey)
        {
            return 0;
        }
        if (wParam == VK_F11)
        {
            GameOptions::fullscreen = !GameOptions::fullscreen;
            ApplyWindowMode(hWnd);
            g_gameManager.OnWindowModeChanged();
            return 0;
        }

        // F1~F4 같은 특수 키 입력을 GameManager로 넘긴다.
        g_gameManager.OnKeyDown(wParam);

        // 화면을 다시 그리게 한다.
        InvalidateRect(hWnd, NULL, FALSE);

        return 0;
    }
    case WM_KEYUP:
    {
        // 방향키를 떼는 입력을 GameManager로 넘긴다.
        g_gameManager.OnKeyUp(wParam);

        // 화면을 다시 그리게 한다.
        InvalidateRect(hWnd, NULL, FALSE);

        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        const POINT mouse = LogicalMouse(hWnd, lParam, true);
        if (mouse.x >= 0)
        {
            g_gameManager.OnMouseMove(mouse.x, mouse.y);
            g_gameManager.OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        }
        InvalidateRect(hWnd, nullptr, FALSE);
        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    {
        const POINT mouse = LogicalMouse(hWnd, lParam);
        if (mouse.x >= 0) g_gameManager.OnMouseClick(mouse.x, mouse.y);
        return 0;
    }
    case WM_TIMER: {
        g_gameManager.OnTimer(hWnd);
        const UINT interval=g_gameManager.GetUpdateInterval();
        if (interval!=g_updateInterval)
        {
            g_updateInterval=interval;
            SetTimer(hWnd,1,interval,nullptr);
        }
        return 0;
    }


    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        GetClientRect(hWnd, &rt);
        hDC = BeginPaint(hWnd, &ps);

        int bufferWidth = rt.right - rt.left;
        int bufferHeight = rt.bottom - rt.top;
        if (!EnsureBackBuffer(hDC, DisplayGeometry::Width, DisplayGeometry::Height))
        {
            EndPaint(hWnd, &ps);
            return 0;
        }

        // 배경 흰색으로 지우기
        RECT logicalRect = { 0, 0, DisplayGeometry::Width, DisplayGeometry::Height };
        FillRect(g_backBufferDC, &logicalRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        // 백버퍼로 그리기 하기!!!
        g_gameManager.Render(g_backBufferDC);


        const RECT viewport = DisplayGeometry::Fit(bufferWidth, bufferHeight);
        FillRect(hDC, &rt, (HBRUSH)GetStockObject(BLACK_BRUSH));
        if (viewport.right > viewport.left && viewport.bottom > viewport.top)
        {
            const int width = viewport.right - viewport.left;
            const int height = viewport.bottom - viewport.top;
            if (width == DisplayGeometry::Width && height == DisplayGeometry::Height)
                BitBlt(hDC, viewport.left, viewport.top, width, height, g_backBufferDC, 0, 0, SRCCOPY);
            else
            {
                SetStretchBltMode(hDC, HALFTONE);
                SetBrushOrgEx(hDC, 0, 0, nullptr);
                StretchBlt(hDC, viewport.left, viewport.top, width, height,
                    g_backBufferDC, 0, 0, DisplayGeometry::Width, DisplayGeometry::Height, SRCCOPY);
            }
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_CAPTURECHANGED:
        g_gameManager.OnCaptureLost();
        return 0;
    case WM_LBUTTONUP:
    {
        const POINT mouse = LogicalMouse(hWnd, lParam);
        g_gameManager.OnMouseUp(mouse.x, mouse.y);
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        const POINT mouse = LogicalMouse(hWnd, lParam);
        g_gameManager.OnMouseMove(mouse.x, mouse.y);
        return 0;
    }
    case WM_APP + 10:
        ApplyWindowMode(hWnd);
        g_gameManager.OnWindowModeChanged();
        return 0;
    case WM_DPICHANGED:
    {
        if (GameOptions::fullscreen) ApplyWindowMode(hWnd);
        else
        {
            const RECT& suggested = *reinterpret_cast<const RECT*>(lParam);
            SetWindowPos(hWnd, nullptr, suggested.left, suggested.top,
                suggested.right - suggested.left, suggested.bottom - suggested.top,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        g_gameManager.OnWindowModeChanged();
        return 0;
    }
    case WM_SIZE:
        InvalidateRect(hWnd, nullptr, FALSE);
        return 0;
    case WM_DISPLAYCHANGE:
        if (GameOptions::fullscreen)
        {
            ApplyWindowMode(hWnd);
            g_gameManager.OnWindowModeChanged();
        }
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_KILLFOCUS:
        g_gameManager.OnKeyUp(VK_LEFT);
        g_gameManager.OnKeyUp(VK_RIGHT);
        g_gameManager.OnKeyUp(VK_DOWN);
        g_gameManager.OnKeyUp(VK_UP);
        g_gameManager.OnCaptureLost();
        g_gameManager.OnFocusLost();
        return 0;
    case WM_DESTROY:
    {
        KillTimer(hWnd, 1);
        g_gameManager.Shutdown();
        ReleaseBackBuffer();
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
