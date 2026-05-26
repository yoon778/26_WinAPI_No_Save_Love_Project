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
    m_deliver = {};

    m_gdiplusToken = 0;
    m_isGdiplusStarted = false;
    m_playerStandLeftImage = nullptr;
    m_playerStandRightImage = nullptr;
    m_playerRunningLeftImage = nullptr;
    m_playerRunningRightImage = nullptr;
    m_kakaoPopupImage = nullptr;
    m_ridderImage = nullptr;
    m_phoneImage = nullptr;
    m_reelsPhoneCache = nullptr;
    m_lastHitImageIndex = HIT_IMAGE_NONE;
    for (int i = 0; i < 3; i++)
    {
        m_foodImages[i] = nullptr;
        m_instarImages[i] = nullptr;
        m_reelsInstarCaches[i] = nullptr;
        m_reelsInstarCacheHeights[i] = 0;
    }
    for (int i = 0; i < 4; i++)
    {
        m_hitImages[i] = nullptr;
    }
}

avoidgame::~avoidgame()
{
    Release();
}

void avoidgame::Initialize()
{
    LoadImages();

    // 랜덤 시드
    static bool isRandomSeeded = false;
    if (!isRandomSeeded)
    {
        srand(GetTickCount());
        isRandomSeeded = true;
    }

    Reset();
}

void avoidgame::Release()
{
    // 리소스 정리
    DestroyImages();
    m_platforms.clear();
    m_attacks.clear();
    m_foods.clear();

    m_player = {};
    m_game = {};
    m_pattern = {};
    m_kakao = {};
    m_foodPattern = {};
    m_deliver = {};
    m_lastHitImageIndex = HIT_IMAGE_NONE;
}

void avoidgame::LoadImages()
{
    // GDI+ 시작
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

    // 리소스 로드
    if (m_playerStandLeftImage == nullptr)
    {
        m_playerStandLeftImage = new Gdiplus::Image(L"resource\\minigame4\\char_stand_left.png");
    }
    if (m_playerStandRightImage == nullptr)
    {
        m_playerStandRightImage = new Gdiplus::Image(L"resource\\minigame4\\char_stand_right.png");
    }
    if (m_playerRunningLeftImage == nullptr)
    {
        m_playerRunningLeftImage = new Gdiplus::Image(L"resource\\minigame4\\running_left.png");
        if (m_playerRunningLeftImage->GetLastStatus() != Gdiplus::Ok)
        {
            delete m_playerRunningLeftImage;
            m_playerRunningLeftImage = new Gdiplus::Image(L"resource\\minigame4\\char_running_left.png");
        }
    }
    if (m_playerRunningRightImage == nullptr)
    {
        m_playerRunningRightImage = new Gdiplus::Image(L"resource\\minigame4\\running_right.png");
        if (m_playerRunningRightImage->GetLastStatus() != Gdiplus::Ok)
        {
            delete m_playerRunningRightImage;
            m_playerRunningRightImage = new Gdiplus::Image(L"resource\\minigame4\\char_running_right.png");
        }
    }
    if (m_kakaoPopupImage == nullptr)
    {
        m_kakaoPopupImage = new Gdiplus::Image(L"resource\\minigame4\\lol_popup.png");
    }
    if (m_ridderImage == nullptr)
    {
        m_ridderImage = new Gdiplus::Image(L"resource\\minigame4\\ridder.png");
    }
    if (m_foodImages[0] == nullptr)
    {
        m_foodImages[0] = new Gdiplus::Image(L"resource\\minigame4\\jajang.png");
    }
    if (m_foodImages[1] == nullptr)
    {
        m_foodImages[1] = new Gdiplus::Image(L"resource\\minigame4\\chicken.png");
    }
    if (m_foodImages[2] == nullptr)
    {
        m_foodImages[2] = new Gdiplus::Image(L"resource\\minigame4\\pizza.png");
    }
    if (m_hitImages[HIT_IMAGE_GAME] == nullptr)
    {
        m_hitImages[HIT_IMAGE_GAME] = new Gdiplus::Image(L"resource\\minigame4\\hit_game.png");
    }
    if (m_hitImages[HIT_IMAGE_REELS] == nullptr)
    {
        m_hitImages[HIT_IMAGE_REELS] = new Gdiplus::Image(L"resource\\minigame4\\hit_reels.png");
    }
    if (m_hitImages[HIT_IMAGE_FOOD] == nullptr)
    {
        m_hitImages[HIT_IMAGE_FOOD] = new Gdiplus::Image(L"resource\\minigame4\\hit_food.png");
    }
    if (m_hitImages[HIT_IMAGE_RIDDER] == nullptr)
    {
        m_hitImages[HIT_IMAGE_RIDDER] = new Gdiplus::Image(L"resource\\minigame4\\hit_ridder.png");
    }
    if (m_phoneImage == nullptr)
    {
        m_phoneImage = new Gdiplus::Image(L"resource\\minigame4\\phone.png");
    }
    if (m_instarImages[0] == nullptr)
    {
        m_instarImages[0] = new Gdiplus::Image(L"resource\\minigame4\\instar_1.png");
    }
    if (m_instarImages[1] == nullptr)
    {
        m_instarImages[1] = new Gdiplus::Image(L"resource\\minigame4\\instar_2.png");
    }
    if (m_instarImages[2] == nullptr)
    {
        m_instarImages[2] = new Gdiplus::Image(L"resource\\minigame4\\instar_3.png");
    }

    BuildReelsImageCache();
}

void avoidgame::DestroyImages()
{
    // 리소스 해제
    DestroyReelsImageCache();

    if (m_playerStandLeftImage != nullptr)
    {
        delete m_playerStandLeftImage;
        m_playerStandLeftImage = nullptr;
    }
    if (m_playerStandRightImage != nullptr)
    {
        delete m_playerStandRightImage;
        m_playerStandRightImage = nullptr;
    }
    if (m_playerRunningLeftImage != nullptr)
    {
        delete m_playerRunningLeftImage;
        m_playerRunningLeftImage = nullptr;
    }
    if (m_playerRunningRightImage != nullptr)
    {
        delete m_playerRunningRightImage;
        m_playerRunningRightImage = nullptr;
    }
    if (m_kakaoPopupImage != nullptr)
    {
        delete m_kakaoPopupImage;
        m_kakaoPopupImage = nullptr;
    }
    if (m_ridderImage != nullptr)
    {
        delete m_ridderImage;
        m_ridderImage = nullptr;
    }
    if (m_phoneImage != nullptr)
    {
        delete m_phoneImage;
        m_phoneImage = nullptr;
    }
    for (int i = 0; i < 3; i++)
    {
        if (m_foodImages[i] != nullptr)
        {
            delete m_foodImages[i];
            m_foodImages[i] = nullptr;
        }
        if (m_instarImages[i] != nullptr)
        {
            delete m_instarImages[i];
            m_instarImages[i] = nullptr;
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (m_hitImages[i] != nullptr)
        {
            delete m_hitImages[i];
            m_hitImages[i] = nullptr;
        }
    }

    if (m_isGdiplusStarted)
    {
        Gdiplus::GdiplusShutdown(m_gdiplusToken);
        m_gdiplusToken = 0;
        m_isGdiplusStarted = false;
    }
}

void avoidgame::BuildReelsImageCache()
{
    // 릴스 원본 축소
    DestroyReelsImageCache();

    if (m_phoneImage == nullptr || m_phoneImage->GetLastStatus() != Gdiplus::Ok)
    {
        return;
    }

    const int phoneSourceX = 238;
    const int phoneSourceY = 36;
    const int phoneSourceW = 629;
    const int phoneSourceH = 1397;

    int cacheWidth = REELS_COLUMN_WIDTH;
    int cacheHeight = static_cast<int>(cacheWidth * static_cast<float>(phoneSourceH) / static_cast<float>(phoneSourceW));
    int maxCacheHeight = SCREEN_HEIGHT - FLOOR_HEIGHT - 20;
    if (cacheHeight > maxCacheHeight)
    {
        cacheHeight = maxCacheHeight;
        cacheWidth = static_cast<int>(cacheHeight * static_cast<float>(phoneSourceW) / static_cast<float>(phoneSourceH));
    }

    m_reelsPhoneCache = new Gdiplus::Bitmap(cacheWidth, cacheHeight, PixelFormat32bppARGB);
    Gdiplus::Graphics phoneGraphics(m_reelsPhoneCache);
    phoneGraphics.SetPageUnit(Gdiplus::UnitPixel);
    phoneGraphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    phoneGraphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    phoneGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    phoneGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    phoneGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));
    phoneGraphics.DrawImage(
        m_phoneImage,
        Gdiplus::Rect(0, 0, cacheWidth, cacheHeight),
        phoneSourceX,
        phoneSourceY,
        phoneSourceW,
        phoneSourceH,
        Gdiplus::UnitPixel);

    for (int i = 0; i < 3; i++)
    {
        if (m_instarImages[i] == nullptr || m_instarImages[i]->GetLastStatus() != Gdiplus::Ok)
        {
            continue;
        }

        int instarHeight = static_cast<int>(cacheWidth * static_cast<float>(m_instarImages[i]->GetHeight()) / static_cast<float>(m_instarImages[i]->GetWidth()));
        m_reelsInstarCacheHeights[i] = instarHeight;
        m_reelsInstarCaches[i] = new Gdiplus::Bitmap(cacheWidth, instarHeight, PixelFormat32bppARGB);

        Gdiplus::Graphics instarGraphics(m_reelsInstarCaches[i]);
        instarGraphics.SetPageUnit(Gdiplus::UnitPixel);
        instarGraphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
        instarGraphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
        instarGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        instarGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
        instarGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));
        instarGraphics.DrawImage(m_instarImages[i], Gdiplus::Rect(0, 0, cacheWidth, instarHeight));
    }
}

void avoidgame::DestroyReelsImageCache()
{
    // 릴스 캐시 해제
    if (m_reelsPhoneCache != nullptr)
    {
        delete m_reelsPhoneCache;
        m_reelsPhoneCache = nullptr;
    }

    for (int i = 0; i < 3; i++)
    {
        if (m_reelsInstarCaches[i] != nullptr)
        {
            delete m_reelsInstarCaches[i];
            m_reelsInstarCaches[i] = nullptr;
        }
        m_reelsInstarCacheHeights[i] = 0;
    }
}

void avoidgame::DrawGdiImage(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight)
{
    // GDI+ 이미지 출력
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

void avoidgame::DrawGdiImageFrame(HDC hDC, Gdiplus::Image* image, int drawX, int drawY, int drawWidth, int drawHeight, int frameIndex, int frameCount)
{
    // GDI+ 프레임 출력
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
    m_lastHitImageIndex = HIT_IMAGE_NONE;
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
        if (m_game.invincibleTimer == 0.0f)
        {
            m_lastHitImageIndex = HIT_IMAGE_NONE;
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
    DrawDeliver(hDC);
    DrawFoodDrops(hDC);
    RenderPlatforms(hDC);
    DrawAttacks(hDC);
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
        m_player.faceRight = false;
        break;

    case VK_RIGHT:
        m_player.moveRight = true;
        m_player.faceRight = true;
        break;

    case VK_DOWN:
        m_player.moveDown = true;
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

    case VK_DOWN:
        m_player.moveDown = false;
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

    // 이동 값
    m_player.gravity = 2.0f;
    m_player.jumpPower = 28.0f;
    m_player.moveSpeed = 12.0f;

    // 물리 초기화
    m_player.velocityX = 0.0f;
    m_player.velocityY = 0.0f;
    m_player.moveLeft = false;
    m_player.moveRight = false;
    m_player.moveDown = false;
    m_player.isGrounded = true;
    m_player.faceRight = true;
    m_player.animationTimer = 0.0f;
    m_player.animationFrame = 0;
    m_player.jumpCount = 0;
    m_player.dropThroughTimer = 0.0f;
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
        m_player.faceRight = false;
    }
    if (m_player.moveRight)
    {
        m_player.velocityX += m_player.moveSpeed;
        m_player.faceRight = true;
    }

    // 중력 적용
    m_player.velocityY += m_player.gravity;

    // 발판 통과 시간
    if (m_player.dropThroughTimer > 0.0f)
    {
        m_player.dropThroughTimer -= UPDATE_DELTA_SECONDS;
        if (m_player.dropThroughTimer < 0.0f)
        {
            m_player.dropThroughTimer = 0.0f;
        }
    }

    // 위치 적용
    m_player.x += m_player.velocityX;
    m_player.y += m_player.velocityY;

    ClampPlayerPosition();

    // 착지 판정
    m_player.isGrounded = false;
    HandleLanding(previousY);

    // 걷기 애니메이션
    if (m_player.velocityX != 0.0f)
    {
        m_player.animationTimer += UPDATE_DELTA_SECONDS;
        if (m_player.animationTimer >= PLAYER_ANIMATION_INTERVAL)
        {
            m_player.animationTimer = 0.0f;
            m_player.animationFrame = (m_player.animationFrame + 1) % PLAYER_RUNNING_FRAME_COUNT;
        }
    }
    else
    {
        m_player.animationTimer = 0.0f;
        m_player.animationFrame = 0;
    }
}

void avoidgame::RenderPlayer(HDC hDC)
{
    RECT playerRect =
    {
        static_cast<LONG>(m_player.x),
        static_cast<LONG>(m_player.y),
        static_cast<LONG>(m_player.x + PLAYER_WIDTH),
        static_cast<LONG>(m_player.y + PLAYER_HEIGHT)
    };
    bool isRunning = (m_player.velocityX != 0.0f);
    Gdiplus::Image* playerImage = nullptr;

    if (m_game.invincibleTimer > 0.0f &&
        m_lastHitImageIndex >= 0 &&
        m_lastHitImageIndex < 4 &&
        m_hitImages[m_lastHitImageIndex] != nullptr &&
        m_hitImages[m_lastHitImageIndex]->GetLastStatus() == Gdiplus::Ok)
    {
        playerImage = m_hitImages[m_lastHitImageIndex];
        isRunning = false;
    }
    else if (isRunning)
    {
        playerImage = m_player.faceRight ? m_playerRunningRightImage : m_playerRunningLeftImage;
    }
    else
    {
        playerImage = m_player.faceRight ? m_playerStandRightImage : m_playerStandLeftImage;
    }

    if (playerImage != nullptr && playerImage->GetLastStatus() == Gdiplus::Ok)
    {
        // stand와 running 크기 통일
        int drawWidth = PLAYER_WIDTH;
        int drawHeight = PLAYER_HEIGHT;
        int sourceWidth = static_cast<int>(playerImage->GetWidth());
        int sourceHeight = static_cast<int>(playerImage->GetHeight());
        if (isRunning)
        {
            sourceWidth = sourceWidth / PLAYER_RUNNING_FRAME_COUNT;
        }

        if (sourceHeight > 0 && sourceWidth != sourceHeight)
        {
            drawWidth = static_cast<int>((drawHeight * sourceWidth + sourceHeight / 2) / sourceHeight);
        }

        int drawX = playerRect.left + ((PLAYER_WIDTH - drawWidth) / 2);
        int drawY = playerRect.bottom - drawHeight;

        if (isRunning)
        {
            DrawGdiImageFrame(hDC, playerImage, drawX, drawY, drawWidth, drawHeight, m_player.animationFrame, PLAYER_RUNNING_FRAME_COUNT);
        }
        else
        {
            DrawGdiImage(hDC, playerImage, drawX, drawY, drawWidth, drawHeight);
        }

        return;
    }

    // 이미지 실패 시 대체 출력
    COLORREF playerColor = RGB(255, 170, 190);

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
    // 아래+점프는 발판 통과
    float playerBottom = m_player.y + PLAYER_HEIGHT;
    float floorTop = static_cast<float>(SCREEN_HEIGHT - FLOOR_HEIGHT);
    if (m_player.moveDown && m_player.isGrounded && playerBottom < floorTop - 1.0f)
    {
        m_player.y += 10.0f;
        m_player.velocityY = m_player.gravity;
        m_player.isGrounded = false;
        m_player.jumpCount = 1;
        m_player.dropThroughTimer = 0.25f;
        return;
    }

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
    if (m_player.dropThroughTimer > 0.0f)
    {
        return;
    }

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
        static_cast<LONG>(m_player.x + PLAYER_HITBOX_MARGIN_X),
        static_cast<LONG>(m_player.y + PLAYER_HITBOX_MARGIN_TOP),
        static_cast<LONG>(m_player.x + PLAYER_WIDTH - PLAYER_HITBOX_MARGIN_X),
        static_cast<LONG>(m_player.y + PLAYER_HEIGHT - PLAYER_HITBOX_MARGIN_BOTTOM)
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

    m_pattern.currentPattern = PATTERN_NONE;
    m_pattern.nextPattern = 0;
    m_pattern.waitTimer = 0.8f;

    m_kakao.comboTimer = 0.0f;
    m_kakao.comboCount = 0;

    m_foodPattern.spawnTimer = 0.0f;
    m_foodPattern.patternTimer = 0.0f;

    m_deliver = {};
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
                StartDeliverPattern();
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
    else if (m_pattern.currentPattern == PATTERN_DELIVER)
    {
        UpdateDeliverPattern();
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
        if (m_kakao.comboCount <= 0 && m_attacks.empty())
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
    else if (m_pattern.currentPattern == PATTERN_DELIVER)
    {
        if (!m_deliver.isWarning && !m_deliver.isActive)
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
}

void avoidgame::UpdateKakaoCombo()
{
    // 카톡 연타 생성
    if (m_kakao.comboCount <= 0)
    {
        return;
    }

    m_kakao.comboTimer -= UPDATE_DELTA_SECONDS;
    if (m_kakao.comboTimer <= 0.0f)
    {
        SpawnKakaoAttack();
        m_kakao.comboCount--;
        m_kakao.comboTimer = KAKAO_COMBO_INTERVAL;
    }
}

void avoidgame::SpawnKakaoAttack()
{
    // 플레이어 위치 조준
    int attackWidth = KAKAO_POPUP_SIZE;
    int attackHeight = KAKAO_POPUP_SIZE;

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

void avoidgame::StartReelsPattern()
{
    // 릴스 패턴 시작
    m_pattern.currentPattern = PATTERN_REELS;
    SpawnReelsAttack();
}

void avoidgame::SpawnReelsAttack()
{
    // 3개 릴스 공격
    const int phoneSourceW = 629;
    const int phoneSourceH = 1397;
    int reelWidth = REELS_COLUMN_WIDTH;
    int reelHeight = static_cast<int>(reelWidth * static_cast<float>(phoneSourceH) / static_cast<float>(phoneSourceW));
    int maxReelHeight = SCREEN_HEIGHT - FLOOR_HEIGHT - 20;
    if (reelHeight > maxReelHeight)
    {
        reelHeight = maxReelHeight;
        reelWidth = static_cast<int>(reelHeight * static_cast<float>(phoneSourceW) / static_cast<float>(phoneSourceH));
    }

    int safeGap = (SCREEN_WIDTH - (reelWidth * REELS_COLUMN_COUNT)) / (REELS_COLUMN_COUNT + 1);
    int reelTop = (SCREEN_HEIGHT - FLOOR_HEIGHT - reelHeight) / 2;

    for (int i = 0; i < REELS_COLUMN_COUNT; i++)
    {
        int reelLeft = safeGap + i * (reelWidth + safeGap);

        AttackWarning attack = {};
        attack.rect = RECT{ reelLeft, reelTop, reelLeft + reelWidth, reelTop + reelHeight };
        attack.warningTime = REELS_WARNING_DURATION;
        attack.attackTime = REELS_ATTACK_DURATION;
        attack.isAttackActive = false;
        attack.messageIndex = i;
        attack.attackType = ATTACK_TYPE_REELS;
        attack.scrollY = 0.0f;

        m_attacks.push_back(attack);
    }
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
    food.foodType = rand() % 3;

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
            DamagePlayer(HIT_IMAGE_FOOD);
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
    for (int i = 0; i < static_cast<int>(m_foods.size()); i++)
    {
        const FoodDrop& food = m_foods[i];
        RECT foodRect = GetFoodRect(food);
        int foodType = food.foodType;
        if (foodType < 0 || foodType >= 3)
        {
            foodType = 0;
        }

        if (m_foodImages[foodType] != nullptr && m_foodImages[foodType]->GetLastStatus() == Gdiplus::Ok)
        {
            DrawGdiImage(hDC, m_foodImages[foodType], foodRect.left, foodRect.top, foodRect.right - foodRect.left, foodRect.bottom - foodRect.top);
            continue;
        }

        HBRUSH foodBrush = CreateSolidBrush(RGB(255, 210, 90));
        HPEN foodPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));

        HGDIOBJ oldBrush = SelectObject(hDC, foodBrush);
        HGDIOBJ oldPen = SelectObject(hDC, foodPen);

        Ellipse(hDC, foodRect.left, foodRect.top, foodRect.right, foodRect.bottom);

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);
        DeleteObject(foodBrush);
        DeleteObject(foodPen);
    }
}

RECT avoidgame::GetFoodRect(const FoodDrop& food) const
{
    // 음식 충돌 영역
    RECT rc =
    {
        static_cast<LONG>(food.x),
        static_cast<LONG>(food.y),
        static_cast<LONG>(food.x + FOOD_SIZE),
        static_cast<LONG>(food.y + FOOD_SIZE)
    };

    return rc;
}

void avoidgame::StartDeliverPattern()
{
    // 배달 패턴 시작
    m_pattern.currentPattern = PATTERN_DELIVER;

    int halfHeight = (SCREEN_HEIGHT - FLOOR_HEIGHT) / 2;
    int top = halfHeight;
    int bottom = SCREEN_HEIGHT - FLOOR_HEIGHT;
    int riderSize = halfHeight;
    int riderY = bottom - riderSize;

    m_deliver.warningRect = RECT{ 0, top, SCREEN_WIDTH, bottom };
    m_deliver.riderRect = RECT{ SCREEN_WIDTH, riderY, SCREEN_WIDTH + riderSize, riderY + riderSize };
    m_deliver.warningTimer = DELIVER_WARNING_DURATION;
    m_deliver.activeTimer = DELIVER_ACTIVE_DURATION;
    m_deliver.isWarning = true;
    m_deliver.isActive = false;
}

void avoidgame::UpdateDeliverPattern()
{
    // 경고 후 배달 이동
    if (m_deliver.isWarning)
    {
        m_deliver.warningTimer -= UPDATE_DELTA_SECONDS;
        if (m_deliver.warningTimer <= 0.0f)
        {
            m_deliver.warningTimer = 0.0f;
            m_deliver.isWarning = false;
            m_deliver.isActive = true;
        }
        return;
    }

    if (!m_deliver.isActive)
    {
        return;
    }

    m_deliver.activeTimer -= UPDATE_DELTA_SECONDS;
    m_deliver.riderRect.left -= static_cast<LONG>(DELIVER_SPEED);
    m_deliver.riderRect.right -= static_cast<LONG>(DELIVER_SPEED);

    if (IsRectOverlap(GetPlayerRect(), m_deliver.riderRect))
    {
        DamagePlayer(HIT_IMAGE_RIDDER);
    }

    if (m_deliver.riderRect.right < 0 || m_deliver.activeTimer <= 0.0f)
    {
        m_deliver.activeTimer = 0.0f;
        m_deliver.isActive = false;
    }
}

void avoidgame::DrawDeliver(HDC hDC)
{
    // 배달 경고/기사 출력
    if (!m_deliver.isWarning && !m_deliver.isActive)
    {
        return;
    }

    SetBkMode(hDC, TRANSPARENT);

    if (m_deliver.isWarning)
    {
        HBRUSH warningBrush = CreateSolidBrush(RGB(80, 0, 0));
        HPEN warningPen = CreatePen(PS_SOLID, 5, RGB(255, 60, 60));

        HGDIOBJ oldBrush = SelectObject(hDC, warningBrush);
        HGDIOBJ oldPen = SelectObject(hDC, warningPen);

        Rectangle(hDC, m_deliver.warningRect.left, m_deliver.warningRect.top, m_deliver.warningRect.right, m_deliver.warningRect.bottom);

        SetTextColor(hDC, RGB(255, 150, 150));
        const wchar_t* warningText = L"배달 접근";
        TextOutW(hDC, 820, m_deliver.warningRect.top + 40, warningText, lstrlenW(warningText));

        SelectObject(hDC, oldBrush);
        SelectObject(hDC, oldPen);
        DeleteObject(warningBrush);
        DeleteObject(warningPen);
        return;
    }

    if (m_ridderImage != nullptr && m_ridderImage->GetLastStatus() == Gdiplus::Ok)
    {
        DrawGdiImage(hDC, m_ridderImage, m_deliver.riderRect.left, m_deliver.riderRect.top, m_deliver.riderRect.right - m_deliver.riderRect.left, m_deliver.riderRect.bottom - m_deliver.riderRect.top);
        return;
    }

    HBRUSH riderBrush = CreateSolidBrush(RGB(255, 190, 80));
    FillRect(hDC, &m_deliver.riderRect, riderBrush);
    DeleteObject(riderBrush);
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
            if (attack.attackType == ATTACK_TYPE_REELS)
            {
                attack.scrollY -= REELS_SCROLL_SPEED * UPDATE_DELTA_SECONDS;
            }

            bool isHit = false;
            if (attack.attackType == ATTACK_TYPE_KAKAO)
            {
                isHit = IsRectCircleOverlap(playerRect, attack.rect);
            }
            else
            {
                isHit = IsRectOverlap(playerRect, attack.rect);
            }

            if (isHit)
            {
                int hitImageIndex = (attack.attackType == ATTACK_TYPE_REELS) ? HIT_IMAGE_REELS : HIT_IMAGE_GAME;
                DamagePlayer(hitImageIndex);
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

        if (attack.attackType == ATTACK_TYPE_KAKAO)
        {
            Ellipse(hDC, attack.rect.left, attack.rect.top, attack.rect.right, attack.rect.bottom);
        }
        else
        {
            Rectangle(hDC, attack.rect.left, attack.rect.top, attack.rect.right, attack.rect.bottom);
        }

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
            DrawReelsScroll(hDC, attack);
            continue;
        }

        // 카톡 팝업 이미지
        DrawKakaoPopup(hDC, attack);
    }
}

void avoidgame::DrawReelsScroll(HDC hDC, const AttackWarning& attack)
{
    // 이미지 실패 대체
    if (m_reelsPhoneCache == nullptr || m_reelsPhoneCache->GetLastStatus() != Gdiplus::Ok)
    {
        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, RGB(255, 255, 255));
        const wchar_t* fallbackText = L"REELS";
        TextOutW(hDC, attack.rect.left + 40, attack.rect.top + 40, fallbackText, lstrlenW(fallbackText));
        return;
    }

    const int phoneSourceH = 1397;
    const int phoneHeaderH = 169;
    const int phoneBottomY = 1261;

    // 폰 위치
    int phoneX = attack.rect.left;
    int phoneY = attack.rect.top;
    int phoneWidth = attack.rect.right - attack.rect.left;
    int phoneHeight = attack.rect.bottom - attack.rect.top;

    // 화면 안쪽
    float screenX = static_cast<float>(phoneX);
    float screenY = static_cast<float>(phoneY) + phoneHeight * (static_cast<float>(phoneHeaderH) / phoneSourceH);
    float screenW = static_cast<float>(phoneWidth);
    float screenH = phoneHeight * (static_cast<float>(phoneBottomY - phoneHeaderH) / phoneSourceH);
    float drawY = screenY + attack.scrollY;

    Gdiplus::Graphics graphics(hDC);
    graphics.SetPageUnit(Gdiplus::UnitPixel);
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    graphics.SetClip(Gdiplus::RectF(screenX, screenY, screenW, screenH));

    // 보이는 릴스만 출력
    for (int i = 0; i < 9 && drawY < screenY + screenH; i++)
    {
        int imageIndex = (attack.messageIndex + i) % 3;
        Gdiplus::Bitmap* instarImage = m_reelsInstarCaches[imageIndex];
        if (instarImage == nullptr || instarImage->GetLastStatus() != Gdiplus::Ok)
        {
            continue;
        }

        float instarH = static_cast<float>(m_reelsInstarCacheHeights[imageIndex]);
        if (drawY + instarH >= screenY)
        {
            Gdiplus::RectF instarRect(screenX, drawY, screenW, instarH);
            graphics.DrawImage(instarImage, instarRect);
        }
        drawY += instarH;
    }

    graphics.ResetClip();

    // 폰 틀은 마지막
    Gdiplus::RectF phoneRect(static_cast<float>(phoneX), static_cast<float>(phoneY), static_cast<float>(phoneWidth), static_cast<float>(phoneHeight));
    graphics.DrawImage(m_reelsPhoneCache, phoneRect);
}

void avoidgame::DrawKakaoPopup(HDC hDC, const AttackWarning& attack)
{
    // lol_popup 이미지
    if (m_kakaoPopupImage != nullptr && m_kakaoPopupImage->GetLastStatus() == Gdiplus::Ok)
    {
        DrawGdiImage(hDC, m_kakaoPopupImage, attack.rect.left, attack.rect.top, attack.rect.right - attack.rect.left, attack.rect.bottom - attack.rect.top);
        return;
    }

    // 이미지 실패 시 대체 출력
    HBRUSH attackBrush = CreateSolidBrush(RGB(255, 245, 120));
    HPEN attackPen = CreatePen(PS_SOLID, 4, RGB(255, 80, 80));

    HGDIOBJ oldBrush = SelectObject(hDC, attackBrush);
    HGDIOBJ oldPen = SelectObject(hDC, attackPen);

    Rectangle(hDC, attack.rect.left, attack.rect.top, attack.rect.right, attack.rect.bottom);

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(30, 30, 30));
    const wchar_t* fallbackText = L"LOL";
    TextOutW(hDC, attack.rect.left + 62, attack.rect.top + 78, fallbackText, lstrlenW(fallbackText));

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(attackBrush);
    DeleteObject(attackPen);
}

bool avoidgame::IsRectOverlap(const RECT& a, const RECT& b) const
{
    // 사각형 충돌
    return a.left < b.right &&
        a.right > b.left &&
        a.top < b.bottom &&
        a.bottom > b.top;
}

bool avoidgame::IsRectCircleOverlap(const RECT& rect, const RECT& circleRect) const
{
    // 사각형과 원 충돌
    int circleCenterX = (circleRect.left + circleRect.right) / 2;
    int circleCenterY = (circleRect.top + circleRect.bottom) / 2;
    int radius = (circleRect.right - circleRect.left) / 2;

    int closestX = circleCenterX;
    if (closestX < rect.left)
    {
        closestX = rect.left;
    }
    else if (closestX > rect.right)
    {
        closestX = rect.right;
    }

    int closestY = circleCenterY;
    if (closestY < rect.top)
    {
        closestY = rect.top;
    }
    else if (closestY > rect.bottom)
    {
        closestY = rect.bottom;
    }

    int deltaX = circleCenterX - closestX;
    int deltaY = circleCenterY - closestY;

    return (deltaX * deltaX) + (deltaY * deltaY) <= radius * radius;
}

void avoidgame::DamagePlayer(int hitImageIndex)
{
    // 무적 중 피격 무시
    if (m_game.invincibleTimer > 0.0f)
    {
        return;
    }

    if (hitImageIndex >= 0 && hitImageIndex < 4)
    {
        m_lastHitImageIndex = hitImageIndex;
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
