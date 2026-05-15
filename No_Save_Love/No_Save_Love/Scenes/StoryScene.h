#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <atlimage.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

// 대사 한 줄의 정보다.
// StoryScene 밖에서도 사용할 수 있게 class 밖에 둔다.
struct DialogueLineInfo
{
    std::wstring speaker; // 말하는 사람 이름
    std::wstring text;    // 실제 대사 내용

    std::wstring backgroundKey = L""; // 배경 이미지 이름 또는 키
    std::wstring characterKey = L"";  // 캐릭터 이미지 이름 또는 키
    std::wstring effectKey = L"";     // 흔들림, 페이드 같은 효과
};
//페이드 아웃 인 
enum class FadeState
{
    None, // 페이드 효과 없음
    FadeOut, // 점점 어두워지는 중
    FadeIn // 다시 밝아지는 중
};

class StoryScene
{
public:
    // 타이핑 효과와 페이드 효과를 한 번에 갱신한다.
    // GameManager는 이 함수만 호출하면 된다.
    void Update();

    // StoryScene 기본 초기화
    void Initialize();

    // StoryScene 종료 시 데이터 정리
    void Shutdown();

    // GameManager가 새로운 대사 묶음을 넣어줄 때 사용한다.
    void SetDialogues(const std::vector<DialogueLineInfo>& newDialogues);

    //플레이어 이름 저장한다.
    void SetPlayerName(const std::wstring& playerName);

    // 마우스 클릭 시 다음 대사로 이동한다.
    void OnMouseClick(int x, int y);

    // 현재 대사 묶음이 끝났는지 확인한다.
    bool IsFinished() const;

    // 대사 화면을 출력한다.
    void Render(HDC hDC);

    // 타이핑 효과 대사 단어 하나씩 늘리기
    void UpdateTyping();

    // 현재 대사의 backgroundKey, characterKey를 보고 출력 상태를 갱신한다.
    // immediateApply가 true면 페이드 없이 바로 적용한다.
    void ApplyCurrentLineVisualInfo(bool immediateApply);

    // SKIP 버튼을 그린다.
    void DrawSkipButton(HDC hDC);



    CImage normal_back_ground;

    // 현재 배경 key에 맞는 배경 이미지를 반환한다. (복사 하지 않기 위해 포인터 사용)
    CImage* GetBackgroundImage(const std::wstring& backgroundKey);

    // 현재 캐릭터 key에 맞는 캐릭터 이미지를 반환한다.
// 캐릭터 PNG는 알파 처리를 위해 GDI+ Image로 관리한다.
    Gdiplus::Image* GetCharacterImage(const std::wstring& characterKey);

    void StartFadeTransition(const std::wstring& newBackgroundKey, const std::wstring& newCharacterKey);

    void UpdateFade();

private:
    // fadeAlpha 값에 따라 검은 반투명 레이어를 화면에 덮는다.
    void DrawFadeOverlay(HDC hDC);

    // GDI+ 초기화 토큰이다.
// GDI+를 시작하고 종료할 때 필요하다.
    ULONG_PTR gdiplusToken = 0;
    // GDI+를 사용해서 캐릭터 이미지를 출력한다.
// PNG 알파 채널과 외곽선을 더 자연스럽게 처리하기 위한 함수이다.
    void DrawCharacterImage(HDC hDC, Gdiplus::Image* image, int x, int y);

    // GDI+가 정상적으로 시작되었는지 확인하는 값이다.
    bool isGdiPlusStarted = false;

    struct SpeakerStyle
    {
        COLORREF nameColor; // 이름 색상
        COLORREF textColor; // 대사 색상
        COLORREF accent;    // 포인트 라인 색상
        COLORREF outline;   // 대화창 테두리 색상
    };

    struct heroine_image
    {
        // GDI+ 이미지 포인터이다.
        // PNG 알파 채널을 더 안정적으로 출력하기 위해 CImage 대신 GDI+ Image를 사용한다.
        Gdiplus::Image* normal = nullptr;
    };



private:
    // 화자 이름에 맞는 색상 스타일을 가져온다.
    SpeakerStyle GetSpeakerStyle(const std::wstring& speaker) const;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꾼다
    std::wstring ReplacePlayerNameToken(const std::wstring& text) const;

    // 현재 대사의 화자 이름을 {PLAYER} 치환까지 적용해서 반환한다.
    std::wstring GetCurrentDisplaySpeaker() const;

    // 현재 대사의 본문을 {PLAYER} 치환까지 적용해서 반환한다.
    std::wstring GetCurrentDisplayText() const;

private:
    CImage story_background_image[3];

    heroine_image hansea;
    heroine_image seoirin;
    heroine_image yuharin;

    // 현재 출력 중인 배경 key
    std::wstring currentBackgroundKey = L"";

    // 현재 출력 중인 캐릭터 key
    std::wstring currentCharacterKey = L"";

    // 다음에 바꿀 배경 key
    std::wstring nextBackgroundKey;

    // 다음에 바꿀 캐릭터 key
    std::wstring nextCharacterKey;

    // 현재 페이드 상태
    FadeState fadeState = FadeState::None;

    // 검은 화면의 투명도
    // 0이면 안 보임, 255면 완전 검정
    int fadeAlpha = 255;

    // 페이드 속도
    int fadeSpeed = 25;

    // 현재 StoryScene이 출력할 대사 목록
    std::vector<DialogueLineInfo> dialogues;

    // 현재 출력 중인 대사 번호
    int currentDialogueIndex = 0;

    // 타이핑 효과 현재 출력중인 단어 번호
    int visibleTextCount = 0;

    // 타이핑이 끝났는지 확인
    bool isTypingFinished = false;

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

    // 플레이어가 입력한 이름
    std::wstring m_playerName = L"윤서"; // 기본은 윤서


    // 오른쪽 위 SKIP 버튼 영역
    RECT skipButtonRect = { 1700, 40, 1860, 100 };


};
