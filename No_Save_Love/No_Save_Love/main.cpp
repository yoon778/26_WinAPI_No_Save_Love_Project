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

//헤더 정리=========================================================
//윤서
#include "GameManager.h"  // 어떤 장면을 실행할지 결정함



//새누




// 선언 정리=========================================================
//윤서
GameManager g_gameManager;




//새누



//===================================================================




LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
    HDC hDC, mDC;
    HBITMAP hBitmap;
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
    case WM_LBUTTONDOWN:
    {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        g_gameManager.OnMouseClick(mouseX, mouseY);
        return 0;
    }
    case WM_TIMER: {

        g_gameManager.OnTimer();

        return 0;
    }


    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        GetClientRect(hWnd, &rt);
        hDC = BeginPaint(hWnd, &ps);
        mDC = CreateCompatibleDC(hDC);
        hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
        SelectObject(mDC, (HBITMAP)hBitmap);

        // 배경 흰색으로 지우기
        FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // mDC로 그리기 하기!!!
        g_gameManager.Render(mDC);







        BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
        DeleteDC(mDC);
        DeleteObject(hBitmap);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
    {
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}