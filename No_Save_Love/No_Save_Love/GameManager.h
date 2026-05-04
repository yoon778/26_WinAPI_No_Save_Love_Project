#pragma once

#include <windows.h>
#include <string>
#include <vector>

class GameManager
{
public:
    // 게임 시작
    bool Initialize(HWND hWnd);

    // 게임 종료
    void Shutdown();

    // 마우스 클릭이 발생했을 때
    void OnMouseClick(int x, int y);

    // 화면을 그릴 때
    void Render(HDC hdc);

private:
    // 현재 게임이 실행되는 윈도우 핸들
    HWND m_hWnd = nullptr;

    // 대사 목록
    std::vector<std::wstring> m_dialogues;

    // 현재 출력 중인 대사 번호
    int m_currentDialogueIndex = 0;
};