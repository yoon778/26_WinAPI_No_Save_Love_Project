#include "RhythmMiniGame.h"
#include <cstdlib>
#include <cmath>

RhythmMiniGame::RhythmMiniGame()
{
    score = 0;
    isGameOver = false;

    mouseX = 0;
    mouseY = 0;
    isMouseDown = false;

    screenWidth = 1920;
    screenHeight = 1080;
}


void RhythmMiniGame::Init()
{
    hitCircleImg.Load(L"resource\\minigame2\\hitcircle\\hitcircle.png");
    hitCircleOverlayImg.Load(L"resource\\minigame2\\hitcircle\\hitcircleoverlay.png");
    approachCircleImg.Load(L"resource\\minigame2\\hitcircle\\approachcircle.png");
    
    hit300Img.Load(L"resource\\minigame2\\judge\\hit300.png");
    hit100Img.Load(L"resource\\minigame2\\judge\\hit100.png");
    hit50Img.Load(L"resource\\minigame2\\judge\\hit50.png");
    hit0Img.Load(L"resource\\minigame2\\judge\\hit0.png");

    sliderStartCircleImg.Load(L"resource\\minigame2\\slider\\sliderstartcircle.png");
    sliderStartCircleOverlayImg.Load(L"resource\\minigame2\\slider\\sliderstartcircleoverlay.png");
    sliderBallImg.Load(L"resource\\minigame2\\slider\\sliderb.png");
    sliderFollowCircleImg.Load(
        L"resource\\minigame2\\slider\\sliderfollowcircle.png"
    );

    PremultiplyAlpha(sliderFollowCircleImg);
    PremultiplyAlpha(sliderStartCircleImg);
    PremultiplyAlpha(sliderStartCircleOverlayImg);
    PremultiplyAlpha(sliderBallImg);

    PremultiplyAlpha(approachCircleImg);
    PremultiplyAlpha(hitCircleImg);
    PremultiplyAlpha(hitCircleOverlayImg);

    PremultiplyAlpha(hit300Img);
    PremultiplyAlpha(hit100Img);
    PremultiplyAlpha(hit50Img);
    PremultiplyAlpha(hit0Img);

    lastJudge = JUDGE_NONE;
    judgeDisplayStartTime = 0;

    score = 0;
    isGameOver = false;

    judgeX = 0;
    judgeY = 0;

    mouseX = 0;
    mouseY = 0;
    isMouseDown = false;

    hitCircleCount = 0;
    sliderCount = 0;
   

    // 히트서클 초기화
    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        hitCircles[i].isActive = false;
        hitCircles[i].isJudged = false;

        hitCircles[i].x = 0;
        hitCircles[i].y = 0;

        hitCircles[i].spawnTime = 0;
        hitCircles[i].hitTime = 0;
    }

    // 슬라이더 초기화
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        sliders[i].isActive = false;
        sliders[i].isStarted = false;
        sliders[i].isFinished = false;
        sliders[i].isFailed = false;

        sliders[i].startX = 0;
        sliders[i].startY = 0;
        sliders[i].endX = 0;
        sliders[i].endY = 0;

        sliders[i].spawnTime = 0;
        sliders[i].hitTime = 0;
        sliders[i].duration = 0;
        sliders[i].slideStartTime = 0;
        sliders[i].isTrackingSuccess = false;
    }

    lastHitCircleSpawnTime = GetTickCount();
    hitCircleSpawnInterval = 1200;
    CreateSlider(600, 700, 1250, 700);
}

void RhythmMiniGame::Update()
{
    DWORD currentTime = GetTickCount();

    // =========================
    // 일정 시간마다 히트서클 생성
    // =========================
    if (currentTime - lastHitCircleSpawnTime >= hitCircleSpawnInterval)
    {
        int marginX = 250;
        int marginY = 200;

        int randomX =
            marginX + rand() % (screenWidth - marginX * 2);

        int randomY =
            marginY + rand() % (screenHeight - marginY * 2);

        CreateHitCircle(randomX, randomY);

        lastHitCircleSpawnTime = currentTime;
    }



    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        if (hitCircles[i].isActive == true)
        {
            // 눌러야 할 시간보다 250ms 이상 늦으면 Miss
            if (currentTime > hitCircles[i].hitTime + 250)
            {
                hitCircles[i].isActive = false;
                hitCircles[i].isJudged = true;
                hitCircleCount--;

                judgeX = hitCircles[i].x;
                judgeY = hitCircles[i].y;
                lastJudge = JUDGE_MISS;
                judgeDisplayStartTime = currentTime;
            }
        }
    }
    // =========================
    // 슬라이더 시작 타이밍 Miss 처리
    // =========================
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true &&
            sliders[i].isStarted == false &&
            sliders[i].isFinished == false)
        {
            if (currentTime > sliders[i].hitTime + 250)
            {
                sliders[i].isActive = false;
                sliders[i].isFailed = true;
                sliders[i].isFinished = true;
                sliderCount--;

                lastJudge = JUDGE_MISS;
                judgeX = sliders[i].startX;
                judgeY = sliders[i].startY;
                judgeDisplayStartTime = currentTime;
            }
        }
    }

    // =========================
// 슬라이더 추적 판정
// =========================
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true &&
            sliders[i].isStarted == true &&
            sliders[i].isFinished == false &&
            sliders[i].isFailed == false)
        {
            DWORD elapsedTime =
                currentTime - sliders[i].slideStartTime;

            double progress =
                (double)elapsedTime / sliders[i].duration;

            if (progress < 0.0)
                progress = 0.0;

            if (progress > 1.0)
                progress = 1.0;

            int ballX =
                (int)(sliders[i].startX +
                    (sliders[i].endX - sliders[i].startX) * progress);

            int ballY =
                (int)(sliders[i].startY +
                    (sliders[i].endY - sliders[i].startY) * progress);

            // 마우스 버튼을 떼면 실패
            if (isMouseDown == false)
            {
                sliders[i].isFailed = true;
                sliders[i].isTrackingSuccess = false;
                continue;
            }

            // 마우스가 슬라이더 볼 주변에 있는지 확인
            int dx = mouseX - ballX;
            int dy = mouseY - ballY;

            int followRadius = sliderFollowCircleImg.GetWidth() / 2;

            if (dx * dx + dy * dy > followRadius * followRadius)
            {
                sliders[i].isFailed = true;
                sliders[i].isTrackingSuccess = false;
            }
        }
    }

    // =========================
    // 슬라이더 볼 이동 완료 처리
    // =========================
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true &&
            sliders[i].isStarted == true &&
            sliders[i].isFinished == false)
        {
            DWORD elapsedTime =
                currentTime - sliders[i].slideStartTime;

            if (elapsedTime >= sliders[i].duration)
            {
                sliders[i].isFinished = true;
                sliders[i].isActive = false;
                sliderCount--;

                judgeX = sliders[i].endX;
                judgeY = sliders[i].endY;
                judgeDisplayStartTime = currentTime;

                if (sliders[i].isTrackingSuccess == true &&
                    sliders[i].isFailed == false)
                {
                    lastJudge = JUDGE_PERFECT;
                    score += 300;
                }
                else
                {
                    lastJudge = JUDGE_MISS;
                }
            }
        }
    }
}

void RhythmMiniGame::Render(HDC hDC)
{
    // 임시 화면 확인용 텍스트
    const wchar_t* title = L"Rhythm MiniGame";
    TextOut(hDC, 50, 50, title, lstrlen(title));

    wchar_t scoreText[100];
    wsprintf(scoreText, L"Score : %d", score);
    TextOut(hDC, 50, 90, scoreText, lstrlen(scoreText));

    // =========================
    // 슬라이더 그리기
    // =========================
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true)
        {
            int startX = sliders[i].startX;
            int startY = sliders[i].startY;
            int endX = sliders[i].endX;
            int endY = sliders[i].endY;

            // -------------------------
            // 슬라이더 몸체
            // -------------------------
            HPEN sliderOutlinePen = CreatePen(PS_SOLID, 90, RGB(255, 255, 255));
            HPEN oldPen = (HPEN)SelectObject(hDC, sliderOutlinePen);

            MoveToEx(hDC, startX, startY, NULL);
            LineTo(hDC, endX, endY);

            SelectObject(hDC, oldPen);
            DeleteObject(sliderOutlinePen);

            HPEN sliderInnerPen = CreatePen(PS_SOLID, 72, RGB(180, 180, 180));
            oldPen = (HPEN)SelectObject(hDC, sliderInnerPen);

            MoveToEx(hDC, startX, startY, NULL);
            LineTo(hDC, endX, endY);

            SelectObject(hDC, oldPen);
            DeleteObject(sliderInnerPen);

            // -------------------------
            // 슬라이더 시작 원의 접근 원
            // -------------------------
            if (sliders[i].isStarted == false)
            {
                DWORD currentTime = GetTickCount();

                DWORD spawnTime = sliders[i].spawnTime;
                DWORD hitTime = sliders[i].hitTime;

                DWORD totalTime = hitTime - spawnTime;
                DWORD elapsedTime = currentTime - spawnTime;

                double progress = 0.0;

                if (totalTime > 0)
                {
                    progress = (double)elapsedTime / totalTime;
                }

                if (progress < 0.0)
                    progress = 0.0;

                if (progress > 1.0)
                    progress = 1.0;

                double startScale = 1.8;
                double endScale = 1.0;

                double currentScale =
                    startScale - (startScale - endScale) * progress;

                int approachBaseWidth = approachCircleImg.GetWidth();
                int approachBaseHeight = approachCircleImg.GetHeight();

                int approachWidth = (int)(approachBaseWidth * currentScale);
                int approachHeight = (int)(approachBaseHeight * currentScale);

                int approachDrawX = startX - approachWidth / 2;
                int approachDrawY = startY - approachHeight / 2;

                approachCircleImg.Draw(
                    hDC,
                    approachDrawX,
                    approachDrawY,
                    approachWidth,
                    approachHeight
                );
            }
            // -------------------------
            // 시작 원 이미지
            // -------------------------
            int startCircleWidth = sliderStartCircleImg.GetWidth();
            int startCircleHeight = sliderStartCircleImg.GetHeight();

            sliderStartCircleImg.Draw(
                hDC,
                startX - startCircleWidth / 2,
                startY - startCircleHeight / 2,
                startCircleWidth,
                startCircleHeight
            );

            sliderStartCircleOverlayImg.Draw(
                hDC,
                startX - startCircleWidth / 2,
                startY - startCircleHeight / 2,
                startCircleWidth,
                startCircleHeight
            );

            // -------------------------
            // 끝 원
            // 지금은 시작 원 이미지를 재사용
            // -------------------------
            sliderStartCircleImg.Draw(
                hDC,
                endX - startCircleWidth / 2,
                endY - startCircleHeight / 2,
                startCircleWidth,
                startCircleHeight
            );

            sliderStartCircleOverlayImg.Draw(
                hDC,
                endX - startCircleWidth / 2,
                endY - startCircleHeight / 2,
                startCircleWidth,
                startCircleHeight
            );
        }
        // -------------------------
        // 슬라이더 볼 이동
        // -------------------------
        if (sliders[i].isStarted == true &&
            sliders[i].isFinished == false)
        {
            DWORD currentTime = GetTickCount();

            DWORD elapsedTime =
                currentTime - sliders[i].slideStartTime;

            double progress =
                (double)elapsedTime / sliders[i].duration;

            if (progress < 0.0)
                progress = 0.0;

            if (progress > 1.0)
                progress = 1.0;

            int ballX =
                (int)(sliders[i].startX +
                    (sliders[i].endX - sliders[i].startX) * progress);

            int ballY =
                (int)(sliders[i].startY +
                    (sliders[i].endY - sliders[i].startY) * progress);
           
            // -------------------------
            // 슬라이더 팔로우 원
            // -------------------------
            int followWidth = sliderFollowCircleImg.GetWidth();
            int followHeight = sliderFollowCircleImg.GetHeight();

            sliderFollowCircleImg.Draw(
                hDC,
                ballX - followWidth / 2,
                ballY - followHeight / 2,
                followWidth,
                followHeight
            );

            int ballWidth = sliderBallImg.GetWidth();
            int ballHeight = sliderBallImg.GetHeight();

            sliderBallImg.Draw(
                hDC,
                ballX - ballWidth / 2,
                ballY - ballHeight / 2,
                ballWidth,
                ballHeight
            );
        }
    }

    DWORD currentTime = GetTickCount();

    if (lastJudge != JUDGE_NONE &&
        currentTime - judgeDisplayStartTime <= 500)
    {
        CImage* judgeImg = nullptr;

        switch (lastJudge)
        {
        case JUDGE_PERFECT:
            judgeImg = &hit300Img;
            break;

        case JUDGE_GOOD:
            judgeImg = &hit100Img;
            break;

        case JUDGE_BAD:
            judgeImg = &hit50Img;
            break;

        case JUDGE_MISS:
            judgeImg = &hit0Img;
            break;
        }

        if (judgeImg != nullptr && !judgeImg->IsNull())
        {
            DWORD elapsedTime = currentTime - judgeDisplayStartTime;
            DWORD displayTime = 500;

            // 0.0 ~ 1.0 진행도
            double progress = (double)elapsedTime / displayTime;

            if (progress > 1.0)
                progress = 1.0;

            // 처음에는 1.2배 크기
            // 시간이 지나면 1.0배 크기로 줄어듦
            double startScale = 1.35;
            double endScale = 1.0;

            double currentScale =
                startScale - (startScale - endScale) * progress;

            int originalWidth = judgeImg->GetWidth();
            int originalHeight = judgeImg->GetHeight();

            int drawWidth = (int)(originalWidth * currentScale);
            int drawHeight = (int)(originalHeight * currentScale);

            int drawX = judgeX - drawWidth / 2;
            int drawY = judgeY - drawHeight / 2;

            judgeImg->Draw(
                hDC,
                drawX,
                drawY,
                drawWidth,
                drawHeight
            );
        }
    }

    // =========================
    // 히트서클 그리기
    // =========================
    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        if (hitCircles[i].isActive == true)
        {
            int imageWidth = hitCircleImg.GetWidth();
            int imageHeight = hitCircleImg.GetHeight();

            int drawX = hitCircles[i].x - imageWidth / 2;
            int drawY = hitCircles[i].y - imageHeight / 2;

            // =========================
            // 접근 원 크기 계산
            // =========================
            DWORD currentTime = GetTickCount();

            DWORD spawnTime = hitCircles[i].spawnTime;
            DWORD hitTime = hitCircles[i].hitTime;

            // 생성 시점부터 정확히 눌러야 하는 시점까지 걸리는 시간
            DWORD totalTime = hitTime - spawnTime;

            // 지금까지 지난 시간
            DWORD elapsedTime = currentTime - spawnTime;

            // 0.0 ~ 1.0 사이의 진행도
            double progress = 0.0;

            if (totalTime > 0)
            {
                progress = (double)elapsedTime / totalTime;
            }

            // 너무 작거나 커지지 않게 제한
            if (progress < 0.0)
                progress = 0.0;

            if (progress > 1.0)
                progress = 1.0;


            // =========================
            // 접근 원 크기 설정
            // =========================
            // 처음에는 히트서클의 2.5배 크기
            // hitTime에 도달하면 히트서클과 같은 크기
            double startScale = 1.8;
            double endScale = 1.0;

            double currentScale =
                startScale - (startScale - endScale) * progress;


            // 접근 원 출력 크기
            int approachBaseWidth = approachCircleImg.GetWidth();
            int approachBaseHeight = approachCircleImg.GetHeight();

            int approachWidth = (int)(approachBaseWidth * currentScale);
            int approachHeight = (int)(approachBaseHeight * currentScale);

            // 중심을 유지하도록 좌표 계산
            int approachDrawX = hitCircles[i].x - approachWidth / 2;
            int approachDrawY = hitCircles[i].y - approachHeight / 2;


            // =========================
            // 접근 원 그리기
            // =========================
            approachCircleImg.Draw(
                hDC,
                approachDrawX,
                approachDrawY,
                approachWidth,
                approachHeight
            );

            // 기본 히트서클
            hitCircleImg.Draw(
                hDC,
                drawX,
                drawY,
                imageWidth,
                imageHeight
            );

            // 히트서클 위에 오버레이
            hitCircleOverlayImg.Draw(
                hDC,
                drawX,
                drawY,
                imageWidth,
                imageHeight
            );
        }
    }
}

void RhythmMiniGame::Release()
{
    if (!hitCircleImg.IsNull())
        hitCircleImg.Destroy();

    if (!hitCircleOverlayImg.IsNull())
        hitCircleOverlayImg.Destroy();

    if (!approachCircleImg.IsNull())
        approachCircleImg.Destroy();

    if (!hit300Img.IsNull())
        hit300Img.Destroy();

    if (!hit100Img.IsNull())
        hit100Img.Destroy();

    if (!hit50Img.IsNull())
        hit50Img.Destroy();

    if (!hit0Img.IsNull())
        hit0Img.Destroy();

    if (!sliderStartCircleImg.IsNull())
        sliderStartCircleImg.Destroy();

    if (!sliderStartCircleOverlayImg.IsNull())
        sliderStartCircleOverlayImg.Destroy();

    if (!sliderBallImg.IsNull())
        sliderBallImg.Destroy();

    if (!sliderFollowCircleImg.IsNull())
        sliderFollowCircleImg.Destroy();
}

void RhythmMiniGame::OnMouseDown(int x, int y)
{
    mouseX = x;
    mouseY = y;
    isMouseDown = true;

    DWORD currentTime = GetTickCount();

    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        if (hitCircles[i].isActive == true)
        {
            // -------------------------
            // 1. 클릭 위치가 히트서클 안인지 확인
            // -------------------------
            int dx = x - hitCircles[i].x;
            int dy = y - hitCircles[i].y;

            int hitRadius = hitCircleImg.GetWidth() / 2;

            if (dx * dx + dy * dy <= hitRadius * hitRadius)
            {
                // -------------------------
                // 2. 클릭 타이밍 차이 계산
                // -------------------------
                int timeDiff = (int)currentTime - (int)hitCircles[i].hitTime;

                if (timeDiff < 0)
                    timeDiff = -timeDiff;

                // -------------------------
                // 3. 판정
                // -------------------------
                if (timeDiff <= 80)
                {
                    lastJudge = JUDGE_PERFECT;
                    score += 300;
                }
                else if (timeDiff <= 160)
                {
                    lastJudge = JUDGE_GOOD;
                    score += 100;
                }
                else if (timeDiff <= 250)
                {
                    lastJudge = JUDGE_BAD;
                    score += 50;
                }
                else
                {
                    // 너무 이르거나 너무 늦은 클릭은 무시
                    break;
                }

                // -------------------------
                // 4. 판정이 난 히트서클 제거
                // -------------------------
                hitCircles[i].isActive = false;
                hitCircles[i].isJudged = true;
                hitCircleCount--;

                judgeX = hitCircles[i].x;
                judgeY = hitCircles[i].y;
                judgeDisplayStartTime = currentTime;

                break;
            }
        }
    }
    // =========================
    // 슬라이더 시작 원 클릭 판정
    // =========================
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true &&
            sliders[i].isStarted == false &&
            sliders[i].isFinished == false)
        {
            int dx = x - sliders[i].startX;
            int dy = y - sliders[i].startY;

            int hitRadius = sliderStartCircleImg.GetWidth() / 2;

            if (dx * dx + dy * dy <= hitRadius * hitRadius)
            {
                int timeDiff =
                    (int)currentTime - (int)sliders[i].hitTime;

                if (timeDiff < 0)
                    timeDiff = -timeDiff;

                if (timeDiff <= 80)
                {
                    lastJudge = JUDGE_PERFECT;
                    score += 300;
                }
                else if (timeDiff <= 160)
                {
                    lastJudge = JUDGE_GOOD;
                    score += 100;
                }
                else if (timeDiff <= 250)
                {
                    lastJudge = JUDGE_BAD;
                    score += 50;
                }
                else
                {
                    break;
                }

                sliders[i].isStarted = true;
                sliders[i].slideStartTime = currentTime;

                sliders[i].isTrackingSuccess = true;

                judgeX = sliders[i].startX;
                judgeY = sliders[i].startY;
                judgeDisplayStartTime = currentTime;

                break;
            }
        }
    }

}

void RhythmMiniGame::OnMouseUp(int x, int y)
{
    mouseX = x;
    mouseY = y;
    isMouseDown = false;

    // 이후 여기에
    // - 슬라이더 중단 처리
    // 를 넣을 예정
}

void RhythmMiniGame::OnMouseMove(int x, int y)
{
    mouseX = x;
    mouseY = y;

    // 이후 여기에
    // - 슬라이더 추적 판정
    // 을 넣을 예정
}

bool RhythmMiniGame::IsGameOver() const
{
    return isGameOver;
}

int RhythmMiniGame::GetScore() const
{
    return score;
}

void RhythmMiniGame::CreateHitCircle(int x, int y)
{
    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        if (hitCircles[i].isActive == false)
        {
            DWORD currentTime = GetTickCount();

            hitCircles[i].isActive = true;
            hitCircles[i].isJudged = false;

            hitCircles[i].x = x;
            hitCircles[i].y = y;

            hitCircles[i].spawnTime = currentTime;
            hitCircles[i].hitTime = currentTime + 1000;

            hitCircleCount++;

            break;
        }
    }
}

void RhythmMiniGame::PremultiplyAlpha(CImage& image)
{
    if (image.IsNull())
        return;

    // 알파 채널이 있는 32비트 이미지에만 적용
    if (image.GetBPP() != 32)
        return;

    BYTE* pBits = (BYTE*)image.GetBits();
    int pitch = image.GetPitch();
    int width = image.GetWidth();
    int height = image.GetHeight();

    for (int y = 0; y < height; y++)
    {
        BYTE* pPixel = pBits + y * pitch;

        for (int x = 0; x < width; x++)
        {
            BYTE& blue = pPixel[x * 4 + 0];
            BYTE& green = pPixel[x * 4 + 1];
            BYTE& red = pPixel[x * 4 + 2];
            BYTE& alpha = pPixel[x * 4 + 3];

            blue = (BYTE)((blue * alpha) / 255);
            green = (BYTE)((green * alpha) / 255);
            red = (BYTE)((red * alpha) / 255);
        }
    }
}

void RhythmMiniGame::CreateSlider(int startX, int startY, int endX, int endY)
{
    
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == false)
        {
            DWORD currentTime = GetTickCount();

            sliders[i].isActive = true;
            sliders[i].isStarted = false;
            sliders[i].isFinished = false;
            sliders[i].isFailed = false;

            sliders[i].startX = startX;
            sliders[i].startY = startY;
            sliders[i].endX = endX;
            sliders[i].endY = endY;

            sliders[i].spawnTime = currentTime;
            sliders[i].hitTime = currentTime + 1000;
            sliders[i].isTrackingSuccess = false;
            int dx = endX - startX;
            int dy = endY - startY;

            double distance = sqrt((double)(dx * dx + dy * dy));

            // 슬라이더 볼 속도
            // 값이 작을수록 느리고, 클수록 빠름
            double sliderSpeed = 350.0;   // 초당 350픽셀 이동

            sliders[i].duration =
                (DWORD)((distance / sliderSpeed) * 1000.0);

            if (sliders[i].duration < 700)
            {
                sliders[i].duration = 700;
            }

            sliders[i].slideStartTime = 0;
            
            sliderCount++;

            break;
        }
    }
}