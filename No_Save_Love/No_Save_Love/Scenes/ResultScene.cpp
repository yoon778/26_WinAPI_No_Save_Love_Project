#include "ResultScene.h"
#include "../MenuUI.h"

void ResultScene::Reset()
{
    m_whichGame = m_score = 0;
    m_hasResult = m_finished = m_retryRequested = m_practice = false;
    m_plusState = {}; m_currentState = {}; m_details.clear();
}
void ResultScene::SetResult(int whichGame, int score)
{
    m_whichGame = whichGame; m_score = ClampStat(score);
    m_plusState = StatBalance::CalculateGain(whichGame, m_score);
    m_hasResult = true; m_finished = m_retryRequested = false;
}
void ResultScene::SetCurrentPlayerState(const Player_state& state) { m_currentState = state; }
void ResultScene::SetPlayerName(const std::wstring& name) { m_playerName = name.empty() ? L"윤서" : name; }
void ResultScene::OnMouseClick(int x, int y)
{
    if (!m_hasResult) return;
    if (MenuUI::Hit(m_retryButton, x, y)) m_retryRequested = true;
    else if (MenuUI::Hit(m_confirmButton, x, y)) m_finished = true;
}
bool ResultScene::IsFinished() const { return m_finished; }
Player_state ResultScene::GetPlusState() const { return m_plusState; }
std::wstring ResultScene::GetGameName(int game) const
{
    const wchar_t* names[] = {L"PC방 알바", L"노래 연습", L"등교길", L"유혹 피하기",
        L"방과 후 마지막 웨이브", L"개화 - 금요일의 작은 무대"};
    return game >= 0 && game < 6 ? names[game] : L"미니게임";
}
void ResultScene::RenderResult(HDC dc)
{
    if (!m_hasResult) return;
    using namespace MenuUI;
    constexpr UINT single = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
    constexpr UINT wrapped = DT_LEFT | DT_WORDBREAK | DT_NOPREFIX;
    Background(dc);

    Text(dc, L"PLAY RECORD  /  " + std::to_wstring(m_whichGame + 1),
        {128, 65, 1300, 102}, 22, Muted, single);
    Text(dc, GetGameName(m_whichGame), {124, 114, 1450, 180}, 45, Ink, single, FW_SEMIBOLD);
    Text(dc, m_practice ? L"부담 없이 쌓아 가는 연습의 기록" : L"이번 도전이 다음 만남의 한 페이지가 됩니다.",
        {128, 179, 1440, 214}, 24, Muted, single);
    const wchar_t* levels[] = {L"스토리", L"보통", L"도전"};
    Badge(dc, {1530, 120, 1792, 167}, std::wstring(m_practice ? L"연습 · " : L"난이도 · ") + levels[m_difficulty], SoftRose, Rose);
    Line(dc, 128, 228, 1792, 228, Border);

    // 등급과 점수를 한눈에 읽고, 등급 기준은 같은 카드 안에서 확인한다.
    Box(dc, {128, 258, 600, 892}, RGB(255, 255, 255), Border);
    Fill(dc, {128, 280, 134, 870}, Plum);
    Text(dc, m_practice ? L"이번 연습의 기록" : L"이번 판의 기록", {170, 290, 555, 334}, 25, Muted, single);
    Text(dc, GetResultGradeText(m_score), {165, 346, 335, 525}, 154, Plum, single, FW_SEMIBOLD);
    Line(dc, 338, 387, 338, 495, Border);
    Text(dc, std::to_wstring(m_score), {365, 386, 555, 451}, 51, Ink, single, FW_SEMIBOLD);
    Text(dc, L"/ 100", {370, 455, 555, 495}, 27, Muted, single);
    const wchar_t* caption = m_score >= 90 ? L"기분 좋은 마무리" : m_score >= 60 ? L"차곡차곡 쌓은 기록" : L"다음 도전을 위한 기록";
    Text(dc, caption, {170, 551, 560, 598}, 29, Ink, single, FW_SEMIBOLD);
    Meter(dc, {170, 621, 558, 631}, m_score, 100, Rose);
    Text(dc, L"S 90   A 80   B 60\nC 40   D 20   F 0", {170, 650, 570, 718}, 23, Muted, wrapped);
    Box(dc, {158, 736, 570, 872}, SoftRose, SoftRose);
    Text(dc, m_practice ? L"이야기와 별개로 남기는 기록" : L"다시 도전해도 괜찮아요", {182, 752, 548, 793}, 25, Rose, single, FW_SEMIBOLD);
    Text(dc, m_practice ? L"연습 최고 점수는 저장되며\n능력치는 증가하지 않아요." : L"재도전하면 이번 결과를\n새 기록으로 바꿉니다.",
        {182, 797, 548, 869}, 24, Muted, wrapped);

    Box(dc, {644, 258, 1792, 668}, RGB(255, 255, 255), Border);
    Text(dc, L"플레이 기록", {678, 282, 1738, 329}, 31, Ink, single, FW_SEMIBOLD);
    const size_t recordCount = m_details.empty() ? 0 : (std::min)(m_details.size() - 1, size_t(3));
    for (size_t i = 0; i < recordCount; ++i)
    {
        const int top = 350 + static_cast<int>(i) * 59;
        Text(dc, L"0" + std::to_wstring(i + 1), {682, top, 740, top + 43}, 22, Rose, single);
        Text(dc, m_details[i], {746, top + 3, 1748, top + 57}, 26, Ink, wrapped);
        if (i + 1 < recordCount) Line(dc, 746, top + 54, 1748, top + 54, Border);
    }
    if (!m_details.empty())
    {
        Fill(dc, {645, 538, 1791, 667}, SoftRose);
        Text(dc, L"다음 판을 위한 힌트", {680, 552, 1748, 583}, 22, Rose, single, FW_SEMIBOLD);
        const auto& detail = m_details.back();
        const std::wstring hint = detail.rfind(L"팁: ", 0) == 0 ? detail.substr(3) : detail;
        Text(dc, hint, {680, 591, 1748, 664}, 25, Ink, wrapped);
    }

    Text(dc, m_practice ? L"현재 능력치" : L"이번에 성장한 능력치", {646, 679, 1420, 723}, 29, Ink, single, FW_SEMIBOLD);
    Text(dc, m_practice ? L"연습에서는 변하지 않아요" : L"이 기록으로 다음 만남을 이어갑니다", {1260, 683, 1792, 719}, 23, Muted,
        DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    const wchar_t* names[] = {L"재력", L"대화", L"매력", L"외모"};
    const int values[] = {m_currentState.money, m_currentState.speech, m_currentState.charm, m_currentState.appearance};
    const int gains[] = {m_plusState.money, m_plusState.speech, m_plusState.charm, m_plusState.appearance};
    for (int i = 0; i < 4; ++i)
    {
        const int left = 644 + i * 292;
        Box(dc, {left, 742, left + 272, 892}, RGB(255, 255, 255), Border);
        Text(dc, names[i], {left + 22, 755, left + 250, 792}, 24, Muted, single);
        Text(dc, std::to_wstring(values[i]), {left + 20, 798, left + 164, 853}, 43, Ink, single, FW_SEMIBOLD);
        Badge(dc, {left + 178, 807, left + 250, 844}, L"+" + std::to_wstring(m_practice ? 0 : gains[i]), SoftRose, Rose);
        Meter(dc, {left + 22, 867, left + 250, 874}, values[i], 100, Rose);
    }
    Button(dc, m_retryButton, L"다시 도전");
    Button(dc, m_confirmButton, m_practice ? L"연습 메뉴" : L"이야기 계속", true);
    Text(dc, L"난이도를 바꾸고 싶다면\nESC에서 설정을 열어 주세요.", {1280, 957, 1792, 1023}, 24, Muted, wrapped);
}
