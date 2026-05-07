#include <windows.h>
#include <tchar.h>
#include <random>
#include "PCroomgame.h"
using namespace std;

// 기본 창 설정
int location_x = 0; // 창 위치
int location_y = 0;
int size_w = 1920; //창 크기
int size_h = 1080;

// pc방 미니게임
PCroomgame pcGame;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dist_x(0, 600);

HINSTANCE g_hinst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2_10";

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
        lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
    HDC hDC, mDC;
    HBITMAP hBitmap;
    RECT rt;


    switch (uMsg) {
    case WM_CREATE: { 
        pcGame.Init();
        SetTimer(hWnd, 0, 100, NULL);
        InvalidateRect(hWnd, NULL, TRUE);  
        break;
    }
    case WM_KEYDOWN: 
    {
        pcGame.KEYDOWN(wParam);
        InvalidateRect(hWnd, NULL, TRUE);  
        break;
    }

    case WM_LBUTTONDOWN: 
    {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        // 마우스 좌표 pc방 게임에 전달
        pcGame.MOUSE(mx, my);

        InvalidateRect(hWnd, NULL, TRUE);  
        break;
    }
    case WM_LBUTTONUP: 
    {
        InvalidateRect(hWnd, NULL, TRUE);  
        break;
    }
    case WM_TIMER: {
        pcGame.Update();
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }

    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        GetClientRect(hWnd, &rt);
        HDC hDC = BeginPaint(hWnd, &ps);
        mDC = CreateCompatibleDC(hDC);
        hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom); 
        SelectObject(mDC, (HBITMAP)hBitmap);

        pcGame.PAINT(mDC);









        BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
        DeleteDC(mDC); 
        DeleteObject(hBitmap);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0); // 종료
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}