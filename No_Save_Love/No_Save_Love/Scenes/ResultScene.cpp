#include "ResultScene.h"
#include "../MenuUI.h"

// 결과 화면은 사용자가 선호한 이전 색상, 테두리, 글자 크기를 유지한다.
// 공통 메뉴 스타일을 바꾸지 않도록 이 화면 안에서만 적용한다.
namespace
{
    namespace ResultUI
    {
        constexpr COLORREF Paper = RGB(255, 250, 242);
        constexpr COLORREF Ink = RGB(65, 48, 65);
        constexpr COLORREF Muted = RGB(117, 99, 115);
        constexpr COLORREF Rose = RGB(184, 74, 111);
        void Box(HDC dc, RECT r, COLORREF fill = Paper)
        {
            HBRUSH brush = CreateSolidBrush(fill);
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(224, 203, 208));
            auto oldBrush = SelectObject(dc, brush); auto oldPen = SelectObject(dc, pen);
            RoundRect(dc, r.left, r.top, r.right, r.bottom, 22, 22);
            SelectObject(dc, oldBrush); SelectObject(dc, oldPen);
            DeleteObject(brush); DeleteObject(pen);
        }
        void Text(HDC dc, const std::wstring& value, RECT r, int size = 28,
            COLORREF color = Ink, UINT format = DT_LEFT | DT_WORDBREAK | DT_NOPREFIX)
        {
            // MenuUI는 음수 글꼴 높이를 사용하므로 부호를 바꿔 이전 셀 높이를 복원한다.
            MenuUI::Text(dc, value, r, -size, color, format, FW_MEDIUM);
        }
        void Button(HDC dc, RECT r, const std::wstring& label, bool selected = false)
        {
            Box(dc, r, selected ? Rose : Paper);
            Text(dc, label, r, 26, selected ? RGB(255, 255, 255) : Ink,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
    }
}

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
    using namespace ResultUI;
    Box(dc, {0, 0, 1920, 1080}, RGB(236, 224, 228));
    Box(dc, {100, 70, 1820, 910});
    Text(dc, GetGameName(m_whichGame), {155, 110, 1460, 180}, 44);
    const wchar_t* levels[] = {L"스토리", L"보통", L"도전"};
    Text(dc, std::wstring(m_practice ? L"연습 · " : L"이번 도전 · ") + levels[m_difficulty],
        {1430, 125, 1760, 175}, 27, Muted, DT_RIGHT | DT_SINGLELINE);
    Box(dc, {150, 220, 650, 815}, RGB(250, 236, 239));
    Text(dc, GetResultGradeText(m_score), {190, 250, 610, 450}, 148, Rose,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    Text(dc, std::to_wstring(m_score) + L" / 100", {190, 470, 610, 540}, 48, Ink,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    Text(dc, L"S 90   A 80   B 60\nC 40   D 20   F 0", {205, 580, 610, 680}, 28, Muted);
    Text(dc, m_practice ? L"연습 기록은 따로 보관합니다." : L"재도전하면 이번 결과를 교체합니다.",
        {190, 725, 620, 795}, 25, Muted);
    Text(dc, L"플레이 기록", {710, 220, 1740, 270}, 34, Rose);
    for (size_t i = 0; i < m_details.size() && i < 4; ++i)
        Text(dc, m_details[i], {715, 285 + int(i) * 65, 1735, 345 + int(i) * 65}, 27);
    Text(dc, m_practice ? L"현재 능력치 · 연습에서는 증가하지 않습니다" : L"이번에 성장한 능력치",
        {710, 565, 1750, 615}, 30, Rose);
    const wchar_t* names[] = {L"재력", L"대화", L"매력", L"외모"};
    const int values[] = {m_currentState.money, m_currentState.speech, m_currentState.charm, m_currentState.appearance};
    const int gains[] = {m_plusState.money, m_plusState.speech, m_plusState.charm, m_plusState.appearance};
    for (int i = 0; i < 4; ++i)
    {
        int left = 710 + i * 260;
        Box(dc, {left, 635, left + 235, 815}, RGB(248, 241, 235));
        Text(dc, names[i], {left + 20, 655, left + 215, 700}, 29);
        Text(dc, std::to_wstring(values[i]), {left + 20, 705, left + 125, 775}, 44);
        Text(dc, L"+" + std::to_wstring(m_practice ? 0 : gains[i]), {left + 135, 720, left + 220, 775}, 30, Rose);
    }
    Text(dc, m_practice ? L"최고 점수는 난이도별로 저장됩니다." : L"이야기 계속을 누르면 이 결과로 다음 만남을 시작합니다.",
        {710, 845, 1750, 890}, 25, Muted);
    Button(dc, m_retryButton, L"다시 도전");
    Button(dc, m_confirmButton, m_practice ? L"연습 메뉴" : L"이야기 계속", true);
    Text(dc, L"난이도 변경은 ESC의 설정에서", {1270, 965, 1780, 1020}, 26, Muted);
}
