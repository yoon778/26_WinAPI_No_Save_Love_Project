#include <windows.h>
#include <tchar.h>
#include <random>

// 기본 창 설정
int location_x = 0; // 창 위치
int location_y = 0;
int size_w = 1920; //창 크기
int size_h = 1080;

HINSTANCE g_hinst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"NO_SAVE_LOVE";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM Wparam, LPARAM IPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevinstance, LPSTR lpszCmdParam, int nCmdShow) {
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

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return Message.wParam;
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
    HDC hDC;
    RECT rt;
    switch (uMsg) {
    case WM_CREATE: {
        g_gameManager.Initialize(hWnd);
        SetTimer(hWnd, 1, 60, nullptr);
        
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
    case WM_LBUTTONDOWN:
    {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);
        
        g_gameManager.OnMouseClick(mouseX, mouseY);
        return 0;
    }
    case WM_TIMER: {

        g_gameManager.OnTimer(hWnd);
        return 0;
    }


    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        GetClientRect(hWnd, &rt);
        hDC = BeginPaint(hWnd, &ps);

        int bufferWidth = rt.right - rt.left;
        int bufferHeight = rt.bottom - rt.top;
        if (!EnsureBackBuffer(hDC, bufferWidth, bufferHeight))
        {
            EndPaint(hWnd, &ps);
            return 0;
        }

        // 배경 흰색으로 지우기
        FillRect(g_backBufferDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // 백버퍼로 그리기 하기!!!
        g_gameManager.Render(g_backBufferDC);


        BitBlt(hDC, 0, 0, bufferWidth, bufferHeight, g_backBufferDC, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_LBUTTONUP:
    {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);
        g_gameManager.OnMouseUp(mouseX, mouseY);
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);
        g_gameManager.OnMouseMove(mouseX, mouseY);
        return 0;
    }
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
