#include "PromiseScene.h"
#include "../AudioManager.h"
#include "ArcadeBackdrop.h"
extern AudioManager audioManager;
#include <algorithm>
#include <cmath>

namespace
{
    using namespace Gdiplus;
    const Color White(255, 236, 242, 255), Muted(255, 157, 178, 207), Gold(255, 255, 204, 113);
    const Color Blue(255, 104, 216, 237), Pink(255, 244, 147, 184), Violet(255, 181, 164, 246);
    const Color UnitColors[] = {Blue, Gold, Pink, Violet};
    const wchar_t* Names[] = {L"한세아", L"유하린", L"서이린"};
    const wchar_t* UnitNames[] = {L"방패병", L"저격수", L"폭죽포", L"충전기"};
    const wchar_t* UnitHints[] = {L"적을 붙잡는 전방 방어", L"강한 적에게 집중 사격", L"모인 적에게 범위 폭발", L"에너지 회복 속도 증가"};
    const wchar_t* EnemyNames[] = {L"일반병", L"질주병", L"중장갑", L"작은 무리", L"거대 파수꾼"};
    const wchar_t* UpgradeNames[] = {L"승리의 수리", L"관통 표식", L"연쇄 폭죽", L"가시 방패", L"과충전", L"회수 전지"};
    const wchar_t* UpgradeHints[] = {
        L"적을 처치하면 같은 줄의 방패병이 체력 24를 회복합니다.",
        L"저격수가 적을 관통합니다. 중장갑과 보스에게 피해가 40% 증가합니다.",
        L"폭죽 폭발이 주변 적 두 명에게 추가 전류를 전달합니다.",
        L"방패병이 받은 공격을 반사합니다. 연결 방어선과 함께 버텨 보세요.",
        L"충전기와 인접한 유닛의 공격 간격이 30% 짧아집니다.",
        L"처치마다 에너지 3을 회수합니다. 회수 시 유닛 가격의 80%를 돌려받습니다."};
    const wchar_t* SupportNames[] = {L"연결 보호막", L"돌파 하이파이브", L"느린 책갈피"};
    const wchar_t* SupportHints[] = {
        L"인접한 유닛은 피해 감소. 지원한 줄은 8초 동안 보호받아요.",
        L"연속 처치로 지원 충전. 선택한 줄의 적을 밀어내고 공격해요.",
        L"적을 느리게 모아요. 영역 안의 적은 폭죽 피해를 더 받아요."};
    const RECT StartButton{760, 800, 1160, 875};
    const RECT SupportButton{1510, 724, 1840, 790};
    const RECT UpgradeButton{1510, 940, 1666, 1000}, RecallButton{1680, 940, 1840, 1000};
    bool Inside(RECT r, int x, int y) { return x >= r.left && x < r.right && y >= r.top && y < r.bottom; }
    float LaneY(int lane) { return 295.f + lane * 194.f; }
    float UnitX(int slot) { return 370.f + slot % 3 * 245.f; }
    void Text(Graphics& g, const std::wstring& value, RectF rect, float size, Color color = White, bool bold = false)
    {
        Font font(L"맑은 고딕", size, bold ? FontStyleBold : FontStyleRegular, UnitPixel);
        SolidBrush ink(color); StringFormat format;
        g.DrawString(value.c_str(), -1, &font, rect, &format, &ink);
    }
    void Panel(Graphics& g, RectF rect, Color fill, Color border)
    {
        GraphicsPath path; constexpr float r = 22;
        path.AddArc(rect.X, rect.Y, r, r, 180, 90);
        path.AddArc(rect.GetRight()-r, rect.Y, r, r, 270, 90);
        path.AddArc(rect.GetRight()-r, rect.GetBottom()-r, r, r, 0, 90);
        path.AddArc(rect.X, rect.GetBottom()-r, r, r, 90, 90); path.CloseFigure();
        SolidBrush brush(fill); Pen pen(border, 1.5f);
        g.FillPath(&brush, &path); g.DrawPath(&pen, &path);
    }
    void Bar(Graphics& g, float x, float y, float w, float amount, Color color, float h = 7)
    {
        SolidBrush back(Color(255, 38, 49, 69)), fill(color);
        g.FillRectangle(&back, x, y, w, h);
        g.FillRectangle(&fill, x, y, w * (std::clamp)(amount, 0.f, 1.f), h);
    }
}

void PromiseScene::Initialize(int who, bool showTutorial)
{
    Shutdown();
    heroine = (std::clamp)(who, 0, 2); tutorial = showTutorial;
    finished = victory = choosing = bossSpawned = false; practice = true;
    practicedPlace = practicedRecall = practicedSupport = false;
    selected = selectedSlot = -1; upgradeRound = kills = combo = spawnIndex = 0;
    time = practiceTime = reaction = visualTime = comboTime = shotSoundCooldown = 0;
    energy = 90; baseHp = support = 100; spawnTimer = 2;
    units.fill(Unit{}); upgrades.fill(false); barrier.fill(0); slow.fill(0);
    enemies.clear(); sparks.clear();
    feedback = L"연습은 점수에 포함되지 않아요. 유닛을 골라 빛나는 칸에 놓아 보세요.";
    GdiplusStartupInput input; GdiplusStartup(&graphicsToken, &input, nullptr);
    const wchar_t* folders[] = {L"hansea", L"yuharin", L"seoirin"};
    const std::wstring root = L"resource/heroine/" + std::wstring(folders[heroine]);
    portrait = new Image((root + L"/normal.png").c_str());
    smile = new Image((root + L"/smile.png").c_str());
    ResetClock();
}
void PromiseScene::Shutdown()
{
    delete portrait; portrait = nullptr; delete smile; smile = nullptr;
    if (graphicsToken) GdiplusShutdown(graphicsToken);
    graphicsToken = 0; enemies.clear(); sparks.clear(); CancelDrag();
}
RECT PromiseScene::SlotRect(int slot) const
{
    const LONG x = static_cast<LONG>(UnitX(slot)), y = static_cast<LONG>(LaneY(slot / 3));
    return {x - 82, y - 68, x + 82, y + 68};
}
bool PromiseScene::PlaceUnit(int slot, int type)
{
    if (slot < 0 || slot >= 9 || type < 0 || type >= 4 || units[slot].type >= 0 || energy < Costs[type]) return false;
    energy -= Costs[type]; units[slot] = {type, Health[type], .15f};
    practicedPlace = true; selectedSlot = slot;
    sparks.push_back({UnitX(slot), LaneY(slot/3), 0, 0, .5f, 2});
    feedback = std::wstring(UnitNames[type]) + L" 배치 완료. 놓인 유닛을 선택하면 강화·회수할 수 있어요.";
    audioManager.PlaySfx(L"click");
    return true;
}
bool PromiseScene::UpgradeUnit(int slot)
{
    if (slot < 0 || slot >= 9 || units[slot].type < 0 || units[slot].level >= 2 || energy < 20) return false;
    auto& unit = units[slot]; energy -= 20; unit.level = 2;
    unit.hp += Health[unit.type] * .5f;
    feedback = L"유닛 강화! 최대 체력 +50%, 공격 +35%. 충전기는 에너지 생산이 늘어요.";
    sparks.push_back({UnitX(slot), LaneY(slot/3), 0, 0, .5f, 2});
    audioManager.PlaySfx(L"click");
    return true;
}
void PromiseScene::RecallUnit(int slot)
{
    if (slot < 0 || slot >= 9 || units[slot].type < 0) return;
    energy = (std::min)(140.f, energy + (Costs[units[slot].type] + (units[slot].level-1)*20) * (upgrades[Recycler] ? .8f : .65f));
    units[slot] = Unit{}; selectedSlot = -1; practicedRecall = true;
    feedback = L"에너지를 돌려받았어요. 다른 유닛으로 바꾸거나 위치를 옮겨 보세요.";
    audioManager.PlaySfx(L"click");
}
bool PromiseScene::Linked(int slot) const
{
    for (int other = 0; other < 9; ++other)
        if (units[other].type >= 0 && std::abs(other / 3 - slot / 3) + std::abs(other % 3 - slot % 3) == 1) return true;
    return false;
}
void PromiseScene::UseSupport(int lane)
{
    if (lane < 0 || lane >= 3 || support < 100) return;
    support = 0; reaction = 2.8f; selected = -1; practicedSupport = true;
    if (heroine == 0)
    {
        barrier[lane] = 8;
        feedback = L"세아  “이쪽은 내가 지킬게. 다른 줄을 부탁해.”";
        for (int i = lane*3; i < lane*3+3; ++i)
            if (units[i].type >= 0) units[i].hp = (std::min)(MaxHp(units[i]), units[i].hp + 45);
    }
    else if (heroine == 1)
    {
        for (auto& enemy : enemies) if (enemy.lane == lane && enemy.hp > 0)
        {
            enemy.x = (std::min)(1430.f, enemy.x + (enemy.type == 4 ? 45.f : 180.f));
            Damage(enemy, 65);
        }
        sparks.push_back({250, LaneY(lane), 1430, LaneY(lane), .9f, 3});
        feedback = L"하린  “지금이야! 같이 밀어붙이자!”";
    }
    else
    {
        slow[lane] = 10;
        feedback = L"이린  “서두르지 말아요. 여기 모이면 한 번에.”";
    }
    audioManager.PlaySfx(L"defense_support");
}
void PromiseScene::BeginBattle()
{
    practice = false; time = 0; spawnIndex = kills = combo = 0; spawnTimer = 2;
    units.fill(Unit{}); enemies.clear(); sparks.clear();
    barrier.fill(0); slow.fill(0); energy = 90; support = baseHp = 100;
    selected = selectedSlot = -1;
    feedback = L"실전 시작! 연습 상태를 초기화했어요. 에너지 90으로 첫 방어선을 만들어요.";
}
int PromiseScene::NextLane() const { return (spawnIndex * 2 + spawnIndex / 4) % 3; }
int PromiseScene::NextType() const
{
    if (practice || time < 18) return 0;
    if (time < 40) return spawnIndex % 3 == 0 ? 1 : 0;
    if (time < 80) return spawnIndex % 4;
    return (spawnIndex + 2) % 4;
}
void PromiseScene::SpawnWave()
{
    const int lane = NextLane(), type = NextType();
    const float hp[] = {105, 70, 310, 55};
    const int count = type == 3 ? 3 : (!practice && time >= 80 ? 2 : 1);
    for (int i = 0; i < count; ++i)
        enemies.push_back({lane, type, 1430.f + i*48, hp[type], hp[type]});
    ++spawnIndex;
    spawnTimer += practice ? 6.f : time < 40 ? 4.2f : time < 80 ? 3.1f : 2.1f;
}
void PromiseScene::Damage(Enemy& enemy, float damage)
{
    if (enemy.hp <= 0) return;
    enemy.hp -= damage; enemy.flash = .12f;
}
void PromiseScene::RemoveDefeated()
{
    for (const auto& enemy : enemies) if (enemy.hp <= 0)
    {
        ++kills; combo = comboTime > 0 ? combo+1 : 1; comboTime = 4;
        if (heroine == 1) support = (std::min)(100.f, support + (combo >= 3 ? 12.f : 6.f));
        if (upgrades[Recycler]) energy = (std::min)(140.f, energy + 3);
        if (upgrades[Repair]) for (int i=enemy.lane*3; i<enemy.lane*3+3; ++i)
            if (units[i].type == Shield) units[i].hp = (std::min)(MaxHp(units[i]), units[i].hp + 24);
        sparks.push_back({enemy.x, LaneY(enemy.lane), 0, 0, .45f, 2});
    }
    std::erase_if(enemies, [](const Enemy& e) { return e.hp <= 0; });
}
void PromiseScene::OpenUpgrades()
{
    choosing = true; selected = -1;
    if (upgradeRound == 0) offers = {Repair, Pierce, Chain};
    else
    {
        // 첫 강화에 맞는 후속 강화가 항상 첫 칸에 나온다.
        const int paired = upgrades[Repair] ? Thorns : upgrades[Pierce] ? Overcharge : Recycler;
        offers = {paired, paired == Thorns ? Overcharge : Thorns, paired == Recycler ? Overcharge : Recycler};
    }
}
void PromiseScene::ChooseUpgrade(int option)
{
    if (!choosing || option < 0 || option >= 3) return;
    upgrades[offers[option]] = true; choosing = false; ++upgradeRound;
    feedback = std::wstring(UpgradeNames[offers[option]]) + L" 장착! 다음 적의 방향을 확인하세요.";
    ResetClock(); audioManager.PlaySfx(L"click");
}
int PromiseScene::GetScore() const
{
    if (victory)
    {
        // 공통 등급표의 S=90, A=80을 유지한다. 체력 75% 이상이면 S.
        return baseHp >= 75 ? 90 + static_cast<int>((baseHp-75) * .4f)
            : 80 + static_cast<int>((std::max)(0.f, baseHp) * 9 / 75);
    }
    return practice ? 0 : (std::min)(79, static_cast<int>(time / Duration * 79));
}
void PromiseScene::Step(float dt)
{
    if (tutorial || choosing || finished) return;
    shotSoundCooldown = (std::max)(0.f, shotSoundCooldown-dt);
    visualTime += dt; reaction = (std::max)(0.f, reaction-dt); comboTime -= dt;
    for (auto& spark : sparks) spark.life -= dt;
    std::erase_if(sparks, [](const Spark& s) { return s.life <= 0; });
    if (practice)
    {
        practiceTime += dt;
        if (practiceTime >= PracticeDuration) { BeginBattle(); return; }
    }
    else
    {
        time = (std::min)(Duration, time + dt);
        if ((upgradeRound == 0 && time >= 40) || (upgradeRound == 1 && time >= 80)) { OpenUpgrades(); return; }
    }
    float charging = 4;
    for (const auto& unit : units) if (unit.type == Charger) charging += unit.level == 2 ? 1.8f : 1.35f;
    energy = (std::min)(140.f, energy + dt * charging);
    support = (std::min)(100.f, support + dt * (practice ? 20.f : 5.f));
    for (int lane=0; lane<3; ++lane) { barrier[lane] = (std::max)(0.f, barrier[lane]-dt); slow[lane] = (std::max)(0.f, slow[lane]-dt); }
    spawnTimer -= dt; if (spawnTimer <= 0) SpawnWave();
    if (!practice && !bossSpawned && time >= 120)
    {
        enemies.push_back({1, 4, 1430, 1150, 1150}); bossSpawned = true;
        feedback = L"거대 파수꾼 등장! 중앙을 지키면서 위아래의 작은 적도 확인해요.";
    }
    for (int i=0; i<9; ++i)
    {
        auto& unit = units[i]; if (unit.type < 0 || unit.type == Charger) continue;
        unit.cooldown -= dt; if (unit.cooldown > 0) continue;
        Enemy* target = nullptr;
        for (auto& e : enemies)
            if (e.hp > 0 && e.lane == i/3 && e.x >= UnitX(i)-65 && e.x < UnitX(i)+(unit.type == Shield ? 125 : unit.type == Sniper ? 690 : 550)
                && (!target || e.x < target->x)) target = &e;
        if (!target) continue;
        if (unit.type != Shield && shotSoundCooldown <= 0)
        { audioManager.PlaySfx(L"defense_shot"); shotSoundCooldown = .22f; }
        const float interval[] = {.85f, 1.1f, 1.5f}; unit.cooldown = interval[unit.type];
        if (upgrades[Overcharge]) for (int other=0; other<9; ++other)
            if (units[other].type == Charger && std::abs(other/3-i/3)+std::abs(other%3-i%3)==1) { unit.cooldown *= .7f; break; }
        const float damage[] = {8, 42, 29};
        const float power = unit.level == 2 ? 1.35f : 1.f;
        const float impactX = target->x;
        if (unit.type == Cannon)
        {
            for (auto& e : enemies) if (e.lane == i/3 && std::abs(e.x-impactX)<115)
                Damage(e, damage[Cannon] * power * (slow[e.lane]>0 && heroine==2 ? 1.5f : 1.f));
            if (upgrades[Chain])
            {
                int chains = 0;
                for (auto& e : enemies) if (&e != target && e.hp>0 && e.lane==i/3 && std::abs(e.x-impactX)<270 && chains++<2)
                { Damage(e, 22 * power); sparks.push_back({impactX, LaneY(i/3), e.x, LaneY(e.lane)-20, .3f, 1}); }
            }
            sparks.push_back({impactX, LaneY(i/3), 0, 0, .45f, 2});
        }
        else if (unit.type == Sniper && upgrades[Pierce])
        {
            for (auto& e : enemies) if (e.hp>0 && e.lane==i/3 && e.x>=impactX)
                Damage(e, power * (&e == target ? 42.f : 22.f) * (e.type==2 || e.type==4 ? 1.4f : 1.f));
        }
        else Damage(*target, damage[unit.type] * power);
        sparks.push_back({UnitX(i)+25, LaneY(i/3)-10, impactX, LaneY(i/3), .18f, unit.type==Cannon ? 1 : 0});
    }
    for (auto& e : enemies)
    {
        if (e.hp <= 0) continue;
        e.flash = (std::max)(0.f, e.flash-dt); e.cooldown -= dt;
        int blocked = -1;
        for (int i=e.lane*3+2; i>=e.lane*3; --i)
            if (units[i].type>=0 && e.x>=UnitX(i)-35 && e.x<=UnitX(i)+90) { blocked=i; break; }
        if (blocked >= 0)
        {
            if (e.cooldown <= 0)
            {
                auto& unit = units[blocked];
                float hit = e.type==4 ? 33.f : e.type==2 ? 22.f : 13.f;
                if (heroine==0 && Linked(blocked)) hit *= .72f;
                if (barrier[e.lane]>0) hit *= .15f;
                unit.hp -= hit; e.cooldown = .9f;
                if (unit.type==Shield && upgrades[Thorns]) Damage(e, unit.level == 2 ? 19*1.35f : 19);
                if (unit.hp<=0) { unit = Unit{}; if (selectedSlot==blocked) selectedSlot=-1; }
            }
        }
        else
        {
            const float speed[] = {44, 78, 29, 48, 18};
            e.x -= dt * speed[e.type] * (slow[e.lane]>0 ? .35f : 1.f);
        }
        if (e.x < 206)
        {
            if (!practice) baseHp = (std::max)(0.f, baseHp-(e.type==4 ? 30.f : e.type==2 ? 12.f : 6.f));
            e.x = -1000; // 거점에 도달한 적은 처치 보상을 주지 않는다.
            feedback = L"거점이 공격받았어요. 빈 줄을 보강하거나 지원으로 시간을 벌어요.";
        }
    }
    std::erase_if(enemies, [](const Enemy& e) { return e.x < 0; });
    RemoveDefeated();
    if (!practice && (baseHp<=0 || time>=Duration))
    {
        victory = baseHp>0; finished = true; selected=-1;
    }
}
void PromiseScene::Update()
{
    const auto now = GetTickCount64();
    const float elapsed = (std::min)(.25f, (now-lastTick)/1000.f); lastTick=now;
    if (tutorial || choosing || finished) { accumulator=0; return; }
    accumulator += elapsed;
    // 고정 간격으로 충돌과 쿨다운을 계산해 렌더 속도에 따른 난이도 차이를 막는다.
    while (accumulator >= 1.f/60)
    {
        accumulator -= 1.f/60; Step(1.f/60);
        if (choosing || finished) { accumulator=0; break; }
    }
}
void PromiseScene::OnMouseDown(int x, int y)
{
    mouse = {x,y}; if (finished) return;
    if (tutorial) { if (Inside(StartButton,x,y)) { tutorial=false; ResetClock(); } return; }
    if (choosing)
    {
        for (int i=0; i<3; ++i) if (Inside({350+i*415,390,735+i*415,770},x,y)) ChooseUpgrade(i);
        return;
    }
    if (Inside(SupportButton,x,y))
    {
        if (support>=100) { selected=4; feedback=L"지원할 진입로를 클릭하세요."; }
        else feedback=L"지원 충전 중이에요. 오른쪽 게이지를 확인하세요.";
        return;
    }
    if (Inside(UpgradeButton,x,y)) { if (!UpgradeUnit(selectedSlot)) feedback=L"강화할 유닛을 선택하세요. 에너지 20이 필요하며 한 번 강화할 수 있어요."; return; }
    if (Inside(RecallButton,x,y)) { RecallUnit(selectedSlot); return; }
    for (int type=0; type<4; ++type)
        if (Inside({90+type*344,840,414+type*344,1008},x,y))
        { selected=type; feedback=std::wstring(UnitHints[type])+L" · 빈 배치 칸을 클릭하세요."; return; }
    if (selected==4 && x>=190 && x<1460 && y>=198 && y<780)
    { UseSupport((y-198)/194); return; }
    for (int slot=0; slot<9; ++slot) if (Inside(SlotRect(slot),x,y))
    {
        if (units[slot].type>=0) { selectedSlot=slot; selected=-1; feedback=L"강화로 체력과 공격을 높이거나, 회수해 다른 유닛으로 바꿀 수 있어요."; }
        else if (selected>=0 && selected<4 && !PlaceUnit(slot,selected)) feedback=L"에너지가 부족해요. 잠깐 충전하거나 기존 유닛을 회수하세요.";
        return;
    }
}

void PromiseScene::DrawArcade(HDC dc) { ArcadeArt::DrawArcadeBackdrop(dc); }
void PromiseScene::DrawUnit(Graphics& g, int type, float x, float y, float scale, bool enemy) const
{
    const auto state=g.Save(); g.TranslateTransform(x,y); g.ScaleTransform(scale,scale);
    const Color color = enemy ? (type==4 ? Color(255,224,108,152) : Color(255,204,129,140)) : UnitColors[type];
    SolidBrush body(color), dark(Color(255,35,44,65)), face(Color(255,245,235,215)), shadow(Color(75,0,0,0));
    g.FillEllipse(&shadow,-36.f,30.f,75.f,18.f);
    Pen outline(Color(255,45,49,70),3), limb(color,9);
    if (enemy)
    {
        const float march=std::sin(visualTime*7+x*.03f)*8;
        g.DrawLine(&limb,-15.f,20.f,-20.f+march,39.f); g.DrawLine(&limb,15.f,20.f,20.f-march,39.f);
        g.FillRectangle(&body,-28.f,-29.f,56.f,58.f);
        g.FillRectangle(&dark,-22.f,-18.f,44.f,15.f);
        SolidBrush eyes(Gold); g.FillRectangle(&eyes,-14.f,-14.f,8.f,6.f); g.FillRectangle(&eyes,7.f,-14.f,8.f,6.f);
        if(type==2 || type==4) { Pen armor(Gold,5); g.DrawRectangle(&armor,-32.f,-34.f,64.f,66.f); }
        if(type==1) { PointF fin[]={{-25,-25},{-45,-45},{10,-28}}; g.FillPolygon(&body,fin,3); }
        if(type==4)
        {
            PointF crown[]={{-29,-33},{-35,-58},{-9,-44},{0,-65},{12,-44},{34,-58},{28,-33}};
            SolidBrush gold(Gold); g.FillPolygon(&gold,crown,7);
        }
    }
    else if(type==Cannon)
    {
        g.FillEllipse(&dark,-34.f,5.f,28.f,34.f); g.FillEllipse(&dark,9.f,5.f,28.f,34.f);
        g.FillRectangle(&body,-22.f,-26.f,54.f,45.f);
        SolidBrush tip(Gold); g.FillRectangle(&tip,15.f,-22.f,37.f,22.f);
        g.DrawRectangle(&outline,-22.f,-26.f,54.f,45.f);
    }
    else if(type==Charger)
    {
        g.FillRectangle(&body,-25.f,-34.f,50.f,68.f);
        g.FillRectangle(&dark,-17.f,-25.f,34.f,50.f);
        PointF bolt[]={{2,-23},{-12,4},{0,4},{-3,24},{15,-6},{3,-6}};
        SolidBrush power(Gold); g.FillPolygon(&power,bolt,6);
        Pen ring(Color(120,181,164,246),2); const float radius=45+std::sin(visualTime*3)*4;
        g.DrawEllipse(&ring,-radius,-radius,radius*2,radius*2);
    }
    else
    {
        g.DrawLine(&limb,-11.f,19.f,-15.f,36.f); g.DrawLine(&limb,12.f,19.f,16.f,36.f);
        g.FillRectangle(&body,-22.f,-15.f,44.f,40.f);
        g.FillEllipse(&face,-19.f,-43.f,38.f,33.f); g.FillRectangle(&body,-23.f,-45.f,46.f,15.f);
        g.FillEllipse(&dark,6.f,-31.f,5.f,6.f);
        if(type==Shield)
        {
            PointF shield[]={{6,-10},{39,-10},{40,22},{23,37},{6,22}};
            g.FillPolygon(&body,shield,5); g.DrawPolygon(&outline,shield,5);
            Pen cross(White,3); g.DrawLine(&cross,23,-1,23,22); g.DrawLine(&cross,13,9,33,9);
        }
        else { Pen barrel(Gold,9); g.DrawLine(&barrel,7,-4,58,-4); }
    }
    g.Restore(state);
}
void PromiseScene::Render(HDC dc)
{
    DrawArcade(dc);
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias); g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    SolidBrush veil(Color(190,7,12,25)); g.FillRectangle(&veil,0,0,1920,1080);
    if(tutorial)
    {
        Panel(g,{260,145,1400,790},Color(248,18,29,48),Color(255,100,165,195));
        Text(g,L"AFTER SCHOOL / CO-OP TACTICS",{350,195,1100,40},25,Blue,true);
        Text(g,L"방과 후 마지막 웨이브",{350,260,1200,90},64,White,true);
        Text(g,std::wstring(Names[heroine])+L"와 세 진입로를 지켜요.",{350,362,1150,60},34,Gold);
        const wchar_t* titles[]={L"01  방어선 만들기",L"02  나만의 조합",L"03  함께 버티기"};
        const wchar_t* details[]={L"유닛을 골라 빈 칸에 놓아요.\n놓인 유닛은 강화·회수해요.",L"40초와 80초에 강화를 골라요.\n고르는 동안 전투는 멈춰요.",L"지원을 누르고 줄을 골라요.\n방어 성공 A · 체력 75%면 S."};
        for(int i=0;i<3;++i)
        {
            Panel(g,{350.f+i*410,470,385,250},Color(255,28,42,63),Color(255,63,82,108));
            DrawUnit(g,i,410.f+i*410,531,.8f);
            Text(g,titles[i],{382.f+i*410,590,325,40},27,White,true);
            Text(g,details[i],{382.f+i*410,638,325,76},21,Muted);
        }
        Text(g,L"먼저 20초 동안 연습해요. 연습 후 자원·체력을 모두 새로 준비합니다.",{350,739,1210,42},24,Muted);
        Panel(g,{760,800,400,75},Color(255,49,109,130),Blue);
        Text(g,L"같이 연습하기",{850,818,260,45},32,White,true);
        return;
    }
    SolidBrush heading(Color(255,13,21,37)); g.FillRectangle(&heading,55,135,1415,65);
    Panel(g,{55,28,1810,106},Color(245,18,28,47),Color(255,57,80,107));
    Text(g,L"방과 후 마지막 웨이브",{85,47,650,52},36,White,true);
    const int seconds = static_cast<int>(std::ceil(practice ? PracticeDuration-practiceTime : Duration-time));
    Text(g,(practice?L"연습  ":L"남은 시간  ")+std::to_wstring(seconds)+L"초",{860,46,275,43},28,practice?Gold:White,true);
    Text(g,L"거점  "+std::to_wstring(static_cast<int>(std::ceil(baseHp)))+L"%",{1170,46,220,42},27,Blue,true);
    Bar(g,1170,98,215,baseHp/100,Blue);
    Text(g,L"에너지  "+std::to_wstring(static_cast<int>(energy))+L" / 140",{1465,46,360,42},30,Gold,true);
    Bar(g,1465,99,355,energy/140,Gold);
    Text(g,practice?L"배치 · 회수 · 지원을 자유롭게 연습해요":time<40?L"WAVE 01   첫 방어선":time<80?L"WAVE 02   조합의 시작":L"FINAL WAVE   함께 만든 작전",{90,155,720,40},25,White,true);
    Text(g,L"다음: "+std::to_wstring(NextLane()+1)+L"번 진입로 · "+EnemyNames[NextType()]+L"  "+std::to_wstring((std::max)(1,static_cast<int>(std::ceil(spawnTimer))))+L"초",{900,155,550,36},22,Gold);
    for(int lane=0;lane<3;++lane)
    {
        const float y=LaneY(lane);
        const bool supportHover=selected==4 && mouse.x>=190 && mouse.x<1460 && mouse.y>=y-97 && mouse.y<y+97;
        Panel(g,{80,y-86,1380,172},Color(245,19+lane*4,36+lane*3,54+lane*5),supportHover?Gold:Color(255,52,75,98));
        Text(g,L"0"+std::to_wstring(lane+1),{103,y-64,85,45},32,Muted,true);
        DrawUnit(g,Shield,175,y+20,.8f);
        Pen path(Color(90,130,166,190),2);
        for(int x=235;x<1450;x+=48) g.DrawLine(&path,x,static_cast<int>(y+65),x+20,static_cast<int>(y+65));
        if(barrier[lane]>0 || slow[lane]>0)
        {
            const auto c=barrier[lane]>0?Blue:Violet;
            SolidBrush field(Color(40,c.GetR(),c.GetG(),c.GetB())); g.FillRectangle(&field,230.f,y-79,1218.f,158.f);
            Pen outline(Color(175,c.GetR(),c.GetG(),c.GetB()),3);
            if(barrier[lane]>0) g.DrawLine(&outline,955.f,y-75,955.f,y+75);
            else for(int x=250;x<1440;x+=90) g.DrawEllipse(&outline,x*1.f,y-54,60.f,108.f);
        }
    }
    for(int i=0;i<9;++i)
    {
        const auto r=SlotRect(i); const auto& unit=units[i];
        const bool hover=Inside(r,mouse.x,mouse.y);
        Panel(g,{static_cast<float>(r.left),static_cast<float>(r.top),164,136},Color(135,39,58,78),selectedSlot==i?Gold:hover?Blue:Color(180,66,92,115));
        if(unit.type<0)
        {
            Pen plus(Color(120,122,158,185),2); g.DrawLine(&plus,UnitX(i)-11,LaneY(i/3),UnitX(i)+11,LaneY(i/3));
            g.DrawLine(&plus,UnitX(i),LaneY(i/3)-11,UnitX(i),LaneY(i/3)+11);
            if(hover && selected>=0 && selected<4) DrawUnit(g,selected,UnitX(i),LaneY(i/3),.75f);
        }
        else
        {
            DrawUnit(g,unit.type,UnitX(i),LaneY(i/3),1);
            Bar(g,UnitX(i)-46,LaneY(i/3)+47,92,unit.hp/MaxHp(unit),UnitColors[unit.type],5);
            if(unit.level==2) Text(g,L"★",{UnitX(i)-68,LaneY(i/3)-58,40,35},25,Gold,true);
            if(heroine==0 && Linked(i)) { Pen link(Color(180,104,216,237),2); g.DrawEllipse(&link,UnitX(i)-52,LaneY(i/3)-51,104.f,103.f); }
        }
    }
    const auto boardClip=g.Save(); g.SetClip(RectF(195,198,1265,582));
    for(const auto& e:enemies)
    {
        const float scale=e.type==4?1.55f:e.type==3?.7f:1;
        DrawUnit(g,e.type,e.x,LaneY(e.lane),scale,true);
        Bar(g,e.x-36*scale,LaneY(e.lane)-60*scale,72*scale,e.hp/e.maxHp,e.type==4?Pink:Gold,5);
        if(e.flash>0) { Pen flash(Color(220,255,255,255),3); g.DrawEllipse(&flash,e.x-42,LaneY(e.lane)-46,84.f,84.f); }
    }
    for(const auto& s:sparks)
    {
        const int alpha=static_cast<int>((std::min)(1.f,s.life*4)*220);
        const Color color=s.kind==0?Gold:s.kind==1?Violet:s.kind==3?Pink:Blue;
        Pen pen(Color(alpha,color.GetR(),color.GetG(),color.GetB()),s.kind==3?10.f:3.f);
        if(s.kind==2)
        {
            const float radius=(.6f-s.life)*110;
            g.DrawEllipse(&pen,s.x-radius,s.y-radius,radius*2,radius*2);
            for(int p=0;p<6;++p) { const float a=p*1.047f; g.DrawLine(&pen,s.x+std::cos(a)*radius,s.y+std::sin(a)*radius,s.x+std::cos(a)*(radius+10),s.y+std::sin(a)*(radius+10)); }
        }
        else g.DrawLine(&pen,s.x,s.y,s.endX,s.endY);
    }
    g.Restore(boardClip);
    Panel(g,{1490,155,375,660},Color(245,25,34,55),Color(255,81,92,123));
    Text(g,L"PARTNER / "+std::wstring(Names[heroine]),{1518,178,330,38},25,UnitColors[heroine],true);
    Image* face=reaction>0?smile:portrait;
    if(face && face->GetLastStatus()==Ok)
    {
        const auto clip=g.Save(); g.SetClip(RectF(1500,222,350,330));
        const float h=420, w=h*face->GetWidth()/face->GetHeight();
        const float bounce=heroine==1 && reaction>0?-std::abs(std::sin(reaction*8))*15:0;
        if(heroine==2 && reaction>0) { g.TranslateTransform(1675,385); g.RotateTransform(-3); g.TranslateTransform(-1675,-385); }
        g.DrawImage(face,RectF(1675-w/2,225+bounce,w,h)); g.Restore(clip);
    }
    if(reaction>0)
    {
        Pen action(UnitColors[heroine],4);
        if(heroine==0) { g.DrawLine(&action,1530,440,1620,410); g.DrawLine(&action,1620,410,1601,407); }
        if(heroine==1) for(int i=0;i<5;++i) g.DrawLine(&action,1790.f,390.f,1790+std::cos(i*1.25f)*32,390+std::sin(i*1.25f)*32);
        if(heroine==2) g.DrawArc(&action,1618,345,110,95,190,100);
    }
    Text(g,SupportNames[heroine],{1518,555,325,44},28,White,true);
    Text(g,SupportHints[heroine],{1518,608,317,89},22,Muted);
    Bar(g,1518,704,315,support/100,UnitColors[heroine]);
    Panel(g,{1510,724,330,66},support>=100?Color(255,48,89,112):Color(255,37,47,67),selected==4?Gold:UnitColors[heroine]);
    Text(g,support>=100?L"지원할 줄 선택":L"지원 충전  "+std::to_wstring(static_cast<int>(support))+L"%",{1550,740,280,41},26,White,true);
    for(int type=0;type<4;++type)
    {
        const float x=90.f+type*344;
        Panel(g,{x,840,324,168},Color(250,24,38,58),selected==type?Gold:energy>=Costs[type]?UnitColors[type]:Color(255,65,75,95));
        DrawUnit(g,type,x+55,899,.8f);
        Text(g,UnitNames[type],{x+105,862,190,40},28,White,true);
        Text(g,std::to_wstring(Costs[type])+L" 에너지",{x+105,906,200,33},23,energy>=Costs[type]?Gold:Muted);
        Text(g,UnitHints[type],{x+18,967,300,33},21,Muted);
    }
    Text(g,L"처치 "+std::to_wstring(kills)+L"  /  연속 "+std::to_wstring(comboTime>0?combo:0),{1515,844,340,36},23,White,true);
    std::wstring active;
    for(int i=0;i<6;++i) if(upgrades[i]) { if(!active.empty()) active+=L" · "; active+=UpgradeNames[i]; }
    Text(g,active.empty()?L"강화: 40초 / 80초":active,{1515,889,330,50},20,Muted);
    Panel(g,{1510,940,156,60},Color(255,35,47,65),selectedSlot>=0 && energy>=20 && units[selectedSlot].level==1?Gold:Color(255,68,81,103));
    Text(g,selectedSlot>=0 && units[selectedSlot].level==2?L"강화 완료":L"강화 20",{1526,955,140,37},23,Gold);
    Panel(g,{1680,940,160,60},Color(255,35,47,65),selectedSlot>=0?Blue:Color(255,68,81,103));
    Text(g,L"회수",{1730,955,100,37},23,selectedSlot>=0?Blue:Muted);
    if(practice)
    {
        Text(g,std::wstring(practicedPlace?L"✓ 배치  ":L"○ 배치  ")+(practicedRecall?L"✓ 회수  ":L"○ 회수  ")+(practicedSupport?L"✓ 지원":L"○ 지원"),{90,790,930,34},22,Gold);
    }
    else Bar(g,90,805,1360,time/Duration,Blue,4);
    Text(g,feedback,{90,1031,1750,37},23,White);
    if(choosing)
    {
        SolidBrush overlay(Color(225,8,14,29)); g.FillRectangle(&overlay,0,0,1920,1080);
        Text(g,upgradeRound==0?L"우리의 첫 번째 작전":L"조합을 완성할 시간",{350,205,1300,85},55,White,true);
        Text(g,L"하나를 선택하세요. 전투는 멈춰 있어요.",{350,309,1250,45},28,Muted);
        for(int i=0;i<3;++i)
        {
            const float x=350.f+i*415;
            Panel(g,{x,390,385,380},Color(255,29,45,68),i==0 && upgradeRound==1?Gold:Blue);
            Text(g,upgradeRound==1 && i==0?L"선택한 강화와 연결":L"TACTICAL UPGRADE",{x+28,425,330,36},21,Gold,true);
            const int icons[] = {Shield, Sniper, Cannon, Shield, Charger, Charger};
            DrawUnit(g,icons[offers[i]],x+190,515,1.15f);
            Text(g,UpgradeNames[offers[i]],{x+28,583,330,48},32,White,true);
            Text(g,UpgradeHints[offers[i]],{x+28,650,325,100},24,Muted);
        }
        Text(g,L"마지막 80초에는 완성된 조합으로 모든 적을 상대합니다.",{350,820,1250,50},27,Gold);
    }
}
