#pragma once

#include <windows.h>
#include <string>
#include <vector>

// 대사 한 줄의 정보다.
// StoryScene 밖에서도 사용할 수 있게 class 밖에 둔다.
struct DialogueLineInfo
{
    std::wstring speaker; // 말하는 사람 이름
    std::wstring text;    // 실제 대사 내용
};

class StoryScene
{
public:
    // StoryScene 기본 초기화
    void Initialize();

    // StoryScene 종료 시 데이터 정리
    void Shutdown();

    // GameManager가 새로운 대사 묶음을 넣어줄 때 사용한다.
    void SetDialogues(const std::vector<DialogueLineInfo>& newDialogues);

    // 마우스 클릭 시 다음 대사로 이동한다.
    void OnMouseClick(int x, int y);

    // 현재 대사 묶음이 끝났는지 확인한다.
    bool IsFinished() const;

    // 대사 화면을 출력한다.
    void Render(HDC hDC);

private:
    struct SpeakerStyle
    {
        COLORREF nameColor; // 이름 색상
        COLORREF textColor; // 대사 색상
        COLORREF accent;    // 포인트 라인 색상
        COLORREF outline;   // 대화창 테두리 색상
    };

private:
    // 화자 이름에 맞는 색상 스타일을 가져온다.
    SpeakerStyle GetSpeakerStyle(const std::wstring& speaker) const;

private:
    // 현재 StoryScene이 출력할 대사 목록
    std::vector<DialogueLineInfo> dialogues;

    // 현재 출력 중인 대사 번호
    int currentDialogueIndex = 0;

    // 대사 묶음이 끝났는지 여부
    bool finished = false;

    // 캐릭터별 색상 스타일
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

    // 이름 출력 영역
    RECT nameBox = { 280, 885, 430, 950 };

    // 대사 출력 영역
    RECT textRect = { 520, 885, 1600, 965 };

    // 대화창 위쪽 포인트 라인 위치
    int lineY = 800;
};