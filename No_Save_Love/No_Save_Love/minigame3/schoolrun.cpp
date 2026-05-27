#include "schoolrun.h"

namespace
{
    const int SCREEN_WIDTH = 1920;
    const int SCREEN_HEIGHT = 1080;
    const int GROUND_TOP = 920;

    const int PLAYER_WIDTH = 130;
    const int PLAYER_STAND_HEIGHT = 158;
    const int PLAYER_SLIDE_HEIGHT = 83;

    const int GROUND_OBSTACLE_WIDTH = 120;
    const int GROUND_OBSTACLE_HEIGHT = 113;
    const int AIR_OBSTACLE_WIDTH = PLAYER_WIDTH;
    const int AIR_OBSTACLE_HEIGHT = 75;

    const float FIXED_PLAYER_X = 220.0f;

    // 조절값: 점프와 낙하
    const float GRAVITY_VALUE = 3.8f;
    const float JUMP_POWER_VALUE = -48.0f;

    // 조절값: 러닝 속도
    const float START_GAME_SPEED = 20.0f;
    const float SPEED_UP_AMOUNT = 10.0f;
    const int SPEED_UP_DISTANCE_STEP = 1000;
    const int DISTANCE_GAIN_PER_FRAME = 2;

    // 조절값: 장애물 생성
    const int OBSTACLE_START_DISTANCE = 20;
    const int OBSTACLE_DISTANCE_STEP = 100;
    const float OBSTACLE_START_X = static_cast<float>(SCREEN_WIDTH);

    // 조절값: 시간과 점수
    const float UPDATE_DELTA_SECONDS = 0.06f;
    const float GAME_TIME_LIMIT = 40.0f;
    const int SCORE_MAX_DISTANCE = 1300;

    const int GROUND_MARK_INTERVAL = 180;
    const int BACKGROUND_IMAGE_COUNT = 3;
    const int BACKGROUND_SCROLL_WIDTH = 1920;
    const float BACKGROUND_SCROLL_RATE = 0.35f;
    const float PLAYER_ANIMATION_INTERVAL = 0.08f;
    const int PLAYER_RUNNING_FRAME_COUNT = 8;
    const int PLAYER_ONE_JUMP_FRAME_COUNT = 8;
    const int PLAYER_TWO_JUMP_FRAME_COUNT = 5;
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
    m_backgroundScrollOffset = 0.0f;
    m_distanceScore = 0;
    m_nextSpeedUpDistance = SPEED_UP_DISTANCE_STEP;
    m_nextObstacleDistance = OBSTACLE_START_DISTANCE;
    m_nextObstacleType = 0;
    m_remainingTime = GAME_TIME_LIMIT;

    m_score = 0;
    m_isFinished = false;
    m_isGrounded = true;
    m_isSliding = false;
    m_jumpCount = 0;
    m_playerAnimationState = RunningAnimation;
    m_playerFrameIndex = 0;
    m_playerAnimationTime = 0.0f;

    m_gdiplusToken = 0;
    m_isGdiplusStarted = false;
    for (int i = 0; i < BACKGROUND_IMAGE_COUNT; ++i)
    {
        m_backgroundImages[i] = nullptr;
    }
    m_playerRunImage = nullptr;
    m_playerOneJumpImage = nullptr;
    m_playerTwoJumpImage = nullptr;
    m_groundObstacleImage = nullptr;
    m_airObstacleImage = nullptr;
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
}

void schoolrun::LoadImages()
{
    if (!m_isGdiplusStarted)
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        if (Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr) == Gdiplus::Ok)
        {
            m_isGdiplusStarted = true;
        }
    }

    if (!m_isGdiplusStarted)
    {
        return;
    }

    if (m_backgroundImages[0] == nullptr)
    {
        m_backgroundImages[0] = new Gdiplus::Image(L"resource\\minigame3\\background1.png");
    }
    if (m_backgroundImages[1] == nullptr)
    {
        m_backgroundImages[1] = new Gdiplus::Image(L"resource\\minigame3\\backgraound2.png");
        if (m_backgroundImages[1]->GetLastStatus() != Gdiplus::Ok)
        {
            delete m_backgroundImages[1];
            m_backgroundImages[1] = new Gdiplus::Image(L"resource\\minigame3\\background2.png");
        }
    }
    if (m_backgroundImages[2] == nullptr)
    {
        m_backgroundImages[2] = new Gdiplus::Image(L"resource\\minigame3\\background3.png");
    }
    if (m_playerRunImage == nullptr)
    {
        m_playerRunImage = new Gdiplus::Image(L"resource\\minigame3\\char_running_right.png");
    }
    if (m_playerOneJumpImage == nullptr)
    {
        m_playerOneJumpImage = new Gdiplus::Image(L"resource\\minigame3\\one_jump.png");
    }
    if (m_playerTwoJumpImage == nullptr)
    {
        m_playerTwoJumpImage = new Gdiplus::Image(L"resource\\minigame3\\two_jump.png");
    }
    if (m_groundObstacleImage == nullptr)
    {
        m_groundObstacleImage = new Gdiplus::Image(L"resource\\minigame3\\construction.png");
    }
    if (m_airObstacleImage == nullptr)
    {
        m_airObstacleImage = new Gdiplus::Image(L"resource\\minigame3\\bird.png");
    }
}

void schoolrun::DestroyImages()
{
    for (int i = 0; i < BACKGROUND_IMAGE_COUNT; ++i)
    {
        delete m_backgroundImages[i];
        m_backgroundImages[i] = nullptr;
    }

    delete m_playerRunImage;
    delete m_playerOneJumpImage;
    delete m_playerTwoJumpImage;
    delete m_groundObstacleImage;
    delete m_airObstacleImage;

    m_playerRunImage = nullptr;
    m_playerOneJumpImage = nullptr;
    m_playerTwoJumpImage = nullptr;
    m_groundObstacleImage = nullptr;
    m_airObstacleImage = nullptr;

    if (m_isGdiplusStarted)
    {
        Gdiplus::GdiplusShutdown(m_gdiplusToken);
        m_gdiplusToken = 0;
        m_isGdiplusStarted = false;
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
    m_backgroundScrollOffset = 0.0f;
    m_distanceScore = 0;
    m_nextSpeedUpDistance = SPEED_UP_DISTANCE_STEP;
    m_nextObstacleDistance = OBSTACLE_START_DISTANCE;
    m_nextObstacleType = 0;
    m_remainingTime = GAME_TIME_LIMIT;

    // 결과 상태
    m_score = 0;
    m_isFinished = false;
    m_isGrounded = true;
    m_isSliding = false;
    m_jumpCount = 0;
    m_playerAnimationState = RunningAnimation;
    m_playerFrameIndex = 0;
    m_playerAnimationTime = 0.0f;
    m_obstacles.clear();
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
    }
    else
    {
        m_isGrounded = false;
    }

    // X 위치 고정
    m_playerX = FIXED_PLAYER_X;

    // 배경 이동
    m_scrollOffset += m_gameSpeed;
    while (m_scrollOffset >= GROUND_MARK_INTERVAL)
    {
        m_scrollOffset -= GROUND_MARK_INTERVAL;
    }
    m_backgroundScrollOffset += m_gameSpeed * BACKGROUND_SCROLL_RATE;
    while (m_backgroundScrollOffset >= BACKGROUND_SCROLL_WIDTH * BACKGROUND_IMAGE_COUNT)
    {
        m_backgroundScrollOffset -= BACKGROUND_SCROLL_WIDTH * BACKGROUND_IMAGE_COUNT;
    }

    // 거리 점수
    m_distanceScore += DISTANCE_GAIN_PER_FRAME;
    m_score = m_distanceScore;

    // 플레이어 애니메이션
    UpdatePlayerAnimation();

    // 속도 증가
    UpdateSpeed();

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
    RenderBackground(hDC);

    // 바닥 이미지가 들어오기 전까지 쓰는 임시 플레이 영역
    RECT groundRect = { 0, GROUND_TOP, SCREEN_WIDTH, SCREEN_HEIGHT };
    HBRUSH groundBrush = CreateSolidBrush(RGB(238, 238, 232));
    FillRect(hDC, &groundRect, groundBrush);
    DeleteObject(groundBrush);

    // 움직이는 바닥선
    RenderScrollGround(hDC);

    // 장애물
    RenderObstacles(hDC);

    // 플레이어
    RenderPlayer(hDC);

    // 충돌 박스
    RECT hitBoxRect = GetHitBoxRect();
    HPEN hitBoxPen = CreatePen(PS_DASH, 2, RGB(255, 80, 80));
    HBRUSH hollowBrush = static_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hDC, hollowBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, hitBoxPen));

    Rectangle(hDC, hitBoxRect.left, hitBoxRect.top, hitBoxRect.right, hitBoxRect.bottom);

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    DeleteObject(hitBoxPen);

    // 점수
    RenderScore(hDC);
}

void schoolrun::OnKeyDown(WPARAM wParam)
{
    // 더블점프
    if (wParam == VK_SPACE && !m_isSliding && m_jumpCount < 2)
    {
        m_velocityY = m_jumpPower;
        m_isGrounded = false;
        m_jumpCount++;
        m_playerAnimationState = (m_jumpCount >= 2) ? TwoJumpAnimation : OneJumpAnimation;
        m_playerFrameIndex = 0;
        m_playerAnimationTime = 0.0f;
        return;
    }

    // 슬라이드
    if (wParam == VK_DOWN && m_isGrounded)
    {
        m_isSliding = true;
        m_playerY = static_cast<float>(GROUND_TOP - PLAYER_SLIDE_HEIGHT);
    }
}

void schoolrun::OnKeyUp(WPARAM wParam)
{
    // 슬라이드 해제
    if (wParam == VK_DOWN && m_isSliding)
    {
        m_isSliding = false;
        m_playerY = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT);
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

void schoolrun::UpdateSpeed()
{
    // 일정 거리마다 속도 증가
    if (m_distanceScore >= m_nextSpeedUpDistance)
    {
        m_gameSpeed += SPEED_UP_AMOUNT;
        m_nextSpeedUpDistance += SPEED_UP_DISTANCE_STEP;
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

void schoolrun::UpdatePlayerAnimation()
{
    PlayerAnimationState nextState = GetPlayerAnimationState();
    if (m_playerAnimationState != nextState)
    {
        m_playerAnimationState = nextState;
        m_playerFrameIndex = 0;
        m_playerAnimationTime = 0.0f;
    }

    int frameCount = PLAYER_RUNNING_FRAME_COUNT;
    if (m_playerAnimationState == OneJumpAnimation)
    {
        frameCount = PLAYER_ONE_JUMP_FRAME_COUNT;
    }
    else if (m_playerAnimationState == TwoJumpAnimation)
    {
        frameCount = PLAYER_TWO_JUMP_FRAME_COUNT;
    }

    m_playerAnimationTime += UPDATE_DELTA_SECONDS;
    while (m_playerAnimationTime >= PLAYER_ANIMATION_INTERVAL)
    {
        m_playerAnimationTime -= PLAYER_ANIMATION_INTERVAL;
        m_playerFrameIndex = (m_playerFrameIndex + 1) % frameCount;
    }
}

void schoolrun::CheckObstacleCollisions()
{
    // 플레이어와 장애물 충돌
    RECT playerHitBox = GetHitBoxRect();

    for (size_t i = 0; i < m_obstacles.size();)
    {
        RECT obstacleRect = GetObstacleRect(m_obstacles[i]);
        RECT intersectRect = {};

        if (IntersectRect(&intersectRect, &playerHitBox, &obstacleRect))
        {
            m_obstacles.erase(m_obstacles.begin() + i);
            m_isFinished = true;
            return;
        }
        else
        {
            ++i;
        }
    }
}

void schoolrun::SpawnObstacle()
{
    // 장애물 생성
    Obstacle obstacle = {};

    if (m_nextObstacleType == 0)
    {
        obstacle.type = GroundObstacle;
        obstacle.width = GROUND_OBSTACLE_WIDTH;
        obstacle.height = GROUND_OBSTACLE_HEIGHT;
        obstacle.x = OBSTACLE_START_X;
        obstacle.y = static_cast<float>(GROUND_TOP - obstacle.height);
    }
    else
    {
        obstacle.type = AirObstacle;
        obstacle.width = AIR_OBSTACLE_WIDTH;
        obstacle.height = AIR_OBSTACLE_HEIGHT;
        obstacle.x = OBSTACLE_START_X;
        obstacle.y = static_cast<float>(GROUND_TOP - PLAYER_STAND_HEIGHT + 12);
    }

    m_obstacles.push_back(obstacle);
    m_nextObstacleType = 1 - m_nextObstacleType;
}

void schoolrun::RenderBackground(HDC hDC) const
{
    RECT screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    HBRUSH fallbackBrush = CreateSolidBrush(RGB(85, 180, 245));
    FillRect(hDC, &screenRect, fallbackBrush);
    DeleteObject(fallbackBrush);

    int scroll = static_cast<int>(m_backgroundScrollOffset);
    int startTile = scroll / BACKGROUND_SCROLL_WIDTH;
    int offsetX = scroll % BACKGROUND_SCROLL_WIDTH;

    for (int i = -1; i <= 2; ++i)
    {
        int imageIndex = (startTile + i) % BACKGROUND_IMAGE_COUNT;
        if (imageIndex < 0)
        {
            imageIndex += BACKGROUND_IMAGE_COUNT;
        }

        DrawGdiImage(
            hDC,
            m_backgroundImages[imageIndex],
            i * BACKGROUND_SCROLL_WIDTH - offsetX,
            0,
            BACKGROUND_SCROLL_WIDTH,
            SCREEN_HEIGHT);
    }
}

void schoolrun::RenderScrollGround(HDC hDC) const
{
    // 바닥선 표시
    HPEN markPen = CreatePen(PS_SOLID, 4, RGB(70, 70, 70));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hDC, markPen));

    for (int x = -GROUND_MARK_INTERVAL; x < SCREEN_WIDTH + GROUND_MARK_INTERVAL; x += GROUND_MARK_INTERVAL)
    {
        int drawX = x - static_cast<int>(m_scrollOffset);
        MoveToEx(hDC, drawX, GROUND_TOP + 68, nullptr);
        LineTo(hDC, drawX + 90, GROUND_TOP + 68);
    }

    SelectObject(hDC, oldPen);
    DeleteObject(markPen);
}

void schoolrun::RenderObstacles(HDC hDC) const
{
    // 장애물 출력
    for (const Obstacle& obstacle : m_obstacles)
    {
        RECT rect = GetObstacleRect(obstacle);

        if (obstacle.type == GroundObstacle)
        {
            if (m_groundObstacleImage != nullptr && m_groundObstacleImage->GetLastStatus() == Gdiplus::Ok)
            {
                DrawGdiImage(hDC, m_groundObstacleImage, rect.left, rect.top, obstacle.width, obstacle.height);
                continue;
            }

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
            if (m_airObstacleImage != nullptr && m_airObstacleImage->GetLastStatus() == Gdiplus::Ok)
            {
                DrawGdiImage(hDC, m_airObstacleImage, rect.left, rect.top, obstacle.width, obstacle.height);
                continue;
            }

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

void schoolrun::RenderPlayer(HDC hDC) const
{
    RECT playerRect = GetPlayerRect();
    Gdiplus::Image* playerImage = m_playerRunImage;
    int frameCount = PLAYER_RUNNING_FRAME_COUNT;

    if (m_playerAnimationState == OneJumpAnimation)
    {
        playerImage = m_playerOneJumpImage;
        frameCount = PLAYER_ONE_JUMP_FRAME_COUNT;
    }
    else if (m_playerAnimationState == TwoJumpAnimation)
    {
        playerImage = m_playerTwoJumpImage;
        frameCount = PLAYER_TWO_JUMP_FRAME_COUNT;
    }

    if (playerImage != nullptr && playerImage->GetLastStatus() == Gdiplus::Ok)
    {
        DrawGdiImageFrame(
            hDC,
            playerImage,
            playerRect.left,
            playerRect.top,
            playerRect.right - playerRect.left,
            playerRect.bottom - playerRect.top,
            m_playerFrameIndex,
            frameCount);
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

void schoolrun::DrawGdiImage(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight) const
{
    if (image == nullptr || image->GetLastStatus() != Gdiplus::Ok)
    {
        return;
    }

    Gdiplus::Graphics graphics(hDC);
    graphics.SetPageUnit(Gdiplus::UnitPixel);
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

    Gdiplus::Rect drawRect(drawX, drawY, drawWidth, drawHeight);
    graphics.DrawImage(image, drawRect);
}

void schoolrun::DrawGdiImageFrame(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight, int frameIndex, int frameCount) const
{
    if (image == nullptr || image->GetLastStatus() != Gdiplus::Ok || frameCount <= 0)
    {
        return;
    }

    int frameWidth = static_cast<int>(image->GetWidth()) / frameCount;
    int frameHeight = static_cast<int>(image->GetHeight());
    int safeFrame = frameIndex % frameCount;

    Gdiplus::Graphics graphics(hDC);
    graphics.SetPageUnit(Gdiplus::UnitPixel);
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

    Gdiplus::Rect drawRect(drawX, drawY, drawWidth, drawHeight);
    graphics.DrawImage(
        image,
        drawRect,
        safeFrame * frameWidth,
        0,
        frameWidth,
        frameHeight,
        Gdiplus::UnitPixel);
}

schoolrun::PlayerAnimationState schoolrun::GetPlayerAnimationState() const
{
    if (m_isGrounded)
    {
        return RunningAnimation;
    }

    return (m_jumpCount >= 2) ? TwoJumpAnimation : OneJumpAnimation;
}
