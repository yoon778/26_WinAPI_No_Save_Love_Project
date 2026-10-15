#include "PromiseScene.h"
#include <algorithm>
#include <cmath>

namespace
{
    using namespace Gdiplus;
    void PromiseText(Graphics& g, const std::wstring& text, RectF rect, float size, Color color, bool bold = false)
    {
        Font font(L"맑은 고딕", size, bold ? FontStyleBold : FontStyleRegular, UnitPixel);
        SolidBrush ink(color);
        StringFormat format;
        format.SetTrimming(StringTrimmingNone);
        g.DrawString(text.c_str(), -1, &font, rect, &format, &ink);
    }
    void PromisePanel(Graphics& g, RectF rect, Color fill, Color border)
    {
        GraphicsPath path;
        constexpr float r = 20;
        path.AddArc(rect.X, rect.Y, r, r, 180, 90);
        path.AddArc(rect.GetRight() - r, rect.Y, r, r, 270, 90);
        path.AddArc(rect.GetRight() - r, rect.GetBottom() - r, r, r, 0, 90);
        path.AddArc(rect.X, rect.GetBottom() - r, r, r, 90, 90);
        path.CloseFigure();
        SolidBrush paper(fill); Pen edge(border, 2);
        g.FillPath(&paper, &path); g.DrawPath(&edge, &path);
    }
    bool Inside(RECT r, int x, int y) { return x >= r.left && x < r.right && y >= r.top && y < r.bottom; }
    const Color Ink(255, 71, 58, 64), Muted(255, 126, 109, 112), Paper(255, 255, 249, 236);
    const wchar_t* Names[] = {L"한세아", L"유하린", L"서이린"};
    const wchar_t* StageNames[] = {L"01  물건의 자리를 찾아요", L"02  함께할 순서를 정해요", L"03  약속을 상자에 담아요"};
    const RECT NextButton{1490, 850, 1828, 920}, HintButton{1490, 945, 1828, 1005};
    const RECT StartButton{760, 800, 1160, 875}, ResetButton{1140, 202, 1370, 250};
}

const PromiseScene::Item PromiseScene::items[3][3][6] = {
    { // 세아: 서로 맡을 일을 나누고, 다음 사람에게도 쓸 수 있는 상자를 남긴다.
        {
            {L"영수증", L"금액을 다시 확인할 사람이 꺼내 볼 종이야.", L"회계 봉투", L"숫자는 네가 봐줘. 나도 다시 확인받고 싶었어."},
            {L"일정표", L"다음 담당자가 시간을 확인할 곳에 두자.", L"공용 게시판", L"내가 없어도 다음 시간을 알 수 있겠네."},
            {L"참가 서명", L"참여한 사람의 이름은 기록으로 남겨야 해.", L"활동 기록철", L"마지막 줄에 우리 둘 이름도 쓰자."},
            {L"수선 테이프", L"종이를 고칠 때 여러 사람이 함께 쓸 도구야.", L"공용 도구함", L"이건 내 서랍보다 여기 있는 게 맞겠다."},
            {L"예비 열쇠", L"문을 여는 책임을 다른 담당자와 나눌 거야.", L"인계 봉투", L"나만 일찍 와야 하는 건 아니었네."},
            {L"빈 카드", L"금요일 이후에도 만날 시간을 새로 적을 종이야.", L"약속 칸", L"이 칸은 업무 말고, 우리 약속으로 남겨두자."}
        },
        {
            {L"목록 읽기", L"누가 맡을지 나누기 전에, 남은 일을 먼저 확인해.", L"", L"먼저 볼 건 내 표정이 아니라 목록이었네."},
            {L"담당 나누기", L"목록을 읽은 다음, 수선을 시작하기 전에 둘이 일을 나눠.", L"", L"절반을 맡겨도 괜찮다는 연습부터."},
            {L"책 수선하기", L"담당을 정한 뒤 책을 고쳐. 완료 사진은 그다음이야.", L"", L"혼자 할 때보다 손이 덜 바쁘네."},
            {L"완료 사진", L"수선이 끝나면 찍어. 새 분류표를 붙이기 전 모습을 남겨.", L"", L"네 손도 나왔어. 같이 했다는 증거네."},
            {L"새 표 붙이기", L"사진을 찍고 나서, 공유 서가로 옮기기 전에 붙여.", L"", L"다음 사람도 바로 찾을 수 있겠다."},
            {L"공유 서가", L"표를 붙인 책을 마지막으로 모두의 자리에 돌려놔.", L"", L"마무리도 둘이 하니까, 끝이 보인다."}
        },
        {
            {L"일정 봉투", L"가로 두 칸. 다시 만날 날짜를 접어 넣었어.", L"", L"업무가 끝난 뒤 시간도 비워둘게."},
            {L"테이프 통", L"세로 두 칸. 세워서 넣는 수선 도구야.", L"", L"이제 누구든 꺼내 쓸 수 있어."},
            {L"이름표 묶음", L"세로 두 칸. 두 사람 이름을 나란히 묶었어.", L"", L"내 이름만 쓰던 칸이 줄었네."},
            {L"보존 기록철", L"가로 두 칸, 세로 두 칸. 가장 큰 기록철이야.", L"", L"무거운 건 같이 들자. 나도 말할 수 있네."},
            {L"예비 열쇠", L"한 칸. 다른 담당자가 가져갈 열쇠야.", L"", L"이것까지 내가 챙길 필요는 없겠지."},
            {L"약속 카드", L"한 칸. 공연에 가져갈 작은 카드야.", L"", L"내가 안 해도 남는 일이 있네."}
        }
    },
    { // 하린: 버릴 물건을 고르지 않고, 앞으로 어디에 둘지 함께 정한다.
        {
            {L"학교 지도", L"새로 올 학생도 길을 찾도록 학교에 남기고 싶어.", L"안내 게시판", L"내가 헤맨 길이 다른 사람에겐 도움이 되겠네."},
            {L"함께 찍은 사진", L"새 학교에서도 바로 꺼내 볼 수 있는 곳에 둘래.", L"내 가방", L"낯선 교실에서도 이건 익숙하겠다."},
            {L"빌린 책", L"챙겨 가기 전에 원래 주인에게 돌려줘야 해.", L"반납 선반", L"헤어지는 인사도 빌린 걸 돌려주는 것부터네."},
            {L"이어폰", L"토요일 버스에서 노래를 들을 때 쓸 거야.", L"버스 주머니", L"창밖만 보면 울 것 같으니까."},
            {L"연락 카드", L"새 주소는 너도 한 장 가지고 있어 줘.", L"네 봉투", L"이번엔 내가 먼저 연락해도 놀라지 마."},
            {L"다음 약속", L"한 사람만 기억하지 않게 두 사람이 함께 볼 곳에 적자.", L"우리 달력", L"다시 만나는 날까지 적으니까 덜 막막해."}
        },
        {
            {L"빌린 책 돌려주기", L"사진을 찍기 전에, 빌린 책부터 돌려주자.", L"", L"고마웠다는 말도 같이 전했어."},
            {L"학교에서 사진 찍기", L"책을 돌려준 뒤, 짐을 싸기 전 모습을 남겨.", L"", L"이번 사진은 억지로 웃지 않았어."},
            {L"가방 챙기기", L"사진을 찍은 뒤 가방에 넣어. 아직 버스를 타지는 않아.", L"", L"사진 넣을 자리는 먼저 비워뒀지."},
            {L"버스 타기", L"가방을 챙긴 다음 출발해. 도착 문자는 나중이야.", L"", L"떠나는 길도 한 칸씩 적으니까 할 만하네."},
            {L"도착 문자 보내기", L"버스에서 내린 뒤 보내. 다시 만나는 날보다는 먼저야.", L"", L"잘 도착했다고, 길게 안 써도 되지?"},
            {L"다시 만나기", L"도착 문자를 주고받은 뒤, 달력에 적은 날 만나.", L"", L"끝 칸이 이별이 아니라서 좋다."}
        },
        {
            {L"학교 사진", L"가로 두 칸. 구겨지지 않게 눕혀 넣어 줘.", L"", L"배경에 네 그림자도 들어갔네."},
            {L"필통", L"세로 두 칸. 새 교실에서도 쓰던 펜을 쓰고 싶어.", L"", L"낯선 것 사이에 익숙한 것 하나."},
            {L"편지 묶음", L"세로 두 칸. 읽는 건 버스에서 할래.", L"", L"답장은 천천히 써도 되지?"},
            {L"추억 앨범", L"가로 두 칸, 세로 두 칸. 가장 넓은 자리가 필요해.", L"", L"다음 사진 붙일 자리도 남겨뒀어."},
            {L"이어폰", L"한 칸. 버스에 타면 가장 먼저 꺼낼 거야.", L"", L"공연 노래도 목록에 넣어둘게."},
            {L"달력 카드", L"한 칸. 금요일 공연에 들고 갈 약속이야.", L"", L"다시 보러 올 핑계가 생겼네."}
        }
    },
    { // 이린: 완벽한 복원보다 다음 문장을 새로 쓰는 경험.
        {
            {L"첫 원고", L"남에게 보여주기 전, 아직 내 것으로 남겨둘 글이에요.", L"개인 봉투", L"아직 안 읽는 것도 도와주는 거예요."},
            {L"관찰 메모", L"문집을 만드는 사람들이 함께 볼 기록이에요.", L"공용 기록철", L"이 부분은 다른 사람이 이어 써도 좋겠어요."},
            {L"꽃 책갈피", L"읽던 자리로 돌아오게 해주는 물건이에요.", L"읽던 책", L"어디까지 읽었는지는 남아 있네요."},
            {L"교정 연필", L"틀린 문장을 지우고 새로 쓸 때 함께 써요.", L"문구함", L"지운 자리가 있어도 망친 종이는 아니니까."},
            {L"대출 카드", L"빌린 날짜와 돌려줄 날짜를 확인할 곳이에요.", L"반납대", L"오늘 날짜부터 다시 확인했어요."},
            {L"빈 약속 카드", L"옛 문장을 베끼지 않고 새로 한 줄 쓸 자리예요.", L"다음 이야기", L"아직 쓰지 않은 줄도 남겨둘 수 있네요."}
        },
        {
            {L"메모 펼치기", L"읽을 문장을 고르기 전에, 관찰 메모를 펼쳐요.", L"", L"다 읽지 않아도 괜찮아요."},
            {L"첫 문장 고르기", L"메모를 편 뒤 한 문장을 골라요. 고치는 건 그다음이에요.", L"", L"이 문장부터 시작하고 싶어요."},
            {L"한 줄 고치기", L"고른 문장을 고친 뒤에야 소리 내어 읽어요.", L"", L"고친 문장이 꼭 더 길 필요는 없네요."},
            {L"소리 내어 읽기", L"고친 문장을 읽어요. 책갈피를 끼우기 전이에요.", L"", L"끝까지 들어줘서 고마워요."},
            {L"책갈피 끼우기", L"읽은 다음 자리를 표시해요. 새 약속은 마지막에 써요.", L"", L"여기부터 다음 이야기를 쓰면 되겠어요."},
            {L"새 약속 쓰기", L"책갈피를 끼운 뒤, 마지막으로 다음 만남을 적어요.", L"", L"기억 못 하면, 새로 한 줄 쓰면 돼요."}
        },
        {
            {L"원고 봉투", L"가로 두 칸. 접은 첫 문장을 담았어요.", L"", L"열어 볼 때는 먼저 물어봐 줘요."},
            {L"관찰 수첩", L"세로 두 칸. 우리 글씨가 다른 색으로 남아 있어요.", L"", L"누가 썼는지 알아볼 수 있겠네요."},
            {L"연필 상자", L"세로 두 칸. 다음 문장을 쓸 도구예요.", L"", L"하나는 빌려줄게요. 같이 쓰게."},
            {L"작은 문집", L"가로 두 칸, 세로 두 칸. 네모난 문집이에요.", L"", L"빈 페이지도 그대로 두고 싶어요."},
            {L"꽃 책갈피", L"한 칸. 웃었던 자리에 꽂아둔 꽃이에요.", L"", L"이 꽃을 보면 오늘 생각이 날 것 같아요."},
            {L"새 약속 카드", L"한 칸. 공연장에 가져갈 새 문장이에요.", L"", L"이번엔 제가 먼저 들고 갈게요."}
        }
    }
};

void PromiseScene::Initialize(int who, bool showTutorial)
{
    Shutdown();
    heroine = (std::clamp)(who, 0, 2); stage = completed = 0; selected = -1;
    tutorial = showTutorial; finished = dragging = moved = false;
    positions.fill(-1); hints.fill(false); feedback = L"물건을 골라보세요. 단서는 여기에도 남아 있어요.";
    celebration = 0; lastTick = GetTickCount64();
    Gdiplus::GdiplusStartupInput input; Gdiplus::GdiplusStartup(&graphicsToken, &input, nullptr);
    const wchar_t* rooms[] = {L"resource/background/hansea_student_council_room.png", L"resource/background/yuharin_class_room.png", L"resource/background/library_day.png"};
    const wchar_t* portraits[] = {L"resource/heroine/hansea/normal.png", L"resource/heroine/yuharin/normal.png", L"resource/heroine/seoirin/normal.png"};
    background = new Gdiplus::Image(rooms[heroine]); portrait = new Gdiplus::Image(portraits[heroine]);
}
void PromiseScene::Shutdown()
{
    delete background; background = nullptr; delete portrait; portrait = nullptr;
    if (graphicsToken) Gdiplus::GdiplusShutdown(graphicsToken);
    graphicsToken = 0; CancelDrag();
}
void PromiseScene::Update()
{
    const auto now = GetTickCount64();
    celebration = (std::max)(0.0, celebration - (std::min)(.1, (now - lastTick) / 1000.0));
    lastTick = now;
}
int PromiseScene::PlacedCount() const { return static_cast<int>(std::count_if(positions.begin(), positions.end(), [](int p) { return p >= 0; })); }
int PromiseScene::TargetFor(int item) const { return stage == 0 ? pairOrder[item] : (stage == 1 ? item : packingSolution[item]); }
int PromiseScene::GetScore() const
{
    const int usedHints = static_cast<int>(std::count(hints.begin(), hints.end(), true));
    return (std::clamp)((completed + PlacedCount()) * 100 / 18 - usedHints * 5, 0, 100);
}
RECT PromiseScene::SlotRect(int slot) const
{
    const int x = 100 + slot % 3 * 420, y = 288 + slot / 3 * 235;
    return {x, y, x + 374, y + 195};
}
RECT PromiseScene::ItemRect(int item) const
{
    if (positions[item] >= 0)
    {
        if (stage < 2) { auto r = SlotRect(positions[item]); return {r.left + 16, r.top + 76, r.right - 16, r.bottom - 16}; }
        const int x = 460 + positions[item] % 4 * 125, y = 300 + positions[item] / 4 * 125;
        return {x + 3, y + 3, x + widths[item] * 125 - 3, y + heights[item] * 125 - 3};
    }
    const int order = static_cast<int>(std::find(std::begin(trayOrder), std::end(trayOrder), item) - std::begin(trayOrder));
    return {80 + order * 219, 828, 280 + order * 219, 1008};
}
bool PromiseScene::Fits(int item, int cell) const
{
    if (cell < 0 || cell >= 12 || cell % 4 + widths[item] > 4 || cell / 4 + heights[item] > 3) return false;
    unsigned occupied = 0;
    const auto mask = [](int p, int w, int h) { unsigned value = 0; for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) value |= 1u << (p + y * 4 + x); return value; };
    for (int i = 0; i < 6; ++i) if (i != item && positions[i] >= 0) occupied |= mask(positions[i], widths[i], heights[i]);
    return !(occupied & mask(cell, widths[item], heights[item]));
}
bool PromiseScene::TryPlace(int item, int x, int y)
{
    if (item < 0 || item >= 6) return false;
    int target = -1;
    if (stage < 2)
    {
        for (int i = 0; i < 6; ++i) if (Inside(SlotRect(i), x, y)) target = i;
        if (target != TargetFor(item)) { feedback = L"아직 맞는 자리가 아니에요. 물건의 단서를 다시 읽어보세요."; return false; }
    }
    else
    {
        if (x >= 460 && x < 960 && y >= 300 && y < 675) target = (y - 300) / 125 * 4 + (x - 460) / 125;
        if (!Fits(item, target)) { feedback = L"물건의 왼쪽 위를 빈 칸에 맞춰요. 겹치거나 상자 밖으로 나갈 수는 없어요."; return false; }
    }
    const bool fresh = positions[item] < 0;
    positions[item] = target; feedback = items[heroine][stage][item].reply;
    if (fresh) celebration = .65;
    selected = -1; dragging = false;
    if (PlacedCount() == 6) feedback = stage == 2 ? L"다 담았어요. 이 약속을 가지고 금요일 무대로 가요." : L"함께 정리했어요. 다음 장에서 이어가요.";
    return true;
}
void PromiseScene::CancelDrag() { dragging = moved = false; selected = -1; }
void PromiseScene::NextStage()
{
    if (PlacedCount() != 6) return;
    CancelDrag();
    if (stage == 2) { finished = true; return; }
    completed += 6; ++stage; positions.fill(-1);
    feedback = stage == 1 ? L"카드의 단서를 읽고, 일어난 순서대로 여섯 칸을 채워요." : L"물건 여섯 개를 빈틈없이 넣어요. 놓은 물건도 다시 옮길 수 있어요.";
}
void PromiseScene::OnMouseDown(int x, int y)
{
    if (finished) return;
    if (tutorial) { if (Inside(StartButton, x, y)) tutorial = false; return; }
    if (Inside(HintButton, x, y)) { hints[stage] = true; feedback = stage == 2 ? L"번호가 같은 칸을 맞춰보세요. 이미 놓은 물건을 옮기거나 비워도 괜찮아요." : L"자리의 작은 글씨가 물건의 이름을 알려줘요. 힌트는 이 장에서 한 번만 감점돼요."; return; }
    if (Inside(NextButton, x, y)) { NextStage(); return; }
    if (stage == 2 && Inside(ResetButton, x, y)) { positions.fill(-1); CancelDrag(); feedback = L"빈 상자에서 다시 시작해요. 점수는 깎이지 않아요."; return; }
    if (selected >= 0)
    {
        const bool destination = stage == 2 && Inside({460, 300, 960, 675}, x, y);
        bool onSlot = destination;
        if (stage < 2) for (int i = 0; i < 6; ++i) onSlot |= Inside(SlotRect(i), x, y);
        if (onSlot && TryPlace(selected, x, y)) return;
    }
    for (int i = 5; i >= 0; --i)
    {
        if (stage < 2 && positions[i] >= 0) continue;
        if (!Inside(ItemRect(i), x, y)) continue;
        selected = i; dragging = true; moved = false; mouse = dragStart = {x, y};
        feedback = items[heroine][stage][i].clue;
        return;
    }
}
void PromiseScene::OnMouseMove(int x, int y)
{
    mouse = {x, y};
    if (dragging && std::abs(x - dragStart.x) + std::abs(y - dragStart.y) > 10) moved = true;
}
void PromiseScene::OnMouseUp(int x, int y)
{
    if (!dragging) return;
    if (moved) TryPlace(selected, x, y);
    dragging = moved = false;
}

void PromiseScene::DrawItem(Gdiplus::Graphics& g, int item, RECT r)
{
    using namespace Gdiplus;
    const Color tones[] = {Color(255, 185, 115, 135), Color(255, 126, 165, 151), Color(255, 155, 137, 180), Color(255, 188, 150, 102), Color(255, 115, 156, 181), Color(255, 192, 134, 117)};
    const float w = float(r.right - r.left), h = float(r.bottom - r.top);
    const bool compact = w < 150;
    PromisePanel(g, RectF(float(r.left), float(r.top), w, h), Paper, selected == item ? tones[item] : Color(255, 216, 198, 183));
    SolidBrush accent(tones[item]);
    if (!compact) g.FillRectangle(&accent, r.left + 12, r.top + 12, 5, static_cast<int>(h) - 24);
    PromiseText(g, stage == 2 ? std::to_wstring(item + 1) : (stage == 0 ? L"물건" : L"단서"),
        RectF(float(r.left + 26), float(r.top + 12), 80, 38), 22, tones[item], true);
    std::wstring name = items[heroine][stage][item].name;
    if (compact && name.size() > 5)
    {
        const auto space = name.find_last_of(L' ');
        if (space != std::wstring::npos) name[space] = L'\n';
    }
    PromiseText(g, name, RectF(float(r.left + (compact ? 12 : 26)), float(r.top + 51), w - (compact ? 24 : 42), h - 54), compact ? 18.f : 25.f, Ink, true);
    if (stage == 2 && positions[item] < 0)
        PromiseText(g, std::to_wstring(widths[item]) + L" × " + std::to_wstring(heights[item]) + L"칸", RectF(float(r.left + 26), float(r.bottom - 42), w - 40, 32), 21, Muted);
}

void PromiseScene::Render(HDC dc)
{
    using namespace Gdiplus;
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias); g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    if (background && background->GetLastStatus() == Ok) g.DrawImage(background, Rect(0, 0, 1920, 1080));
    SolidBrush wash(Color(237, 242, 232, 222)); g.FillRectangle(&wash, 0, 0, 1920, 1080);
    const Color accent = heroine == 0 ? Color(255, 158, 88, 112) : heroine == 1 ? Color(255, 78, 135, 122) : Color(255, 126, 103, 158);
    PromiseText(g, L"다섯 번째 이야기  /  " + std::wstring(Names[heroine]), RectF(65, 27, 1150, 40), 22, Muted);
    PromiseText(g, L"노트 밖의 약속", RectF(60, 66, 1000, 66), 44, Ink, true);
    PromiseText(g, StageNames[stage], RectF(65, 145, 900, 40), 28, accent, true);
    PromiseText(g, std::to_wstring(completed + PlacedCount()) + L" / 18  정리", RectF(1180, 146, 240, 40), 26, Muted);
    PromisePanel(g, RectF(50, 195, 1360, 585), Color(246, 252, 247, 237), Color(255, 216, 199, 181));
    PromisePanel(g, RectF(1450, 195, 420, 833), Paper, Color(255, 216, 199, 181));
    PromiseText(g, stage == 0 ? L"물건을 고르면 단서가 보여요. 알맞은 보관함으로 옮겨요." : stage == 1 ? L"단서의 앞뒤 관계를 읽고 1번부터 6번까지 순서대로 놓아요." : L"물건의 왼쪽 위를 칸에 맞춰요. 겹치지 않게 여섯 개를 모두 담아요.", RectF(85, 215, 1040, 60), 25, Ink);
    if (stage < 2)
    {
        for (int slot = 0; slot < 6; ++slot)
        {
            const auto r = SlotRect(slot);
            const int item = stage == 0 ? static_cast<int>(std::find(std::begin(pairOrder), std::end(pairOrder), slot) - std::begin(pairOrder)) : slot;
            PromisePanel(g, RectF(float(r.left), float(r.top), float(r.right-r.left), float(r.bottom-r.top)), Color(255, 239, 229, 212), Color(255, 208, 188, 168));
            PromiseText(g, stage == 0 ? items[heroine][stage][item].destination : std::to_wstring(slot + 1) + L"번째", RectF(float(r.left + 20), float(r.top + 16), 330, 42), 27, accent, true);
            if (hints[stage]) PromiseText(g, items[heroine][stage][item].name, RectF(float(r.left + 20), float(r.top + 53), 330, 32), 20, Muted);
        }
    }
    else
    {
        PromisePanel(g, RectF(445, 285, 530, 405), Color(255, 229, 210, 182), Color(255, 177, 149, 121));
        Pen grid(Color(255, 192, 167, 137), 2);
        for (int x = 0; x <= 4; ++x) g.DrawLine(&grid, 460 + x * 125, 300, 460 + x * 125, 675);
        for (int y = 0; y <= 3; ++y) g.DrawLine(&grid, 460, 300 + y * 125, 960, 300 + y * 125);
        if (hints[stage])
            for (int i = 0; i < 6; ++i)
            {
                const int x = 460 + packingSolution[i] % 4 * 125, y = 300 + packingSolution[i] / 4 * 125;
                Pen hint(accent, 3); hint.SetDashStyle(DashStyleDash);
                g.DrawRectangle(&hint, x + 7, y + 7, widths[i] * 125 - 14, heights[i] * 125 - 14);
                PromiseText(g, std::to_wstring(i + 1), RectF(float(x + 18), float(y + 18), 50, 45), 28, accent, true);
            }
        PromisePanel(g, RectF(1140, 202, 230, 48), Paper, Color(255, 208, 188, 168));
        PromiseText(g, L"상자 다시 비우기", RectF(1160, 211, 200, 38), 22, Muted);
    }
    PromiseText(g, L"물건을 끌어 놓거나, 물건과 목적지를 차례로 클릭하세요.", RectF(80, 792, 1290, 32), 21, Muted);
    for (int i = 0; i < 6; ++i) if (!(dragging && moved && selected == i)) DrawItem(g, i, ItemRect(i));
    PromiseText(g, std::wstring(Names[heroine]) + L"와 함께", RectF(1490, 226, 340, 45), 29, accent, true);
    if (portrait && portrait->GetLastStatus() == Ok)
    {
        const float w = float(portrait->GetWidth()), h = float(portrait->GetHeight());
        g.DrawImage(portrait, RectF(1530, 290, 260, 300), w * .06f, h * .02f, w * .88f, h * .55f, UnitPixel);
    }
    PromiseText(g, selected >= 0 ? items[heroine][stage][selected].name : L"함께 남기는 말", RectF(1490, 605, 335, 42), 24, accent, true);
    PromiseText(g, feedback, RectF(1490, 651, 335, 180), 24, Ink);
    const bool complete = PlacedCount() == 6;
    PromisePanel(g, RectF(1490, 850, 338, 70), complete ? accent : Color(255, 222, 215, 207), complete ? accent : Color(255, 222, 215, 207));
    PromiseText(g, complete ? (stage == 2 ? L"약속 마무리하기" : L"다음 장으로") : L"물건을 모두 정리해요", RectF(1516, 869, 300, 44), 24, complete ? Paper : Muted, true);
    PromisePanel(g, RectF(1490, 945, 338, 60), Paper, Color(255, 208, 188, 168));
    PromiseText(g, hints[stage] ? L"이 장의 힌트 표시 중" : L"힌트 보기  ·  이 장 −5점", RectF(1511, 960, 300, 38), 22, Muted);
    if (selected >= 0 && stage == 2 && Inside({460, 300, 960, 675}, mouse.x, mouse.y))
    {
        const int cell = (mouse.y - 300) / 125 * 4 + (mouse.x - 460) / 125;
        const Color preview = Fits(selected, cell) ? Color(100, 78, 155, 118) : Color(100, 199, 91, 101);
        const auto saved = g.Save();
        g.SetClip(Rect(460, 300, 500, 375));
        SolidBrush ghost(preview);
        g.FillRectangle(&ghost, 460 + cell % 4 * 125, 300 + cell / 4 * 125, widths[selected] * 125, heights[selected] * 125);
        g.Restore(saved);
    }
    if (dragging && moved && selected >= 0) DrawItem(g, selected, {mouse.x + 15, mouse.y + 15, mouse.x + 215, mouse.y + 195});
    if (celebration > 0)
    {
        SolidBrush spark(accent);
        for (int i = 0; i < 8; ++i) { const float a = i * .7854f, radius = float((.65 - celebration) * 100); g.FillEllipse(&spark, 1380 + std::cos(a) * radius, 167 + std::sin(a) * radius, 5.f, 5.f); }
    }
    if (tutorial)
    {
        SolidBrush shade(Color(175, 68, 49, 57)); g.FillRectangle(&shade, 0, 0, 1920, 1080);
        PromisePanel(g, RectF(470, 160, 980, 780), Paper, Color(255, 201, 160, 163));
        PromiseText(g, L"노트 밖의 약속", RectF(560, 230, 800, 75), 48, accent, true);
        PromiseText(g, L"기억이 흐려져도, 함께 남긴 물건은 여기 있으니까.", RectF(560, 320, 800, 65), 27, Ink);
        PromiseText(g, L"01  단서를 읽고 물건의 짝 찾기\n\n02  여섯 사건을 순서대로 놓기\n\n03  작은 상자에 물건 여섯 개 담기", RectF(580, 420, 760, 255), 30, Ink);
        PromiseText(g, L"끌어 놓거나, 물건과 목적지를 차례로 클릭해요. 시간 제한은 없어요.\n오답 감점 없이 다시 시도해요. 힌트는 한 장마다 5점씩만 차감돼요.\n모두 정리하면 85~100점을 받아요.", RectF(560, 675, 810, 110), 23, Muted);
        PromisePanel(g, RectF(760, 800, 400, 75), accent, accent);
        PromiseText(g, L"함께 정리하기", RectF(860, 820, 285, 45), 28, Paper, true);
    }
}
