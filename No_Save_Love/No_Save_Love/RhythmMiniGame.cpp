#include "RhythmMiniGame.h"
#include <cstdlib>

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
    }

    lastHitCircleSpawnTime = GetTickCount();
    hitCircleSpawnInterval = 1200;
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
}

void RhythmMiniGame::Render(HDC hDC)
{
    // 임시 화면 확인용 텍스트
    const wchar_t* title = L"Rhythm MiniGame";
    TextOut(hDC, 50, 50, title, lstrlen(title));

    wchar_t scoreText[100];
    wsprintf(scoreText, L"Score : %d", score);
    TextOut(hDC, 50, 90, scoreText, lstrlen(scoreText));

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