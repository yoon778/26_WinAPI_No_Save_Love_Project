#include "avoidgame.h"

#include <cmath>
#include <cstdlib>

avoidgame::avoidgame()
{
    // 플레이어 기본값
    m_player = {};
    m_player.gravity = 2.0f;
    m_player.jumpPower = 28.0f;
    m_player.moveSpeed = 12.0f;

    // 게임 기본값
    m_game = {};
    m_game.life = MAX_LIFE;
    m_game.remainingTime = GAME_TIME_LIMIT;

    // 패턴 기본값
    m_pattern = {};
    m_pattern.currentPattern = PATTERN_NONE;
    m_pattern.nextPattern = 0;
    m_pattern.waitTimer = PATTERN_DELAY;

    m_kakao = {};
    m_foodPattern = {};
    m_blackhole = {};
}

void avoidgame::Initialize()
{
    // 랜덤 시드
    static bool isRandomSeeded = false;
    if (!isRandomSeeded)
    {
        srand(GetTickCount());
        isRandomSeeded = true;
    }

    Reset();
}

void avoidgame::Reset()
{
    CreatePlatforms();
    ResetPlayer();
    ResetPatterns();

    // 게임 상태 초기화
    m_game.life = MAX_LIFE;
    m_game.remainingTime = GAME_TIME_LIMIT;
    m_game.invincibleTimer = 0.0f;
    m_game.isFinished = false;
    m_game.isSuccess = false;
}

void avoidgame::Update()
{
    // 종료 상태
    if (m_game.isFinished)
    {
        return;
    }

    UpdatePlayer();

    // 무적 시간
    if (m_game.invincibleTimer > 0.0f)
    {
        m_game.invincibleTimer -= UPDATE_DELTA_SECONDS;
        if (m_game.invincibleTimer < 0.0f)
        {
            m_game.invincibleTimer = 0.0f;
        }
    }

    UpdatePatternScheduler();
    UpdateAttacks();
    UpdateFoodDrops();
    FinishPatternIfEmpty();
    UpdateGameTimer();
}

void avoidgame::Render(HDC hDC)
{
    RenderMap(hDC);
    DrawKakaoMole(hDC);
    DrawBlackhole(hDC);
    DrawAttacks(hDC);
    DrawFoodDrops(hDC);
    RenderPlatforms(hDC);
    RenderPlayer(hDC);
    RenderHud(hDC);

    // 결과 표시
    if (m_game.isFinished)
    {
        const wchar_t* resultText = m_game.isSuccess ? L"성공! 40초 버팀" : L"실패! 목숨이 모두 사라짐";
        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, m_game.isSuccess ? RGB(120, 255, 160) : RGB(255, 90, 90));
        TextOutW(hDC, SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2, resultText, lstrlenW(resultText));
    }
}

void avoidgame::OnKeyDown(WPARAM wParam)
{
    // 이동 입력
    switch (wParam)
    {
    case VK_LEFT:
        m_player.moveLeft = true;
        break;

    case VK_RIGHT:
        m_player.moveRight = true;
        break;

    case VK_UP:
    case VK_SPACE:
        Jump();
        break;

    default:
        break;
    }
}

void avoidgame::OnKeyUp(WPARAM wParam)
{
    // 이동 해제
    switch (wParam)
    {
    case VK_LEFT:
        m_player.moveLeft = false;
        break;

    case VK_RIGHT:
        m_player.moveRight = false;
        break;

    default:
        break;
    }
}

bool avoidgame::IsFinished() const
{
    return m_game.isFinished;
}

int avoidgame::GetScore() const
{
    return CalculateScore();
}

void avoidgame::ResetPlayer()
{
    // 시작 위치
    m_player.x = static_cast<float>((SCREEN_WIDTH - PLAYER_WIDTH) / 2);
    m_player.y = static_cast<float>(SCREEN_HEIGHT - FLOOR_HEIGHT - PLAYER_HEIGHT);

    // 물리 초기화
    m_player.velocityX = 0.0f;
    m_player.velocityY = 0.0f;
    m_player.moveLeft = false;
    m_player.moveRight = false;
    m_player.isGrounded = true;
    m_player.jumpCount = 0;
}

void avoidgame::UpdatePlayer()
{
    // 이전 위치
    float previousY = m_player.y;

    // 좌우 이동
    m_player.velocityX = 0.0f;
    if (m_player.moveLeft)
    {
        m_player.velocityX -= m_player.moveSpeed;
    }
    if (m_player.moveRight)
    {
        m_player.velocityX += m_player.moveSpeed;
    }

    // 중력 적용
    m_player.velocityY += m_player.gravity;

    // 위치 적용
    m_player.x += m_player.velocityX;
    m_player.y += m_player.velocityY;

    ClampPlayerPosition();

    // 착지 판정
    m_player.isGrounded = false;
    HandleLanding(previousY);
}

void avoidgame::RenderPlayer(HDC hDC)
{
    // 무적 색상
    RECT playerRect = GetPlayerRect();
    COLORREF playerColor = (m_game.invincibleTimer > 0.0f) ? RGB(255, 230, 120) : RGB(255, 170, 190);

    HBRUSH playerBrush = CreateSolidBrush(playerColor);
    HPEN playerPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));

    HGDIOBJ oldBrush = SelectObject(hDC, playerBrush);
    HGDIOBJ oldPen = SelectObject(hDC, playerPen);

    RoundRect(hDC, playerRect.left, playerRect.top, playerRect.right, playerRect.bottom, 14, 14);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(playerBrush);
    DeleteObject(playerPen);
}

void avoidgame::Jump()
{
    // 2단 점프 허용
    if (m_player.isGrounded || m_player.jumpCount < MAX_JUMP_COUNT)
    {
        m_player.velocityY = -m_player.jumpPower;
        m_player.isGrounded = false;
        m_player.jumpCount++;
    }
}

void avoidgame::HandleLanding(float previousY)
{
    // 상승 중 착지 금지
    if (m_player.velocityY < 0.0f)
    {
        return;
    }

    float previousBottom = previousY + PLAYER_HEIGHT;
    float currentBottom = m_player.y + PLAYER_HEIGHT;

    // 바닥 착지
    const float floorTop = static_cast<float>(SCREEN_HEIGHT - FLOOR_HEIGHT);
    if (currentBottom >= floorTop)
    {
        m_player.y = floorTop - PLAYER_HEIGHT;
        m_player.velocityY = 0.0f;
        m_player.isGrounded = true;
        m_player.jumpCount = 0;
        return;
    }

    // 발판 착지
    RECT playerRect = GetPlayerRect();
    for (int i = 0; i < static_cast<int>(m_platforms.size()); i++)
    {
        const RECT& platform = m_platforms[i].rect;

        bool isOverlappingX =
            playerRect.right > platform.left &&
            playerRect.left < platform.right;

        bool isFallingOntoPlatform =
            previousBottom <= platform.top &&
            currentBottom >= platform.top;

        if (isOverlappingX && isFallingOntoPlatform)
        {
            m_player.y = static_cast<float>(platform.top - PLAYER_HEIGHT);
            m_player.velocityY = 0.0f;
            m_player.isGrounded = true;
            m_player.jumpCount = 0;
            return;
        }
    }
}

void avoidgame::ClampPlayerPosition()
{
    // 좌우 제한
    if (m_player.x < 0.0f)
    {
        m_player.x = 0.0f;
        m_player.velocityX = 0.0f;
    }
    else if (m_player.x > SCREEN_WIDTH - PLAYER_WIDTH)
    {
        m_player.x = static_cast<float>(SCREEN_WIDTH - PLAYER_WIDTH);
        m_player.velocityX = 0.0f;
    }

    // 위쪽 제한
    if (m_player.y < 0.0f)
    {
        m_player.y = 0.0f;
        m_player.velocityY = 0.0f;
    }
}

RECT avoidgame::GetPlayerRect() const
{
    // 플레이어 충돌 영역
    RECT playerRect =
    {
        static_cast<LONG>(m_player.x),
        static_cast<LONG>(m_player.y),
        static_cast<LONG>(m_player.x + PLAYER_WIDTH),
        static_cast<LONG>(m_player.y + PLAYER_HEIGHT)
    };

    return playerRect;
}

void avoidgame::CreatePlatforms()
{
    // 기본 발판 배치
    m_platforms.clear();
    m_platforms.push_back(Platform{ RECT{ 260, 710, 640, 760 } });
    m_platforms.push_back(Platform{ RECT{ 260, 410, 640, 460 } });
    m_platforms.push_back(Platform{ RECT{ 1280, 710, 1660, 760 } });
    m_platforms.push_back(Platform{ RECT{ 1280, 430, 1660, 480 } });
}

void avoidgame::RenderMap(HDC hDC)
{
    // 검은 배경
    RECT backgroundRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    HBRUSH backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hDC, &backgroundRect, backgroundBrush);
    DeleteObject(backgroundBrush);

    // 흰색 바닥
    RECT floorRect = { 0, SCREEN_HEIGHT - FLOOR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT };
    HBRUSH floorBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hDC, &floorRect, floorBrush);
    DeleteObject(floorBrush);

    RenderPlatforms(hDC);
}

void avoidgame::RenderPlatforms(HDC hDC)
{
    // 흰색 발판
    HPEN platformPen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
    HGDIOBJ oldPen = SelectObject(hDC, platformPen);
    HGDIOBJ oldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));

    for (int i = 0; i < static_cast<int>(m_platforms.size()); i++)
    {
        const RECT& rc = m_platforms[i].rect;
        Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    }

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(platformPen);
}

void avoidgame::RenderHud(HDC hDC)
{
    SetBkMode(hDC, TRANSPARENT);

    // 목숨 표시
    for (int i = 0; i < MAX_LIFE; i++)
    {
        const wchar_t* heartText = (i < m_game.life) ? L"♥" : L"♡";
        SetTextColor(hDC, (i < m_game.life) ? RGB(255, 80, 110) : RGB(120, 120, 120));
        TextOutW(hDC, 40 + (i * 34), 40, heartText, lstrlenW(heartText));
    }

    // 시간 표시
    wchar_t timerText[64];
    wsprintfW(timerText, L"남은 시간: %02d초", static_cast<int>(m_game.remainingTime + 0.99f));

    SIZE timerTextSize = { 0, 0 };
    GetTextExtentPoint32W(hDC, timerText, lstrlenW(timerText), &timerTextSize);
    SetTextColor(hDC, RGB(255, 255, 255));
    TextOutW(hDC, SCREEN_WIDTH - timerTextSize.cx - 40, 40, timerText, lstrlenW(timerText));

    // 조작 안내
    const wchar_t* guideText = L"← → 이동 / ↑ 또는 SPACE 점프 / 빨간 영역 회피";
    SetTextColor(hDC, RGB(255, 255, 255));
    TextOutW(hDC, 40, 90, guideText, lstrlenW(guideText));
}

void avoidgame::ResetPatterns()
{
    // 패턴 상태 초기화
    m_attacks.clear();
    m_foods.clear();
    m_blackholeTexts.clear();

    m_pattern.currentPattern = PATTERN_NONE;
    m_pattern.nextPattern = 0;
    m_pattern.waitTimer = 0.8f;

    m_kakao.comboTimer = 0.0f;
    m_kakao.comboCount = 0;
    m_kakao.moleWarningRect = RECT{ 0, 0, 0, 0 };
    m_kakao.moleWarningTimer = 0.0f;
    m_kakao.moleX = static_cast<float>((SCREEN_WIDTH - KAKAO_MOLE_WIDTH) / 2);
    m_kakao.moleY = static_cast<float>(SCREEN_HEIGHT);
    m_kakao.isMoleWarning = false;
    m_kakao.isMoleActive = false;
    m_kakao.isMoleGoingUp = false;

    m_foodPattern.spawnTimer = 0.0f;
    m_foodPattern.patternTimer = 0.0f;

    m_blackhole = {};
}

void avoidgame::UpdatePatternScheduler()
{
    // 패턴 대기
    if (m_pattern.currentPattern == PATTERN_NONE)
    {
        m_pattern.waitTimer -= UPDATE_DELTA_SECONDS;
        if (m_pattern.waitTimer <= 0.0f)
        {
            int pattern = m_pattern.nextPattern;
            m_pattern.nextPattern = (m_pattern.nextPattern + 1) % 4;

            if (pattern == 0)
            {
                StartKakaoCombo();
            }
            else if (pattern == 1)
            {
                StartReelsPattern();
            }
            else if (pattern == 2)
            {
                StartFoodPattern();
            }
            else
            {
                StartBlackholePattern();
            }
        }

        return;
    }

    // 진행 중 패턴
    if (m_pattern.currentPattern == PATTERN_KAKAO)
    {
        UpdateKakaoCombo();
    }
    else if (m_pattern.currentPattern == PATTERN_FOOD)
    {
        UpdateFoodPattern();
    }
    else if (m_pattern.currentPattern == PATTERN_BLACKHOLE)
    {
        UpdateBlackholePattern();
    }
}

void avoidgame::FinishPatternIfEmpty()
{
    // 종료 조건 확인
    if (m_pattern.currentPattern == PATTERN_NONE)
    {
        return;
    }

    if (m_pattern.currentPattern == PATTERN_KAKAO)
    {
        if (m_kakao.comboCount <= 0 && m_attacks.empty() && !m_kakao.isMoleWarning && !m_kakao.isMoleActive)
        {
            m_pattern.currentPattern = PATTERN_NONE;
            m_pattern.waitTimer = PATTERN_DELAY;
        }
    }
    else if (m_pattern.currentPattern == PATTERN_REELS)
    {
        if (m_attacks.empty())
        {
            m_pattern.currentPattern = PATTERN_NONE;
            m_pattern.waitTimer = PATTERN_DELAY;
        }
    }
    else if (m_pattern.currentPattern == PATTERN_FOOD)
    {
        if (m_foodPattern.patternTimer <= 0.0f && m_foods.empty())
        {
            m_pattern.currentPattern = PATTERN_NONE;
            m_pattern.waitTimer = PATTERN_DELAY;
        }
    }
    else if (m_pattern.currentPattern == PATTERN_BLACKHOLE)
    {
        if (!m_blackhole.isWarning && !m_blackhole.isActive && m_blackholeTexts.empty())
        {
            m_pattern.currentPattern = PATTERN_NONE;
            m_pattern.waitTimer = PATTERN_DELAY;
        }
    }
}

void avoidgame::StartKakaoCombo()
{
    // 카톡 패턴 시작
    m_pattern.currentPattern = PATTERN_KAKAO;
    m_kakao.comboCount = KAKAO_COMBO_COUNT;
    m_kakao.comboTimer = 0.0f;
    StartKakaoMole();
}

void avoidgame::UpdateKakaoCombo()
{
    // 카톡 연타 생성
    if (m_kakao.comboCount <= 0)
    {
        UpdateKakaoMole();
        return;
    }

    m_kakao.comboTimer -= UPDATE_DELTA_SECONDS;
    if (m_kakao.comboTimer <= 0.0f)
    {
        SpawnKakaoAttack();
        m_kakao.comboCount--;
        m_kakao.comboTimer = KAKAO_COMBO_INTERVAL;
    }

    UpdateKakaoMole();
}

void avoidgame::SpawnKakaoAttack()
{
    // 플레이어 위치 조준
    int attackWidth = 300;
    int attackHeight = 90;

    int left = static_cast<int>(m_player.x) + (PLAYER_WIDTH / 2) - (attackWidth / 2);
    int top = static_cast<int>(m_player.y) + (PLAYER_HEIGHT / 2) - (attackHeight / 2);

    if (left < 0)
    {
        left = 0;
    }
    if (left > SCREEN_WIDTH - attackWidth)
    {
        left = SCREEN_WIDTH - attackWidth;
    }
    if (top < 120)
    {
        top = 120;
    }
    if (top > SCREEN_HEIGHT - FLOOR_HEIGHT - attackHeight)
    {
        top = SCREEN_HEIGHT - FLOOR_HEIGHT - attackHeight;
    }

    AttackWarning attack = {};
    attack.rect = RECT{ left, top, left + attackWidth, top + attackHeight };
    attack.warningTime = WARNING_DURATION;
    attack.attackTime = ATTACK_DURATION;
    attack.isAttackActive = false;
    attack.messageIndex = KAKAO_COMBO_COUNT - m_kakao.comboCount;
    attack.attackType = ATTACK_TYPE_KAKAO;

    m_attacks.push_back(attack);
}

void avoidgame::StartKakaoMole()
{
    // 하단 절반 경고
    m_kakao.moleWarningRect = RECT
    {
        0,
        SCREEN_HEIGHT / 2,
        SCREEN_WIDTH,
        SCREEN_HEIGHT - FLOOR_HEIGHT
    };
    m_kakao.moleWarningTimer = KAKAO_MOLE_WARNING_DURATION;
    m_kakao.moleX = static_cast<float>(rand() % (SCREEN_WIDTH - KAKAO_MOLE_WIDTH));
    m_kakao.moleY = static_cast<float>(SCREEN_HEIGHT);
    m_kakao.isMoleWarning = true;
    m_kakao.isMoleActive = false;
    m_kakao.isMoleGoingUp = true;
}

void avoidgame::UpdateKakaoMole()
{
    // 경고 후 상승
    if (m_kakao.isMoleWarning)
    {
        m_kakao.moleWarningTimer -= UPDATE_DELTA_SECONDS;
        if (m_kakao.moleWarningTimer <= 0.0f)
        {
            m_kakao.moleWarningTimer = 0.0f;
            m_kakao.isMoleWarning = false;
            m_kakao.isMoleActive = true;
            m_kakao.isMoleGoingUp = true;
        }

        return;
    }

    if (!m_kakao.isMoleActive)
    {
        return;
    }

    const float targetY = static_cast<float>(SCREEN_HEIGHT - FLOOR_HEIGHT - KAKAO_MOLE_HEIGHT);
    const float hiddenY = static_cast<float>(SCREEN_HEIGHT);

    if (m_kakao.isMoleGoingUp)
    {
        m_kakao.moleY -= KAKAO_MOLE_SPEED;
        if (m_kakao.moleY <= targetY)
        {
            m_kakao.moleY = targetY;
            m_kakao.isMoleGoingUp = false;
        }
    }
    else
    {
        m_kakao.moleY += KAKAO_MOLE_SPEED;
        if (m_kakao.moleY >= hiddenY)
        {
            m_kakao.moleY = hiddenY;
            m_kakao.isMoleActive = false;
        }
    }

    if (IsRectOverlap(GetPlayerRect(), GetKakaoMoleRect()))
    {
        DamagePlayer();
    }
}

void avoidgame::DrawKakaoMole(HDC hDC)
{
    // 두더지 경고
    if (m_kakao.isMoleWarning)
    {
        HBRUSH warningBrush = CreateSolidBrush(RGB(80, 0, 0));
        HPEN warningPen = CreatePen(PS_SOLID, 5, RGB(255, 60, 60));

        HGDIOBJ oldBrush = SelectObject(hDC, warningBrush);
        HGDIOBJ oldPen = SelectObject(hDC, warningPen);

        Rectangle(hDC, m_kakao.moleWarningRect.left, m_kakao.moleWarningRect.top, m_kakao.moleWarningRect.right, m_kakao.moleWarningRect.bottom);

        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, RGB(255, 150, 150));
        const wchar_t* warningText = L"카톡이 아래에서 올라옵니다";
        TextOutW(hDC, 720, (SCREEN_HEIGHT / 2) + 40, warningText, lstrlenW(warningText));

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);
        DeleteObject(warningBrush);
        DeleteObject(warningPen);
        return;
    }

    if (!m_kakao.isMoleActive)
    {
        return;
    }

    // 두더지 카톡 판넬
    RECT moleRect = GetKakaoMoleRect();
    HBRUSH moleBrush = CreateSolidBrush(RGB(255, 245, 120));
    HPEN molePen = CreatePen(PS_SOLID, 5, RGB(255, 80, 80));

    HGDIOBJ oldBrush = SelectObject(hDC, moleBrush);
    HGDIOBJ oldPen = SelectObject(hDC, molePen);

    RoundRect(hDC, moleRect.left, moleRect.top, moleRect.right, moleRect.bottom, 28, 28);

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(30, 30, 30));
    const wchar_t* titleText = L"KakaoTalk";
    const wchar_t* bodyText = L"답장 안 해?";
    TextOutW(hDC, moleRect.left + 36, moleRect.top + 40, titleText, lstrlenW(titleText));
    TextOutW(hDC, moleRect.left + 36, moleRect.top + 120, bodyText, lstrlenW(bodyText));

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(moleBrush);
    DeleteObject(molePen);
}

RECT avoidgame::GetKakaoMoleRect() const
{
    // 두더지 충돌 영역
    const int left = static_cast<int>(m_kakao.moleX);
    RECT rc =
    {
        left,
        static_cast<LONG>(m_kakao.moleY),
        left + KAKAO_MOLE_WIDTH,
        static_cast<LONG>(m_kakao.moleY + KAKAO_MOLE_HEIGHT)
    };

    return rc;
}

void avoidgame::StartReelsPattern()
{
    // 릴스 패턴 시작
    m_pattern.currentPattern = PATTERN_REELS;
    SpawnReelsAttack();
}

void avoidgame::SpawnReelsAttack()
{
    // 화면 절반 공격
    bool isLeftSide = (rand() % 2) == 0;

    AttackWarning attack = {};
    attack.rect = RECT
    {
        isLeftSide ? 0 : SCREEN_WIDTH / 2,
        0,
        isLeftSide ? SCREEN_WIDTH / 2 : SCREEN_WIDTH,
        SCREEN_HEIGHT - FLOOR_HEIGHT
    };
    attack.warningTime = REELS_WARNING_DURATION;
    attack.attackTime = REELS_ATTACK_DURATION;
    attack.isAttackActive = false;
    attack.messageIndex = 0;
    attack.attackType = ATTACK_TYPE_REELS;

    m_attacks.push_back(attack);
}

void avoidgame::StartFoodPattern()
{
    // 음식 패턴 시작
    m_pattern.currentPattern = PATTERN_FOOD;
    m_foodPattern.patternTimer = FOOD_PATTERN_DURATION;
    m_foodPattern.spawnTimer = 0.0f;
}

void avoidgame::UpdateFoodPattern()
{
    // 음식 생성 시간
    m_foodPattern.patternTimer -= UPDATE_DELTA_SECONDS;

    if (m_foodPattern.patternTimer > 0.0f)
    {
        m_foodPattern.spawnTimer -= UPDATE_DELTA_SECONDS;
        if (m_foodPattern.spawnTimer <= 0.0f)
        {
            for (int i = 0; i < 4; i++)
            {
                SpawnFoodDrop();
            }
            m_foodPattern.spawnTimer = FOOD_SPAWN_INTERVAL;
        }
    }
}

void avoidgame::SpawnFoodDrop()
{
    // 음식 생성
    FoodDrop food = {};
    food.baseX = static_cast<float>(rand() % (SCREEN_WIDTH - 80));
    food.x = food.baseX;
    food.y = -40.0f;
    food.wave = static_cast<float>(rand() % 628) / 100.0f;
    food.speed = 5.5f + static_cast<float>(rand() % 4);
    food.foodType = rand() % 4;

    m_foods.push_back(food);
}

void avoidgame::UpdateFoodDrops()
{
    // 음식 낙하
    RECT playerRect = GetPlayerRect();

    for (int i = 0; i < static_cast<int>(m_foods.size());)
    {
        FoodDrop& food = m_foods[i];

        food.y += food.speed;
        food.wave += 0.18f;
        food.x = food.baseX + static_cast<float>(20.0 * std::sin(food.wave));

        if (IsRectOverlap(playerRect, GetFoodRect(food)))
        {
            DamagePlayer();
            m_foods.erase(m_foods.begin() + i);
            continue;
        }

        if (food.y > SCREEN_HEIGHT)
        {
            m_foods.erase(m_foods.begin() + i);
            continue;
        }

        i++;
    }
}

void avoidgame::DrawFoodDrops(HDC hDC)
{
    // 음식 출력
    const wchar_t* foodNames[4] =
    {
        L"라면",
        L"콜라",
        L"과자",
        L"햄버거"
    };

    for (int i = 0; i < static_cast<int>(m_foods.size()); i++)
    {
        const FoodDrop& food = m_foods[i];
        RECT foodRect = GetFoodRect(food);

        HBRUSH foodBrush = CreateSolidBrush(RGB(255, 210, 90));
        HPEN foodPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));

        HGDIOBJ oldBrush = SelectObject(hDC, foodBrush);
        HGDIOBJ oldPen = SelectObject(hDC, foodPen);

        Ellipse(hDC, foodRect.left, foodRect.top, foodRect.right, foodRect.bottom);

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);
        DeleteObject(foodBrush);
        DeleteObject(foodPen);

        int foodType = food.foodType;
        if (foodType < 0 || foodType >= 4)
        {
            foodType = 0;
        }

        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, RGB(255, 255, 255));
        TextOutW(hDC, foodRect.left - 4, foodRect.bottom + 4, foodNames[foodType], lstrlenW(foodNames[foodType]));
    }
}

RECT avoidgame::GetFoodRect(const FoodDrop& food) const
{
    // 음식 충돌 영역
    RECT rc =
    {
        static_cast<LONG>(food.x),
        static_cast<LONG>(food.y),
        static_cast<LONG>(food.x + 28),
        static_cast<LONG>(food.y + 28)
    };

    return rc;
}

void avoidgame::StartBlackholePattern()
{
    // 왼쪽 경고 시작
    m_pattern.currentPattern = PATTERN_BLACKHOLE;

    m_blackholeTexts.clear();

    const int coreCenterX = SCREEN_WIDTH - BLACKHOLE_RIGHT_MARGIN;
    const int centerY = SCREEN_HEIGHT / 2;

    m_blackhole.warningRect = RECT
    {
        0,
        0,
        SCREEN_WIDTH / 2,
        SCREEN_HEIGHT - FLOOR_HEIGHT
    };

    m_blackhole.coreRect = RECT
    {
        coreCenterX - (BLACKHOLE_SIZE / 2),
        centerY - (BLACKHOLE_SIZE / 2),
        coreCenterX + (BLACKHOLE_SIZE / 2),
        centerY + (BLACKHOLE_SIZE / 2)
    };

    m_blackhole.warningTimer = BLACKHOLE_WARNING_DURATION;
    m_blackhole.activeTimer = BLACKHOLE_ACTIVE_DURATION;
    m_blackhole.spawnTimer = 0.0f;
    m_blackhole.spawnedCount = 0;
    m_blackhole.isWarning = true;
    m_blackhole.isActive = false;
}

void avoidgame::UpdateBlackholePattern()
{
    // 경고 후 활성
    if (m_blackhole.isWarning)
    {
        m_blackhole.warningTimer -= UPDATE_DELTA_SECONDS;
        if (m_blackhole.warningTimer <= 0.0f)
        {
            m_blackhole.warningTimer = 0.0f;
            m_blackhole.isWarning = false;
            m_blackhole.isActive = true;
        }

        return;
    }

    if (!m_blackhole.isActive)
    {
        return;
    }

    m_blackhole.activeTimer -= UPDATE_DELTA_SECONDS;
    ApplyBlackholePull();
    UpdateBlackholeTexts();

    if (IsRectOverlap(GetPlayerRect(), m_blackhole.coreRect))
    {
        DamagePlayer();
    }

    if (m_blackhole.activeTimer <= 0.0f)
    {
        m_blackhole.activeTimer = 0.0f;
        m_blackhole.isActive = false;
        m_blackholeTexts.clear();
    }
}

void avoidgame::ApplyBlackholePull()
{
    // 블랙홀 방향 계산
    const float blackholeX = static_cast<float>((m_blackhole.coreRect.left + m_blackhole.coreRect.right) / 2);
    const float blackholeY = static_cast<float>((m_blackhole.coreRect.top + m_blackhole.coreRect.bottom) / 2);
    const float playerCenterX = m_player.x + (PLAYER_WIDTH / 2.0f);
    const float playerCenterY = m_player.y + (PLAYER_HEIGHT / 2.0f);

    const float deltaX = blackholeX - playerCenterX;
    const float deltaY = blackholeY - playerCenterY;
    const float distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));

    if (distance <= 1.0f)
    {
        return;
    }

    const float directionY = deltaY / distance;

    bool isResisting =
        (deltaX < 0.0f && m_player.moveRight) ||
        (deltaX > 0.0f && m_player.moveLeft);

    float horizontalPull = isResisting ? BLACKHOLE_RESIST_PULL_FORCE : BLACKHOLE_PULL_FORCE;

    if (std::fabs(deltaX) > 1.0f)
    {
        m_player.x += (deltaX > 0.0f ? horizontalPull : -horizontalPull);
    }
    m_player.y += directionY * BLACKHOLE_VERTICAL_PULL_FORCE;

    if (directionY < -0.1f)
    {
        m_player.isGrounded = false;
    }

    ClampPlayerPosition();

    // 바닥 아래 방지
    const float floorTop = static_cast<float>(SCREEN_HEIGHT - FLOOR_HEIGHT);
    if (m_player.y + PLAYER_HEIGHT > floorTop)
    {
        m_player.y = floorTop - PLAYER_HEIGHT;
        m_player.velocityY = 0.0f;
        m_player.isGrounded = true;
        m_player.jumpCount = 0;
    }
}

void avoidgame::SpawnBlackholeText()
{
    // 무작위 Y 생성
    int yRange = BLACKHOLE_TEXT_MAX_Y - BLACKHOLE_TEXT_MIN_Y;
    if (yRange < 1)
    {
        yRange = 1;
    }

    BlackholeText text = {};
    text.x = static_cast<float>(-BLACKHOLE_TEXT_WIDTH);
    text.y = static_cast<float>(BLACKHOLE_TEXT_MIN_Y + (rand() % yRange));
    text.messageIndex = m_blackhole.spawnedCount % 3;

    m_blackholeTexts.push_back(text);
    m_blackhole.spawnedCount++;
}

void avoidgame::UpdateBlackholeTexts()
{
    // 텍스트 생성
    if (m_blackhole.spawnedCount < BLACKHOLE_TEXT_COUNT)
    {
        m_blackhole.spawnTimer -= UPDATE_DELTA_SECONDS;
        if (m_blackhole.spawnTimer <= 0.0f)
        {
            SpawnBlackholeText();
            m_blackhole.spawnTimer = BLACKHOLE_TEXT_SPAWN_INTERVAL;
        }
    }

    RECT playerRect = GetPlayerRect();

    // 텍스트 이동
    for (int i = 0; i < static_cast<int>(m_blackholeTexts.size());)
    {
        BlackholeText& text = m_blackholeTexts[i];
        text.x += BLACKHOLE_TEXT_SPEED;

        if (IsRectOverlap(playerRect, GetBlackholeTextRect(text)))
        {
            DamagePlayer();
            m_blackholeTexts.erase(m_blackholeTexts.begin() + i);
            continue;
        }

        if (text.x > m_blackhole.coreRect.left)
        {
            m_blackholeTexts.erase(m_blackholeTexts.begin() + i);
            continue;
        }

        i++;
    }
}

void avoidgame::DrawBlackhole(HDC hDC)
{
    // 블랙홀 패턴 출력
    if (!m_blackhole.isWarning && !m_blackhole.isActive && m_blackholeTexts.empty())
    {
        return;
    }

    SetBkMode(hDC, TRANSPARENT);

    if (m_blackhole.isWarning)
    {
        HBRUSH warningBrush = CreateSolidBrush(RGB(80, 0, 0));
        HPEN warningPen = CreatePen(PS_SOLID, 5, RGB(255, 40, 40));

        HGDIOBJ oldBrush = SelectObject(hDC, warningBrush);
        HGDIOBJ oldPen = SelectObject(hDC, warningPen);

        Rectangle(hDC, m_blackhole.warningRect.left, m_blackhole.warningRect.top, m_blackhole.warningRect.right, m_blackhole.warningRect.bottom);

        SetTextColor(hDC, RGB(255, 120, 120));
        const wchar_t* warningText = L"왼쪽 릴스 접근";
        TextOutW(hDC, m_blackhole.warningRect.left + 110, m_blackhole.warningRect.top + 40, warningText, lstrlenW(warningText));

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);
        DeleteObject(warningBrush);
        DeleteObject(warningPen);
        return;
    }

    // 텍스트 탄막 출력
    const wchar_t* messages[3] =
    {
        L"10초만 볼까?",
        L"다음 영상",
        L"알고리즘"
    };

    for (int i = 0; i < static_cast<int>(m_blackholeTexts.size()); i++)
    {
        const BlackholeText& text = m_blackholeTexts[i];
        RECT textRect = GetBlackholeTextRect(text);

        HBRUSH textBrush = CreateSolidBrush(RGB(250, 250, 250));
        HPEN textPen = CreatePen(PS_SOLID, 3, RGB(255, 80, 120));

        HGDIOBJ textOldBrush = SelectObject(hDC, textBrush);
        HGDIOBJ textOldPen = SelectObject(hDC, textPen);

        RoundRect(hDC, textRect.left, textRect.top, textRect.right, textRect.bottom, 22, 22);

        int messageIndex = text.messageIndex;
        if (messageIndex < 0 || messageIndex >= 3)
        {
            messageIndex = 0;
        }

        SetTextColor(hDC, RGB(30, 30, 30));
        TextOutW(hDC, textRect.left + 34, textRect.top + 48, messages[messageIndex], lstrlenW(messages[messageIndex]));

        SelectObject(hDC, textOldBrush);
        SelectObject(hDC, textOldPen);
        DeleteObject(textBrush);
        DeleteObject(textPen);
    }

    HBRUSH blackholeBrush = CreateSolidBrush(RGB(8, 8, 16));
    HPEN blackholePen = CreatePen(PS_SOLID, 5, RGB(255, 80, 120));

    HGDIOBJ oldBrush = SelectObject(hDC, blackholeBrush);
    HGDIOBJ oldPen = SelectObject(hDC, blackholePen);

    Rectangle(hDC, m_blackhole.coreRect.left, m_blackhole.coreRect.top, m_blackhole.coreRect.right, m_blackhole.coreRect.bottom);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(blackholeBrush);
    DeleteObject(blackholePen);

    SetTextColor(hDC, RGB(255, 255, 255));
    const wchar_t* coreText = L"릴스";
    TextOutW(hDC, m_blackhole.coreRect.left + 44, m_blackhole.coreRect.top + 54, coreText, lstrlenW(coreText));
}

RECT avoidgame::GetBlackholeTextRect(const BlackholeText& text) const
{
    // 텍스트 충돌 영역
    RECT rc =
    {
        static_cast<LONG>(text.x),
        static_cast<LONG>(text.y),
        static_cast<LONG>(text.x + BLACKHOLE_TEXT_WIDTH),
        static_cast<LONG>(text.y + BLACKHOLE_TEXT_HEIGHT)
    };

    return rc;
}

void avoidgame::UpdateAttacks()
{
    // 장판 상태 갱신
    RECT playerRect = GetPlayerRect();

    for (int i = 0; i < static_cast<int>(m_attacks.size());)
    {
        AttackWarning& attack = m_attacks[i];

        if (!attack.isAttackActive)
        {
            attack.warningTime -= UPDATE_DELTA_SECONDS;
            if (attack.warningTime <= 0.0f)
            {
                attack.warningTime = 0.0f;
                attack.isAttackActive = true;
            }
        }
        else
        {
            attack.attackTime -= UPDATE_DELTA_SECONDS;

            if (IsRectOverlap(playerRect, attack.rect))
            {
                DamagePlayer();
            }

            if (attack.attackTime <= 0.0f)
            {
                m_attacks.erase(m_attacks.begin() + i);
                continue;
            }
        }

        i++;
    }
}

void avoidgame::DrawAttacks(HDC hDC)
{
    // 장판 출력
    const wchar_t* kakaoMessages[5] =
    {
        L"뭐해?",
        L"답장 안 해?",
        L"자는 척?",
        L"나 삐짐",
        L"진짜?"
    };
    const wchar_t* reelsMessages[3] =
    {
        L"이 영상만 보고",
        L"다음 영상",
        L"알고리즘 추천"
    };

    // 경고 먼저 출력
    for (int i = 0; i < static_cast<int>(m_attacks.size()); i++)
    {
        const AttackWarning& attack = m_attacks[i];
        if (attack.isAttackActive)
        {
            continue;
        }

        HBRUSH warningBrush = CreateSolidBrush(RGB(80, 0, 0));
        HPEN warningPen = CreatePen(PS_SOLID, 4, RGB(255, 60, 60));

        HGDIOBJ oldBrush = SelectObject(hDC, warningBrush);
        HGDIOBJ oldPen = SelectObject(hDC, warningPen);

        Rectangle(hDC, attack.rect.left, attack.rect.top, attack.rect.right, attack.rect.bottom);

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);
        DeleteObject(warningBrush);
        DeleteObject(warningPen);
    }

    // 활성 장판은 경고 위에 출력
    for (int i = 0; i < static_cast<int>(m_attacks.size()); i++)
    {
        const AttackWarning& attack = m_attacks[i];

        if (!attack.isAttackActive)
        {
            continue;
        }

        if (attack.attackType == ATTACK_TYPE_REELS)
        {
            // 릴스 스크롤 출력
            HBRUSH reelsBrush = CreateSolidBrush(RGB(24, 24, 32));
            HPEN reelsPen = CreatePen(PS_SOLID, 4, RGB(255, 60, 120));

            HGDIOBJ oldBrush = SelectObject(hDC, reelsBrush);
            HGDIOBJ oldPen = SelectObject(hDC, reelsPen);

            Rectangle(hDC, attack.rect.left, attack.rect.top, attack.rect.right, attack.rect.bottom);

            float elapsed = REELS_ATTACK_DURATION - attack.attackTime;
            int scrollY = static_cast<int>(elapsed * 260.0f);
            int cardWidth = 300;
            int cardHeight = 460;
            int cardGap = 70;
            int centerX = (attack.rect.left + attack.rect.right) / 2;
            int startY = attack.rect.bottom - (scrollY % (cardHeight + cardGap));

            for (int j = 0; j < 4; j++)
            {
                int cardTop = startY - j * (cardHeight + cardGap);
                RECT cardRect =
                {
                    centerX - cardWidth / 2,
                    cardTop,
                    centerX + cardWidth / 2,
                    cardTop + cardHeight
                };

                HBRUSH cardBrush = CreateSolidBrush(RGB(245, 245, 245));
                HPEN cardPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));

                HGDIOBJ cardOldBrush = SelectObject(hDC, cardBrush);
                HGDIOBJ cardOldPen = SelectObject(hDC, cardPen);

                RoundRect(hDC, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom, 24, 24);

                SetBkMode(hDC, TRANSPARENT);
                SetTextColor(hDC, RGB(20, 20, 20));
                TextOutW(hDC, cardRect.left + 35, cardRect.top + 210, reelsMessages[j % 3], lstrlenW(reelsMessages[j % 3]));

                SelectObject(hDC, cardOldBrush);
                SelectObject(hDC, cardOldPen);
                DeleteObject(cardBrush);
                DeleteObject(cardPen);
            }

            SelectObject(hDC, oldBrush);
            SelectObject(hDC, oldPen);
            DeleteObject(reelsBrush);
            DeleteObject(reelsPen);
            continue;
        }

        // 카톡 알림 출력
        HBRUSH attackBrush = CreateSolidBrush(RGB(255, 245, 120));
        HPEN attackPen = CreatePen(PS_SOLID, 4, RGB(255, 80, 80));

        HGDIOBJ oldBrush = SelectObject(hDC, attackBrush);
        HGDIOBJ oldPen = SelectObject(hDC, attackPen);

        RoundRect(hDC, attack.rect.left, attack.rect.top, attack.rect.right, attack.rect.bottom, 18, 18);

        RECT iconRect =
        {
            attack.rect.left + 16,
            attack.rect.top + 18,
            attack.rect.left + 66,
            attack.rect.top + 68
        };

        HBRUSH iconBrush = CreateSolidBrush(RGB(255, 220, 0));
        HPEN iconPen = CreatePen(PS_SOLID, 2, RGB(40, 40, 40));

        HGDIOBJ iconOldBrush = SelectObject(hDC, iconBrush);
        HGDIOBJ iconOldPen = SelectObject(hDC, iconPen);

        Ellipse(hDC, iconRect.left, iconRect.top, iconRect.right, iconRect.bottom);

        SelectObject(hDC, iconOldBrush);
        SelectObject(hDC, iconOldPen);
        DeleteObject(iconBrush);
        DeleteObject(iconPen);

        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, RGB(30, 30, 30));
        TextOutW(hDC, iconRect.left + 18, iconRect.top + 13, L"K", 1);

        int messageIndex = attack.messageIndex;
        if (messageIndex < 0 || messageIndex >= 5)
        {
            messageIndex = 0;
        }

        TextOutW(hDC, attack.rect.left + 84, attack.rect.top + 30, kakaoMessages[messageIndex], lstrlenW(kakaoMessages[messageIndex]));

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);
        DeleteObject(attackBrush);
        DeleteObject(attackPen);
    }
}

bool avoidgame::IsRectOverlap(const RECT& a, const RECT& b) const
{
    // 사각형 충돌
    return a.left < b.right &&
        a.right > b.left &&
        a.top < b.bottom &&
        a.bottom > b.top;
}

void avoidgame::DamagePlayer()
{
    // 무적 중 피격 무시
    if (m_game.invincibleTimer > 0.0f)
    {
        return;
    }

    m_game.life--;
    if (m_game.life < 0)
    {
        m_game.life = 0;
    }

    m_game.invincibleTimer = INVINCIBLE_DURATION;
}

void avoidgame::UpdateGameTimer()
{
    // 생존 시간 감소
    m_game.remainingTime -= UPDATE_DELTA_SECONDS;
    if (m_game.remainingTime <= 0.0f)
    {
        m_game.remainingTime = 0.0f;
        m_game.isFinished = true;
        m_game.isSuccess = true;
    }

    // 목숨 소진
    if (m_game.life <= 0)
    {
        m_game.life = 0;
        m_game.isFinished = true;
        m_game.isSuccess = false;
    }
}

int avoidgame::CalculateScore() const
{
    // 남은 하트 점수
    return m_game.life * 100 / MAX_LIFE;
}
