#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <array>
#include <vector>
#include <string>

// 기존 다섯 번째 게임 슬롯을 사용하는 3분 전술 디펜스.
class PromiseScene
{
public:
    void Initialize(int heroine, bool tutorial = false);
    void Shutdown();
    void Update();
    void Render(HDC dc);
    void OnMouseDown(int x, int y);
    void OnMouseMove(int x, int y) { mouse = {x, y}; }
    void OnMouseUp(int, int) {}
    void CancelDrag() { selected = -1; }
    bool IsDragging() const { return false; }
    bool IsTutorial() const { return tutorial; }
    bool IsFinished() const { return finished; }
    int GetScore() const;
    void FinishImmediately() { finished = true; }
    void ResetClock() { lastTick = GetTickCount64(); accumulator = 0; }
    static void DrawArcade(HDC dc);

private:
    enum UnitType { Shield, Sniper, Cannon, Charger };
    enum Upgrade { Repair, Pierce, Chain, Thorns, Overcharge, Recycler };
    struct Unit { int type = -1; float hp = 0, cooldown = 0; int level = 1; };
    struct Enemy { int lane, type; float x, hp, maxHp, cooldown = 0, flash = 0; };
    struct Spark { float x, y, endX, endY, life; int kind; };
    static constexpr float Duration = 160, PracticeDuration = 20;
    static constexpr int Costs[4] = {22, 30, 38, 26};
    static constexpr float Health[4] = {240, 110, 150, 100};
    std::array<Unit, 9> units{};
    std::vector<Enemy> enemies;
    std::vector<Spark> sparks;
    std::array<bool, 6> upgrades{};
    std::array<int, 3> offers{};
    std::array<float, 3> barrier{}, slow{};
    int heroine = 0, selected = -1, selectedSlot = -1, upgradeRound = 0;
    int spawnIndex = 0, kills = 0, combo = 0;
    float time = 0, practiceTime = 0, energy = 90, baseHp = 100;
    float support = 100, reaction = 0, visualTime = 0, spawnTimer = 2, comboTime = 0;
    float accumulator = 0, shotSoundCooldown = 0;
    bool tutorial = false, practice = true, choosing = false, finished = false, victory = false;
    bool practicedPlace = false, practicedRecall = false, practicedSupport = false;
    bool bossSpawned = false;
    POINT mouse{};
    ULONGLONG lastTick = 0;
    ULONG_PTR graphicsToken = 0;
    Gdiplus::Image* portrait = nullptr;
    Gdiplus::Image* smile = nullptr;
    std::wstring feedback;

    void Step(float dt);
    void BeginBattle();
    bool PlaceUnit(int slot, int type);
    void RecallUnit(int slot);
    bool UpgradeUnit(int slot);
    float MaxHp(const Unit& unit) const { return Health[unit.type] * (unit.level == 2 ? 1.5f : 1.f); }
    void UseSupport(int lane);
    void ChooseUpgrade(int option);
    void OpenUpgrades();
    void SpawnWave();
    void Damage(Enemy& enemy, float damage);
    void RemoveDefeated();
    int NextLane() const;
    int NextType() const;
    bool Linked(int slot) const;
    RECT SlotRect(int slot) const;
    void DrawUnit(Gdiplus::Graphics& g, int type, float x, float y, float scale, bool enemy = false) const;
};
