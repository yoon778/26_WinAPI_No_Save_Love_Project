#pragma once

#include <windows.h>

// 모든 장면이 따라야 하는 최소 규칙이다.

class IScene
{
public:
    // 부모 포인터로 자식 Scene을 삭제할 수 있으므로 가상 소멸자를 둔다.
    virtual ~IScene() = default;

    // 이 장면에 들어올 때 한 번 호출된다.
    virtual void OnEnter() {}

    // 이 장면에서 나갈 때 한 번 호출된다.
    virtual void OnExit() {}

    // 마우스 클릭이 들어왔을 때 호출된다.
    virtual void OnMouseClick(int x, int y) {}

    // 화면을 그릴 때 호출된다.
    virtual void Render(HDC hdc) = 0;
};