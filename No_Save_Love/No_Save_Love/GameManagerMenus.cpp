#include "GameManager.h"
#include "AudioManager.h"
#include "MenuUI.h"
#include "Scenes/ArcadeBackdrop.h"
#include <algorithm>

extern AudioManager audioManager;

namespace
{
    const wchar_t* HeroineNames[] = {L"한세아", L"유하린", L"서이린"};
    const wchar_t* GameNames[] = {L"PC방 알바", L"노래 연습", L"등교길", L"유혹 피하기", L"방과 후 마지막 웨이브", L"개화 공연"};
    const RECT PanelClose = {1510, 115, 1730, 175};
    const RECT SettingsTitle = {520, 825, 780, 895};
    const RECT GalleryTitle = {830, 825, 1090, 895};
    const RECT PracticeTitle = {1140, 825, 1400, 895};
    RECT DifficultyButton(int i) { return {1030 + i * 225, 255, 1230 + i * 225, 315}; }
    RECT EventCard(int i) { int x = 245 + i % 3 * 485, y = 300 + i / 3 * 115; return {x, y, x + 460, y + 95}; }
    RECT PracticeCard(int i) { int x = 245 + i % 2 * 725, y = 390 + i / 2 * 155; return {x, y, x + 690, y + 130}; }
}

bool GameManager::IsMiniGameRunning() const
{
    return now_game_mode >= game_mode_info::MiniGame1 && now_game_mode <= game_mode_info::MiniGame6;
}

void GameManager::ApplyOptions(bool windowChanged)
{
    audioManager.ApplyUserVolumes();
    if (m_isMiniGame2Initialized) minigame2.ApplyUserVolumes();
    if (windowChanged && m_hWnd)
    {
        SendMessageW(m_hWnd, WM_APP + 10, 0, 0);
        if (m_isMiniGame2Initialized) minigame2.RememberWindowBounds();
    }
    m_profile.Save();
}

void GameManager::OnWindowModeChanged()
{
    if (m_isMiniGame2Initialized) minigame2.RememberWindowBounds();
    m_profile.Save();
}

void GameManager::OnFocusLost()
{
    if (IsMiniGameRunning()) SetPauseMenuOpen(true);
}

void GameManager::OpenPanel(Panel panel)
{
    m_panelKeepsPause = m_exitConfirmOpen;
    SetPauseMenuOpen(true);
    m_panel = panel;
    if (panel == Panel::Notebook && !m_notebookContinuesStory)
    {
        m_notebookPage = 0;
        for (int i = 0; i < 5; ++i) if (m_completedChoices[i] >= 0) m_notebookPage = i;
    }
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void GameManager::ClosePanel()
{
    const bool continueStory = m_notebookContinuesStory;
    m_panel = Panel::None;
    m_notebookContinuesStory = false;
    if (!m_panelKeepsPause) SetPauseMenuOpen(false);
    if (continueStory) AdvanceFinishedStory();
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void GameManager::RenderTitleActions(HDC dc)
{
    MenuUI::Box(dc, {490, 805, 1430, 915}, RGB(250, 247, 242));
    MenuUI::Button(dc, SettingsTitle, L"설정");
    MenuUI::Button(dc, GalleryTitle, L"장면 회상");
    MenuUI::Button(dc, PracticeTitle, L"미니게임 연습");
    MenuUI::Badge(dc, {815, 940, 1105, 982}, L"난이도 · " + std::wstring(GameOptions::DifficultyName()),
        MenuUI::Paper, MenuUI::Muted);
}

bool GameManager::HandleTitleAction(int x, int y)
{
    if (MenuUI::Hit(SettingsTitle, x, y)) OpenPanel(Panel::Settings);
    else if (MenuUI::Hit(GalleryTitle, x, y)) OpenPanel(Panel::Gallery);
    else if (MenuUI::Hit(PracticeTitle, x, y)) OpenPanel(Panel::Practice);
    else return false;
    return true;
}

void GameManager::RenderPanel(HDC dc)
{
    using namespace MenuUI;
    Background(dc);
    Box(dc, {176, 78, 1756, 1008}, RGB(218, 206, 211), RGB(218, 206, 211));
    Box(dc, {170, 70, 1750, 1000}, Paper);
    const wchar_t* category = m_panel == Panel::Settings ? L"플레이 설정" :
        m_panel == Panel::Notebook ? L"우리의 기록" :
        m_panel == Panel::Gallery ? L"장면 보관함" : L"자유 연습";
    const wchar_t* title = m_panel == Panel::Settings ? L"나에게 맞는 플레이" :
        m_panel == Panel::Notebook ? L"다섯 장의 사건 노트" :
        m_panel == Panel::Gallery ? L"다시 펼치는 장면" : L"미니게임 연습실";
    Text(dc, category, {235, 99, 1470, 130}, 20, Rose, DT_LEFT | DT_SINGLELINE, FW_SEMIBOLD);
    Text(dc, title, {235, 137, 1470, 195}, 40, Ink, DT_LEFT | DT_SINGLELINE, FW_BOLD);
    Button(dc, PanelClose, m_notebookContinuesStory ? L"이야기 계속" : L"닫기", m_notebookContinuesStory);
    Line(dc, 235, 210, 1680, 210);

    if (m_panel == Panel::Settings)
    {
        Box(dc, {235, 235, 1680, 380}, SoftRose, SoftRose);
        Text(dc, L"나만의 속도로", {265, 255, 975, 305}, 30, Ink, DT_LEFT | DT_SINGLELINE, FW_SEMIBOLD);
        Text(dc, IsMiniGameRunning() ? L"이번 게임의 난이도는 고정됩니다.\n결과 화면에서 바꾸고 다시 도전할 수 있어요." :
            L"여유로운 이야기부터 짜릿한 도전까지.\n선택한 난이도는 다음 게임부터 적용됩니다.", {265, 313, 995, 374}, 21, Muted);
        const wchar_t* levels[] = {L"스토리", L"보통", L"도전"};
        for (int i = 0; i < 3; ++i) Button(dc, DifficultyButton(i), levels[i], GameOptions::difficulty == i, !IsMiniGameRunning());
        const wchar_t* labels[] = {L"배경 음악", L"효과음", L"글자 속도", L"화면 흔들림", L"화면 모드"};
        const wchar_t* descriptions[] = {L"장면과 함께 흐르는 음악", L"클릭, 타격, 노트의 소리", L"대사가 나타나는 빠르기",
            L"움직임이 부담스럽다면 꺼 보세요", L"내 화면에 맞게 편안하게"};
        const wchar_t* speeds[] = {L"느리게", L"보통", L"빠르게"};
        const std::wstring values[] = {std::to_wstring(GameOptions::musicVolume) + L"%", std::to_wstring(GameOptions::effectsVolume) + L"%",
            speeds[GameOptions::textSpeed], GameOptions::screenShake ? L"켜짐" : L"꺼짐", GameOptions::fullscreen ? L"전체 화면" : L"창 모드"};
        for (int i = 0; i < 5; ++i)
        {
            int y = 410 + i * 90;
            Box(dc, {235, y - 13, 1680, y + 66}, RGB(255, 253, 250), RGB(255, 253, 250));
            Fill(dc, {263, y + 3, 267, y + 46}, i < 2 ? Rose : RGB(171, 154, 167));
            Text(dc, labels[i], {285, y - 5, 955, y + 35}, 27, Ink, DT_LEFT | DT_SINGLELINE, FW_SEMIBOLD);
            Text(dc, descriptions[i], {285, y + 32, 985, y + 64}, 19, Muted, DT_LEFT | DT_SINGLELINE);
            const wchar_t* left = i == 2 ? L"느리게" : i == 3 ? L"끄기" : i == 4 ? L"창" : L"−";
            const wchar_t* right = i == 2 ? L"빠르게" : i == 3 ? L"켜기" : i == 4 ? L"전체" : L"+";
            const bool canDecrease = i == 0 ? GameOptions::musicVolume > 0 : i == 1 ? GameOptions::effectsVolume > 0 :
                i == 2 ? GameOptions::textSpeed > 0 : true;
            const bool canIncrease = i == 0 ? GameOptions::musicVolume < 100 : i == 1 ? GameOptions::effectsVolume < 100 :
                i == 2 ? GameOptions::textSpeed < 2 : true;
            Button(dc, {1030, y - 5, 1130, y + 55}, left,
                (i == 3 && !GameOptions::screenShake) || (i == 4 && !GameOptions::fullscreen), canDecrease);
            Text(dc, values[i], {1150, y - 3, 1510, y + 40}, 28, Ink,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE, FW_SEMIBOLD);
            if (i < 2) Meter(dc, {1150, y + 48, 1510, y + 53}, i == 0 ? GameOptions::musicVolume : GameOptions::effectsVolume);
            Button(dc, {1540, y - 5, 1640, y + 55}, right,
                (i == 3 && GameOptions::screenShake) || (i == 4 && GameOptions::fullscreen), canIncrease);
        }
        Line(dc, 245, 870, 1680, 870);
        Text(dc, m_profile.saveFailed ? L"설정을 파일에 저장하지 못했습니다. 현재 실행에는 적용됩니다." :
            L"취향에 맞게 바꾸면 자동으로 저장돼요.", {255, 903, 1680, 955}, 23, m_profile.saveFailed ? Rose : Muted);
    }
    else if (m_panel == Panel::Notebook)
    {
        const int heroine = m_completedChoices[m_notebookPage];
        Box(dc, {240, 235, 940, 860}, RGB(243, 235, 227), RGB(235, 225, 217));
        Box(dc, {960, 235, 1680, 860}, RGB(255, 253, 250));
        Badge(dc, {280, 265, 465, 307}, L"기억 0" + std::to_wstring(m_notebookPage + 1), Paper, Rose);
        Text(dc, heroine >= 0 ? L"함께 보낸 하루" : L"기다리는 기억", {585, 270, 890, 310}, 21, Muted, DT_RIGHT | DT_SINGLELINE);
        if (heroine >= 0)
        {
            PrepareNotebookPhoto();
            Box(dc, {270, 325, 910, 700}, RGB(255, 255, 255), RGB(255, 255, 255));
            if (!m_notebookPhoto.IsNull()) m_notebookPhoto.Draw(dc, 280, 335);
            Text(dc, HeroineNames[heroine], {290, 725, 885, 775}, 32, Ink, DT_LEFT | DT_SINGLELINE, FW_SEMIBOLD);
            Text(dc, L"기록한 날짜  " + m_eventDates[m_notebookPage], {290, 790, 885, 832}, 21, Muted);
            Text(dc, storyData.GetEventTitle(m_notebookPage, heroine), {1010, 285, 1630, 395}, 35, Ink, DT_LEFT | DT_WORDBREAK, FW_BOLD);
            Line(dc, 1010, 410, 1630, 410);
            Text(dc, storyData.GetEventSummary(m_notebookPage, heroine), {1010, 438, 1625, 625}, 27);
            int counts[3]{};
            for (int who : m_completedChoices) if (who >= 0) ++counts[who];
            Text(dc, L"지금까지 함께한 만남", {1010, 655, 1620, 695}, 22, Muted);
            for (int i = 0; i < 3; ++i)
            {
                const int x = 1010 + i * 210;
                Box(dc, {x, 707, x + 195, 815}, i == heroine ? SoftRose : Paper, i == heroine ? SoftRose : Paper);
                Text(dc, HeroineNames[i], {x + 16, 720, x + 179, 752}, 20, Muted);
                Text(dc, std::to_wstring(counts[i]) + L"번", {x + 16, 757, x + 179, 802}, 30, i == heroine ? Rose : Ink,
                    DT_LEFT | DT_SINGLELINE, FW_SEMIBOLD);
            }
        }
        else
        {
            Text(dc, L"0" + std::to_wstring(m_notebookPage + 1), {300, 390, 870, 570}, 120, RGB(211, 191, 192), DT_CENTER | DT_SINGLELINE, FW_BOLD);
            Text(dc, L"아직 쓰지 않은 페이지", {300, 620, 880, 680}, 30, Muted, DT_CENTER | DT_SINGLELINE);
            Text(dc, L"어떤 하루가 기다리고 있을까요?", {1010, 355, 1620, 445}, 31, Ink, DT_LEFT | DT_WORDBREAK, FW_SEMIBOLD);
            Text(dc, L"사건을 마치면 사진과 함께\n그날의 기억이 이곳에 남습니다.", {1010, 480, 1615, 610}, 27, Muted);
        }
        int completed = 0;
        for (int who : m_completedChoices) if (who >= 0) ++completed;
        Text(dc, std::to_wstring(completed) + L" / 5  기록한 사건", {245, 915, 600, 960}, 22, Muted);
        for (int i = 0; i < 5; ++i)
            Button(dc, {650 + i * 125, 905, 755 + i * 125, 960}, std::to_wstring(i + 1), m_notebookPage == i);
    }
    else if (m_panel == Panel::Gallery)
    {
        Button(dc, {245, 220, 510, 275}, L"다섯 사건", m_galleryTab == 0);
        Button(dc, {535, 220, 800, 275}, L"엔딩", m_galleryTab == 1);
        int unlockedCount = 0;
        const unsigned mask = m_galleryTab == 0 ? m_profile.events : m_profile.endings;
        for (int i = 0; i < (m_galleryTab == 0 ? 15 : 7); ++i) if (mask & (1u << i)) ++unlockedCount;
        Text(dc, L"열어 본 기억  " + std::to_wstring(unlockedCount) + (m_galleryTab == 0 ? L" / 15" : L" / 7"),
            {850, 230, 1680, 275}, 23, Muted, DT_RIGHT | DT_SINGLELINE);
        const int count = m_galleryTab == 0 ? 15 : 7;
        for (int i = 0; i < count; ++i)
        {
            const bool unlocked = m_galleryTab == 0 ? m_profile.HasEvent(i / 3, i % 3) : (m_profile.endings & (1u << i)) != 0;
            const RECT card = EventCard(i);
            const bool hovered = unlocked && Interactive(card);
            Box(dc, card, unlocked ? (hovered ? SoftRose : RGB(255, 253, 250)) : RGB(240, 236, 233), hovered ? Rose : Border);
            std::wstring meta = m_galleryTab == 0 ? L"사건 0" + std::to_wstring(i / 3 + 1) + L"  ·  " + HeroineNames[i % 3] :
                i == 6 ? L"특별한 결말" : std::wstring(HeroineNames[i % 3]);
            std::wstring label = !unlocked ? L"아직 만나지 않은 장면" :
                m_galleryTab == 0 ? storyData.GetEventTitle(i / 3, i % 3) : i == 6 ? L"새누 · 숨겨진 결말" :
                i < 3 ? L"해피 엔딩" : L"배드 엔딩";
            Text(dc, meta, {card.left + 22, card.top + 12, card.right - 100, card.top + 42}, 19, unlocked ? Rose : Muted,
                DT_LEFT | DT_SINGLELINE, FW_SEMIBOLD);
            Text(dc, label, {card.left + 22, card.top + 48, card.right - 20, card.bottom - 8}, 23, unlocked ? Ink : Muted,
                DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS, unlocked ? FW_SEMIBOLD : FW_NORMAL);
            Badge(dc, {card.right - 85, card.top + 10, card.right - 15, card.top + 40}, unlocked ? L"회상" : L"잠김",
                unlocked ? SoftRose : RGB(232, 226, 224), unlocked ? Rose : Muted);
        }
        Text(dc, m_profile.saveFailed ? L"해금 기록을 파일에 저장하지 못했습니다." : L"해금한 장면은 게임을 다시 켜도 남아 있습니다.",
            {245, 920, 1680, 970}, 23, Muted);
    }
    else if (m_panel == Panel::Practice)
    {
        Text(dc, L"마음에 드는 게임을 골라, 부담 없이 한 번 더.", {245, 230, 1680, 275}, 25, Muted);
        Text(dc, L"동행", {245, 310, 405, 365}, 28);
        for (int i = 0; i < 3; ++i) Button(dc, {410 + i * 240, 300, 630 + i * 240, 360}, HeroineNames[i], m_practiceHeroine == i);
        Button(dc, {1200, 300, 1660, 360}, L"난이도 · " + std::wstring(GameOptions::DifficultyName()));
        for (int i = 0; i < 6; ++i)
        {
            const RECT card = PracticeCard(i);
            const wchar_t* genres[] = {L"주문과 배달", L"박자와 리듬", L"달리기와 점프", L"순발력과 회피", L"배치와 전략", L"마지막 무대"};
            const bool hovered = Interactive(card);
            Box(dc, card, hovered ? SoftRose : RGB(255, 253, 250), hovered ? Rose : Border);
            Badge(dc, {card.left + 20, card.top + 25, card.left + 82, card.top + 86}, L"0" + std::to_wstring(i + 1), SoftRose, Rose);
            Text(dc, GameNames[i], {card.left + 102, card.top + 17, card.right - 30, card.top + 57}, 27, Ink,
                DT_LEFT | DT_SINGLELINE, FW_SEMIBOLD);
            Text(dc, genres[i], {card.left + 102, card.top + 61, card.right - 180, card.top + 94}, 20, Muted);
            Text(dc, L"최고 " + std::to_wstring(m_profile.bestScores[GameOptions::difficulty][i]) + L"점",
                {card.right - 180, card.top + 62, card.right - 30, card.top + 94}, 21, Rose, DT_RIGHT | DT_SINGLELINE, FW_SEMIBOLD);
            Meter(dc, {card.left + 102, card.bottom - 21, card.right - 30, card.bottom - 16}, m_profile.bestScores[GameOptions::difficulty][i]);
        }
        Text(dc, L"연습에서는 이야기 능력치와 사건 기록이 바뀌지 않아요.", {245, 920, 1680, 970}, 23, Muted);
    }
}

void GameManager::PrepareNotebookPhoto()
{
    const int heroine = m_completedChoices[m_notebookPage];
    if (heroine < 0 || (m_notebookPhotoPage == m_notebookPage && m_notebookPhotoHeroine == heroine)) return;
    if (!m_notebookPhoto.IsNull()) m_notebookPhoto.Destroy();
    CImage full; full.Create(1920, 1080, 24);
    HDC fullDC = full.GetDC();
    const std::wstring key = storyData.GetEventBackgroundKey(m_notebookPage, heroine);
    auto* background = storyScene.GetBackgroundImage(key);
    if (key == L"arcade") ArcadeArt::DrawArcadeBackdrop(fullDC);
    else if (background && !background->IsNull()) background->Draw(fullDC, 0, 0, 1920, 1080);
    else MenuUI::Box(fullDC, {0, 0, 1920, 1080}, RGB(238, 221, 225));
    const wchar_t* portraits[] = {L"hansea_smile", L"yuharin_smile", L"seoirin_smile"};
    if (auto* portrait = storyScene.GetCharacterImage(portraits[heroine]))
    {
        Gdiplus::Graphics graphics(fullDC);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
        const float height = 1040.0f, width = height * portrait->GetWidth() / portrait->GetHeight();
        graphics.DrawImage(portrait, Gdiplus::RectF((1920 - width) / 2, 40, width, height));
    }
    m_notebookPhoto.Create(620, 349, 24);
    HDC photoDC = m_notebookPhoto.GetDC();
    SetStretchBltMode(photoDC, HALFTONE); SetBrushOrgEx(photoDC, 0, 0, nullptr);
    StretchBlt(photoDC, 0, 0, 620, 349, fullDC, 0, 0, 1920, 1080, SRCCOPY);
    m_notebookPhoto.ReleaseDC(); full.ReleaseDC();
    m_notebookPhotoPage = m_notebookPage;
    m_notebookPhotoHeroine = heroine;
}

void GameManager::HandlePanelClick(int x, int y)
{
    using namespace MenuUI;
    if (Hit(PanelClose, x, y)) { ClosePanel(); return; }
    if (m_panel == Panel::Settings)
    {
        bool changed = false, windowChanged = false;
        for (int i = 0; i < 3; ++i)
            if (!IsMiniGameRunning() && Hit(DifficultyButton(i), x, y)) { GameOptions::difficulty = i; changed = true; }
        for (int i = 0; i < 5; ++i)
        {
            const int row = 410 + i * 90;
            int direction = Hit({1030, row - 5, 1130, row + 55}, x, y) ? -1 : Hit({1540, row - 5, 1640, row + 55}, x, y) ? 1 : 0;
            if (!direction) continue;
            changed = true;
            if (i == 0) GameOptions::musicVolume = std::clamp(GameOptions::musicVolume + direction * 10, 0, 100);
            else if (i == 1) GameOptions::effectsVolume = std::clamp(GameOptions::effectsVolume + direction * 10, 0, 100);
            else if (i == 2) GameOptions::textSpeed = std::clamp(GameOptions::textSpeed + direction, 0, 2);
            else if (i == 3) GameOptions::screenShake = direction > 0;
            else { windowChanged = GameOptions::fullscreen != (direction > 0); GameOptions::fullscreen = direction > 0; }
        }
        if (changed) ApplyOptions(windowChanged);
    }
    else if (m_panel == Panel::Notebook)
    {
        for (int i = 0; i < 5; ++i) if (Hit({650 + i * 125, 905, 755 + i * 125, 960}, x, y)) m_notebookPage = i;
    }
    else if (m_panel == Panel::Gallery)
    {
        if (Hit({245, 220, 510, 275}, x, y)) m_galleryTab = 0;
        else if (Hit({535, 220, 800, 275}, x, y)) m_galleryTab = 1;
        else for (int i = 0; i < (m_galleryTab == 0 ? 15 : 7); ++i)
            if (Hit(EventCard(i), x, y)) { StartReplay(i); return; }
    }
    else if (m_panel == Panel::Practice)
    {
        for (int i = 0; i < 3; ++i) if (Hit({410 + i * 240, 300, 630 + i * 240, 360}, x, y)) m_practiceHeroine = i;
        if (Hit({1200, 300, 1660, 360}, x, y)) { GameOptions::difficulty = (GameOptions::difficulty + 1) % 3; ApplyOptions(); }
        for (int i = 0; i < 6; ++i) if (Hit(PracticeCard(i), x, y)) { StartPractice(i); return; }
    }
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

bool GameManager::CompleteEventIfNeeded()
{
    if (m_practice || m_replaying || m_activeEvent < 0) return false;
    // 다섯 번째 사건은 오락실 도입이 아니라 게임 뒤 대화까지 마쳐야 기록한다.
    if (m_activeEvent == 4 && (currentMiniGameIndex != 5 || m_performanceIntroShown)) return false;
    const int round = m_activeEvent, heroine = m_activeEventHeroine;
    m_completedChoices[round] = heroine;
    SYSTEMTIME today{}; GetLocalTime(&today);
    wchar_t date[32]{};
    swprintf_s(date, L"%04u.%02u.%02u", today.wYear, today.wMonth, today.wDay);
    m_eventDates[round] = date;
    m_profile.events |= 1u << (round * 3 + heroine);
    m_profile.Save();
    m_activeEvent = m_activeEventHeroine = -1;
    m_notebookPage = round;
    m_notebookContinuesStory = true;
    OpenPanel(Panel::Notebook);
    return true;
}

void GameManager::RetryMiniGame()
{
    if (!m_resultPending || m_lastResultGame < 0 || sceneTransition.IsActive()) return;
    player = m_resultBasePlayer;
    currentMiniGameIndex = m_lastResultGame;
    m_resultPending = false;
    resultScene.Reset();
    ReleasePendingMiniGame();
    StartCurrentMiniGame();
}

void GameManager::StartPractice(int game)
{
    if (game < 0 || game >= 6) return;
    ClosePanel();
    SetPauseMenuOpen(false);
    m_practice = true;
    m_resultPending = false;
    currentMiniGameIndex = game;
    promiseHeroineIndex = finalHeroineIndex = m_practiceHeroine;
    EnterCurrentMiniGameTutorial();
}

void GameManager::StartReplay(int item)
{
    if (m_galleryTab == 0 ? !m_profile.HasEvent(item / 3, item % 3) :
        (item < 0 || item >= 7 || (m_profile.endings & (1u << item)) == 0)) return;
    ClosePanel();
    SetPauseMenuOpen(false);
    m_replaying = true;
    m_replayEndingType = -1;
    const std::array<int, 5> unseen = {{-1, -1, -1, -1, -1}};
    if (m_galleryTab == 0)
    {
        auto lines = storyData.GetContextualBranchStory(item / 3, item % 3, unseen);
        if (item / 3 == 4)
        {
            const auto& after = storyData.GetPromiseStory(item % 3, true);
            lines.insert(lines.end(), after.begin(), after.end());
        }
        storyScene.SetDialogues(lines);
        RequestSceneChange(game_mode_info::Story);
    }
    else
    {
        m_replayEndingType = item == 6 ? 2 : item / 3;
        finalHeroineIndex = item == 6 ? 0 : item % 3;
        auto lines = storyData.GetContextualEndingStory(m_replayEndingType, finalHeroineIndex, unseen);
        std::erase_if(lines, [](const DialogueLineInfo& line) { return line.speaker == L"엔딩 CG"; });
        storyScene.SetDialogues(lines);
        RequestSceneChange(game_mode_info::EndingDialogue);
    }
}

void GameManager::ReturnToCollection(Panel panel)
{
    SetPauseMenuOpen(false);
    // 목록은 페이드가 끝난 뒤 연다. 전환 중 일시정지를 걸면 검은 화면에 멈춘다.
    ReturnToTitle();
    m_panelAfterTransition = panel;
}
