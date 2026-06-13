#include "schoolrun.h"

#include "../AudioManager.h"

#include <algorithm>

#pragma comment(lib, "msimg32.lib")

extern AudioManager audioManager;

namespace
{
    const bool SHOW_HIT_BOX = false;

    const int SCREEN_WIDTH = 1920;
    const int SCREEN_HEIGHT = 1080;
    const int GROUND_TOP = 950;

    const int PLAYER_WIDTH = 130;
    const int PLAYER_STAND_HEIGHT = 158;
    const int PLAYER_SLIDE_HEIGHT = 83;

    const int GROUND_OBSTACLE_WIDTH = 120;
    const int GROUND_OBSTACLE_HEIGHT = 113;

    const int AIR_OBSTACLE_WIDTH = PLAYER_WIDTH;
    const int AIR_OBSTACLE_HEIGHT = 75;

    const int PAPER_OBSTACLE_WIDTH = 100;
    const int PAPER_OBSTACLE_HEIGHT = 100;

    const int BIRD_OBSTACLE_WIDTH = 150;
    const int BIRD_OBSTACLE_HEIGHT = 50;

    const int BOX_OBSTACLE_WIDTH = 160;
    const int BOX_OBSTACLE_HEIGHT = 320;

    const int CONSTRUCTION_OBSTACLE_WIDTH = 120;
    const int CONSTRUCTION_OBSTACLE_HEIGHT = 120;

    const int WATER_OBSTACLE_WIDTH = 160;
    const int WATER_OBSTACLE_HEIGHT = 53;

    const int SIGNBOARD_OBSTACLE_WIDTH = 200;
    const int SIGNBOARD_OBSTACLE_HEIGHT = SCREEN_HEIGHT - PLAYER_WIDTH*2+20;

    const int CLOUD_COUNT = 3;
    const int CLOUD_WIDTHS[CLOUD_COUNT] = { 330, 250, 190 };
    const int CLOUD_HEIGHTS[CLOUD_COUNT] = { 141, 107, 81 };
    const float CLOUD_Y_POSITIONS[CLOUD_COUNT] = { 85.0f, 190.0f, 135.0f };
    const float CLOUD_START_OFFSETS[CLOUD_COUNT] = { 40.0f, 620.0f, 1080.0f };
    const float CLOUD_RESPAWN_GAP = 340.0f;
    const float CLOUD_SPEED_MULTIPLIER = 1.15f;
    const float BACKGROUND_SPEED_MULTIPLIER = 0.25f;
    const int BACKGROUND_CYCLE_WIDTH = SCREEN_WIDTH * 2;

    const int JELLY_WIDTH = 64;
    const int JELLY_HEIGHT = 64;
    const float JELLY_GAP_X = 78.0f;
    const float JELLY_GROUND_CENTER_Y = static_cast<float>(GROUND_TOP - 50);
    const float JELLY_OBSTACLE_CLEARANCE = 24.0f;
    const float JELLY_MIN_JUMP_ARC_HEIGHT = 120.0f;
    const float JELLY_CONNECTED_OBSTACLE_GAP = 120.0f;
    const float JELLY_SAFE_SIDE_GAP = (JELLY_WIDTH * 0.5f) + JELLY_OBSTACLE_CLEARANCE;
    const float JELLY_JUMP_EXTRA_HEIGHT = 120.0f;
    const int JELLY_JUMP_ARC_COUNT = 5;

    const float FIXED_PLAYER_X = 220.0f;

    // 조절값: 점프와 낙하
    const float GRAVITY_VALUE = 10.0f;
    const float JUMP_POWER_VALUE = -80.0f;
    const float JUMP_BUFFER_DURATION = 0.15f;

    // 조절값: 러닝 속도
    const float START_GAME_SPEED = 30.0f;
    const int DISTANCE_GAIN_PER_FRAME = 2;

    // 조절값: 장애물 생성
    const int OBSTACLE_START_DISTANCE = 20;
    const int OBSTACLE_DISTANCE_STEP = 150;
    const float OBSTACLE_START_X = static_cast<float>(SCREEN_WIDTH);
    const float FIRST_OBSTACLE_SPAWN_TRAVEL_X =
        START_GAME_SPEED * (static_cast<float>(OBSTACLE_START_DISTANCE) / static_cast<float>(DISTANCE_GAIN_PER_FRAME));
    const float PATTERN_SPAWN_TRAVEL_X =
        START_GAME_SPEED * (static_cast<float>(OBSTACLE_DISTANCE_STEP) / static_cast<float>(DISTANCE_GAIN_PER_FRAME));

    // 조절값: 시간과 점수
    const float UPDATE_DELTA_SECONDS = 0.06f;
    const float GAME_TIME_LIMIT = 40.0f;
    const int MAX_LIFE = 3;
    const int HEART_SIZE = 100;
    const int HEART_GAP = 8;
    const float INVINCIBLE_DURATION = 1.0f;
    const BYTE DAMAGE_OVERLAY_ALPHA = 70;

    const float PLAYER_ANIMATION_INTERVAL = 0.12f;
    const int PLAYER_RUNNING_FRAME_COUNT = 7;
    const int PLAYER_SHEET_FRAME_COUNT = 8;
    const int PLAYER_JUMP_FRAME_INDEX = 7;
}

schoolrun::schoolrun()
{
    // 기본값
    m_playerX = FIXED_PLAYER_X;
    m_playerY = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT);
    m_velocityY = 0.0f;
    m_gravity = GRAVITY_VALUE;
    m_jumpPower = JUMP_POWER_VALUE;

    m_gameSpeed = START_GAME_SPEED;
    m_backgroundOffset = 0.0f;
    m_scrollOffset = 0.0f;
    m_distanceScore = 0;
    m_nextObstacleDistance = OBSTACLE_START_DISTANCE;
    m_nextObstaclePatternIndex = 0;
    m_remainingTime = GAME_TIME_LIMIT;

    m_score = 0;
    m_life = MAX_LIFE;
    m_invincibleTime = 0.0f;
    m_isFinished = false;
    m_isGrounded = true;
    m_isSliding = false;
    m_isDownKeyPressed = false;
    m_jumpCount = 0;
    m_jumpBufferTime = 0.0f;
    m_playerFrameIndex = 0;
    m_playerAnimationTime = 0.0f;
    m_jellyCount = 0;
    m_totalJellyCount = 0;

}

schoolrun::~schoolrun()
{
    Release();
}

void schoolrun::Initialize()
{
    LoadImages();

    // 시작 상태
    Reset();
}

void schoolrun::Release()
{
    DestroyImages();
    m_obstacles.clear();
    m_obstaclePatterns.clear();
    m_jellies.clear();
}

void schoolrun::LoadImages()
{
    if (m_playerRunImage.IsNull())
    {
        m_playerRunImage.Load(L"resource\\minigame3\\run_jump.png");
    }
    if (m_playerSlideImage.IsNull())
    {
        m_playerSlideImage.Load(L"resource\\minigame3\\slide.png");
    }
    if (m_backgroundImage.IsNull())
    {
        m_backgroundImage.Load(L"resource\\minigame3\\school_background.png");
    }
    if (m_backgroundImage2.IsNull())
    {
        m_backgroundImage2.Load(L"resource\\minigame3\\school_background2.png");
    }
    if (m_heartImage.IsNull())
    {
        m_heartImage.Load(L"resource\\minigame3\\heart.png");
    }
    if (m_groundImage.IsNull())
    {
        m_groundImage.Load(L"resource\\minigame3\\ground.png");
    }
    if (m_groundObstacleImage.IsNull())
    {
        m_groundObstacleImage.Load(L"resource\\minigame3\\construction.png");
    }
    if (m_airObstacleImage.IsNull())
    {
        m_airObstacleImage.Load(L"resource\\minigame3\\bird.png");
    }
    if (m_paperObstacleImage.IsNull())
    {
        m_paperObstacleImage.Load(L"resource\\minigame3\\paper.png");
    }
    if (m_boxObstacleImage.IsNull())
    {
        m_boxObstacleImage.Load(L"resource\\minigame3\\box.png");
    }
    if (m_waterObstacleImage.IsNull())
    {
        m_waterObstacleImage.Load(L"resource\\minigame3\\water.png");
    }
    if (m_signboardObstacleImage.IsNull())
    {
        m_signboardObstacleImage.Load(L"resource\\minigame3\\signboard.png");
    }
    if (m_cloudImage.IsNull())
    {
        m_cloudImage.Load(L"resource\\minigame3\\cloud.png");
    }
}

void schoolrun::DestroyImages()
{
    if (!m_playerRunImage.IsNull())
    {
        m_playerRunImage.Destroy();
    }
    if (!m_playerSlideImage.IsNull())
    {
        m_playerSlideImage.Destroy();
    }
    if (!m_backgroundImage.IsNull())
    {
        m_backgroundImage.Destroy();
    }
    if (!m_backgroundImage2.IsNull())
    {
        m_backgroundImage2.Destroy();
    }
    if (!m_heartImage.IsNull())
    {
        m_heartImage.Destroy();
    }
    if (!m_groundImage.IsNull())
    {
        m_groundImage.Destroy();
    }
    if (!m_groundObstacleImage.IsNull())
    {
        m_groundObstacleImage.Destroy();
    }
    if (!m_airObstacleImage.IsNull())
    {
        m_airObstacleImage.Destroy();
    }
    if (!m_paperObstacleImage.IsNull())
    {
        m_paperObstacleImage.Destroy();
    }
    if (!m_boxObstacleImage.IsNull())
    {
        m_boxObstacleImage.Destroy();
    }
    if (!m_waterObstacleImage.IsNull())
    {
        m_waterObstacleImage.Destroy();
    }
    if (!m_signboardObstacleImage.IsNull())
    {
        m_signboardObstacleImage.Destroy();
    }
    if (!m_cloudImage.IsNull())
    {
        m_cloudImage.Destroy();
    }
}

void schoolrun::Reset()
{
    // 플레이어 위치
    m_playerX = FIXED_PLAYER_X;
    m_playerY = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT);
    m_velocityY = 0.0f;

    // 러닝 상태
    m_gameSpeed = START_GAME_SPEED;
    m_backgroundOffset = 0.0f;
    m_scrollOffset = 0.0f;
    m_distanceScore = 0;
    m_nextObstacleDistance = OBSTACLE_START_DISTANCE;
    m_nextObstaclePatternIndex = 0;
    m_remainingTime = GAME_TIME_LIMIT;

    // 결과 상태
    m_score = 0;
    m_life = MAX_LIFE;
    m_invincibleTime = 0.0f;
    m_isFinished = false;
    m_isGrounded = true;
    m_isSliding = false;
    m_isDownKeyPressed = false;
    m_jumpCount = 0;
    m_jumpBufferTime = 0.0f;
    m_playerFrameIndex = 0;
    m_playerAnimationTime = 0.0f;
    m_jellyCount = 0;
    ResetClouds();
    m_obstacles.clear();
    m_jellies.clear();
    CreateObstaclePatterns();
    m_totalJellyCount =
        static_cast<int>(BuildInitialJellyRoute().size()) +
        CalculateTotalJellyCount();
    SpawnInitialJellies();
}

void schoolrun::Update()
{
    // 종료 후 정지
    if (m_isFinished)
    {
        return;
    }

    // 중력 적용
    m_velocityY += m_gravity;
    m_playerY += m_velocityY;

    // 시간 갱신
    m_remainingTime -= UPDATE_DELTA_SECONDS;
    if (m_invincibleTime > 0.0f)
    {
        m_invincibleTime -= UPDATE_DELTA_SECONDS;
        if (m_invincibleTime < 0.0f)
        {
            m_invincibleTime = 0.0f;
        }
    }
    if (m_remainingTime <= 0.0f)
    {
        m_remainingTime = 0.0f;
        m_isFinished = true;
    }

    // 바닥 판정
    const int currentHeight = m_isSliding ? PLAYER_SLIDE_HEIGHT : PLAYER_STAND_HEIGHT;
    const float groundY = static_cast<float>(GROUND_TOP - currentHeight);

    if (m_playerY >= groundY)
    {
        m_playerY = groundY;
        m_velocityY = 0.0f;
        m_isGrounded = true;
        m_jumpCount = 0;

        if (m_isDownKeyPressed)
        {
            m_isSliding = true;
            m_playerY = static_cast<float>(GROUND_TOP - PLAYER_SLIDE_HEIGHT);
        }
    }
    else
    {
        m_isGrounded = false;
    }

    if (m_jumpBufferTime > 0.0f)
    {
        TryJump();
    }
    if (m_jumpBufferTime > 0.0f)
    {
        m_jumpBufferTime -= UPDATE_DELTA_SECONDS;
        if (m_jumpBufferTime < 0.0f)
        {
            m_jumpBufferTime = 0.0f;
        }
    }

    // X 위치 고정
    m_playerX = FIXED_PLAYER_X;

    // 바닥 이미지 이동
    UpdateBackground();
    m_scrollOffset += m_gameSpeed;
    while (m_scrollOffset >= SCREEN_WIDTH)
    {
        m_scrollOffset -= SCREEN_WIDTH;
    }

    // 배경 구름 이동
    UpdateClouds();

    // 거리 점수
    m_distanceScore += DISTANCE_GAIN_PER_FRAME;

    // 플레이어 애니메이션
    UpdatePlayerAnimation();

    // 장애물 갱신
    UpdateObstacles();
    UpdateJellies();

    // 충돌 확인
    CheckJellyCollisions();
    CheckObstacleCollisions();
    m_score = GetScore();

    // 장애물 생성
    if (m_distanceScore >= m_nextObstacleDistance)
    {
        SpawnObstacle();
        m_nextObstacleDistance += OBSTACLE_DISTANCE_STEP;
    }
}

void schoolrun::Render(HDC hDC)
{
    RECT screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    HBRUSH backgroundBrush = CreateSolidBrush(RGB(85, 180, 245));
    FillRect(hDC, &screenRect, backgroundBrush);
    DeleteObject(backgroundBrush);

    // 등굣길 원경 배경
    RenderBackground(hDC);

    // 배경 구름
    RenderClouds(hDC);

    // 이동 바닥
    RenderGround(hDC);

    // 젤리
    RenderJellies(hDC);

    // 장애물
    RenderObstacles(hDC);

    // 플레이어
    RenderPlayer(hDC);

    // 충돌 박스
    RenderHitBoxes(hDC);

    // 점수
    RenderScore(hDC);

    // 목숨
    RenderLife(hDC);

    // 피격 화면
    RenderDamageOverlay(hDC);
}

void schoolrun::OnKeyDown(WPARAM wParam)
{
    // 더블점프
    if (wParam == VK_SPACE)
    {
        if (!TryJump())
        {
            m_jumpBufferTime = JUMP_BUFFER_DURATION;
        }
        return;
    }

    // 슬라이드
    if (wParam == VK_DOWN)
    {
        m_isDownKeyPressed = true;
        if (m_isGrounded && !m_isSliding)
        {
            m_isSliding = true;
            m_playerY = static_cast<float>(GROUND_TOP - PLAYER_SLIDE_HEIGHT);
            audioManager.PlaySfx(L"minigame3_slide");
        }
    }
}

void schoolrun::OnKeyUp(WPARAM wParam)
{
    // 슬라이드 해제
    if (wParam == VK_DOWN && m_isSliding)
    {
        m_isDownKeyPressed = false;
        m_isSliding = false;
        m_playerY = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT);
        if (m_jumpBufferTime > 0.0f)
        {
            TryJump();
        }
    }
    else if (wParam == VK_DOWN)
    {
        m_isDownKeyPressed = false;
    }
}

bool schoolrun::IsFinished() const
{
    // 종료 상태
    return m_isFinished;
}

int schoolrun::GetScore() const
{
    if (m_totalJellyCount <= 0)
    {
        return 0;
    }

    // 전체 젤리를 다 먹으면 100점
    int score = m_jellyCount * 100 / m_totalJellyCount;
    if (score > 85)
    {
        score = 100;
    }

    return score;
}

RECT schoolrun::GetPlayerRect() const
{
    // 표시 영역
    const int currentHeight = m_isSliding ? PLAYER_SLIDE_HEIGHT : PLAYER_STAND_HEIGHT;
    RECT rect =
    {
        static_cast<LONG>(m_playerX),
        static_cast<LONG>(m_playerY),
        static_cast<LONG>(m_playerX + PLAYER_WIDTH),
        static_cast<LONG>(m_playerY + currentHeight)
    };

    return rect;
}

RECT schoolrun::GetHitBoxRect() const
{
    // 충돌 영역
    RECT playerRect = GetPlayerRect();
    RECT hitBoxRect =
    {
        playerRect.left + 8,
        playerRect.top + 6,
        playerRect.right - 8,
        playerRect.bottom - 4
    };

    return hitBoxRect;
}

RECT schoolrun::GetObstacleRect(const Obstacle& obstacle) const
{
    // 장애물 영역
    RECT rect =
    {
        static_cast<LONG>(obstacle.x),
        static_cast<LONG>(obstacle.y),
        static_cast<LONG>(obstacle.x + obstacle.width),
        static_cast<LONG>(obstacle.y + obstacle.height)
    };

    return rect;
}

RECT schoolrun::GetJellyRect(const Jelly& jelly) const
{
    RECT rect =
    {
        static_cast<LONG>(jelly.x),
        static_cast<LONG>(jelly.y),
        static_cast<LONG>(jelly.x + jelly.width),
        static_cast<LONG>(jelly.y + jelly.height)
    };

    return rect;
}

bool schoolrun::TryJump()
{
    if (m_isSliding || m_jumpCount >= 2)
    {
        return false;
    }

    m_velocityY = m_jumpPower;
    m_isGrounded = false;
    m_jumpCount++;
    m_jumpBufferTime = 0.0f;
    audioManager.PlaySfx(L"minigame3_jump");
    return true;
}

void schoolrun::UpdateBackground()
{
    m_backgroundOffset += m_gameSpeed * BACKGROUND_SPEED_MULTIPLIER;
    while (m_backgroundOffset >= BACKGROUND_CYCLE_WIDTH)
    {
        m_backgroundOffset -= BACKGROUND_CYCLE_WIDTH;
    }
}

void schoolrun::ResetClouds()
{
    m_clouds.clear();

    for (int i = 0; i < CLOUD_COUNT; ++i)
    {
        Cloud cloud = {};
        cloud.x = static_cast<float>(SCREEN_WIDTH) + CLOUD_START_OFFSETS[i];
        cloud.y = CLOUD_Y_POSITIONS[i];
        cloud.width = CLOUD_WIDTHS[i];
        cloud.height = CLOUD_HEIGHTS[i];
        m_clouds.push_back(cloud);
    }
}

void schoolrun::UpdateClouds()
{
    const float cloudSpeed = m_gameSpeed * CLOUD_SPEED_MULTIPLIER;

    for (Cloud& cloud : m_clouds)
    {
        cloud.x -= cloudSpeed;
    }

    float rightMostX = static_cast<float>(SCREEN_WIDTH);
    for (const Cloud& cloud : m_clouds)
    {
        if (cloud.x > rightMostX)
        {
            rightMostX = cloud.x;
        }
    }

    for (Cloud& cloud : m_clouds)
    {
        if (cloud.x + cloud.width < 0.0f)
        {
            cloud.x = rightMostX + CLOUD_RESPAWN_GAP;
            rightMostX = cloud.x;
        }
    }
}

void schoolrun::UpdateObstacles()
{
    // 장애물 이동과 삭제
    for (size_t i = 0; i < m_obstacles.size();)
    {
        m_obstacles[i].x -= m_gameSpeed;

        if (m_obstacles[i].x + m_obstacles[i].width < 0.0f)
        {
            m_obstacles.erase(m_obstacles.begin() + i);
        }
        else
        {
            ++i;
        }
    }
}

void schoolrun::UpdateJellies()
{
    for (Jelly& jelly : m_jellies)
    {
        jelly.x -= m_gameSpeed;
    }

    m_jellies.erase(
        std::remove_if(
            m_jellies.begin(),
            m_jellies.end(),
            [](const Jelly& jelly)
            {
                return jelly.collected || jelly.x + jelly.width < 0.0f;
            }),
        m_jellies.end());
}

void schoolrun::UpdatePlayerAnimation()
{
    if (!m_isGrounded)
    {
        return;
    }

    m_playerAnimationTime += UPDATE_DELTA_SECONDS;
    while (m_playerAnimationTime >= PLAYER_ANIMATION_INTERVAL)
    {
        m_playerAnimationTime -= PLAYER_ANIMATION_INTERVAL;
        m_playerFrameIndex = (m_playerFrameIndex + 1) % PLAYER_RUNNING_FRAME_COUNT;
    }
}

void schoolrun::CheckObstacleCollisions()
{
    if (m_invincibleTime > 0.0f)
    {
        return;
    }

    // 플레이어와 장애물 충돌
    RECT playerHitBox = GetHitBoxRect();

    for (size_t i = 0; i < m_obstacles.size(); ++i)
    {
        RECT obstacleRect = GetObstacleRect(m_obstacles[i]);
        RECT intersectRect = {};

        if (IntersectRect(&intersectRect, &playerHitBox, &obstacleRect))
        {
            m_life--;
            audioManager.PlaySfx(L"minigame3_hit");
            m_invincibleTime = INVINCIBLE_DURATION;
            if (m_life <= 0)
            {
                m_life = 0;
                m_isFinished = true;
            }
            return;
        }
    }
}

void schoolrun::CheckJellyCollisions()
{
    RECT playerHitBox = GetHitBoxRect();
    bool collectedAnyJelly = false;

    for (Jelly& jelly : m_jellies)
    {
        if (jelly.collected)
        {
            continue;
        }

        RECT jellyRect = GetJellyRect(jelly);
        RECT intersectRect = {};

        if (IntersectRect(&intersectRect, &playerHitBox, &jellyRect))
        {
            jelly.collected = true;
            m_jellyCount++;
            collectedAnyJelly = true;
        }
    }

    if (collectedAnyJelly)
    {
        audioManager.PlaySfx(L"minigame3_jelly_collect");
    }
}

void schoolrun::SpawnObstacle()
{
    if (m_obstaclePatterns.empty())
    {
        return;
    }

    if (m_nextObstaclePatternIndex < 0 ||
        m_nextObstaclePatternIndex >= static_cast<int>(m_obstaclePatterns.size()))
    {
        m_nextObstaclePatternIndex = 0;
    }

    const int patternIndex = m_nextObstaclePatternIndex;
    std::vector<Obstacle> patternObstacles = BuildPatternObstacles(patternIndex);
    for (const Obstacle& obstacle : patternObstacles)
    {
        m_obstacles.push_back(obstacle);
    }
    SpawnJelliesForPattern(patternIndex);

    m_nextObstaclePatternIndex++;
    if (m_nextObstaclePatternIndex >= static_cast<int>(m_obstaclePatterns.size()))
    {
        m_nextObstaclePatternIndex = 0;
    }
}

void schoolrun::SpawnJelliesForPattern(int patternIndex)
{
    std::vector<JellyPoint> points = BuildJellyRouteForPattern(patternIndex);
    for (const JellyPoint& point : points)
    {
        AddJelly(point.centerX, point.centerY);
    }
}

void schoolrun::SpawnInitialJellies()
{
    std::vector<JellyPoint> points = BuildInitialJellyRoute();
    for (const JellyPoint& point : points)
    {
        AddJelly(point.centerX, point.centerY);
    }
}

std::vector<schoolrun::Obstacle> schoolrun::BuildPatternObstacles(int patternIndex) const
{
    std::vector<Obstacle> obstacles;
    if (patternIndex < 0 || patternIndex >= static_cast<int>(m_obstaclePatterns.size()))
    {
        return obstacles;
    }

    const ObstaclePattern& pattern = m_obstaclePatterns[patternIndex];
    for (const PatternObstacle& patternObstacle : pattern.obstacles)
    {
        Obstacle obstacle = {};
        SetObstacleSpec(obstacle, patternObstacle.type);
        obstacle.x += patternObstacle.offsetX;
        obstacles.push_back(obstacle);
    }

    std::sort(
        obstacles.begin(),
        obstacles.end(),
        [](const Obstacle& left, const Obstacle& right)
        {
            return left.x < right.x;
        });

    return obstacles;
}

bool schoolrun::IsJumpObstacle(ObstacleType type) const
{
    return type == GroundObstacle ||
        type == BoxObstacle ||
        type == ConstructionObstacle ||
        type == WaterObstacle;
}

bool schoolrun::AreObstaclesConnected(const Obstacle& left, const Obstacle& right) const
{
    const float leftRight = left.x + left.width;
    return right.x - leftRight <= JELLY_CONNECTED_OBSTACLE_GAP;
}

bool schoolrun::HasConnectedObstacleNeighbor(const std::vector<Obstacle>& obstacles, size_t index) const
{
    if (index > 0 && AreObstaclesConnected(obstacles[index - 1], obstacles[index]))
    {
        return true;
    }

    if (index + 1 < obstacles.size() && AreObstaclesConnected(obstacles[index], obstacles[index + 1]))
    {
        return true;
    }

    return false;
}

bool schoolrun::ShouldRaisePaperJelly(const std::vector<Obstacle>& obstacles, size_t index) const
{
    if (index >= obstacles.size() || obstacles[index].type != PaperObstacle)
    {
        return false;
    }

    if (index > 0 &&
        AreObstaclesConnected(obstacles[index - 1], obstacles[index]) &&
        IsJumpObstacle(obstacles[index - 1].type))
    {
        return true;
    }

    if (index + 1 < obstacles.size() &&
        AreObstaclesConnected(obstacles[index], obstacles[index + 1]) &&
        IsJumpObstacle(obstacles[index + 1].type))
    {
        return true;
    }

    return false;
}

float schoolrun::GetRaisedJellyCenterY(const Obstacle& obstacle) const
{
    const float centerY =
        obstacle.y -
        (JELLY_HEIGHT * 0.5f) -
        JELLY_OBSTACLE_CLEARANCE -
        JELLY_JUMP_EXTRA_HEIGHT;
    return centerY > JELLY_HEIGHT * 0.5f ? centerY : JELLY_HEIGHT * 0.5f;
}

float schoolrun::GetJellyCenterY(const Obstacle& obstacle) const
{
    if (!IsJumpObstacle(obstacle.type))
    {
        return JELLY_GROUND_CENTER_Y;
    }

    return GetRaisedJellyCenterY(obstacle);
}

std::vector<schoolrun::JellyPoint> schoolrun::BuildInitialJellyRoute() const
{
    std::vector<JellyPoint> points;
    const float startX = FIXED_PLAYER_X + PLAYER_WIDTH + JELLY_GAP_X;
    const float endX =
        OBSTACLE_START_X +
        FIRST_OBSTACLE_SPAWN_TRAVEL_X -
        (JELLY_GAP_X * 2.0f);

    AppendJellyLine(points, startX, endX, JELLY_GROUND_CENTER_Y);
    return points;
}

std::vector<schoolrun::JellyPoint> schoolrun::BuildJellyRouteForPattern(int patternIndex) const
{
    std::vector<JellyPoint> points;
    std::vector<Obstacle> obstacles = BuildPatternObstacles(patternIndex);
    if (obstacles.empty())
    {
        AppendJellyLine(points, OBSTACLE_START_X, OBSTACLE_START_X + (JELLY_GAP_X * 4.0f), JELLY_GROUND_CENTER_Y);
        return points;
    }

    const float obstacleEndX = obstacles.back().x + obstacles.back().width + (JELLY_GAP_X * 2.0f);
    const float nextPatternEndX =
        OBSTACLE_START_X +
        PATTERN_SPAWN_TRAVEL_X -
        (JELLY_GAP_X * 2.0f);
    const float patternEndX = obstacleEndX > nextPatternEndX ? obstacleEndX : nextPatternEndX;
    float groundLineStartX = OBSTACLE_START_X - JELLY_GAP_X;

    for (size_t i = 0; i < obstacles.size(); ++i)
    {
        const Obstacle& obstacle = obstacles[i];
        const float centerX = obstacle.x + (obstacle.width * 0.5f);
        const bool hasConnectedNeighbor = HasConnectedObstacleNeighbor(obstacles, i);
        const bool raisePaperJelly = ShouldRaisePaperJelly(obstacles, i);

        if (raisePaperJelly)
        {
            AppendJellyLine(points, groundLineStartX, obstacle.x - JELLY_SAFE_SIDE_GAP, JELLY_GROUND_CENTER_Y);
            AddJellyPoint(points, centerX, GetRaisedJellyCenterY(obstacle));
            groundLineStartX = obstacle.x + obstacle.width + JELLY_SAFE_SIDE_GAP;
        }
        else if (IsJumpObstacle(obstacle.type))
        {
            if (hasConnectedNeighbor)
            {
                AppendJellyLine(points, groundLineStartX, obstacle.x - JELLY_SAFE_SIDE_GAP, JELLY_GROUND_CENTER_Y);
                AddJellyPoint(points, centerX, GetJellyCenterY(obstacle));
                groundLineStartX = obstacle.x + obstacle.width + JELLY_SAFE_SIDE_GAP;
            }
            else
            {
                const float peakY = GetJellyCenterY(obstacle);
                const float neededArcHeight = JELLY_GROUND_CENTER_Y - peakY;
                const float arcHeight = neededArcHeight > JELLY_MIN_JUMP_ARC_HEIGHT ?
                    neededArcHeight :
                    JELLY_MIN_JUMP_ARC_HEIGHT;
                const float arcHalfWidth =
                    (obstacle.width * 0.5f + JELLY_WIDTH * 0.5f + JELLY_OBSTACLE_CLEARANCE) * 2.0f;
                const float arcStartX = centerX - arcHalfWidth;
                const float arcEndX = centerX + arcHalfWidth;

                AppendJellyLine(points, groundLineStartX, arcStartX - JELLY_GAP_X, JELLY_GROUND_CENTER_Y);
                AppendJellyArc(
                    points,
                    arcStartX,
                    arcEndX,
                    JELLY_GROUND_CENTER_Y,
                    arcHeight,
                    JELLY_JUMP_ARC_COUNT);
                groundLineStartX = arcEndX + JELLY_GAP_X;
            }
        }
        else
        {
            const float slideRouteEndX = obstacle.x + obstacle.width + JELLY_SAFE_SIDE_GAP;
            AppendJellyLine(points, groundLineStartX, slideRouteEndX, JELLY_GROUND_CENTER_Y);
            groundLineStartX = slideRouteEndX + JELLY_GAP_X;
        }
    }

    AppendJellyLine(points, groundLineStartX, patternEndX, JELLY_GROUND_CENTER_Y);
    return points;
}

int schoolrun::CountJelliesForPattern(int patternIndex) const
{
    return static_cast<int>(BuildJellyRouteForPattern(patternIndex).size());
}

int schoolrun::CalculateTotalJellyCount() const
{
    if (m_obstaclePatterns.empty())
    {
        return 0;
    }

    int totalJellyCount = 0;
    int distanceScore = 0;
    int nextObstacleDistance = OBSTACLE_START_DISTANCE;
    int patternIndex = 0;
    float remainingTime = GAME_TIME_LIMIT;

    while (remainingTime > 0.0f)
    {
        remainingTime -= UPDATE_DELTA_SECONDS;
        distanceScore += DISTANCE_GAIN_PER_FRAME;

        if (distanceScore >= nextObstacleDistance)
        {
            totalJellyCount += CountJelliesForPattern(patternIndex);
            patternIndex++;
            if (patternIndex >= static_cast<int>(m_obstaclePatterns.size()))
            {
                patternIndex = 0;
            }
            nextObstacleDistance += OBSTACLE_DISTANCE_STEP;
        }
    }

    return totalJellyCount;
}

void schoolrun::AppendJellyLine(std::vector<JellyPoint>& points, float startCenterX, float endCenterX, float centerY) const
{
    if (endCenterX < startCenterX)
    {
        return;
    }

    float lastCenterX = startCenterX - JELLY_GAP_X;
    for (float centerX = startCenterX; centerX <= endCenterX; centerX += JELLY_GAP_X)
    {
        AddJellyPoint(points, centerX, centerY);
        lastCenterX = centerX;
    }

    if (endCenterX - lastCenterX > JELLY_GAP_X * 0.45f)
    {
        AddJellyPoint(points, endCenterX, centerY);
    }
}

void schoolrun::AppendJellyArc(std::vector<JellyPoint>& points, float startCenterX, float endCenterX, float baseCenterY, float arcHeight, int count) const
{
    if (count <= 0)
    {
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        const float t = (count == 1) ? 0.0f : static_cast<float>(i) / static_cast<float>(count - 1);
        const float centerX = startCenterX + ((endCenterX - startCenterX) * t);
        const float centerY = baseCenterY - (arcHeight * 4.0f * t * (1.0f - t));
        AddJellyPoint(points, centerX, centerY);
    }
}

void schoolrun::AddJellyPoint(std::vector<JellyPoint>& points, float centerX, float centerY) const
{
    for (const JellyPoint& point : points)
    {
        float diffX = centerX - point.centerX;
        if (diffX < 0.0f)
        {
            diffX = -diffX;
        }

        float diffY = centerY - point.centerY;
        if (diffY < 0.0f)
        {
            diffY = -diffY;
        }

        if (diffX < JELLY_WIDTH * 0.25f && diffY < JELLY_HEIGHT * 0.25f)
        {
            return;
        }
    }

    JellyPoint point = {};
    point.centerX = centerX;
    point.centerY = centerY;
    points.push_back(point);
}

void schoolrun::AddJelly(float centerX, float centerY)
{
    Jelly jelly = {};
    jelly.x = centerX - (JELLY_WIDTH * 0.5f);
    jelly.y = centerY - (JELLY_HEIGHT * 0.5f);
    jelly.width = JELLY_WIDTH;
    jelly.height = JELLY_HEIGHT;
    jelly.collected = false;
    m_jellies.push_back(jelly);
}

void schoolrun::CreateObstaclePatterns()
{
    m_obstaclePatterns.clear();

    m_obstaclePatterns.push_back({ { { ConstructionObstacle, 0.0f }, { SignboardObstacle, 500.0f },{ SignboardObstacle, 700.0f },{ SignboardObstacle, 900.0f } } });
    m_obstaclePatterns.push_back({ { { PaperObstacle, 0.0f }, { BoxObstacle, 130.0f } } });
    m_obstaclePatterns.push_back({ { { SignboardObstacle, 0.0f },{ WaterObstacle, 500.0f }, { SignboardObstacle, 1000.0f } } });
    m_obstaclePatterns.push_back({ { { PaperObstacle, 0.0f }, { WaterObstacle, 500.0f }, { BirdObstacle, 1000.0f }} });
    m_obstaclePatterns.push_back({ { { BoxObstacle, 0.0f }, { BoxObstacle, 800.0f }, { BoxObstacle, 1600.0f } } });
    m_obstaclePatterns.push_back({ { { ConstructionObstacle, 0.0f }, { WaterObstacle, 200.0f } ,{ ConstructionObstacle, 400.0f },{ ConstructionObstacle, 900.0f }, { ConstructionObstacle, 1500.0f } } });
    m_obstaclePatterns.push_back({ { { BirdObstacle, 0.0f }, { BirdObstacle, 200.0f } ,{ WaterObstacle, 600.0f },{ PaperObstacle, 1000.0f }, { WaterObstacle, 1400.0f } } });
}

void schoolrun::SetObstacleSpec(Obstacle& obstacle, ObstacleType type) const
{
    obstacle.type = type;
    obstacle.x = OBSTACLE_START_X;

    switch (type)
    {
    case PaperObstacle:
        obstacle.width = PAPER_OBSTACLE_WIDTH;
        obstacle.height = PAPER_OBSTACLE_HEIGHT;
        obstacle.y = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT - 80);
        break;

    case BirdObstacle:
        obstacle.width = BIRD_OBSTACLE_WIDTH;
        obstacle.height = BIRD_OBSTACLE_HEIGHT;
        obstacle.y = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT - 75);
        break;

    case BoxObstacle:
        obstacle.width = BOX_OBSTACLE_WIDTH;
        obstacle.height = BOX_OBSTACLE_HEIGHT;
        obstacle.y = static_cast<float>(GROUND_TOP - obstacle.height);
        break;

    case ConstructionObstacle:
        obstacle.width = CONSTRUCTION_OBSTACLE_WIDTH;
        obstacle.height = CONSTRUCTION_OBSTACLE_HEIGHT;
        obstacle.y = static_cast<float>(GROUND_TOP - obstacle.height);
        break;

    case WaterObstacle:
        obstacle.width = WATER_OBSTACLE_WIDTH;
        obstacle.height = WATER_OBSTACLE_HEIGHT;
        obstacle.y = static_cast<float>(GROUND_TOP - obstacle.height);
        break;

    case SignboardObstacle:
        obstacle.width = SIGNBOARD_OBSTACLE_WIDTH;
        obstacle.height = SIGNBOARD_OBSTACLE_HEIGHT;
        obstacle.y = -10.0f;
        break;

    case GroundObstacle:
        obstacle.width = GROUND_OBSTACLE_WIDTH;
        obstacle.height = GROUND_OBSTACLE_HEIGHT;
        obstacle.y = static_cast<float>(GROUND_TOP - obstacle.height);
        break;

    case AirObstacle:
    default:
        obstacle.width = AIR_OBSTACLE_WIDTH;
        obstacle.height = AIR_OBSTACLE_HEIGHT;
        obstacle.y = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT + 12);
        break;
    }
}

CImage* schoolrun::GetObstacleImage(ObstacleType type)
{
    switch (type)
    {
    case PaperObstacle:
        return &m_paperObstacleImage;

    case BirdObstacle:
    case AirObstacle:
        return &m_airObstacleImage;

    case BoxObstacle:
        return &m_boxObstacleImage;

    case ConstructionObstacle:
    case GroundObstacle:
        return &m_groundObstacleImage;

    case WaterObstacle:
        return &m_waterObstacleImage;

    case SignboardObstacle:
        return &m_signboardObstacleImage;

    default:
        return nullptr;
    }
}

void schoolrun::RenderBackground(HDC hDC)
{
    if (m_backgroundImage.IsNull())
    {
        return;
    }

    int segmentIndex = static_cast<int>(m_backgroundOffset) / SCREEN_WIDTH;
    int offsetX = static_cast<int>(m_backgroundOffset) % SCREEN_WIDTH;

    for (int i = 0; i < 3; ++i)
    {
        CImage* backgroundImage = ((segmentIndex + i) % 2 == 0 || m_backgroundImage2.IsNull()) ?
            &m_backgroundImage :
            &m_backgroundImage2;

        DrawImage(
            hDC,
            *backgroundImage,
            -offsetX + (i * SCREEN_WIDTH),
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);
    }
}

void schoolrun::RenderClouds(HDC hDC)
{
    if (m_cloudImage.IsNull())
    {
        return;
    }

    for (const Cloud& cloud : m_clouds)
    {
        DrawImage(
            hDC,
            m_cloudImage,
            static_cast<int>(cloud.x),
            static_cast<int>(cloud.y),
            cloud.width,
            cloud.height);
    }
}

void schoolrun::RenderGround(HDC hDC)
{
    int groundHeight = 1080;
    int offsetX = static_cast<int>(m_scrollOffset) % SCREEN_WIDTH + 10;

    if (!m_groundImage.IsNull())
    {
        DrawImage(hDC, m_groundImage, -offsetX, 0, SCREEN_WIDTH+50, groundHeight);
        DrawImage(hDC, m_groundImage, SCREEN_WIDTH - offsetX, 0, SCREEN_WIDTH+50, groundHeight);
        return;
    }

    RECT groundRect = { 0, GROUND_TOP, SCREEN_WIDTH, SCREEN_HEIGHT };
    HBRUSH groundBrush = CreateSolidBrush(RGB(238, 238, 232));
    FillRect(hDC, &groundRect, groundBrush);
    DeleteObject(groundBrush);
}

void schoolrun::RenderJellies(HDC hDC)
{
    if (m_jellies.empty())
    {
        return;
    }

    HBRUSH jellyBrush = CreateSolidBrush(RGB(255, 214, 55));
    HBRUSH shineBrush = CreateSolidBrush(RGB(255, 250, 175));
    HPEN outlinePen = CreatePen(PS_SOLID, 2, RGB(255, 246, 140));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, jellyBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, outlinePen));

    for (const Jelly& jelly : m_jellies)
    {
        if (jelly.collected)
        {
            continue;
        }

        RECT rect = GetJellyRect(jelly);
        if (rect.right < 0 || rect.left > SCREEN_WIDTH)
        {
            continue;
        }

        SelectObject(hDC, jellyBrush);
        Ellipse(hDC, rect.left, rect.top, rect.right, rect.bottom);

        SelectObject(hDC, shineBrush);
        Ellipse(
            hDC,
            rect.left + 14,
            rect.top + 10,
            rect.left + 31,
            rect.top + 27);
    }

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    DeleteObject(outlinePen);
    DeleteObject(shineBrush);
    DeleteObject(jellyBrush);
}

void schoolrun::RenderObstacles(HDC hDC)
{
    // 장애물 출력
    for (const Obstacle& obstacle : m_obstacles)
    {
        RECT rect = GetObstacleRect(obstacle);
        CImage* obstacleImage = GetObstacleImage(obstacle.type);
        if (obstacleImage != nullptr && !obstacleImage->IsNull())
        {
            DrawImage(hDC, *obstacleImage, rect.left, rect.top, obstacle.width, obstacle.height);
            continue;
        }

        if (obstacle.type == GroundObstacle)
        {
            HBRUSH brush = CreateSolidBrush(RGB(230, 90, 80));
            HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 230, 220));
            HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, brush));
            HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, pen));

            Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
            TextOutW(hDC, rect.left + 8, rect.top + 20, L"점프", 2);

            SelectObject(hDC, oldPen);
            SelectObject(hDC, oldBrush);
            DeleteObject(pen);
            DeleteObject(brush);
        }
        else
        {
            HBRUSH brush = CreateSolidBrush(RGB(255, 210, 80));
            HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 255, 230));
            HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, brush));
            HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, pen));

            Ellipse(hDC, rect.left, rect.top, rect.right, rect.bottom);
            TextOutW(hDC, rect.left + 12, rect.top + 16, L"슬라이드", 4);

            SelectObject(hDC, oldPen);
            SelectObject(hDC, oldBrush);
            DeleteObject(pen);
            DeleteObject(brush);
        }
    }
}

void schoolrun::RenderPlayer(HDC hDC)
{
    RECT playerRect = GetPlayerRect();
    int frameIndex = m_isGrounded ? m_playerFrameIndex : PLAYER_JUMP_FRAME_INDEX;

    if (m_isSliding && !m_playerSlideImage.IsNull())
    {
        int drawWidth = m_playerSlideImage.GetWidth();
        int drawHeight = m_playerSlideImage.GetHeight();
        int drawX = playerRect.left + ((PLAYER_WIDTH - drawWidth) / 2);
        int drawY = GROUND_TOP - drawHeight;

        DrawImage(
            hDC,
            m_playerSlideImage,
            drawX,
            drawY,
            drawWidth,
            drawHeight);
        return;
    }

    if (!m_playerRunImage.IsNull())
    {
        DrawImageFrame(
            hDC,
            m_playerRunImage,
            playerRect.left,
            playerRect.top,
            playerRect.right - playerRect.left,
            playerRect.bottom - playerRect.top,
            frameIndex,
            PLAYER_SHEET_FRAME_COUNT);
        return;
    }

    HBRUSH playerBrush = CreateSolidBrush(m_isSliding ? RGB(100, 220, 180) : RGB(120, 190, 255));
    HPEN playerPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, playerBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, playerPen));

    RoundRect(hDC, playerRect.left, playerRect.top, playerRect.right, playerRect.bottom, 27, 27);

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    DeleteObject(playerPen);
    DeleteObject(playerBrush);
}

void schoolrun::RenderHitBoxes(HDC hDC)
{
    if (!SHOW_HIT_BOX)
    {
        return;
    }

    HPEN hitBoxPen = CreatePen(PS_DASH, 2, RGB(255, 80, 80));
    HBRUSH hollowBrush = static_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, hollowBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, hitBoxPen));

    RECT playerHitBox = GetHitBoxRect();
    Rectangle(hDC, playerHitBox.left, playerHitBox.top, playerHitBox.right, playerHitBox.bottom);

    for (const Obstacle& obstacle : m_obstacles)
    {
        RECT obstacleRect = GetObstacleRect(obstacle);
        Rectangle(hDC, obstacleRect.left, obstacleRect.top, obstacleRect.right, obstacleRect.bottom);
    }

    for (const Jelly& jelly : m_jellies)
    {
        RECT jellyRect = GetJellyRect(jelly);
        Ellipse(hDC, jellyRect.left, jellyRect.top, jellyRect.right, jellyRect.bottom);
    }

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    DeleteObject(hitBoxPen);
}

void schoolrun::RenderScore(HDC hDC) const
{
    // 거리 점수 표시
    wchar_t scoreText[128];
    wsprintfW(
        scoreText,
        L"시간: %d   거리: %d m   젤리: %d   점수: %d",
        static_cast<int>(m_remainingTime + 0.9f),
        m_distanceScore,
        m_jellyCount,
        GetScore());

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(255, 255, 255));
    TextOutW(hDC, 40, 35, scoreText, lstrlenW(scoreText));
}

void schoolrun::RenderLife(HDC hDC)
{
    int startX = 40;
    int startY = 75;

    for (int i = 0; i < m_life; i++)
    {
        int drawX = startX + i * (HEART_SIZE + HEART_GAP);
        if (!m_heartImage.IsNull())
        {
            DrawImage(hDC, m_heartImage, drawX, startY, HEART_SIZE, HEART_SIZE);
        }
        else
        {
            HBRUSH heartBrush = CreateSolidBrush(RGB(255, 80, 110));
            RECT heartRect = { drawX, startY, drawX + HEART_SIZE, startY + HEART_SIZE };
            FillRect(hDC, &heartRect, heartBrush);
            DeleteObject(heartBrush);
        }
    }
}

void schoolrun::RenderDamageOverlay(HDC hDC)
{
    if (m_invincibleTime <= 0.0f)
    {
        return;
    }

    HDC overlayDC = CreateCompatibleDC(hDC);
    HBITMAP overlayBitmap = CreateCompatibleBitmap(hDC, SCREEN_WIDTH, SCREEN_HEIGHT);
    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(overlayDC, overlayBitmap));

    RECT overlayRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    HBRUSH overlayBrush = CreateSolidBrush(RGB(255, 60, 60));
    FillRect(overlayDC, &overlayRect, overlayBrush);

    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = DAMAGE_OVERLAY_ALPHA;

    AlphaBlend(
        hDC,
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        overlayDC,
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        blend);

    DeleteObject(overlayBrush);
    SelectObject(overlayDC, oldBitmap);
    DeleteObject(overlayBitmap);
    DeleteDC(overlayDC);
}

void schoolrun::DrawImage(HDC hDC, CImage& image, int drawX, int drawY, int drawWidth, int drawHeight)
{
    if (image.IsNull())
    {
        return;
    }

    image.Draw(hDC, drawX, drawY, drawWidth, drawHeight);
}

void schoolrun::DrawImageFrame(HDC hDC, CImage& image, int drawX, int drawY, int drawWidth, int drawHeight, int frameIndex, int frameCount)
{
    if (image.IsNull() || frameCount <= 0)
    {
        return;
    }

    int frameWidth = image.GetWidth() / frameCount;
    int frameHeight = image.GetHeight();
    int safeFrame = frameIndex % frameCount;

    image.Draw(
        hDC,
        drawX,
        drawY,
        drawWidth,
        drawHeight,
        safeFrame * frameWidth,
        0,
        frameWidth,
        frameHeight);
}

