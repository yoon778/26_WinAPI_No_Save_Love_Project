#pragma once

#include <windows.h>
#include <string>

class NameInputScene
{
public:
    // 이름 입력 화면 초기화
    void Initialize();

    void Shutdown();

    void Reset();

    void OnMouseClick(int x, int y);

    // 키보드 문자 입력 처리
    void OnChar(wchar_t inputChar);

    void Render(HDC hDC);

    // 이름 입력이 끝났는지 확인
    bool IsFinished() const;

    // gamemanager 한테 전달
    std::wstring GetPlayerName() const;

private:
    // 확인 버튼을 눌렀는지 확인
    bool IsClickConfirmButton(int x, int y) const;

    // 입력 박스를 눌렀는지 확인
    bool IsClickInputBox(int x, int y) const;

    // 현재 입력된 이름을 확정한다.
    void ConfirmName();

    // 폰트를 만들 때 사용하는 보조 함수
    HFONT CreateSceneFont(int fontSize, int fontWeight) const;

private:
    // 플레이어가 입력 중인 이름
    std::wstring m_inputName;

    // 이름 입력 완료 여부
    bool m_isFinished = false;

    // 입력 박스가 활성화되어 있는지 여부
    bool m_isInputActive = true;

    // 이름 최대 글자 수
    static const int MAX_NAME_LENGTH = 6;

    // 1920x1080 기준 입력 박스 영역
    RECT m_inputBoxRect = { 560, 410, 1360, 540 };

    // 입력 박스 안쪽 텍스트 영역
    RECT m_inputTextRect = { 600, 435, 1320, 515 };

    // 확인 버튼 영역
    RECT m_confirmButtonRect = { 800, 650, 1120, 750 };
};