#pragma once

#include <windows.h>
#include <string>

class FinalChoiceScene
{
public:
    // 최종 히로인 정보를 받는다.
    void SetFinalHeroine(int heroineIndex, const std::wstring& heroineName);

    // 화면 상태를 초기화한다.
    void Reset();

    // 마우스 클릭 처리
    void OnMouseClick(int x, int y);

    // 화면이 끝났는지 확인한다.
    bool IsFinished() const;

    // 화면 출력
    void Render(HDC hDC);

private:
    // 최종 히로인 번호
    int m_heroineIndex = -1;

    // 최종 히로인 이름
    std::wstring m_heroineName;

    // 클릭해서 넘어갈 준비가 되었는지
    bool m_finished = false;
};