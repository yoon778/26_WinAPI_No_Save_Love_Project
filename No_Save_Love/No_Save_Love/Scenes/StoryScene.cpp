#include "StoryScene.h"

void StoryScene::Initialize()
{
    // =========================
    // GDI+ 초기화
    // =========================
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    // GDI+를 시작한다.
    // 성공하면 gdiplusToken에 종료할 때 필요한 값이 저장된다.
    if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) == Gdiplus::Ok)
    {
        isGdiPlusStarted = true;
    }

    // =========================
    // 배경 이미지는 기존 CImage로 로드한다.
    // =========================
    story_background_image[0].Load(L"resource\\background\\hansea_student_council_room.png");
    story_background_image[1].Load(L"resource\\background\\yuharin_class_room.png");
    story_background_image[2].Load(L"resource\\background\\seoirin_library.png");

    // =========================
    // 캐릭터 이미지는 GDI+ Image로 로드한다.
    // PNG 투명 알파 처리를 더 안정적으로 하기 위해 CImage 대신 사용한다.
    // =========================
    hansea.normal = new Gdiplus::Image(L"resource\\heroine\\hansea\\normal.png");
    seoirin.normal = new Gdiplus::Image(L"resource\\heroine\\seoirin\\normal.png");
    yuharin.normal = new Gdiplus::Image(L"resource\\heroine\\yuharin\\normal.png");

    // 처음에는 비워둔다.
    dialogues.clear();
    currentDialogueIndex = 0;
    visibleTextCount = 0;
    isTypingFinished = false;
    finished = false;
}

void StoryScene::Shutdown()
{
    dialogues.clear();

    // 배경 이미지는 기존 CImage 방식으로 해제한다.
    for (int i = 0; i < 3; i++)
    {
        if (!story_background_image[i].IsNull())
        {
            story_background_image[i].Destroy();
        }
    }

    // 캐릭터 이미지는 GDI+ Image 포인터이므로 delete로 해제한다.
    if (hansea.normal != nullptr)
    {
        delete hansea.normal;
        hansea.normal = nullptr;
    }

    if (yuharin.normal != nullptr)
    {
        delete yuharin.normal;
        yuharin.normal = nullptr;
    }

    if (seoirin.normal != nullptr)
    {
        delete seoirin.normal;
        seoirin.normal = nullptr;
    }

    // GDI+를 종료한다.
    if (isGdiPlusStarted)
    {
        Gdiplus::GdiplusShutdown(gdiplusToken);
        isGdiPlusStarted = false;
        gdiplusToken = 0;
    }

    currentDialogueIndex = 0;
    visibleTextCount = 0;
    isTypingFinished = false;
    finished = false;
}

void StoryScene::SetDialogues(const std::vector<DialogueLineInfo>& newDialogues)
{
    dialogues = newDialogues;

    currentDialogueIndex = 0;
    finished = false;

    visibleTextCount = 0;
    isTypingFinished = false;

    // 새 대사 묶음의 첫 줄에 배경/캐릭터 정보가 있으면 적용한다.
    ApplyCurrentLineVisualInfo();
}

void StoryScene::OnMouseClick(int x, int y)
{
    // 대사가 없으면 바로 끝난 것으로 처리
    if (dialogues.empty())
    {
        finished = true;
        return;
    }
    if (finished)
    {
        return;
    }

    // 현재 대사의 전체 텍스트를 가져온다
    std::wstring displayText = GetCurrentDisplayText();

    // 아직 타이핑 중이라면 전체 문장을 보여준다.
    if (!isTypingFinished)
    {
        visibleTextCount = static_cast<int>(displayText.length());
        isTypingFinished = true;
        return;
    }

    // 타이핑이 끝난 상태에서 클릭하면 다음 대사로 이동
    if (currentDialogueIndex < static_cast<int>(dialogues.size()) - 1)
    {
        currentDialogueIndex++;

        visibleTextCount = 0;
        isTypingFinished = false;

        // 새 대사로 넘어갔으므로 배경/캐릭터 정보를 갱신한다.
        ApplyCurrentLineVisualInfo();
    }
    else
    {
        // 마지막 대사에서 클릭하면 끝난 상태로 만든다.
        finished = true;
    }
}

bool StoryScene::IsFinished() const
{
    return finished;
}

StoryScene::SpeakerStyle StoryScene::GetSpeakerStyle(const std::wstring& speaker) const
{
    // 기본 스타일
    SpeakerStyle style = {
        RGB(230, 220, 255),
        RGB(245, 245, 245),
        RGB(210, 190, 255),
        RGB(80, 70, 120)
    };

    if (speaker == L"한세아")
    {
        return hanseaStyle;

    }
    else if (speaker == L"유하린")
    {
        return yuharinStyle;
    }
    else if (speaker == L"서이린")
    {
        return seoirinStyle;
    }

    return style;
}



void StoryScene::Render(HDC hDC)
{


    // =========================
    //  배경 이미지 출력
    // =========================
    CImage* backgroundImage = GetBackgroundImage(currentBackgroundKey);

    if (backgroundImage != nullptr && !backgroundImage->IsNull())
    {
        backgroundImage->Draw(hDC, 0, 0, 1920, 1080);
    }
    else
    {
        // 배경 key가 없거나 이미지가 없을 때 임시 배경색
        HBRUSH bgBrush = CreateSolidBrush(RGB(230, 230, 240));
        RECT bgRect = { 0, 0, 1920, 1080 };
        FillRect(hDC, &bgRect, bgBrush);
        DeleteObject(bgBrush);
    }

    // =========================
    // 캐릭터 이미지 출력
    // =========================
    Gdiplus::Image* characterImage = GetCharacterImage(currentCharacterKey);

    if (characterImage != nullptr && characterImage->GetLastStatus() == Gdiplus::Ok)
    {
        // 1408 x 3046 원본 캐릭터를 1920 x 1080 화면에 맞춰 출력하는 예시 좌표이다.
        int characterX = 725;
        int characterY = 25;

        // GDI+로 캐릭터 이미지를 출력한다.
        DrawCharacterImage(hDC, characterImage, characterX, characterY);
    }


    // 출력할 대사가 없으면 그리지 않는다.
    if (dialogues.empty())
    {
        return;
    }
    // =========================
    //  대사 출력
    // =========================
    const std::wstring& originalSpeaker = dialogues[currentDialogueIndex].speaker;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꿔서 출력한다.
    std::wstring displaySpeaker = GetCurrentDisplaySpeaker();
    std::wstring displayText = GetCurrentDisplayText();

    SpeakerStyle style = GetSpeakerStyle(originalSpeaker);

    // 현재 보여줄 글자 수가 전체 길이를 넘지 않도록 보정한다.
    int safeVisibleCount = visibleTextCount;
    if (safeVisibleCount > static_cast<int>(displayText.length()))
    {
        safeVisibleCount = static_cast<int>(displayText.length());
    }

    // 현재 보여줄 글자 수만큼 잘라낸다.
    std::wstring visibleText = displayText.substr(0, safeVisibleCount);

    HFONT dialogueFont = CreateFontW(
        42, 
        0,
        0,
        0,
        600,
        FALSE,
        FALSE,
        FALSE,
        HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"맑은 고딕"
    );

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, dialogueFont));

    SetBkMode(hDC, TRANSPARENT);

    HBRUSH boxBrush = CreateSolidBrush(RGB(20, 20, 25));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, boxBrush));

    HPEN boxPen = CreatePen(PS_SOLID, 2, style.outline);
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, boxPen));

    // 대화창 출력
    RoundRect(
        hDC,
        dialogueBox.left,
        dialogueBox.top,
        dialogueBox.right,
        dialogueBox.bottom,
        20,
        20
    );

    // 포인트 라인 출력
    HPEN linePen = CreatePen(PS_SOLID, 3, style.accent);
    SelectObject(hDC, linePen);

    MoveToEx(hDC, 0, lineY, nullptr);
    LineTo(hDC, 1920, lineY);

    // 이름 출력
    SetTextColor(hDC, style.nameColor);
    DrawTextW(
        hDC,
        displaySpeaker.c_str(),
        -1,
        &nameBox,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // 대사 출력
    SetTextColor(hDC, style.textColor);
    DrawTextW(
        hDC,
        visibleText.c_str(),
        -1,
        &textRect,
        DT_LEFT | DT_TOP | DT_WORDBREAK
    );

    // GDI 객체 복구
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldFont);

    // 직접 만든 GDI 객체 삭제
    DeleteObject(linePen);
    DeleteObject(boxPen);
    DeleteObject(boxBrush);
    DeleteObject(dialogueFont);
}

void StoryScene::SetPlayerName(const std::wstring& playerName)
{
    // 빈 이름이 들어오면 기본 이름 윤서를 사용한다.
    if (playerName.empty())
    {
        m_playerName = L"윤서";
        return;
    }

    // 입력받은 이름을 저장한다.
    m_playerName = playerName;
}

std::wstring StoryScene::ReplacePlayerNameToken(const std::wstring& text) const
{
    std::wstring result = text;
    const std::wstring token = L"{PLAYER}";

    size_t position = result.find(token);
    while (position != std::wstring::npos)
    {
        result.replace(position, token.length(), m_playerName);
        position = result.find(token, position + m_playerName.length());
    }

    return result;
}

std::wstring StoryScene::GetCurrentDisplaySpeaker() const
{
    // 대사가 없으면 빈 문자열을 반환한다.
    if (dialogues.empty())
    {
        return L"";
    }

    // 현재 대사의 원래 화자를 가져온다.
    const std::wstring& originalSpeaker = dialogues[currentDialogueIndex].speaker;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꿔 반환한다.
    return ReplacePlayerNameToken(originalSpeaker);
}

std::wstring StoryScene::GetCurrentDisplayText() const
{
    // 대사가 없으면 빈 문자열을 반환한다.
    if (dialogues.empty())
    {
        return L"";
    }

    // 현재 대사의 원래 본문을 가져온다.
    const std::wstring& originalText = dialogues[currentDialogueIndex].text;

    // {PLAYER} 토큰을 실제 플레이어 이름으로 바꿔 반환한다.
    return ReplacePlayerNameToken(originalText);
}

void StoryScene::UpdateTyping() {
    if (dialogues.empty() || finished) {
        return;
    }

    std::wstring displayText = GetCurrentDisplayText();

    if (visibleTextCount < static_cast<int>(displayText.length()))
    {
        visibleTextCount++;
        isTypingFinished = false;
    }
    else
    {
        isTypingFinished = true;
    }
}

void StoryScene::ApplyCurrentLineVisualInfo()
{
    // 대사가 없으면 아무것도 하지 않는다.
    if (dialogues.empty())
    {
        return;
    }

    const DialogueLineInfo& currentLine = dialogues[currentDialogueIndex];

    // backgroundKey가 비어 있지 않으면 현재 배경을 변경한다.
    // 비어 있으면 이전 배경을 유지한다.
    if (!currentLine.backgroundKey.empty())
    {
        currentBackgroundKey = currentLine.backgroundKey;
    }

    // characterKey가 비어 있지 않으면 현재 캐릭터를 변경한다.
    // 비어 있으면 이전 캐릭터를 유지한다.
    if (!currentLine.characterKey.empty())
    {
        currentCharacterKey = currentLine.characterKey;
    }
}

CImage* StoryScene::GetBackgroundImage(const std::wstring& backgroundKey)
{
    if (backgroundKey == L"hansea_room")
    {
        return &story_background_image[0];
    }
    else if (backgroundKey == L"yuharin_classroom")
    {
        return &story_background_image[1];
    }
    else if (backgroundKey == L"seoirin_library")
    {
        return &story_background_image[2];
    }

    return nullptr;
}

Gdiplus::Image* StoryScene::GetCharacterImage(const std::wstring& characterKey)
{
    if (characterKey == L"hansea_normal")
    {
        return hansea.normal;
    }
    else if (characterKey == L"yuharin_normal")
    {
        return yuharin.normal;
    }
    else if (characterKey == L"seoirin_normal")
    {
        return seoirin.normal;
    }

    return nullptr;
}

void StoryScene::DrawCharacterImage(HDC hDC, Gdiplus::Image* image, int x, int y)
{
    // 이미지가 없으면 출력하지 않는다.
    if (image == nullptr)
    {
        return;
    }

    // 이미지 로드에 실패했으면 출력하지 않는다.
    if (image->GetLastStatus() != Gdiplus::Ok)
    {
        return;
    }

    // HDC를 기반으로 GDI+ Graphics 객체를 만든다.
    Gdiplus::Graphics graphics(hDC);

    // 픽셀 단위로 출력하도록 설정한다.
    graphics.SetPageUnit(Gdiplus::UnitPixel);

    // PNG 알파 합성 품질을 높인다.
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);

    // 확대/축소가 있을 때 품질을 높인다.
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    // 외곽선과 반투명 픽셀을 조금 더 부드럽게 처리한다.
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    // 지정한 위치와 크기로 캐릭터를 출력한다.
    graphics.DrawImage(image, x, y);
}