#pragma once

#include <windows.h>
#include <string>
#include <vector>

// 대사 화면 담당

class StoryScene
{
public:
    void Initialize();
    void Shutdown();
    void OnMouseClick(int x, int y);
    void Render(HDC hDC);
    bool IsFinished() const; // gamemanager가 끝났는지 확인

private:

    struct DialogueLineInfo
    {
        std::wstring speaker;
        std::wstring text;
    };

    struct SpeakerStyle
    {
        COLORREF nameColor;
        COLORREF textColor;
        COLORREF accent;
        COLORREF outline;
    };

private:
    SpeakerStyle GetSpeakerStyle(const std::wstring& speaker) const;

private:
    std::vector<DialogueLineInfo> dialogues;
    //대사 번호
    int currentDialogueIndex = 0;

    // 대사 끝
    bool finished = false;

    // 캐릭터별 스타일
    SpeakerStyle hanseaStyle = {
        RGB(238, 242, 248),
        RGB(120, 155, 210),
        RGB(185, 205, 240),
        RGB(70, 95, 140)
    };

    SpeakerStyle yuharinStyle = {
        RGB(245, 242, 235),
        RGB(170, 70, 75),
        RGB(210, 120, 130),
        RGB(120, 80, 85)
    };

    SpeakerStyle seoirinStyle = {
        RGB(250, 250, 252),
        RGB(85, 120, 200),
        RGB(140, 180, 240),
        RGB(60, 90, 160)
    };

    // 대화창 영역
    RECT dialogueBox = { 100, 830, 1820, 1030 };

    // 이름창 영역
    RECT nameBox = { 280, 885, 430, 950 };

    // 대사 텍스트 영역
    RECT textRect = { 520, 885, 1600, 965 };

    // 대화창 위 연한 색깔 선
    int lineY = 825;
};