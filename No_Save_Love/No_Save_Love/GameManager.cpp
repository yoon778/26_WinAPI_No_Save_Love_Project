#include "GameManager.h"

bool GameManager::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;

    // 임시 대사 데이터
    m_dialogues.push_back(L"윤서: 오늘도 평범한 하루가 시작됐다.");
    m_dialogues.push_back(L"윤서: 그런데 동아리방에서 이상한 노트를 발견했다.");
    m_dialogues.push_back(L"??? : 이 봄은 반복되고 있어.");
    m_dialogues.push_back(L"윤서: 뭐라고...?");

    m_currentDialogueIndex = 0;

    return true;
}

void GameManager::Shutdown()
{
    
    m_dialogues.clear();
    m_hWnd = nullptr;
}

void GameManager::OnMouseClick(int x, int y)
{
    

    if (m_currentDialogueIndex < static_cast<int>(m_dialogues.size()) - 1)
    {
        m_currentDialogueIndex++;
    }

    InvalidateRect(m_hWnd, nullptr, TRUE);
}

void GameManager::Render(HDC hdc)
{
    if (m_dialogues.empty())
    {
        return;
    }

    const std::wstring& currentDialogue = m_dialogues[m_currentDialogueIndex];

    TextOutW(
        hdc,
        50,
        50,
        currentDialogue.c_str(),
        static_cast<int>(currentDialogue.length())
    );
}