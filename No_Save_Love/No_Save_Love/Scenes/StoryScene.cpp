#include "StoryScene.h"

#include <algorithm>
#include <cmath>

namespace
{
    // 한 번 진행한 동작은 끝 자세에 머문다. 다음 대사에서도 같은 효과 키면 이어진다.
    float StoryMotion(double seconds, double start, double duration)
    {
        const double p = (std::clamp)((seconds - start) / duration, 0.0, 1.0);
        return static_cast<float>(p * p * (3.0 - 2.0 * p));
    }
}

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
    story_background_image[14].Load(L"resource\\cutscene\\spring_rewind.png");
    story_background_image[15].Load(L"resource\\cutscene\\rain_promise.png");
    story_background_image[16].Load(L"resource\\cutscene\\memory_bookmark.png");
    story_background_image[17].Load(L"resource\\cutscene\\hansea_hand.png");
    story_background_image[18].Load(L"resource\\cutscene\\harin_umbrella.png");
    story_background_image[19].Load(L"resource\\cutscene\\irin_reading.png");

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
    SetCutsceneEffect(L"");
    nextEffectKey.clear();
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
        SetCutsceneEffect(L"");
        nextEffectKey.clear();

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
        DrawCutsceneBackground(hDC, *backgroundImage);
    }
    else
    {
        normal_back_ground.Draw(hDC, 0, 0, 1920, 1080);
    }

    // =========================
    // 캐릭터 이미지 출력
    // =========================
    Gdiplus::Image* characterImage = GetCharacterImage(currentCharacterKey);

    if ((!IsCutscene() || IsStoryAnimation()) && characterImage != nullptr && characterImage->GetLastStatus() == Gdiplus::Ok)
    {
        if (IsStoryAnimation())
        {
            // 소품은 왼쪽, 히로인은 오른쪽. 작은 호흡 움직임과 입장 동작을 더한다.
            const int height = 700;
            const int width = static_cast<int>(characterImage->GetWidth() * 700.0 / characterImage->GetHeight());
            const int x = 1330 + static_cast<int>(100.0f * (1.0f - StoryMotion(cutsceneSeconds, 0.0, 0.8)));
            const int y = 114 + static_cast<int>(std::sin(cutsceneSeconds * 1.8) * 3.0);
            DrawImageWithAlpha(hDC, characterImage, x, y, width, height, characterAlpha);
        }
        else
        {
            DrawCharacterImage(hDC, characterImage, 725, 25);
        }
    }

    if (!IsCutscene() && !dialogues.empty() && dialogues[currentDialogueIndex].speaker == L"노트")
    {
        DrawBlurOverlay(hDC);
        DrawBookImage(hDC);
    }

    // 대사와 버튼은 효과 이후에 그려 흔들림/섬광 중에도 읽을 수 있게 한다.
    DrawCutsceneEffects(hDC);
    DrawStoryAnimation(hDC);
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
        36,
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

    HFONT nameFont = CreateFontW(
        50,
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
    SelectObject(hDC, nameFont);
    SetTextColor(hDC, style.nameColor);
    DrawTextW(
        hDC,
        displaySpeaker.c_str(),
        -1,
        &nameBox,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    // 대사 출력
    SelectObject(hDC, dialogueFont);
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
    DeleteObject(nameFont);
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
    const BYTE darkenAlpha = 90;

    RECT overlayRect = { 0, 0, screenWidth, screenHeight };
    HDC overlayDC = CreateCompatibleDC(hDC);
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, screenWidth, screenHeight);
    HBITMAP oldOverlayBitmap = static_cast<HBITMAP>(SelectObject(overlayDC, overlayBitmap));
    HBRUSH overlayBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(overlayDC, &overlayRect, overlayBrush);

    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = darkenAlpha;

    AlphaBlend(hDC, 0, 0, screenWidth, screenHeight, overlayDC, 0, 0, screenWidth, screenHeight, blend);

    SelectObject(overlayDC, oldOverlayBitmap);
    DeleteObject(overlayBrush);
    DeleteObject(overlayBitmap);
    DeleteDC(overlayDC);
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
    const ULONGLONG now = GetTickCount64();
    if (!finished && fadeState == FadeState::None && lastCutsceneTick != 0)
    {
        // 창을 오래 멈춰두었다 복귀해도 컷씬이 한 번에 끝나지 않는다.
        cutsceneSeconds += (std::min)(0.1, static_cast<double>(now - lastCutsceneTick) / 1000.0);
    }
    lastCutsceneTick = now;
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
        SetCutsceneEffect(nextEffectKey);
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
    nextEffectKey = line.effectKey;

    // 기본값은 현재 출력 중인 배경/캐릭터 key를 유지한다.
    std::wstring newBackgroundKey = currentBackgroundKey;
    std::wstring newCharacterKey = currentCharacterKey;

    // 현재 대사에 배경 key가 있으면 새 배경 후보로 설정한다.
    if (!line.backgroundKey.empty())
    {
        newBackgroundKey = line.backgroundKey;
    }

    // 빈 characterKey는 이전 캐릭터 유지라서, none은 캐릭터 제거를 명시한다.
    if (line.characterKey == L"none")
    {
        newCharacterKey = L"";
    }
    // 현재 대사에 캐릭터 key가 있으면 새 캐릭터 후보로 설정한다.
    else if (!line.characterKey.empty())
    {
        newCharacterKey = line.characterKey;
    }

    // 첫 줄이거나 강제 적용이면 바로 적용한다.
    if (immediateApply)
    {
        currentBackgroundKey = newBackgroundKey;
        currentCharacterKey = newCharacterKey;
        SetCutsceneEffect(line.effectKey);
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
    // 같은 컷씬의 다음 대사에서는 확대와 파티클 시간을 이어간다.
    if (currentEffectKey != line.effectKey)
    {
        SetCutsceneEffect(line.effectKey);
    }

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
    if (backgroundKey == L"cut_rewind") return &story_background_image[14];
    if (backgroundKey == L"cut_rain") return &story_background_image[15];
    if (backgroundKey == L"cut_memory") return &story_background_image[16];
    if (backgroundKey == L"cut_hansea") return &story_background_image[17];
    if (backgroundKey == L"cut_harin") return &story_background_image[18];
    if (backgroundKey == L"cut_irin") return &story_background_image[19];
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

void StoryScene::SetCutsceneEffect(const std::wstring& effectKey)
{
    currentEffectKey = effectKey;
    cutsceneSeconds = 0.0;
    lastCutsceneTick = GetTickCount64();
}

bool StoryScene::IsCutscene() const
{
    return (!currentEffectKey.empty() && !IsStoryAnimation()) || currentBackgroundKey.rfind(L"cut_", 0) == 0;
}

bool StoryScene::IsStoryAnimation() const
{
    return currentEffectKey.rfind(L"anim_", 0) == 0;
}

void StoryScene::DrawCutsceneBackground(HDC hDC, CImage& image)
{
    if (IsStoryAnimation())
    {
        image.Draw(hDC, 0, 0, 1920, 1080);
        Gdiplus::Graphics graphics(hDC);
        Gdiplus::LinearGradientBrush shade(Gdiplus::Point(0, 0), Gdiplus::Point(1920, 0),
            Gdiplus::Color(190, 20, 25, 42), Gdiplus::Color(50, 20, 25, 42));
        graphics.FillRectangle(&shade, 0, 0, 1920, 820);
        return;
    }
    if (!IsCutscene())
    {
        image.Draw(hDC, 0, 0, 1920, 1080);
        return;
    }

    // 화면 비율이 다른 CG도 늘이지 않고 중앙을 기준으로 채운다.
    const double progress = (std::min)(cutsceneSeconds / 12.0, 1.0);
    const double eased = progress * progress * (3.0 - 2.0 * progress);
    const double scale = (std::max)(1920.0 / image.GetWidth(), 1080.0 / image.GetHeight())
        * (1.025 + 0.035 * eased);
    const int width = static_cast<int>(std::ceil(image.GetWidth() * scale));
    const int height = static_cast<int>(std::ceil(image.GetHeight() * scale));
    int offsetX = 0;
    int offsetY = 0;
    if (currentEffectKey == L"rewind" && cutsceneSeconds < 1.2)
    {
        const double strength = (1.0 - cutsceneSeconds / 1.2) * 10.0;
        offsetX = static_cast<int>(std::sin(cutsceneSeconds * 47.0) * strength);
        offsetY = static_cast<int>(std::cos(cutsceneSeconds * 39.0) * strength * 0.5);
    }
    else if (currentEffectKey == L"glitch")
    {
        offsetX = static_cast<int>(std::sin(cutsceneSeconds * 11.0) * 3.0);
    }
    const int savedDC = SaveDC(hDC);
    // 컷씬은 대화창 위의 넓은 영상 영역에 구도를 맞춘다.
    // 손잡기 같은 핵심 동작이 대화창 아래에 가려지지 않게 한다.
    RECT canvas = { 0, 0, 1920, 1080 };
    HBRUSH matte = CreateSolidBrush(RGB(7, 8, 15));
    FillRect(hDC, &canvas, matte);
    DeleteObject(matte);
    IntersectClipRect(hDC, 0, 0, 1920, 820);
    SetStretchBltMode(hDC, HALFTONE);
    SetBrushOrgEx(hDC, 0, 0, nullptr);
    image.Draw(hDC, (1920 - width) / 2 + offsetX, (820 - height) / 2 + offsetY, width, height);
    RestoreDC(hDC, savedDC);
}

void StoryScene::DrawCutsceneEffects(HDC hDC)
{
    if (!IsCutscene() || IsStoryAnimation() || finished) return;

    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    const double t = cutsceneSeconds;
    if (currentEffectKey == L"petals" || currentEffectKey == L"memory")
    {
        const bool memory = currentEffectKey == L"memory";
        for (int i = 0; i < 26; ++i)
        {
            const float x = static_cast<float>(std::fmod(i * 173.0 + t * (14 + i % 5), 2000.0) - 40.0);
            const float y = static_cast<float>(std::fmod(i * 97.0 + t * (memory ? -19.0 : 27.0) + 10800.0, 1160.0) - 40.0);
            const float size = static_cast<float>(4 + i % 6);
            Gdiplus::SolidBrush particle(memory ? Gdiplus::Color(100, 239, 227, 255) : Gdiplus::Color(140, 255, 204, 221));
            graphics.FillEllipse(&particle, x, y, size * 1.8f, size);
        }
        if (memory)
        {
            const BYTE alpha = static_cast<BYTE>(18.0 + 24.0 * (0.5 + 0.5 * std::sin(t * 0.6)));
            Gdiplus::SolidBrush haze(Gdiplus::Color(alpha, 226, 223, 251));
            graphics.FillRectangle(&haze, 0, 0, 1920, 1080);
        }
    }
    else if (currentEffectKey == L"rain")
    {
        Gdiplus::SolidBrush tint(Gdiplus::Color(25, 25, 45, 80));
        graphics.FillRectangle(&tint, 0, 0, 1920, 1080);
        Gdiplus::Pen rain(Gdiplus::Color(75, 193, 213, 241), 1.4f);
        for (int i = 0; i < 62; ++i)
        {
            const float x = static_cast<float>((i * 137) % 1980 - 30);
            const float y = static_cast<float>(std::fmod(i * 89.0 + t * (290 + i % 7 * 18), 1180.0) - 60.0);
            graphics.DrawLine(&rain, x, y, x - 11.0f, y + 42.0f);
        }
    }
    else if (currentEffectKey == L"rewind")
    {
        // 입장 때 한 번만 약한 섬광. 반복 점멸하지 않는다.
        if (t < 0.9 && fadeState == FadeState::None)
        {
            Gdiplus::SolidBrush flash(Gdiplus::Color(static_cast<BYTE>(105.0 * (1.0 - t / 0.9)), 235, 241, 255));
            graphics.FillRectangle(&flash, 0, 0, 1920, 1080);
        }
        Gdiplus::Pen ring(Gdiplus::Color(static_cast<BYTE>((std::max)(0.0, 100.0 - t * 35.0)), 235, 227, 255), 2.5f);
        const float radius = static_cast<float>((std::min)(1100.0, 100.0 + t * 330.0));
        graphics.DrawEllipse(&ring, 960.0f - radius, 500.0f - radius, radius * 2.0f, radius * 2.0f);
    }
    else if (currentEffectKey == L"glitch")
    {
        Gdiplus::SolidBrush darkness(Gdiplus::Color(65, 12, 3, 25));
        graphics.FillRectangle(&darkness, 0, 0, 1920, 1080);
        Gdiplus::SolidBrush scanline(Gdiplus::Color(32, 155, 36, 86));
        for (int i = 0; i < 14; ++i)
        {
            const int y = static_cast<int>(std::fmod(i * 83.0 + t * 28.0, 1080.0));
            graphics.FillRectangle(&scanline, 0, y, 1920, 3 + i % 3);
        }
        Gdiplus::SolidBrush tear(Gdiplus::Color(44, 91, 211, 225));
        const int tearY = static_cast<int>(std::fmod(t * 95.0 + 150.0, 760.0));
        graphics.FillRectangle(&tear, 140, tearY, 1320, 7);
    }

    // 얇은 레터박스. 하단 대화창 및 우측 상단 SKIP 영역은 가리지 않는다.
    Gdiplus::SolidBrush bars(Gdiplus::Color(210, 7, 8, 15));
    graphics.FillRectangle(&bars, 0, 0, 1920, 30);
    graphics.FillRectangle(&bars, 0, 1050, 1920, 30);
}

void StoryScene::DrawStoryAnimation(HDC hDC)
{
    if (!IsStoryAnimation() || finished) return;

    using namespace Gdiplus;
    Graphics graphics(hDC);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    // 소품, 그림자, 입자는 대사창과 SKIP 영역 밖으로 나가지 않는다.
    graphics.SetClip(Rect(60, 110, 1180, 675));
    const double t = cutsceneSeconds;
    const float entrance = StoryMotion(t, 0.0, 0.7);
    graphics.TranslateTransform(660.0f, 450.0f + (1.0f - entrance) * 55.0f);

    SolidBrush paper(Color(255, 255, 247, 225));
    SolidBrush ink(Color(255, 62, 70, 88));
    SolidBrush shadow(Color(65, 8, 12, 24));
    SolidBrush rose(Color(255, 211, 124, 145));
    Pen line(Color(255, 116, 124, 138), 2.5f);
    Font captionFont(L"맑은 고딕", 22.0f, FontStyleRegular, UnitPixel);
    Font title(L"맑은 고딕", 30.0f, FontStyleBold, UnitPixel);
    const auto label = [&](const wchar_t* text, float x, float y, bool heading = false)
    {
        graphics.DrawString(text, -1, heading ? &title : &captionFont, PointF(x, y), &ink);
    };
    const auto star = [&](float x, float y, float radius)
    {
        PointF points[10];
        for (int i = 0; i < 10; ++i)
        {
            const float angle = static_cast<float>(i * 3.14159265 / 5.0 - 3.14159265 / 2.0);
            const float r = radius * (i % 2 == 0 ? 1.0f : 0.45f);
            points[i] = PointF(x + std::cos(angle) * r, y + std::sin(angle) * r);
        }
        SolidBrush gold(Color(255, 239, 180, 84));
        graphics.FillPolygon(&gold, points, 10);
    };

    if (currentEffectKey == L"anim_letter")
    {
        const float flap = StoryMotion(t, 0.2, 1.0);
        const float rise = StoryMotion(t, 1.0, 1.8);
        graphics.RotateTransform(-5.0f + 1.2f * static_cast<float>(std::sin(t * 0.7)));
        graphics.FillRectangle(&shadow, RectF(-237.0f, -58.0f, 500.0f, 310.0f));
        SolidBrush envelope(Color(255, 210, 169, 113));
        graphics.FillRectangle(&envelope, RectF(-250.0f, -80.0f, 500.0f, 310.0f));
        const PointF back[] = { {-250.0f, -80.0f}, {0.0f, -80.0f - flap * 170.0f}, {250.0f, -80.0f} };
        graphics.FillPolygon(&envelope, back, 3);
        const float paperY = 15.0f - rise * 245.0f;
        const auto sheetState = graphics.Save();
        graphics.SetClip(RectF(-220.0f, -300.0f, 440.0f, 530.0f), CombineModeIntersect);
        graphics.FillRectangle(&paper, RectF(-211.0f, paperY, 422.0f, 360.0f));
        label(currentCharacterKey.rfind(L"seoirin", 0) == 0 ? L"쉼표" : L"문예부 · 정산 기록", -177.0f, paperY + 25.0f, true);
        for (int i = 0; i < 5; ++i)
        {
            const float writing = StoryMotion(t, 2.3 + i * 0.18, 0.65);
            graphics.DrawLine(&line, -175.0f, paperY + 95.0f + i * 32.0f,
                -175.0f + (320.0f - (i % 2) * 70.0f) * writing, paperY + 95.0f + i * 32.0f);
        }
        graphics.Restore(sheetState);
        // 앞면이 올라오는 종이의 아랫부분을 가려 실제로 봉투에서 꺼내는 모양을 만든다.
        SolidBrush front(Color(255, 229, 191, 137));
        const PointF left[] = { {-250.0f, -80.0f}, {30.0f, 100.0f}, {-250.0f, 230.0f} };
        const PointF right[] = { {250.0f, -80.0f}, {-30.0f, 100.0f}, {250.0f, 230.0f} };
        const PointF bottom[] = { {-250.0f, 230.0f}, {0.0f, 55.0f}, {250.0f, 230.0f} };
        graphics.FillPolygon(&front, left, 3);
        graphics.FillPolygon(&front, right, 3);
        graphics.FillPolygon(&front, bottom, 3);
        if (flap < 1.0f)
        {
            const PointF closed[] = { {-250.0f, -80.0f}, {0.0f, -80.0f + (1.0f - flap) * 170.0f}, {250.0f, -80.0f} };
            graphics.FillPolygon(&envelope, closed, 3);
        }
    }
    else if (currentEffectKey == L"anim_photo")
    {
        const float develop = StoryMotion(t, 0.6, 2.7);
        graphics.TranslateTransform(0.0f, (1.0f - entrance) * -100.0f);
        graphics.RotateTransform(-7.0f + (1.0f - entrance) * 16.0f);
        graphics.FillRectangle(&shadow, RectF(-223.0f, -263.0f, 480.0f, 570.0f));
        graphics.FillRectangle(&paper, RectF(-240.0f, -280.0f, 480.0f, 570.0f));
        const RectF photo(-210.0f, -250.0f, 420.0f, 430.0f);
        LinearGradientBrush light(photo, Color(255, 182, 206, 216), Color(255, 245, 220, 208), LinearGradientModeVertical);
        graphics.FillRectangle(&light, photo);
        Image* heroine = GetCharacterImage(L"yuharin_normal");
        if (heroine != nullptr && heroine->GetLastStatus() == Ok)
        {
            // 기존 하린 원화를 사진 안에 넣는다. 얼굴과 상반신의 비율을 유지한다.
            const float sourceHeight = heroine->GetWidth() * photo.Height / photo.Width;
            graphics.DrawImage(heroine, photo, 0.0f, 0.0f, static_cast<float>(heroine->GetWidth()), sourceHeight, UnitPixel);
        }
        SolidBrush developing(Color(static_cast<BYTE>(255.0f * (1.0f - develop)), 240, 235, 222));
        graphics.FillRectangle(&developing, photo);
        label(L"장난치지 않은 두 번째 사진", -192.0f, 207.0f);
        star(195.0f, 238.0f, 19.0f * StoryMotion(t, 3.2, 0.5));
    }
    else if (currentEffectKey == L"anim_map")
    {
        graphics.RotateTransform(-3.0f);
        graphics.ScaleTransform(0.15f + 0.85f * entrance, 1.0f);
        graphics.FillRectangle(&shadow, RectF(-322.0f, -237.0f, 680.0f, 490.0f));
        graphics.FillRectangle(&paper, RectF(-340.0f, -255.0f, 680.0f, 490.0f));
        label(L"쓸데있는 학교 지도", -285.0f, -221.0f, true);
        Pen fold(Color(95, 170, 153, 125), 2.0f);
        graphics.DrawLine(&fold, -115.0f, -255.0f, -115.0f, 235.0f);
        graphics.DrawLine(&fold, 110.0f, -255.0f, 110.0f, 235.0f);
        SolidBrush building(Color(255, 213, 224, 215));
        graphics.FillRectangle(&building, RectF(-290.0f, -130.0f, 145.0f, 85.0f));
        graphics.FillRectangle(&building, RectF(92.0f, -112.0f, 180.0f, 100.0f));
        graphics.FillRectangle(&building, RectF(-90.0f, 66.0f, 155.0f, 95.0f));
        label(L"교실", -250.0f, -105.0f);
        label(L"도서관", 135.0f, -80.0f);
        label(L"계단", -55.0f, 96.0f);
        label(L"교문", 220.0f, 168.0f);
        const PointF route[] = { {-230.0f, -27.0f}, {-230.0f, 30.0f}, {150.0f, 30.0f},
            {150.0f, 130.0f}, {95.0f, 130.0f}, {95.0f, 195.0f}, {210.0f, 195.0f} };
        Pen path(Color(255, 205, 119, 128), 6.0f);
        path.SetStartCap(LineCapRound);
        path.SetEndCap(LineCapRound);
        const float progress = StoryMotion(t, 0.6, 3.6) * 6.0f;
        PointF tip = route[0];
        for (int i = 0; i < 6; ++i)
        {
            const float part = (std::clamp)(progress - i, 0.0f, 1.0f);
            if (part <= 0.0f) break;
            tip = PointF(route[i].X + (route[i + 1].X - route[i].X) * part,
                route[i].Y + (route[i + 1].Y - route[i].Y) * part);
            graphics.DrawLine(&path, route[i], tip);
            if (part >= 1.0f && i % 2 == 0) star(tip.X, tip.Y, 15.0f);
        }
        const float radius = 8.0f + 2.0f * static_cast<float>(std::sin(t * 3.0));
        graphics.FillEllipse(&rose, RectF(tip.X - radius, tip.Y - radius, radius * 2.0f, radius * 2.0f));
    }
    else if (currentEffectKey == L"anim_voice")
    {
        graphics.RotateTransform(3.0f + static_cast<float>(std::sin(t * 0.7)));
        graphics.FillRectangle(&shadow, RectF(-177.0f, -279.0f, 390.0f, 580.0f));
        SolidBrush phone(Color(255, 42, 49, 66));
        graphics.FillRectangle(&phone, RectF(-195.0f, -295.0f, 390.0f, 580.0f));
        SolidBrush screen(Color(255, 244, 237, 225));
        graphics.FillRectangle(&screen, RectF(-172.0f, -255.0f, 344.0f, 480.0f));
        label(L"금요일에 전할 말", -141.0f, -215.0f, true);
        label(L"목소리를 남기는 중", -114.0f, -150.0f);
        Pen wave(Color(255, 199, 106, 128), 10.0f);
        wave.SetStartCap(LineCapRound);
        wave.SetEndCap(LineCapRound);
        for (int i = 0; i < 17; ++i)
        {
            const float amplitude = 12.0f + 70.0f * static_cast<float>(std::abs(std::sin(t * 3.6 + i * 0.72)
                * std::cos(t * 1.3 - i * 0.3)));
            graphics.DrawLine(&wave, -135.0f + i * 17.0f, -amplitude, -135.0f + i * 17.0f, amplitude);
        }
        const float pulse = 18.0f + 3.0f * static_cast<float>(std::sin(t * 2.8));
        graphics.FillEllipse(&rose, RectF(-pulse, 152.0f - pulse, pulse * 2.0f, pulse * 2.0f));
        Pen speaker(Color(255, 125, 134, 150), 5.0f);
        graphics.DrawLine(&speaker, -48.0f, -275.0f, 48.0f, -275.0f);
    }
    else if (currentEffectKey == L"anim_bookmark")
    {
        const float exchange = StoryMotion(t, 3.3, 2.3);
        for (int card = 0; card < 2; ++card)
        {
            const auto saved = graphics.Save();
            const float side = card == 0 ? -1.0f : 1.0f;
            const float x = side * 170.0f * (1.0f - 2.0f * exchange);
            const float y = side * 65.0f * std::sin(exchange * 3.14159265f);
            graphics.TranslateTransform(x, y);
            graphics.RotateTransform(side * (-9.0f + 18.0f * exchange));
            graphics.FillRectangle(&shadow, RectF(-93.0f, -225.0f, 205.0f, 460.0f));
            SolidBrush cardPaper(card == 0 ? Color(255, 252, 229, 221) : Color(255, 224, 236, 237));
            graphics.FillRectangle(&cardPaper, RectF(-105.0f, -240.0f, 210.0f, 460.0f));
            Pen stem(Color(255, 96, 143, 120), 4.0f);
            graphics.DrawLine(&stem, 0.0f, 35.0f, 0.0f, 35.0f + 90.0f * StoryMotion(t, 0.6, 1.0));
            Pen petal(card == 0 ? Color(255, 199, 123, 144) : Color(255, 104, 142, 182), 3.5f);
            const int petals = card == 0 ? 5 : 6;
            for (int i = 0; i < petals; ++i)
            {
                const auto flowerState = graphics.Save();
                graphics.RotateTransform(i * 360.0f / petals);
                const float stroke = StoryMotion(t, 0.8 + i * 0.2, 0.8);
                // 두 번째 꽃은 대본의 '문어 같은 꽃'처럼 길고 납작하게 그린다.
                graphics.DrawArc(&petal, RectF(-18.0f, -75.0f, 36.0f, card == 0 ? 65.0f : 90.0f), 90.0f, 350.0f * stroke);
                graphics.Restore(flowerState);
            }
            star(0.0f, 0.0f, 13.0f * StoryMotion(t, 2.2, 0.5));
            label(card == 0 ? L"이린의 꽃" : L"너의 꽃", -65.0f, 160.0f);
            Pen ribbon(Color(255, 193, 150, 126), 5.0f);
            graphics.DrawBezier(&ribbon, -10.0f, -240.0f, -60.0f, -295.0f, 50.0f, -290.0f, 10.0f, -230.0f);
            graphics.Restore(saved);
        }
    }
    else if (currentEffectKey == L"anim_bread")
    {
        const float share = StoryMotion(t, 0.8, 2.4);
        SolidBrush plate(Color(235, 226, 232, 230));
        graphics.FillEllipse(&shadow, RectF(-350.0f, 83.0f, 700.0f, 140.0f));
        graphics.FillEllipse(&plate, RectF(-355.0f, 65.0f, 700.0f, 140.0f));
        for (int half = 0; half < 2; ++half)
        {
            const auto saved = graphics.Save();
            const float side = half == 0 ? -1.0f : 1.0f;
            graphics.TranslateTransform(side * (12.0f + share * 100.0f), -share * 22.0f);
            graphics.RotateTransform(side * share * 11.0f);
            graphics.ScaleTransform(half == 0 ? 1.1f : -0.9f, half == 0 ? 1.1f : 0.9f);
            GraphicsPath bun;
            bun.AddBezier(0.0f, -125.0f, -235.0f, -145.0f, -240.0f, 130.0f, 0.0f, 135.0f);
            bun.CloseFigure();
            LinearGradientBrush crust(RectF(-210.0f, -130.0f, 220.0f, 270.0f),
                Color(255, 249, 203, 117), Color(255, 173, 99, 54), LinearGradientModeVertical);
            graphics.FillPath(&crust, &bun);
            SolidBrush filling(Color(255, 109, 62, 61));
            graphics.FillEllipse(&paper, RectF(-24.0f, -119.0f, 48.0f, 248.0f));
            graphics.FillEllipse(&filling, RectF(-12.0f, -82.0f, 24.0f, 172.0f));
            for (int seed = 0; seed < 6; ++seed)
                graphics.FillEllipse(&paper, RectF(-72.0f - (seed % 2) * 30.0f,
                    -60.0f + (seed / 2) * 31.0f, 5.0f, 12.0f));
            graphics.Restore(saved);
        }
        SolidBrush crumbs(Color(210, 218, 173, 110));
        for (int i = 0; i < 7; ++i)
            graphics.FillEllipse(&crumbs, RectF(-45.0f + i * 16.0f, 80.0f + share * (30.0f + i % 3 * 16.0f), 7.0f, 5.0f));
        label(L"큰 쪽은 네 몫", -92.0f, 240.0f);
    }
    else if (currentEffectKey == L"anim_label")
    {
        const bool archive = currentCharacterKey.rfind(L"hansea", 0) == 0;
        const float attach = StoryMotion(t, 0.5, 2.0);
        SolidBrush surface(archive ? Color(255, 159, 122, 90) : Color(255, 104, 137, 151));
        graphics.RotateTransform(-4.0f);
        graphics.FillRectangle(&shadow, RectF(-305.0f, -200.0f, 630.0f, 450.0f));
        graphics.FillRectangle(&surface, RectF(-320.0f, -215.0f, 640.0f, 450.0f));
        if (archive)
        {
            Pen handle(Color(255, 223, 204, 174), 14.0f);
            graphics.DrawLine(&handle, -65.0f, 140.0f, 65.0f, 140.0f);
        }
        else
        {
            graphics.DrawLine(&line, -280.0f, -215.0f, -280.0f, 235.0f);
            label(L"봄이 끝나지 않는 날", -210.0f, -150.0f, true);
        }
        const float y = -15.0f - (1.0f - attach) * 170.0f;
        graphics.FillRectangle(&shadow, RectF(-193.0f, y + 13.0f, 410.0f, 112.0f));
        graphics.FillRectangle(&paper, RectF(-205.0f, y, 410.0f, 112.0f));
        const auto writing = graphics.Save();
        graphics.SetClip(RectF(-180.0f, y, 370.0f * StoryMotion(t, 1.8, 1.8), 112.0f), CombineModeIntersect);
        label(archive ? L"봉투 대피소" : L"소설 · 제자리로", -170.0f, y + 30.0f, true);
        graphics.Restore(writing);
        const float corner = 30.0f * (1.0f - StoryMotion(t, 3.0, 0.9));
        const PointF fold[] = { {205.0f - corner, y}, {205.0f, y + corner}, {205.0f - corner, y + corner} };
        SolidBrush folded(Color(255, 220, 209, 180));
        graphics.FillPolygon(&folded, fold, 3);
    }
    else if (currentEffectKey == L"anim_bandage")
    {
        // 손 전체 대신 손가락을 가까이 보여준다. 들뜬 밴드를 눌러 붙이는 동작이다.
        graphics.RotateTransform(-8.0f);
        const RectF closeup(-440.0f, -240.0f, 880.0f, 480.0f);
        LinearGradientBrush warmth(closeup, Color(255, 233, 224, 211), Color(255, 204, 190, 179), LinearGradientModeVertical);
        graphics.FillRectangle(&warmth, closeup);
        graphics.SetClip(closeup, CombineModeIntersect);
        SolidBrush nail(Color(255, 255, 225, 211));
        Pen finger(Color(255, 244, 204, 180), 116.0f);
        finger.SetStartCap(LineCapRound);
        finger.SetEndCap(LineCapRound);
        graphics.DrawLine(&finger, -510.0f, 55.0f, 170.0f, 55.0f);
        graphics.FillEllipse(&nail, RectF(105.0f, 22.0f, 90.0f, 49.0f));
        Pen crease(Color(150, 196, 150, 128), 2.0f);
        graphics.DrawArc(&crease, RectF(-166.0f, 23.0f, 13.0f, 52.0f), -70.0f, 140.0f);
        graphics.DrawArc(&crease, RectF(-177.0f, 28.0f, 13.0f, 42.0f), -70.0f, 140.0f);
        SolidBrush tape(Color(255, 226, 184, 139));
        graphics.FillRectangle(&tape, RectF(-90.0f, 0.0f, 138.0f, 110.0f));
        const float press = StoryMotion(t, 0.7, 1.5);
        const float release = StoryMotion(t, 3.0, 1.6);
        const PointF flap[] = { {-90.0f, 0.0f}, {48.0f, 0.0f},
            {48.0f, -65.0f * (1.0f - press)}, {-90.0f, -65.0f * (1.0f - press)} };
        graphics.FillPolygon(&tape, flap, 4);
        SolidBrush pad(Color(255, 244, 220, 183));
        graphics.FillRectangle(&pad, RectF(-55.0f, 16.0f, 67.0f, 77.0f));
        for (int i = 0; i < 6; ++i)
            graphics.FillEllipse(&shadow, RectF(-78.0f + (i % 2) * 109.0f, 20.0f + (i / 2) * 28.0f, 4.0f, 4.0f));
        const float touchY = -150.0f * (1.0f - press) - 170.0f * release;
        Pen touch(Color(static_cast<BYTE>(255.0f * (1.0f - release)), 255, 216, 194), 65.0f);
        touch.SetStartCap(LineCapRound);
        touch.SetEndCap(LineCapRound);
        graphics.DrawLine(&touch, 490.0f, touchY - 290.0f, -20.0f, touchY - 30.0f);
        SolidBrush touchNail(Color(static_cast<BYTE>(255.0f * (1.0f - release)), 255, 233, 218));
        graphics.FillEllipse(&touchNail, RectF(-25.0f, touchY - 53.0f, 45.0f, 25.0f));
        star(-23.0f, -75.0f, 15.0f * StoryMotion(t, 3.8, 0.7));
        label(L"우리끼리만 쓰는 도장", -139.0f, 165.0f);
    }
    else if (currentEffectKey == L"anim_milk")
    {
        const float offer = StoryMotion(t, 0.6, 2.2);
        for (int carton = 0; carton < 2; ++carton)
        {
            const auto saved = graphics.Save();
            const float x = carton == 0 ? -170.0f : 370.0f - offer * 200.0f;
            graphics.TranslateTransform(x, carton == 0 ? 0.0f : 45.0f - offer * 45.0f);
            graphics.RotateTransform(carton == 0 ? -5.0f : 7.0f - offer * 4.0f);
            graphics.FillRectangle(&shadow, RectF(-94.0f, -130.0f, 210.0f, 340.0f));
            graphics.FillRectangle(&paper, RectF(-105.0f, -145.0f, 210.0f, 340.0f));
            SolidBrush blue(Color(255, 135, 175, 199));
            graphics.FillRectangle(&blue, RectF(-105.0f, 25.0f, 210.0f, 170.0f));
            const PointF roof[] = { {-105.0f, -145.0f}, {-65.0f, -210.0f}, {65.0f, -210.0f}, {105.0f, -145.0f} };
            graphics.FillPolygon(&paper, roof, 4);
            graphics.DrawLine(&line, -65.0f, -210.0f, 65.0f, -210.0f);
            label(L"우유", -34.0f, -80.0f, true);
            if (carton == 0) // 건넨 팩은 새것으로 남겨둔다.
            {
                Pen straw(Color(255, 233, 158, 165), 11.0f);
                const float insert = StoryMotion(t, 1.0, 1.5);
                const PointF points[] = { {30.0f, -155.0f - (1.0f - insert) * 75.0f},
                    {30.0f, -270.0f - (1.0f - insert) * 75.0f}, {92.0f, -290.0f - (1.0f - insert) * 75.0f} };
                graphics.DrawLines(&straw, points, 3);
            }
            graphics.Restore(saved);
        }
    }
    else if (currentEffectKey == L"anim_projector")
    {
        SolidBrush frame(Color(255, 65, 78, 94));
        graphics.FillRectangle(&frame, RectF(-370.0f, -275.0f, 740.0f, 390.0f));
        // 밝은 섬광 대신 화면이 천천히 꺼지고 종이 안내판이 올라온다.
        SolidBrush screen(Color(255, 187, 203, 211));
        graphics.FillRectangle(&screen, RectF(-350.0f, -255.0f, 700.0f, 350.0f));
        label(L"문예부 자료 보존", -144.0f, -190.0f, true);
        for (int i = 0; i < 3; ++i)
            graphics.DrawLine(&line, -245.0f, -100.0f + i * 47.0f, 245.0f - i * 40.0f, -100.0f + i * 47.0f);
        SolidBrush disconnected(Color(static_cast<BYTE>(230.0f * StoryMotion(t, 0.6, 1.5)), 24, 33, 46));
        graphics.FillRectangle(&disconnected, RectF(-350.0f, -255.0f, 700.0f, 350.0f));
        const float backup = StoryMotion(t, 2.1, 2.3);
        const float y = 300.0f - backup * 290.0f;
        graphics.FillRectangle(&shadow, RectF(-266.0f, y + 13.0f, 560.0f, 245.0f));
        graphics.FillRectangle(&paper, RectF(-280.0f, y, 560.0f, 245.0f));
        label(L"화면 없이도 전할 이야기", -227.0f, y + 30.0f, true);
        label(L"고쳐 쓴 흔적까지 함께 남깁니다", -210.0f, y + 106.0f);
        for (int i = 0; i < 3; ++i) star(-135.0f + i * 135.0f, y + 182.0f, 14.0f);
    }
    else if (currentEffectKey == L"anim_message")
    {
        SolidBrush phone(Color(255, 45, 54, 72));
        graphics.FillRectangle(&shadow, RectF(-237.0f, -283.0f, 500.0f, 590.0f));
        graphics.FillRectangle(&phone, RectF(-250.0f, -295.0f, 500.0f, 590.0f));
        graphics.FillRectangle(&paper, RectF(-225.0f, -265.0f, 450.0f, 520.0f));
        label(L"아직 보내지 않은 말", -174.0f, -233.0f, true);
        const wchar_t* words[] = { L"엄마 일 때문에 이사해.", L"금요일까지 같이 다닐 수 있어.",
            L"남은 시간도 평소처럼", L"대해주면 좋겠어." };
        for (int i = 0; i < 4; ++i)
        {
            const auto writing = graphics.Save();
            graphics.SetClip(RectF(-190.0f, -140.0f + i * 56.0f,
                390.0f * StoryMotion(t, 0.5 + i * 0.65, 1.0), 50.0f), CombineModeIntersect);
            label(words[i], -185.0f, -140.0f + i * 56.0f);
            graphics.Restore(writing);
        }
        SolidBrush draft(Color(255, 215, 226, 219));
        graphics.FillRectangle(&draft, RectF(-190.0f, 160.0f, 380.0f, 62.0f));
        label(L"초안 · 저녁의 나에게", -137.0f, 175.0f);
        // 자동 전송이나 보내기 버튼 연출 없이 초안으로 끝난다.
    }
    else if (currentEffectKey == L"anim_window")
    {
        const RectF pane(-340.0f, -285.0f, 680.0f, 560.0f);
        LinearGradientBrush glass(pane, Color(235, 112, 160, 185), Color(245, 188, 211, 212), LinearGradientModeVertical);
        graphics.FillRectangle(&glass, pane);
        const auto paneState = graphics.Save();
        graphics.SetClip(pane, CombineModeIntersect);
        Pen rain(Color(100, 238, 250, 252), 4.0f);
        for (int i = 0; i < 22; ++i)
        {
            const float x = -320.0f + i * 30.0f;
            const float y = -290.0f + static_cast<float>(std::fmod(t * (33.0 + i % 4 * 8.0) + i * 71.0, 610.0));
            graphics.DrawLine(&rain, x, y - 25.0f, x - 3.0f, y);
        }
        Pen trace(Color(245, 242, 253, 248), 7.0f);
        trace.SetStartCap(LineCapRound);
        trace.SetEndCap(LineCapRound);
        const PointF square[] = { {-145.0f, -140.0f}, {145.0f, -140.0f}, {145.0f, 135.0f}, {-145.0f, 135.0f}, {-145.0f, -140.0f} };
        const float draw = StoryMotion(t, 0.4, 2.0) * 4.0f;
        for (int i = 0; i < 4; ++i)
        {
            const float p = (std::clamp)(draw - i, 0.0f, 1.0f);
            graphics.DrawLine(&trace, square[i], PointF(square[i].X + (square[i + 1].X - square[i].X) * p,
                square[i].Y + (square[i + 1].Y - square[i].Y) * p));
        }
        const float eyes = StoryMotion(t, 2.2, 0.6);
        graphics.DrawLine(&trace, -58.0f, -25.0f, -58.0f, -25.0f + 13.0f * eyes);
        graphics.DrawLine(&trace, 58.0f, -25.0f, 58.0f, -25.0f + 13.0f * eyes);
        graphics.DrawArc(&trace, RectF(-50.0f, 35.0f, 100.0f, 22.0f), 10.0f, 160.0f * StoryMotion(t, 3.0, 1.1));
        graphics.Restore(paneState);
        Pen border(Color(255, 212, 223, 224), 13.0f);
        graphics.DrawRectangle(&border, pane);
    }
    else if (currentEffectKey == L"anim_umbrella")
    {
        Pen rain(Color(110, 193, 226, 247), 3.0f);
        for (int i = 0; i < 32; ++i)
        {
            const float x = -510.0f + i * 33.0f;
            const float y = -320.0f + static_cast<float>(std::fmod(t * 120.0 + i * 41.0, 600.0));
            // 우산 아래에는 비가 들지 않는다.
            if (std::abs(x) > 325.0f || y < -170.0f)
                graphics.DrawLine(&rain, x, y, x - 8.0f, y + 27.0f);
        }
        const float open = StoryMotion(t, 0.4, 1.7);
        graphics.TranslateTransform(0.0f, 5.0f * static_cast<float>(std::sin(t * 1.3)));
        Pen shaft(Color(255, 211, 217, 225), 10.0f);
        graphics.DrawLine(&shaft, 0.0f, -255.0f, 0.0f, 185.0f);
        graphics.DrawArc(&shaft, RectF(-64.0f, 150.0f, 64.0f, 72.0f), 0.0f, 180.0f);
        const auto canopyState = graphics.Save();
        graphics.ScaleTransform(0.12f + open * 0.88f, 1.0f);
        SolidBrush blue(Color(255, 93, 152, 203));
        GraphicsPath canopy;
        canopy.AddBezier(-325.0f, -35.0f, -260.0f, -310.0f, 260.0f, -310.0f, 325.0f, -35.0f);
        canopy.AddBezier(325.0f, -35.0f, 225.0f, -100.0f, 125.0f, -100.0f, 0.0f, -35.0f);
        canopy.AddBezier(0.0f, -35.0f, -125.0f, -100.0f, -225.0f, -100.0f, -325.0f, -35.0f);
        graphics.FillPath(&blue, &canopy);
        Pen ribs(Color(180, 200, 225, 245), 3.0f);
        graphics.DrawBezier(&ribs, 0.0f, -242.0f, -150.0f, -220.0f, -150.0f, -130.0f, -160.0f, -82.0f);
        graphics.DrawBezier(&ribs, 0.0f, -242.0f, 150.0f, -220.0f, 150.0f, -130.0f, 160.0f, -82.0f);
        graphics.DrawLine(&ribs, 0.0f, -242.0f, 0.0f, -35.0f);
        graphics.Restore(canopyState);
        // 두 사람의 발자국이 우산 안쪽으로 가까워진다.
        const float together = StoryMotion(t, 2.0, 2.0);
        SolidBrush footprints(Color(185, 217, 225, 237));
        for (int person = 0; person < 2; ++person)
        {
            const float side = person == 0 ? -1.0f : 1.0f;
            for (int foot = 0; foot < 2; ++foot)
                graphics.FillEllipse(&footprints, RectF(side * (230.0f - together * 120.0f) + foot * 27.0f - 20.0f,
                    155.0f + foot * 35.0f, 24.0f, 51.0f));
        }
    }
    else if (currentEffectKey == L"anim_ribbon")
    {
        SolidBrush box(Color(255, 192, 153, 111));
        SolidBrush lid(Color(255, 212, 175, 131));
        graphics.FillRectangle(&shadow, RectF(-285.0f, -90.0f, 600.0f, 350.0f));
        graphics.FillRectangle(&box, RectF(-300.0f, -105.0f, 600.0f, 350.0f));
        graphics.FillRectangle(&lid, RectF(-315.0f, -145.0f, 630.0f, 65.0f));
        Pen ribbon(Color(255, 179, 91, 115), 16.0f);
        ribbon.SetStartCap(LineCapRound);
        ribbon.SetEndCap(LineCapRound);
        graphics.DrawLine(&ribbon, 0.0f, -130.0f, 0.0f, 245.0f);
        const float tie = StoryMotion(t, 0.6, 2.7);
        for (int side : {-1, 1})
        {
            const float reach = side * (230.0f - tie * 60.0f);
            graphics.DrawBezier(&ribbon, 0.0f, -130.0f, reach, -130.0f - tie * 180.0f,
                reach, -130.0f + tie * 60.0f, 0.0f, -130.0f);
            graphics.DrawBezier(&ribbon, 0.0f, -130.0f, side * 60.0f, -80.0f,
                side * 65.0f, 20.0f, side * (130.0f + (side == 1 ? 55.0f : 0.0f)),
                45.0f + static_cast<float>(std::sin(t * 1.4)) * 5.0f);
        }
        graphics.FillEllipse(&rose, RectF(-23.0f, -153.0f, 46.0f, 46.0f));
        label(L"같이 들고 갈 것들", -125.0f, 127.0f);
    }
    else if (currentEffectKey == L"anim_clock")
    {
        graphics.FillEllipse(&shadow, RectF(-235.0f, -235.0f, 500.0f, 500.0f));
        graphics.FillEllipse(&paper, RectF(-250.0f, -250.0f, 500.0f, 500.0f));
        Pen rim(Color(255, 100, 117, 143), 13.0f);
        graphics.DrawEllipse(&rim, RectF(-250.0f, -250.0f, 500.0f, 500.0f));
        for (int i = 0; i < 12; ++i)
        {
            const float angle = i * 3.14159265f / 6.0f;
            graphics.DrawLine(&line, std::sin(angle) * 200.0f, -std::cos(angle) * 200.0f,
                std::sin(angle) * 218.0f, -std::cos(angle) * 218.0f);
        }
        Pen hand(Color(255, 63, 79, 103), 10.0f);
        graphics.DrawLine(&hand, 0.0f, 0.0f, -85.0f, -97.0f);
        graphics.DrawLine(&hand, 0.0f, 0.0f, 146.0f, 86.0f);
        // 한 번 멎고 떨린 뒤 다시 간다. 계속 역행하는 시계로 세계 규칙을 바꾸지 않는다.
        double tick = t;
        if (t > 0.8 && t < 2.5) tick = 0.8 + std::sin((t - 0.8) * 16.0) * 0.035;
        else if (t >= 2.5) tick = t - 1.7;
        const float angle = static_cast<float>(tick * 0.55 - 0.8);
        Pen second(Color(255, 193, 109, 128), 4.0f);
        graphics.DrawLine(&second, -std::sin(angle) * 30.0f, std::cos(angle) * 30.0f,
            std::sin(angle) * 192.0f, -std::cos(angle) * 192.0f);
        graphics.FillEllipse(&rose, RectF(-12.0f, -12.0f, 24.0f, 24.0f));
    }
    else if (currentEffectKey == L"anim_reading")
    {
        const float settle = StoryMotion(t, 0.0, 2.8);
        graphics.RotateTransform(static_cast<float>(std::sin(t * 9.0)) * (1.0f - settle) * 1.7f);
        graphics.FillRectangle(&shadow, RectF(-320.0f, -260.0f, 660.0f, 550.0f));
        graphics.FillRectangle(&paper, RectF(-335.0f, -275.0f, 660.0f, 550.0f));
        label(L"쉼표 · 작은 낭독회", -180.0f, -239.0f, true);
        const wchar_t* sentences[] = { L"말을 고르는 동안,", L"버스가 한 대 지나갔다.",
            L"기다리는 동안,", L"옆자리 사람이 말을 걸었다." };
        for (int i = 0; i < 4; ++i)
        {
            label(sentences[i], -270.0f, -150.0f + i * 57.0f);
            Pen underline(Color(190, 169, 140, 190), 3.5f);
            graphics.DrawLine(&underline, -270.0f, -112.0f + i * 57.0f,
                -270.0f + (i % 2 ? 350.0f : 230.0f) * StoryMotion(t, 0.5 + i * 0.9, 0.85), -112.0f + i * 57.0f);
        }
        // 원고 속 버스는 한 번만 지나간다. 낭독이 끝나도 다음 대사를 기다린다.
        const float busX = -440.0f + StoryMotion(t, 0.4, 4.6) * 880.0f;
        const auto drawing = graphics.Save();
        graphics.SetClip(RectF(-300.0f, 140.0f, 600.0f, 110.0f), CombineModeIntersect);
        SolidBrush bus(Color(255, 136, 173, 168));
        graphics.FillRectangle(&bus, RectF(busX, 158.0f, 145.0f, 61.0f));
        for (int i = 0; i < 3; ++i)
            graphics.FillRectangle(&paper, RectF(busX + 12.0f + i * 43.0f, 167.0f, 32.0f, 26.0f));
        graphics.FillEllipse(&ink, RectF(busX + 18.0f, 206.0f, 24.0f, 24.0f));
        graphics.FillEllipse(&ink, RectF(busX + 108.0f, 206.0f, 24.0f, 24.0f));
        graphics.Restore(drawing);
    }
    else if (currentEffectKey == L"anim_promise")
    {
        const float share = StoryMotion(t, 2.6, 2.5);
        for (int copy = 0; copy < 2; ++copy)
        {
            const auto saved = graphics.Save();
            const float side = copy == 0 ? -1.0f : 1.0f;
            graphics.TranslateTransform(side * (12.0f + share * 235.0f), side * share * 22.0f);
            graphics.RotateTransform(side * share * 6.0f);
            graphics.FillRectangle(&shadow, RectF(-207.0f, -193.0f, 440.0f, 400.0f));
            graphics.FillRectangle(&paper, RectF(-220.0f, -210.0f, 440.0f, 400.0f));
            label(L"토요일 · 열한 시", -170.0f, -170.0f, true);
            label(L"도서관에서", -160.0f, -105.0f);
            graphics.DrawLine(&line, -180.0f, -48.0f, 180.0f, -48.0f);
            const auto signatures = graphics.Save();
            graphics.SetClip(RectF(-180.0f, -10.0f, 365.0f * StoryMotion(t, 0.7, 1.8), 150.0f), CombineModeIntersect);
            label(L"서이린", -160.0f, 0.0f);
            label(m_playerName.c_str(), -160.0f, 60.0f);
            graphics.Restore(signatures);
            star(156.0f, 122.0f, 17.0f * StoryMotion(t, 2.0, 0.8));
            graphics.Restore(saved);
        }
    }
    else if (currentEffectKey == L"anim_ink")
    {
        graphics.RotateTransform(-5.0f);
        SolidBrush cover(Color(255, 32, 29, 48));
        SolidBrush page(Color(255, 208, 203, 220));
        graphics.FillRectangle(&shadow, RectF(-52.0f, -192.0f, 500.0f, 420.0f));
        graphics.FillRectangle(&cover, RectF(-70.0f, -210.0f, 500.0f, 420.0f));
        graphics.FillRectangle(&page, RectF(-50.0f, -193.0f, 460.0f, 386.0f));
        Pen spine(Color(180, 83, 75, 105), 7.0f);
        graphics.DrawLine(&spine, 180.0f, -190.0f, 180.0f, 190.0f);
        // 노트 안에 끼인 끝 부분만 흐려진다. 바깥의 관찰 기록은 그대로 남는다.
        graphics.FillRectangle(&paper, RectF(250.0f, -275.0f, 120.0f, 390.0f));
        Pen safeInk(Color(255, 107, 115, 132), 4.0f);
        for (int i = 0; i < 2; ++i)
            graphics.DrawLine(&safeInk, 267.0f, -245.0f + i * 24.0f, 346.0f, -245.0f + i * 24.0f);
        for (int i = 0; i < 8; ++i)
        {
            const float dissolve = StoryMotion(t, 0.6 + i * 0.2, 2.2);
            Pen fading(Color(static_cast<BYTE>(255.0f * (1.0f - dissolve)), 97, 89, 130), 4.0f);
            const float y = -130.0f + i * 28.0f;
            graphics.DrawLine(&fading, 267.0f, y, 346.0f, y);
            for (int mote = 0; mote < 4; ++mote)
            {
                const float x = 271.0f + mote * 22.0f + dissolve * (65.0f + i * 4.0f);
                SolidBrush dust(Color(static_cast<BYTE>(210.0f * std::sin(dissolve * 3.14159265f)), 193, 176, 239));
                graphics.FillEllipse(&dust, RectF(x, y - dissolve * (130.0f + mote * 18.0f), 5.0f, 5.0f));
            }
        }
        graphics.FillRectangle(&paper, RectF(-395.0f, -125.0f, 230.0f, 250.0f));
        label(L"바깥의 기록", -370.0f, -99.0f);
        for (int i = 0; i < 4; ++i)
            graphics.DrawLine(&safeInk, -369.0f, -30.0f + i * 33.0f, -195.0f, -30.0f + i * 33.0f);
    }
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
            SetCutsceneEffect(nextEffectKey);
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
