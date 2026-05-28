#include "schoolrun.h"

#pragma comment(lib, "msimg32.lib")

namespace
{
    const bool SHOW_HIT_BOX = true;

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

    const float FIXED_PLAYER_X = 220.0f;

    // 조절값: 점프와 낙하
    const float GRAVITY_VALUE = 10.0f;
    const float JUMP_POWER_VALUE = -80.0f;

    // 조절값: 러닝 속도
    const float START_GAME_SPEED = 30.0f;
    const int DISTANCE_GAIN_PER_FRAME = 2;

    // 조절값: 장애물 생성
    const int OBSTACLE_START_DISTANCE = 20;
    const int OBSTACLE_DISTANCE_STEP = 150;
    const float OBSTACLE_START_X = static_cast<float>(SCREEN_WIDTH);

    // 조절값: 시간과 점수
    const float UPDATE_DELTA_SECONDS = 0.06f;
    const float GAME_TIME_LIMIT = 40.0f;
    const int SCORE_MAX_DISTANCE = 1300;
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
    m_playerFrameIndex = 0;
    m_playerAnimationTime = 0.0f;

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
}

void schoolrun::Reset()
{
    // 플레이어 위치
    m_playerX = FIXED_PLAYER_X;
    m_playerY = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT);
    m_velocityY = 0.0f;

    // 러닝 상태
    m_gameSpeed = START_GAME_SPEED;
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
    m_playerFrameIndex = 0;
    m_playerAnimationTime = 0.0f;
    m_obstacles.clear();
    CreateObstaclePatterns();
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

    // X 위치 고정
    m_playerX = FIXED_PLAYER_X;

    // 바닥 이미지 이동
    m_scrollOffset += m_gameSpeed;
    while (m_scrollOffset >= SCREEN_WIDTH)
    {
        m_scrollOffset -= SCREEN_WIDTH;
    }

    // 거리 점수
    m_distanceScore += DISTANCE_GAIN_PER_FRAME;
    m_score = m_distanceScore;

    // 플레이어 애니메이션
    UpdatePlayerAnimation();

    // 장애물 갱신
    UpdateObstacles();

    // 충돌 확인
    CheckObstacleCollisions();

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

    // 이동 바닥
    RenderGround(hDC);

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
    if (wParam == VK_SPACE && !m_isSliding && m_jumpCount < 2)
    {
        m_velocityY = m_jumpPower;
        m_isGrounded = false;
        m_jumpCount++;
        return;
    }

    // 슬라이드
    if (wParam == VK_DOWN)
    {
        m_isDownKeyPressed = true;
        if (m_isGrounded)
        {
            m_isSliding = true;
            m_playerY = static_cast<float>(GROUND_TOP - PLAYER_SLIDE_HEIGHT);
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
    // 거리 점수 환산
    int score = m_distanceScore * 100 / SCORE_MAX_DISTANCE;
    if (score > 100)
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

    const ObstaclePattern& pattern = m_obstaclePatterns[m_nextObstaclePatternIndex];
    for (const PatternObstacle& patternObstacle : pattern.obstacles)
    {
        Obstacle obstacle = {};
        SetObstacleSpec(obstacle, patternObstacle.type);
        obstacle.x += patternObstacle.offsetX;
        m_obstacles.push_back(obstacle);
    }

    m_nextObstaclePatternIndex++;
    if (m_nextObstaclePatternIndex >= static_cast<int>(m_obstaclePatterns.size()))
    {
        m_nextObstaclePatternIndex = 0;
    }
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
        L"시간: %d   거리: %d m   점수: %d",
        static_cast<int>(m_remainingTime + 0.9f),
        m_distanceScore,
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

