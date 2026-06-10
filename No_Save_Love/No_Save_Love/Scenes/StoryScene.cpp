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
    story_background_image[3].Load(L"resource\\background\\school_day.png");
    story_background_image[4].Load(L"resource\\background\\library_day.png");
    story_background_image[5].Load(L"resource\\background\\room.png");
    story_background_image[6].Load(L"resource\\background\\choice.png");
    story_background_image[7].Load(L"resource\\endingscene\\hansea_happyend.png");
    story_background_image[8].Load(L"resource\\endingscene\\harin_happyend.png");
    story_background_image[9].Load(L"resource\\endingscene\\seoirin_happyend.png");
    story_background_image[10].Load(L"resource\\endingscene\\hansea_badend.png");
    story_background_image[11].Load(L"resource\\endingscene\\harin_badend.png");
    story_background_image[12].Load(L"resource\\endingscene\\seoirin_badend.png");
    story_background_image[13].Load(L"resource\\endingscene\\hidden_end.png");

    normal_back_ground.Load(L"resource\\background\\choicescene_background.png");
    book_image.Load(L"resource\\background\\open_book.png");

    if (bookGdiImage != nullptr)
    {
        delete bookGdiImage;
        bookGdiImage = nullptr;
    }

    bookGdiImage = new Gdiplus::Image(L"resource\\background\\open_book.png");

    // =========================
    // 캐릭터 이미지는 GDI+ Image로 로드한다.
    // PNG 투명 알파 처리를 더 안정적으로 하기 위해 CImage 대신 사용한다.
    // =========================
    hansea.normal = new Gdiplus::Image(L"resource\\heroine\\hansea\\normal.png");
    hansea.smile = new Gdiplus::Image(L"resource\\heroine\\hansea\\smile.png");
    seoirin.normal = new Gdiplus::Image(L"resource\\heroine\\seoirin\\normal.png");
    seoirin.smile = new Gdiplus::Image(L"resource\\heroine\\seoirin\\smile.png");
    yuharin.normal = new Gdiplus::Image(L"resource\\heroine\\yuharin\\normal.png");
    yuharin.smile = new Gdiplus::Image(L"resource\\heroine\\yuharin\\smile.png");



    // 처음에는 비워둔다.
    dialogues.clear();
    currentDialogueIndex = 0;
    visibleTextCount = 0;
    isTypingFinished = false;
    finished = false;
    pendingBgmKey = L"";
    hasPendingBgmKey = false;
}

void StoryScene::Shutdown()
{
    dialogues.clear();

    // 배경 이미지는 기존 CImage 방식으로 해제한다.
    for (int i = 0; i < BACKGROUND_IMAGE_COUNT; i++)
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

    if (hansea.smile != nullptr)
    {
        delete hansea.smile;
        hansea.smile = nullptr;
    }

    if (yuharin.normal != nullptr)
    {
        delete yuharin.normal;
        yuharin.normal = nullptr;
    }

    if (yuharin.smile != nullptr)
    {
        delete yuharin.smile;
        yuharin.smile = nullptr;
    }

    if (seoirin.normal != nullptr)
    {
        delete seoirin.normal;
        seoirin.normal = nullptr;
    }

    if (seoirin.smile != nullptr)
    {
        delete seoirin.smile;
        seoirin.smile = nullptr;
    }

    if (!book_image.IsNull())
    {
        book_image.Destroy();
    }

    if (bookGdiImage != nullptr)
    {
        delete bookGdiImage;
        bookGdiImage = nullptr;
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
    pendingBgmKey = L"";
    hasPendingBgmKey = false;
}

void StoryScene::SetDialogues(const std::vector<DialogueLineInfo>& newDialogues)
{
    // 새로운 대사 목록을 저장한다.
    dialogues = newDialogues;

    // 첫 번째 대사부터 시작한다.
    currentDialogueIndex = 0;

    // 스토리 종료 상태를 초기화한다.
    finished = false;

    // 타이핑 상태를 초기화한다.
    visibleTextCount = 0;
    isTypingFinished = false;

    // 페이드 상태를 초기화한다.
    fadeState = FadeState::None;
    fadeAlpha = 0;

    // 이전 스토리에서 쓰던 배경/캐릭터 key를 초기화한다.
    currentBackgroundKey = L"";
    currentCharacterKey = L"";
    nextBackgroundKey = L"";
    nextCharacterKey = L"";
    characterAlpha = 255;
    pendingCharacterFade = false;
    bookAlpha = 0;
    isBookShowing = false;
    pendingBgmKey = L"";
    hasPendingBgmKey = false;

    // 대사가 없으면 바로 종료 상태로 만든다.
    if (dialogues.empty())
    {
        finished = true;
        return;
    }

    // 첫 번째 줄의 배경/캐릭터는 페이드 없이 바로 적용한다.
    ApplyCurrentLineVisualInfo(false);
    QueueCurrentLineBgmChange();
}

void StoryScene::OnMouseClick(int x, int y)
{
    POINT mousePoint = { x, y };

      //스킵 버튼
    if (PtInRect(&skipButtonRect, mousePoint))
    {
        finished = true;
        fadeState = FadeState::None;
        fadeAlpha = 0;

        if (!dialogues.empty())
        {
            std::wstring displayText = GetCurrentDisplayText();
            visibleTextCount = static_cast<int>(displayText.length());
        }

        isTypingFinished = true;
        return;
    }

    if (fadeState != FadeState::None)
    {
        return;
    }

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
        ApplyCurrentLineVisualInfo(false);
        QueueCurrentLineBgmChange();
    }
    else
    {
        // 마지막 대사에서 클릭하면 끝난 상태로 만든다.
        finished = true;
    }
}

bool StoryScene::IsClickSkipButton(int x, int y) const
{
    POINT mousePoint = { x, y };
    return PtInRect(&skipButtonRect, mousePoint);
}

bool StoryScene::IsFinished() const
{
    return finished;
}

std::wstring StoryScene::ConsumePendingBgmKey()
{
    if (!hasPendingBgmKey)
    {
        return L"";
    }

    hasPendingBgmKey = false;
    return pendingBgmKey;
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

void StoryScene::DrawSkipButton(HDC hDC)
{
    HBRUSH buttonBrush = CreateSolidBrush(RGB(35, 35, 45));

    HPEN buttonPen = CreatePen(PS_SOLID, 2, RGB(230, 210, 255));

    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, buttonBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, buttonPen));

    RoundRect(
        hDC,
        skipButtonRect.left,
        skipButtonRect.top,
        skipButtonRect.right,
        skipButtonRect.bottom,
        18,
        18
    );


    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);

    DeleteObject(buttonBrush);
    DeleteObject(buttonPen);

    HFONT skipFont = CreateFontW(
        28,
        0,
        0,
        0,
        FW_BOLD,
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

    HFONT oldFont = static_cast<HFONT>(SelectObject(hDC, skipFont));

    // 배경 투명
    SetBkMode(hDC, TRANSPARENT);

    // 글자 색상을 지정한다.
    SetTextColor(hDC, RGB(245, 245, 250));

    // SKIP 글자를 출력한다.
    DrawTextW(
        hDC,
        L"SKIP",
        -1,
        &skipButtonRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );
    SelectObject(hDC, oldFont);
    DeleteObject(skipFont);
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
        normal_back_ground.Draw(hDC, 0, 0, 1920, 1080);
    }

    // =========================
    // 캐릭터 이미지 출력
    // =========================
    Gdiplus::Image* characterImage = GetCharacterImage(currentCharacterKey);

    if (characterImage != nullptr && characterImage->GetLastStatus() == Gdiplus::Ok)
    {
        int characterX = 725;
        int characterY = 25;

        // GDI+로 캐릭터 이미지를 출력한다.
        DrawCharacterImage(hDC, characterImage, characterX, characterY);
    }

    if (!dialogues.empty() && dialogues[currentDialogueIndex].speaker == L"노트")
    {
        DrawBlurOverlay(hDC);
        DrawBookImage(hDC);
    }

    DrawFadeOverlay(hDC);

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

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldFont);

    DeleteObject(linePen);
    DeleteObject(boxPen);
    DeleteObject(boxBrush);
    DeleteObject(dialogueFont);

    //스킵 생성
    DrawSkipButton(hDC);

}

void StoryScene::DrawBookImage(HDC hDC)
{
    if (bookGdiImage == nullptr)
    {
        return;
    }

    if (bookGdiImage->GetLastStatus() != Gdiplus::Ok)
    {
        return;
    }

    // 대사창과 겹치지 않게 화면 중앙 위쪽에 배치한다.
    int drawWidth = 760;
    int drawHeight = 520;
    int drawX = (1920 - drawWidth) / 2;
    int drawY = 140;

    // PNG 알파 채널에 등장 투명도를 더해서 서서히 출력한다.
    DrawImageWithAlpha(hDC, bookGdiImage, drawX, drawY, drawWidth, drawHeight, bookAlpha);
}

void StoryScene::DrawBlurOverlay(HDC hDC)
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    const int smallWidth = 160;
    const int smallHeight = 90;

    HDC screenCopyDC = CreateCompatibleDC(hDC);
    HDC smallDC = CreateCompatibleDC(hDC);

    HBITMAP screenBitmap = CreateCompatibleBitmap(hDC, screenWidth, screenHeight);
    HBITMAP smallBitmap = CreateCompatibleBitmap(hDC, smallWidth, smallHeight);

    HBITMAP oldScreenBitmap = static_cast<HBITMAP>(SelectObject(screenCopyDC, screenBitmap));
    HBITMAP oldSmallBitmap = static_cast<HBITMAP>(SelectObject(smallDC, smallBitmap));

    BitBlt(screenCopyDC, 0, 0, screenWidth, screenHeight, hDC, 0, 0, SRCCOPY);

    int oldSmallMode = SetStretchBltMode(smallDC, HALFTONE);
    SetBrushOrgEx(smallDC, 0, 0, nullptr);
    StretchBlt(
        smallDC,
        0,
        0,
        smallWidth,
        smallHeight,
        screenCopyDC,
        0,
        0,
        screenWidth,
        screenHeight,
        SRCCOPY
    );

    int oldScreenMode = SetStretchBltMode(hDC, HALFTONE);
    SetBrushOrgEx(hDC, 0, 0, nullptr);
    StretchBlt(
        hDC,
        0,
        0,
        screenWidth,
        screenHeight,
        smallDC,
        0,
        0,
        smallWidth,
        smallHeight,
        SRCCOPY
    );

    RECT overlayRect = { 0, 0, screenWidth, screenHeight };
    HDC overlayDC = CreateCompatibleDC(hDC);
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, screenWidth, screenHeight);
    HBITMAP oldOverlayBitmap = static_cast<HBITMAP>(SelectObject(overlayDC, overlayBitmap));
    HBRUSH overlayBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(overlayDC, &overlayRect, overlayBrush);

    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 95;

    AlphaBlend(hDC, 0, 0, screenWidth, screenHeight, overlayDC, 0, 0, screenWidth, screenHeight, blend);

    SetStretchBltMode(smallDC, oldSmallMode);
    SetStretchBltMode(hDC, oldScreenMode);

    SelectObject(overlayDC, oldOverlayBitmap);
    DeleteObject(overlayBrush);
    DeleteObject(overlayBitmap);
    DeleteDC(overlayDC);

    SelectObject(smallDC, oldSmallBitmap);
    SelectObject(screenCopyDC, oldScreenBitmap);
    DeleteObject(smallBitmap);
    DeleteObject(screenBitmap);
    DeleteDC(smallDC);
    DeleteDC(screenCopyDC);
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
    bool hasFinalConsonant = HasFinalConsonant(m_playerName);

    auto replaceAll = [&result](const std::wstring& from, const std::wstring& to)
    {
        size_t position = result.find(from);
        while (position != std::wstring::npos)
        {
            result.replace(position, from.length(), to);
            position = result.find(from, position + to.length());
        }
    };

    const std::wstring topicJosa = hasFinalConsonant ? L"은" : L"는";
    const std::wstring subjectJosa = hasFinalConsonant ? L"이" : L"가";

    replaceAll(L"{PLAYER}은(는)", m_playerName + topicJosa);
    replaceAll(L"{PLAYER}는(은)", m_playerName + topicJosa);
    replaceAll(L"{PLAYER}이(가)", m_playerName + subjectJosa);
    replaceAll(L"{PLAYER}가(이)", m_playerName + subjectJosa);

    const std::wstring token = L"{PLAYER}";
    size_t position = result.find(token);
    while (position != std::wstring::npos)
    {
        result.replace(position, token.length(), m_playerName);
        position = result.find(token, position + m_playerName.length());
    }

    return result;
}

bool StoryScene::HasFinalConsonant(const std::wstring& text) const
{
    for (int i = static_cast<int>(text.length()) - 1; i >= 0; i--)
    {
        wchar_t ch = text[i];
        if (ch == L' ' || ch == L'\t')
        {
            continue;
        }

        if (ch >= L'\xAC00' && ch <= L'\xD7A3')
        {
            return ((ch - L'\xAC00') % 28) != 0;
        }

        return false;
    }

    return false;
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

bool StoryScene::ShouldFadeCharacterChange(const std::wstring& newCharacterKey) const
{
    if (newCharacterKey.empty())
    {
        return false;
    }

    if (currentCharacterKey.empty())
    {
        return true;
    }

    size_t oldDivider = currentCharacterKey.find(L"_");
    size_t newDivider = newCharacterKey.find(L"_");

    std::wstring oldCharacter =
        oldDivider == std::wstring::npos ? currentCharacterKey : currentCharacterKey.substr(0, oldDivider);

    std::wstring newCharacter =
        newDivider == std::wstring::npos ? newCharacterKey : newCharacterKey.substr(0, newDivider);

    return oldCharacter != newCharacter;
}

void StoryScene::Update()
{
    // 페이드 효과를 먼저 갱신한다.
    UpdateFade();

    // 책과 캐릭터의 서서히 등장 효과를 갱신한다.
    UpdateObjectFades();

    // 타이핑 효과를 갱신한다.
    UpdateTyping();
}

void StoryScene::UpdateTyping()
{
    // 대사가 없거나 스토리가 끝났으면 타이핑을 진행하지 않는다.
    if (dialogues.empty() || finished)
    {
        return;
    }

    // 페이드 중에는 타이핑을 잠시 멈춘다.
    if (fadeState != FadeState::None)
    {
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

void StoryScene::UpdateObjectFades()
{
    if (fadeState != FadeState::None)
    {
        return;
    }

    if (characterAlpha < 255)
    {
        characterAlpha += characterFadeSpeed;

        if (characterAlpha > 255)
        {
            characterAlpha = 255;
        }
    }

    if (isBookShowing && bookAlpha < 255)
    {
        bookAlpha += bookFadeSpeed;

        if (bookAlpha > 255)
        {
            bookAlpha = 255;
        }
    }
}

void StoryScene::UpdateBookFadeState()
{
    bool shouldShowBook = false;

    if (!dialogues.empty() &&
        currentDialogueIndex >= 0 &&
        currentDialogueIndex < static_cast<int>(dialogues.size()))
    {
        shouldShowBook = (dialogues[currentDialogueIndex].speaker == L"노트");
    }

    if (shouldShowBook)
    {
        if (!isBookShowing)
        {
            bookAlpha = 0;
        }

        isBookShowing = true;
        return;
    }

    isBookShowing = false;
    bookAlpha = 0;
}

void StoryScene::QueueCurrentLineBgmChange()
{
    if (dialogues.empty())
    {
        return;
    }

    if (currentDialogueIndex < 0 || currentDialogueIndex >= static_cast<int>(dialogues.size()))
    {
        return;
    }

    const std::wstring& bgmKey = dialogues[currentDialogueIndex].bgmKey;
    if (bgmKey.empty())
    {
        return;
    }

    pendingBgmKey = bgmKey;
    hasPendingBgmKey = true;
}

void StoryScene::StartFadeTransition(
    const std::wstring& newBackgroundKey,
    const std::wstring& newCharacterKey)
{
    // 다음에 적용할 배경 key를 저장한다.
    nextBackgroundKey = newBackgroundKey;

    // 다음에 적용할 캐릭터 key를 저장한다.
    nextCharacterKey = newCharacterKey;
    pendingCharacterFade = ShouldFadeCharacterChange(newCharacterKey);

    // 현재 배경과 캐릭터가 모두 비어 있다면
    // 이것은 스토리의 첫 이미지 등장으로 본다. // 시작은 false
    bool isFirstVisual =
        currentBackgroundKey.empty() &&
        currentCharacterKey.empty();

    // 첫 등장일 때는 FadeOut 없이 바로 FadeIn만 한다.
    if (isFirstVisual)
    {
        // 먼저 실제 출력할 이미지를 적용한다.
        currentBackgroundKey = newBackgroundKey;
        currentCharacterKey = newCharacterKey;
        characterAlpha = pendingCharacterFade ? 0 : 255;
        pendingCharacterFade = false;

        // 완전 검은 상태에서 시작한다.
        fadeAlpha = 255;

        // 검은 화면이 점점 투명해지는 FadeIn으로 시작한다.
        fadeState = FadeState::FadeIn;

        return;
    }

    // 기존 이미지가 이미 있는 상태에서 바뀌는 경우에는
    // FadeOut → 이미지 교체 → FadeIn 방식으로 전환한다.
    fadeState = FadeState::FadeOut;
    fadeAlpha = 0;
}

void StoryScene::ApplyCurrentLineVisualInfo(bool immediateApply)
{
    // 대사가 없으면 처리하지 않는다.
    if (dialogues.empty())
    {
        return;
    }

    // 현재 대사 번호가 범위를 벗어나면 처리하지 않는다.
    if (currentDialogueIndex < 0 || currentDialogueIndex >= static_cast<int>(dialogues.size()))
    {
        return;
    }

    // 현재 대사 정보를 가져온다.
    const DialogueLineInfo& line = dialogues[currentDialogueIndex];

    // 기본값은 현재 출력 중인 배경/캐릭터 key를 유지한다.
    std::wstring newBackgroundKey = currentBackgroundKey;
    std::wstring newCharacterKey = currentCharacterKey;

    // 현재 대사에 배경 key가 있으면 새 배경 후보로 설정한다.
    if (!line.backgroundKey.empty())
    {
        newBackgroundKey = line.backgroundKey;
    }

    // 현재 대사에 캐릭터 key가 있으면 새 캐릭터 후보로 설정한다.
    if (!line.characterKey.empty())
    {
        newCharacterKey = line.characterKey;
    }

    // 첫 줄이거나 강제 적용이면 바로 적용한다.
    if (immediateApply)
    {
        currentBackgroundKey = newBackgroundKey;
        currentCharacterKey = newCharacterKey;
        characterAlpha = 255;
        pendingCharacterFade = false;
        UpdateBookFadeState();
        return;
    }

    // 배경이 실제로 바뀌었는지 확인한다.
    bool isBackgroundChanged = (newBackgroundKey != currentBackgroundKey);

    // 캐릭터가 실제로 바뀌었는지 확인한다.
    bool isCharacterChanged = (newCharacterKey != currentCharacterKey);

    // 배경이 바뀌는 경우에만 페이드 전환을 실행한다.
    if (isBackgroundChanged)
    {
        StartFadeTransition(newBackgroundKey, newCharacterKey);
        UpdateBookFadeState();
        return;
    }

    // 배경은 그대로이고 캐릭터만 바뀌는 경우에는 즉시 변경한다.
    if (isCharacterChanged)
    {
        bool shouldFadeCharacter = ShouldFadeCharacterChange(newCharacterKey);
        currentCharacterKey = newCharacterKey;
        characterAlpha = shouldFadeCharacter ? 0 : 255;
        UpdateBookFadeState();
        return;
    }

    UpdateBookFadeState();
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
    else if (backgroundKey == L"school_day")
    {
        return &story_background_image[3];
    }
    else if (backgroundKey == L"library_day")
    {
        return &story_background_image[4];
    }
    else if (backgroundKey == L"room")
    {
        return &story_background_image[5];
    }
    else if (backgroundKey == L"school_hallway")
    {
        return &story_background_image[6];
    }
    else if (backgroundKey == L"hansea_happyend")
    {
        return &story_background_image[7];
    }
    else if (backgroundKey == L"harin_happyend")
    {
        return &story_background_image[8];
    }
    else if (backgroundKey == L"seoirin_happyend")
    {
        return &story_background_image[9];
    }
    else if (backgroundKey == L"hansea_badend")
    {
        return &story_background_image[10];
    }
    else if (backgroundKey == L"harin_badend")
    {
        return &story_background_image[11];
    }
    else if (backgroundKey == L"seoirin_badend")
    {
        return &story_background_image[12];
    }
    else if (backgroundKey == L"hidden_end")
    {
        return &story_background_image[13];
    }

    return nullptr;
}

Gdiplus::Image* StoryScene::GetCharacterImage(const std::wstring& characterKey)
{
    if (characterKey == L"hansea_normal")
    {
        return hansea.normal;
    }
    else if (characterKey == L"hansea_smile")
    {
        return hansea.smile;
    }
    else if (characterKey == L"yuharin_normal")
    {
        return yuharin.normal;
    }
    else if (characterKey == L"yuharin_smile")
    {
        return yuharin.smile;
    }
    else if (characterKey == L"seoirin_normal")
    {
        return seoirin.normal;
    }
    else if (characterKey == L"seoirin_smile")
    {
        return seoirin.smile;
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

    int drawWidth = static_cast<int>(image->GetWidth());
    int drawHeight = static_cast<int>(image->GetHeight());
    DrawImageWithAlpha(hDC, image, x, y, drawWidth, drawHeight, characterAlpha);
}

void StoryScene::DrawImageWithAlpha(HDC hDC, Gdiplus::Image* image, int x, int y, int width, int height, int alpha)
{
    if (image == nullptr)
    {
        return;
    }

    if (image->GetLastStatus() != Gdiplus::Ok)
    {
        return;
    }

    if (alpha <= 0)
    {
        return;
    }

    if (alpha > 255)
    {
        alpha = 255;
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

    if (alpha >= 255)
    {
        graphics.DrawImage(image, x, y, width, height);
        return;
    }

    float alphaRatio = static_cast<float>(alpha) / 255.0f;
    Gdiplus::ColorMatrix colorMatrix =
    {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, alphaRatio, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    Gdiplus::ImageAttributes imageAttributes;
    imageAttributes.SetColorMatrix(
        &colorMatrix,
        Gdiplus::ColorMatrixFlagsDefault,
        Gdiplus::ColorAdjustTypeBitmap
    );

    Gdiplus::Rect drawRect(x, y, width, height);
    graphics.DrawImage(
        image,
        drawRect,
        0,
        0,
        image->GetWidth(),
        image->GetHeight(),
        Gdiplus::UnitPixel,
        &imageAttributes
    );
}

void StoryScene::UpdateFade()
{
    // 페이드 효과가 없으면 아무것도 하지 않는다.
    if (fadeState == FadeState::None)
    {
        return;
    }

    // FadeOut 단계: 화면을 점점 검게 만든다.
    if (fadeState == FadeState::FadeOut)
    {
        fadeAlpha += fadeSpeed;

        // 완전히 검게 되었으면 이미지를 교체한다.
        if (fadeAlpha >= 255)
        {
            fadeAlpha = 255;

            // 검은 화면으로 덮인 순간에 실제 이미지 key를 교체한다.
            currentBackgroundKey = nextBackgroundKey;
            currentCharacterKey = nextCharacterKey;
            characterAlpha = pendingCharacterFade ? 0 : 255;
            pendingCharacterFade = false;

            // 이제 다시 밝아지는 단계로 넘어간다.
            fadeState = FadeState::FadeIn;
        }
    }
    // FadeIn 단계: 검은 화면을 점점 투명하게 만든다.
    else if (fadeState == FadeState::FadeIn)
    {
        fadeAlpha -= fadeSpeed;

        // 완전히 밝아지면 페이드 종료
        if (fadeAlpha <= 0)
        {
            fadeAlpha = 0;
            fadeState = FadeState::None;
        }
    }
}
void StoryScene::DrawFadeOverlay(HDC hDC)
{
    // alpha가 0이면 검은 레이어를 그릴 필요가 없다.
    if (fadeAlpha <= 0)
    {
        return;
    }

    // HDC를 기반으로 GDI+ Graphics 객체를 만든다.
    Gdiplus::Graphics graphics(hDC);

    // 픽셀 단위로 출력하도록 설정한다.
    graphics.SetPageUnit(Gdiplus::UnitPixel);

    // 기존 화면 위에 반투명 색을 자연스럽게 합성한다.
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);

    // fadeAlpha 값을 가진 검은색 브러시를 만든다.
    Gdiplus::SolidBrush blackBrush(
        Gdiplus::Color(fadeAlpha, 0, 0, 0)
    );

    // 현재 프로젝트 해상도 기준으로 전체 화면에 검은 레이어를 덮는다.
    graphics.FillRectangle(
        &blackBrush,
        0,
        0,
        1920,
        1080
    );
}
