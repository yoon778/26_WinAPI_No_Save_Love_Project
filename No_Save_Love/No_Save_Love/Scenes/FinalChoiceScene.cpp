#include "FinalChoiceScene.h"

void FinalChoiceScene::SetFinalHeroine(int heroineIndex, const std::wstring& heroineName)
{
    // 최종 히로인 번호를 저장한다.
    m_heroineIndex = heroineIndex;

    // 최종 히로인 이름을 저장한다.
    m_heroineName = heroineName;
}

void FinalChoiceScene::Reset()
{
    m_finished = false;
}

void FinalChoiceScene::OnMouseClick(int x, int y)
{

    m_finished = true;
}

bool FinalChoiceScene::IsFinished() const
{
    return m_finished;
}

void FinalChoiceScene::Render(HDC hDC)
{
    RECT backgroundRect = { 0, 0, 1920, 1080 };
    HBRUSH backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hDC, &backgroundRect, backgroundBrush);
    DeleteObject(backgroundBrush);
}
