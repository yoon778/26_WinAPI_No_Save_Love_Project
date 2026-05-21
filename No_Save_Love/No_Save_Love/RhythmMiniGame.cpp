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


void RhythmMiniGame::Init(HWND hWnd)
{
    gameHwnd = hWnd;

    gameStartTime = GetTickCount();
    windowGimmickStartTime = 0;

    hasWindowGimmickTriggered = false;
    isWindowGimmickActive = false;

    // 원래 창 위치와 크기 저장
    GetWindowRect(gameHwnd, &originalWindowRect);

    // 원래 클라이언트 영역 크기 저장
    RECT clientRect;
    GetClientRect(gameHwnd, &clientRect);

    originalClientWidth = clientRect.right - clientRect.left;
    originalClientHeight = clientRect.bottom - clientRect.top;

    // 이후 노트 생성 범위도 실제 창 내부 크기를 기준으로 사용
    screenWidth = originalClientWidth;
    screenHeight = originalClientHeight;

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

    cursorImg.Load(L"resource\\minigame2\\cursor\\cursor.png");
    cursorTrailImg.Load(L"resource\\minigame2\\cursor\\cursortrail.png");
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    cursorRotateImg =
        new Gdiplus::Image(
            L"resource\\minigame2\\cursor\\cursor.png"
        );

    cursorMiddleImg =
        new Gdiplus::Image(
            L"resource\\minigame2\\cursor\\cursormiddle.png"
        );

    cursorRotationAngle = 0.0f;
    back.Load(L"resource\\minigame2\\back.png");

    PremultiplyAlpha(cursorImg);
    PremultiplyAlpha(cursorTrailImg);

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

    hasWindowJumpGimmickTriggered = false;
    isWindowJumpGimmickActive = false;

    windowJumpLastMoveTime = 0;
    windowJumpStep = 0;

    hasSliderFollowWindowGimmickTriggered = false;
    isSliderFollowWindowGimmickActive = false;

    sliderFollowBaseWindowRect = { 0, 0, 0, 0 };
    sliderFollowPatternStep = 0;

    isSecondWindowGimmickWaiting = false;
    secondWindowGimmickReserveTime = 0;

    score = 0;
    isGameOver = false;

    judgeX = 0;
    judgeY = 0;

    mouseX = 0;
    mouseY = 0;
    isMouseDown = false;

    hitCircleCount = 0;
    sliderCount = 0;
   
    spawnedObjectCount = 0;

    combo = 0;
    comboEffectStartTime = 0;

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
        sliders[i].isWindowFollowTarget = false;
    }

    // =========================
    // 커서 초기화
    // =========================
    cursorTrailIndex = 0;

    for (int i = 0; i < MAX_CURSOR_TRAIL; i++)
    {
        cursorTrail[i].x = 0;
        cursorTrail[i].y = 0;
        cursorTrail[i].isActive = false;
        cursorTrail[i].createTime = 0;
    }

    lastHitCircleSpawnTime = GetTickCount();
    hitCircleSpawnInterval = 1200;

}

void RhythmMiniGame::Update()
{
    DWORD currentTime = GetTickCount();
    UpdateWindowGimmick(currentTime);
    UpdateWindowJumpGimmick(currentTime);
    UpdateSliderFollowWindowGimmick(currentTime);
    // =========================
    // 두 번째 작은 창 기믹 예약 실행
    // =========================
    if (isSecondWindowGimmickWaiting == true)
    {
        if (currentTime - secondWindowGimmickReserveTime >= SECOND_WINDOW_GIMMICK_DELAY)
        {
            isSecondWindowGimmickWaiting = false;

            // 다른 위치에서 작은 창 기믹 2차 실행
            TriggerWindowGimmick(currentTime, true);
        }
    }

    bool hasActiveSlider = false;

    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true)
        {
            hasActiveSlider = true;
            break;
        }
    }

    cursorRotationAngle += 3.0f;

    if (cursorRotationAngle >= 360.0f)
    {
        cursorRotationAngle -= 360.0f;
    }

    // =========================
    // 일정 시간마다 히트서클 생성
    // =========================
    DWORD currentSpawnInterval =
        isWindowGimmickActive
        ? SMALL_WINDOW_SPAWN_INTERVAL
        : hitCircleSpawnInterval;

    if (hasActiveSlider == false &&
        currentTime - lastHitCircleSpawnTime >= currentSpawnInterval)
    {
        // =========================
        // 작은 창 기믹 중
        // 히트서클만 생성
        // =========================
        if (isWindowGimmickActive == true)
        {
            int marginX = 80;
            int marginY = 70;

            int randomX =
                marginX + rand() % (screenWidth - marginX * 2);

            int randomY =
                marginY + rand() % (screenHeight - marginY * 2);

            CreateHitCircle(randomX, randomY);
        }

        // =========================
        // 평상시
        // 히트서클 / 슬라이더 섞어서 생성
        // =========================
        else
        {
            spawnedObjectCount++;

            int marginX = 300;
            int marginY = 220;

            if (spawnedObjectCount % 4 == 0)
            {
                int startX =
                    marginX + rand() % (screenWidth - marginX * 2);

                int startY =
                    marginY + rand() % (screenHeight - marginY * 2);

                int endX = startX;
                int endY = startY;

                while (true)
                {
                    endX =
                        marginX + rand() % (screenWidth - marginX * 2);

                    endY =
                        marginY + rand() % (screenHeight - marginY * 2);

                    int dx = endX - startX;
                    int dy = endY - startY;

                    int distanceSquared = dx * dx + dy * dy;

                    if (distanceSquared >= 350 * 350)
                    {
                        break;
                    }
                }

                CreateSlider(startX, startY, endX, endY);
            }
            else
            {
                int randomX =
                    marginX + rand() % (screenWidth - marginX * 2);

                int randomY =
                    marginY + rand() % (screenHeight - marginY * 2);

                CreateHitCircle(randomX, randomY);
            }
        }

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

                AddCombo();

                judgeX = hitCircles[i].x;
                judgeY = hitCircles[i].y;
                lastJudge = JUDGE_MISS;
                ResetCombo();
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
                ResetCombo();

                judgeX = sliders[i].startX;
                judgeY = sliders[i].startY;
                judgeDisplayStartTime = currentTime;
                lastHitCircleSpawnTime = currentTime;

                if (sliders[i].isWindowFollowTarget == true)
                {
                    sliderFollowPatternStep++;

                    // 다음 패턴이 남아 있다면 이어서 생성
                    if (sliderFollowPatternStep < WINDOW_SLIDER_FOLLOW_PATTERN_COUNT)
                    {
                        // 다음 방향으로 바뀌기 전에 창을 랜덤 위치에 다시 생성
                        MoveSliderFollowWindowToRandomPosition();
                        CreateSliderFollowPattern(sliderFollowPatternStep, currentTime);
                    }
                    // 5개를 전부 끝냈다면 원래 창으로 복귀
                    else
                    {
                        isSliderFollowWindowGimmickActive = false;

                        RestoreOriginalWindow(currentTime);

                        isSecondWindowGimmickWaiting = true;
                        secondWindowGimmickReserveTime = currentTime;
                    }
                }
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

            double followScale = 0.75;

            int followRadius =
                (int)(sliderFollowCircleImg.GetWidth() * followScale / 2);

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

                if (sliders[i].isWindowFollowTarget == true)
                {
                    sliderFollowPatternStep++;

                    // 다음 패턴이 남아 있다면 이어서 생성
                    if (sliderFollowPatternStep < WINDOW_SLIDER_FOLLOW_PATTERN_COUNT)
                    {
                        // 다음 방향으로 바뀌기 전에 창을 랜덤 위치에 다시 생성
                        MoveSliderFollowWindowToRandomPosition();
                        CreateSliderFollowPattern(sliderFollowPatternStep, currentTime);
                    }
                    else
                    {
                        isSliderFollowWindowGimmickActive = false;

                        // 일단 원래 창으로 돌아가서 기본 플레이 진행
                        RestoreOriginalWindow(currentTime);

                        // 두 번째 작은 창 기믹을 일정 시간 뒤 실행하도록 예약
                        isSecondWindowGimmickWaiting = true;
                        secondWindowGimmickReserveTime = currentTime;
                    }
                }

                if (sliders[i].isTrackingSuccess == true &&
                    sliders[i].isFailed == false)
                {
                    lastJudge = JUDGE_PERFECT;
                    score += 300;
                    AddCombo();
                }
                else
                {
                    lastJudge = JUDGE_MISS;
                    ResetCombo();
                }

                lastHitCircleSpawnTime = currentTime;
            }
        }
    }
}

void RhythmMiniGame::Render(HDC hDC)
{

    back.Draw(hDC, 0, 0, 1920, 1080);
 
    // =========================
    // 점수 / 콤보 출력
    // =========================
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(255, 255, 255));

    wchar_t scoreText[100];
    wsprintf(scoreText, L"Score : %d", score);
    TextOut(hDC, 50, 50, scoreText, lstrlen(scoreText));

    wchar_t comboText[100];

    if (combo > 0)
    {
        wsprintf(comboText, L"%d COMBO", combo);
    }
    else
    {
        wsprintf(comboText, L"");
    }

    TextOut(hDC, 50, 90, comboText, lstrlen(comboText));

    
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
            double followScale = 0.75;

            int followWidth =
                (int)(sliderFollowCircleImg.GetWidth() * followScale);

            int followHeight =
                (int)(sliderFollowCircleImg.GetHeight() * followScale);

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
            double hitCircleScale =
                isWindowGimmickActive
                ? SMALL_WINDOW_HITCIRCLE_SCALE
                : 1.0;

            int imageWidth =
                (int)(hitCircleImg.GetWidth() * hitCircleScale);

            int imageHeight =
                (int)(hitCircleImg.GetHeight() * hitCircleScale);

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

            int approachWidth =
                (int)(approachBaseWidth * currentScale * hitCircleScale);

            int approachHeight =
                (int)(approachBaseHeight * currentScale * hitCircleScale);

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

    // =========================
    // 커서 궤적 그리기
    // =========================
    for (int i = 0; i < MAX_CURSOR_TRAIL; i++)
    {
        int index = (cursorTrailIndex + i) % MAX_CURSOR_TRAIL;

        if (cursorTrail[index].isActive == true)
        {
            DWORD elapsedTime =
                currentTime - cursorTrail[index].createTime;

            if (elapsedTime > CURSOR_TRAIL_LIFETIME)
            {
                cursorTrail[index].isActive = false;
                continue;
            }

            int trailWidth = cursorTrailImg.GetWidth();
            int trailHeight = cursorTrailImg.GetHeight();

            cursorTrailImg.Draw(
                hDC,
                cursorTrail[index].x - trailWidth / 2,
                cursorTrail[index].y - trailHeight / 2,
                trailWidth,
                trailHeight
            );
        }
    }

    // =========================
    // 현재 커서 그리기
    // =========================

        // =========================
        // 회전하는 커서 중앙 장식
        // =========================
        if (cursorMiddleImg != nullptr &&
            cursorMiddleImg->GetLastStatus() == Gdiplus::Ok)
        {
            Gdiplus::Graphics graphics(hDC);

            int middleWidth =
                (int)cursorMiddleImg->GetWidth();

            int middleHeight =
                (int)cursorMiddleImg->GetHeight();

            Gdiplus::GraphicsState state = graphics.Save();

            // 회전 중심을 현재 커서 위치로 이동
            graphics.TranslateTransform(
                (Gdiplus::REAL)mouseX,
                (Gdiplus::REAL)mouseY
            );

            // 커서 중심에서 회전
            graphics.RotateTransform(cursorRotationAngle);

            // 중심 기준으로 이미지 출력
            graphics.DrawImage(
                cursorMiddleImg,
                -middleWidth / 2,
                -middleHeight / 2,
                middleWidth,
                middleHeight
            );

            graphics.Restore(state);
        }
        // =========================
// 회전하는 커서 본체
// =========================
        if (cursorRotateImg != nullptr &&
            cursorRotateImg->GetLastStatus() == Gdiplus::Ok)
        {
            Gdiplus::Graphics graphics(hDC);

            int cursorWidth =
                (int)cursorRotateImg->GetWidth();

            int cursorHeight =
                (int)cursorRotateImg->GetHeight();

            Gdiplus::GraphicsState state = graphics.Save();

            // 현재 마우스 위치를 회전 중심으로 이동
            graphics.TranslateTransform(
                (Gdiplus::REAL)mouseX,
                (Gdiplus::REAL)mouseY
            );

            // 커서 회전
            graphics.RotateTransform(cursorRotationAngle);

            // 중심 기준으로 출력
            graphics.DrawImage(
                cursorRotateImg,
                -cursorWidth / 2,
                -cursorHeight / 2,
                cursorWidth,
                cursorHeight
            );

            graphics.Restore(state);
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

    if (!cursorImg.IsNull())
        cursorImg.Destroy();

    if (!cursorTrailImg.IsNull())
        cursorTrailImg.Destroy();

    if (cursorMiddleImg != nullptr)
    {
        delete cursorMiddleImg;
        cursorMiddleImg = nullptr;
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);

    if (cursorRotateImg != nullptr)
    {
        delete cursorRotateImg;
        cursorRotateImg = nullptr;
    }
}

void RhythmMiniGame::OnMouseDown(int x, int y)
{
    mouseX = x;
    mouseY = y;
    isMouseDown = true;

    DWORD currentTime = GetTickCount();

    // =========================
    // 히트서클 클릭 판정
    // 겹쳤을 경우 먼저 생성된 히트서클 우선
    // =========================
    int targetHitCircleIndex = -1;

    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        if (hitCircles[i].isActive == true)
        {
            double hitCircleScale =
                isWindowGimmickActive
                ? SMALL_WINDOW_HITCIRCLE_SCALE
                : 1.0;

            int hitRadius =
                (int)(hitCircleImg.GetWidth() * hitCircleScale / 2) + 15;

            int dx = x - hitCircles[i].x;
            int dy = y - hitCircles[i].y;

            if (dx * dx + dy * dy <= hitRadius * hitRadius)
            {
                // 아직 선택된 대상이 없으면 선택
                if (targetHitCircleIndex == -1)
                {
                    targetHitCircleIndex = i;
                }
                // 더 먼저 생성된 히트서클이면 교체
                else if (
                    hitCircles[i].spawnTime <
                    hitCircles[targetHitCircleIndex].spawnTime
                    )
                {
                    targetHitCircleIndex = i;
                }
            }
        }
    }
    if (targetHitCircleIndex != -1)
    {
        int i = targetHitCircleIndex;

        int signedTimeDiff =
            (int)currentTime - (int)hitCircles[i].hitTime;

        int timeDiff = signedTimeDiff;

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
        else if (
            (signedTimeDiff < 0 && timeDiff <= 350) ||
            (signedTimeDiff > 0 && timeDiff <= 250)
            )
        {
            lastJudge = JUDGE_BAD;
            score += 50;
        }
        else
        {
            // 히트서클 위를 눌렀지만 판정 타이밍이 아니면
            // 다른 겹친 원으로 넘어가지 않고 그대로 종료
            return;
        }

        hitCircles[i].isActive = false;
        hitCircles[i].isJudged = true;
        hitCircleCount--;

        judgeX = hitCircles[i].x;
        judgeY = hitCircles[i].y;
        judgeDisplayStartTime = currentTime;

        return;
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

            int hitRadius = sliderStartCircleImg.GetWidth() / 2 + 15;

            if (dx * dx + dy * dy <= hitRadius * hitRadius)
            {
                int signedTimeDiff =
                    (int)currentTime - (int)sliders[i].hitTime;

                int timeDiff = signedTimeDiff;

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
                else if (
                    // 조금 빠른 클릭은 350ms까지 허용
                    (signedTimeDiff < 0 && timeDiff <= 350) ||

                    // 늦은 클릭은 250ms까지 허용
                    (signedTimeDiff > 0 && timeDiff <= 250)
                    )
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

                AddCombo();

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
    DWORD currentTime = GetTickCount();

    // 이전 마우스 위치
    int prevX = mouseX;
    int prevY = mouseY;

    // 이동 거리 계산
    int dx = x - prevX;
    int dy = y - prevY;

    double distance = sqrt((double)(dx * dx + dy * dy));

    // 일정 간격마다 trail 점 추가
    int pointCount =
        (int)(distance / CURSOR_TRAIL_POINT_SPACING);

    if (pointCount < 1)
    {
        pointCount = 1;
    }

    for (int i = 1; i <= pointCount; i++)
    {
        double t = (double)i / pointCount;

        int trailX =
            (int)(prevX + dx * t);

        int trailY =
            (int)(prevY + dy * t);

        AddCursorTrailPoint(trailX, trailY, currentTime);
    }

    // 현재 마우스 위치 갱신
    mouseX = x;
    mouseY = y;
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

void RhythmMiniGame::CreateSlider(
    int startX,
    int startY,
    int endX,
    int endY,
    bool isWindowFollowTarget
) {
    
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
            sliders[i].isWindowFollowTarget = isWindowFollowTarget;

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

void RhythmMiniGame::UpdateWindowGimmick(DWORD currentTime)
{
    DWORD elapsedGameTime = currentTime - gameStartTime;

    // 15초 뒤 한 번 발동
    if (hasWindowGimmickTriggered == false &&
        isWindowJumpGimmickActive == false &&
        elapsedGameTime >= WINDOW_GIMMICK_TRIGGER_TIME)
    {
        TriggerWindowGimmick(currentTime);
        hasWindowGimmickTriggered = true;
    }

    // 유지 시간이 끝나면 원래 창으로 복귀
    if (isWindowGimmickActive == true &&
        currentTime - windowGimmickStartTime >= WINDOW_GIMMICK_DURATION)
    {
        RestoreOriginalWindow(currentTime);
    }
}

void RhythmMiniGame::CalculateGimmickClientSize(int& outWidth, int& outHeight)
{
    int maxX = 0;
    int maxY = 0;

    // 현재 떠 있는 히트서클 확인
    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        if (hitCircles[i].isActive == true)
        {
            if (hitCircles[i].x > maxX)
                maxX = hitCircles[i].x;

            if (hitCircles[i].y > maxY)
                maxY = hitCircles[i].y;
        }
    }

    // 현재 떠 있는 슬라이더 확인
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true)
        {
            if (sliders[i].startX > maxX)
                maxX = sliders[i].startX;

            if (sliders[i].endX > maxX)
                maxX = sliders[i].endX;

            if (sliders[i].startY > maxY)
                maxY = sliders[i].startY;

            if (sliders[i].endY > maxY)
                maxY = sliders[i].endY;
        }
    }

    int requiredWidth = maxX + GIMMICK_OBJECT_PADDING;
    int requiredHeight = maxY + GIMMICK_OBJECT_PADDING;

    // 최소 창 크기 보장
    if (requiredWidth < MIN_GIMMICK_CLIENT_WIDTH)
        requiredWidth = MIN_GIMMICK_CLIENT_WIDTH;

    if (requiredHeight < MIN_GIMMICK_CLIENT_HEIGHT)
        requiredHeight = MIN_GIMMICK_CLIENT_HEIGHT;


    // 기믹 중에는 너무 커지지 않도록 제한
    if (requiredWidth > MAX_GIMMICK_CLIENT_WIDTH)
        requiredWidth = MAX_GIMMICK_CLIENT_WIDTH;

    if (requiredHeight > MAX_GIMMICK_CLIENT_HEIGHT)
        requiredHeight = MAX_GIMMICK_CLIENT_HEIGHT;

    outWidth = requiredWidth;
    outHeight = requiredHeight;
}

void RhythmMiniGame::TriggerWindowGimmick(DWORD currentTime, bool isSecondRun)
{
    if (gameHwnd == NULL)
        return;

    int targetClientWidth = 0;
    int targetClientHeight = 0;

    CalculateGimmickClientSize(targetClientWidth, targetClientHeight);

    // 원하는 클라이언트 크기를 실제 윈도우 전체 크기로 변환
    RECT adjustedRect = { 0, 0, targetClientWidth, targetClientHeight };

    DWORD windowStyle =
        (DWORD)GetWindowLongPtr(gameHwnd, GWL_STYLE);

    DWORD exWindowStyle =
        (DWORD)GetWindowLongPtr(gameHwnd, GWL_EXSTYLE);

    AdjustWindowRectEx(
        &adjustedRect,
        windowStyle,
        FALSE,
        exWindowStyle
    );

    int targetWindowWidth =
        adjustedRect.right - adjustedRect.left;

    int targetWindowHeight =
        adjustedRect.bottom - adjustedRect.top;

    // 화면 오른쪽 위로 갑자기 이동
    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);

    int targetX = 0;
    int targetY = 0;

    if (isSecondRun == false)
    {
        // 첫 번째 1번 기믹: 오른쪽 위
        targetX = desktopWidth - targetWindowWidth - 40;
        targetY = 40;
    }
    else
    {
        // 두 번째 1번 기믹: 왼쪽 아래
        int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

        targetX = 40;
        targetY = desktopHeight - targetWindowHeight - 80;
    }
    if (targetX < 0)
        targetX = 0;

    if (targetY < 0)
        targetY = 0;

    SetWindowPos(
        gameHwnd,
        NULL,
        targetX,
        targetY,
        targetWindowWidth,
        targetWindowHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // 바뀐 실제 클라이언트 영역 크기 반영
    RECT clientRect;
    GetClientRect(gameHwnd, &clientRect);

    screenWidth = clientRect.right - clientRect.left;
    screenHeight = clientRect.bottom - clientRect.top;

    // 기존 노트는 모두 제거
    ClearAllNotes();

    // 바뀐 창 크기 안에서 즉시 새 히트서클 생성
    CreateImmediateHitCircle();

    // 다음 자동 생성 시간이 꼬이지 않도록 기준 시간 갱신
    lastHitCircleSpawnTime = currentTime;

    windowGimmickStartTime = currentTime;
    isWindowGimmickActive = true;

    InvalidateRect(gameHwnd, NULL, FALSE);
}

void RhythmMiniGame::RestoreOriginalWindow(DWORD currentTime)
{
    if (gameHwnd == NULL)
        return;

    int originalWindowWidth =
        originalWindowRect.right - originalWindowRect.left;

    int originalWindowHeight =
        originalWindowRect.bottom - originalWindowRect.top;

    SetWindowPos(
        gameHwnd,
        NULL,
        originalWindowRect.left,
        originalWindowRect.top,
        originalWindowWidth,
        originalWindowHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    RECT clientRect;
    GetClientRect(gameHwnd, &clientRect);

    screenWidth = clientRect.right - clientRect.left;
    screenHeight = clientRect.bottom - clientRect.top;

    // 작은 창 기믹 종료
    isWindowGimmickActive = false;

    // 기존 작은 창 노트 제거 후 큰 창 기준으로 새 노트 생성
    ClearAllNotes();
    CreateImmediateHitCircle();

    lastHitCircleSpawnTime = currentTime;

    InvalidateRect(gameHwnd, NULL, FALSE);
}
void RhythmMiniGame::ClearAllNotes()
{
    for (int i = 0; i < MAX_HIT_CIRCLES; i++)
    {
        hitCircles[i].isActive = false;
        hitCircles[i].isJudged = false;
    }

    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        sliders[i].isActive = false;
        sliders[i].isStarted = false;
        sliders[i].isFinished = false;
        sliders[i].isFailed = false;
        sliders[i].isTrackingSuccess = false;
        sliders[i].isWindowFollowTarget = false;
    }

    hitCircleCount = 0;
    sliderCount = 0;
}

void RhythmMiniGame::CreateImmediateHitCircle()
{
    int marginX;
    int marginY;

    if (isWindowGimmickActive == true)
    {
        marginX = 80;
        marginY = 70;
    }
    else
    {
        marginX = 180;
        marginY = 140;
    }

    int randomX =
        marginX + rand() % (screenWidth - marginX * 2);

    int randomY =
        marginY + rand() % (screenHeight - marginY * 2);

    CreateHitCircle(randomX, randomY);
}

void RhythmMiniGame::UpdateWindowJumpGimmick(DWORD currentTime)
{
    DWORD elapsedGameTime = currentTime - gameStartTime;

    // 35초에 처음 발동
    if (hasWindowJumpGimmickTriggered == false &&
        elapsedGameTime >= WINDOW_JUMP_GIMMICK_TRIGGER_TIME)
    {
        TriggerWindowJumpGimmick(currentTime);
        hasWindowJumpGimmickTriggered = true;
    }

    // 기믹이 진행 중이면 0.7초마다 다음 창 상태로 이동
    if (isWindowJumpGimmickActive == true)
    {
        if (currentTime - windowJumpLastMoveTime >= WINDOW_JUMP_INTERVAL)
        {
            windowJumpStep++;

            if (windowJumpStep >= WINDOW_JUMP_COUNT)
            {
                // 5번 이동이 끝나면 원래 창으로 복귀
                RestoreOriginalWindow(currentTime);
                isWindowJumpGimmickActive = false;
                return;
            }

            MoveWindowJumpStep(windowJumpStep, currentTime);
            windowJumpLastMoveTime = currentTime;
        }
    }
}

void RhythmMiniGame::TriggerWindowJumpGimmick(DWORD currentTime)
{
    if (gameHwnd == NULL)
        return;

    // 혹시 작은 창 기믹이 남아 있다면 종료 처리
    isWindowGimmickActive = false;

    isWindowJumpGimmickActive = true;
    windowJumpStep = 0;
    windowJumpLastMoveTime = currentTime;

    MoveWindowJumpStep(windowJumpStep, currentTime);
}

void RhythmMiniGame::MoveWindowJumpStep(int step, DWORD currentTime)
{
    if (gameHwnd == NULL)
        return;

    int clientWidth = 1000;
    int clientHeight = 600;

    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
    int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

    int targetX = 0;
    int targetY = 0;

    // 패턴형 기믹이므로 순서를 고정
    switch (step)
    {
    case 0:
        clientWidth = 1000;
        clientHeight = 600;
        targetX = 40;
        targetY = 40;
        break;

    case 1:
        clientWidth = 850;
        clientHeight = 520;
        targetX = desktopWidth - clientWidth - 100;
        targetY = 80;
        break;

    case 2:
        clientWidth = 1100;
        clientHeight = 650;
        targetX = (desktopWidth - clientWidth) / 2;
        targetY = (desktopHeight - clientHeight) / 2;
        break;

    case 3:
        clientWidth = 900;
        clientHeight = 540;
        targetX = 80;
        targetY = desktopHeight - clientHeight - 120;
        break;

    case 4:
        clientWidth = 980;
        clientHeight = 580;
        targetX = desktopWidth - clientWidth - 80;
        targetY = desktopHeight - clientHeight - 100;
        break;
    }

    // 클라이언트 크기를 실제 창 크기로 변환
    RECT adjustedRect = { 0, 0, clientWidth, clientHeight };

    DWORD windowStyle =
        (DWORD)GetWindowLongPtr(gameHwnd, GWL_STYLE);

    DWORD exWindowStyle =
        (DWORD)GetWindowLongPtr(gameHwnd, GWL_EXSTYLE);

    AdjustWindowRectEx(
        &adjustedRect,
        windowStyle,
        FALSE,
        exWindowStyle
    );

    int targetWindowWidth =
        adjustedRect.right - adjustedRect.left;

    int targetWindowHeight =
        adjustedRect.bottom - adjustedRect.top;

    // 실제 창 위치 적용
    SetWindowPos(
        gameHwnd,
        NULL,
        targetX,
        targetY,
        targetWindowWidth,
        targetWindowHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // 바뀐 클라이언트 크기 반영
    RECT clientRect;
    GetClientRect(gameHwnd, &clientRect);

    screenWidth = clientRect.right - clientRect.left;
    screenHeight = clientRect.bottom - clientRect.top;

    // 이전 노트가 잘리지 않도록 전부 제거
    ClearAllNotes();

    // 새 창 기준으로 즉시 히트서클 하나 생성
    CreateImmediateHitCircle();

    // 다음 자동 생성 시간 기준 초기화
    lastHitCircleSpawnTime = currentTime;

    InvalidateRect(gameHwnd, NULL, FALSE);
}

void RhythmMiniGame::UpdateSliderFollowWindowGimmick(DWORD currentTime)
{
    DWORD elapsedGameTime = currentTime - gameStartTime;

    if (hasSliderFollowWindowGimmickTriggered == false &&
        isWindowGimmickActive == false &&
        isWindowJumpGimmickActive == false &&
        elapsedGameTime >= WINDOW_SLIDER_FOLLOW_TRIGGER_TIME)
    {
        TriggerSliderFollowWindowGimmick(currentTime);
        hasSliderFollowWindowGimmickTriggered = true;
    }

    if (isSliderFollowWindowGimmickActive == true)
    {
        MoveWindowWithSliderBall(currentTime);
    }
}

void RhythmMiniGame::TriggerSliderFollowWindowGimmick(DWORD currentTime)
{
    if (gameHwnd == NULL)
        return;

    // 원하는 클라이언트 크기
    RECT adjustedRect = {
        0,
        0,
        WINDOW_SLIDER_FOLLOW_CLIENT_WIDTH,
        WINDOW_SLIDER_FOLLOW_CLIENT_HEIGHT
    };

    DWORD windowStyle =
        (DWORD)GetWindowLongPtr(gameHwnd, GWL_STYLE);

    DWORD exWindowStyle =
        (DWORD)GetWindowLongPtr(gameHwnd, GWL_EXSTYLE);

    AdjustWindowRectEx(
        &adjustedRect,
        windowStyle,
        FALSE,
        exWindowStyle
    );

    int targetWindowWidth =
        adjustedRect.right - adjustedRect.left;

    int targetWindowHeight =
        adjustedRect.bottom - adjustedRect.top;

    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
    int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

    int maxX = desktopWidth - targetWindowWidth;
    int maxY = desktopHeight - targetWindowHeight;

    int targetX = rand() % (maxX + 1);
    int targetY = rand() % (maxY + 1);

    SetWindowPos(
        gameHwnd,
        NULL,
        targetX,
        targetY,
        targetWindowWidth,
        targetWindowHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // 바뀐 창 크기 반영
    RECT clientRect;
    GetClientRect(gameHwnd, &clientRect);

    screenWidth = clientRect.right - clientRect.left;
    screenHeight = clientRect.bottom - clientRect.top;

    // 이 위치를 창 이동 기준점으로 저장
    GetWindowRect(gameHwnd, &sliderFollowBaseWindowRect);

    // 기존 노트 제거
ClearAllNotes();

isSliderFollowWindowGimmickActive = true;
sliderFollowPatternStep = 0;

// 첫 번째 패턴 슬라이더 생성
MoveSliderFollowWindowToRandomPosition();
CreateSliderFollowPattern(sliderFollowPatternStep, currentTime);

lastHitCircleSpawnTime = currentTime;

InvalidateRect(gameHwnd, NULL, FALSE);
}

void RhythmMiniGame::MoveWindowWithSliderBall(DWORD currentTime)
{
    for (int i = 0; i < MAX_SLIDERS; i++)
    {
        if (sliders[i].isActive == true &&
            sliders[i].isStarted == true &&
            sliders[i].isFinished == false &&
            sliders[i].isWindowFollowTarget == true)
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

            int moveX =
                (int)((ballX - sliders[i].startX) *
                    WINDOW_SLIDER_FOLLOW_POWER);

            int moveY =
                (int)((ballY - sliders[i].startY) *
                    WINDOW_SLIDER_FOLLOW_POWER);

            int windowWidth =
                sliderFollowBaseWindowRect.right -
                sliderFollowBaseWindowRect.left;

            int windowHeight =
                sliderFollowBaseWindowRect.bottom -
                sliderFollowBaseWindowRect.top;

            int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
            int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

            int targetX =
                sliderFollowBaseWindowRect.left + moveX;

            int targetY =
                sliderFollowBaseWindowRect.top + moveY;

            // 화면 밖으로 빠지지 않게 제한
            if (targetX < 0)
                targetX = 0;

            if (targetY < 0)
                targetY = 0;

            if (targetX + windowWidth > desktopWidth)
                targetX = desktopWidth - windowWidth;

            if (targetY + windowHeight > desktopHeight)
                targetY = desktopHeight - windowHeight;

            SetWindowPos(
                gameHwnd,
                NULL,
                targetX,
                targetY,
                0,
                0,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE
            );

            return;
        }
    }
}

void RhythmMiniGame::CreateSliderFollowPattern(int patternIndex, DWORD currentTime)
{
    int startX = 0;
    int startY = 0;
    int endX = 0;
    int endY = 0;

    // 너무 가장자리에 붙지 않게 하는 여백
    int marginX = 110;
    int marginY = 90;

    // 랜덤 보정 범위
    int randomXRange = 120;
    int randomYRange = 80;

    switch (patternIndex)
    {
    case 0:
        // 1. 좌하단 → 우상단 대각선
        startX = marginX + rand() % randomXRange;
        startY = screenHeight - marginY - rand() % randomYRange;

        endX = screenWidth - marginX - rand() % randomXRange;
        endY = marginY + rand() % randomYRange;
        break;

    case 1:
        // 2. 좌상단 → 우하단 대각선
        startX = marginX + rand() % randomXRange;
        startY = marginY + rand() % randomYRange;

        endX = screenWidth - marginX - rand() % randomXRange;
        endY = screenHeight - marginY - rand() % randomYRange;
        break;

    case 2:
        // 3. 위 → 아래 세로 이동
        startX =
            marginX + rand() % (screenWidth - marginX * 2);
        startY =
            marginY + rand() % 50;

        endX =
            startX + (-50 + rand() % 101); // 약간 기울어질 수도 있음
        endY =
            screenHeight - marginY - rand() % 50;

        // 끝 X가 화면 밖으로 나가지 않게 보정
        if (endX < marginX)
            endX = marginX;

        if (endX > screenWidth - marginX)
            endX = screenWidth - marginX;
        break;

    case 3:
        // 4. 아래 → 위 세로 이동
        startX =
            marginX + rand() % (screenWidth - marginX * 2);
        startY =
            screenHeight - marginY - rand() % 50;

        endX =
            startX + (-50 + rand() % 101); // 약간 기울어질 수도 있음
        endY =
            marginY + rand() % 50;

        if (endX < marginX)
            endX = marginX;

        if (endX > screenWidth - marginX)
            endX = screenWidth - marginX;
        break;

    case 4:
        // 5. 왼쪽 → 오른쪽 가로 이동
        startX =
            marginX + rand() % 50;
        startY =
            marginY + rand() % (screenHeight - marginY * 2);

        endX =
            screenWidth - marginX - rand() % 50;
        endY =
            startY + (-50 + rand() % 101); // 약간 기울어질 수도 있음

        if (endY < marginY)
            endY = marginY;

        if (endY > screenHeight - marginY)
            endY = screenHeight - marginY;
        break;
    }

    // 현재 창 위치를 창 이동 기준점으로 저장
    GetWindowRect(gameHwnd, &sliderFollowBaseWindowRect);

    CreateSlider(
        startX,
        startY,
        endX,
        endY,
        true
    );

    lastHitCircleSpawnTime = currentTime;
}

void RhythmMiniGame::MoveSliderFollowWindowToRandomPosition()
{
    if (gameHwnd == NULL)
        return;

    RECT windowRect;
    GetWindowRect(gameHwnd, &windowRect);

    int windowWidth =
        windowRect.right - windowRect.left;

    int windowHeight =
        windowRect.bottom - windowRect.top;

    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
    int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

    int maxX = desktopWidth - windowWidth;
    int maxY = desktopHeight - windowHeight;

    if (maxX < 0)
        maxX = 0;

    if (maxY < 0)
        maxY = 0;

    int randomX = rand() % (maxX + 1);
    int randomY = rand() % (maxY + 1);

    SetWindowPos(
        gameHwnd,
        NULL,
        randomX,
        randomY,
        0,
        0,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE
    );

    // 이후 슬라이더 볼을 따라 움직일 기준 위치 갱신
    GetWindowRect(gameHwnd, &sliderFollowBaseWindowRect);

    InvalidateRect(gameHwnd, NULL, FALSE);
}

void RhythmMiniGame::AddCursorTrailPoint(int x, int y, DWORD createTime)
{
    cursorTrail[cursorTrailIndex].x = x;
    cursorTrail[cursorTrailIndex].y = y;
    cursorTrail[cursorTrailIndex].isActive = true;
    cursorTrail[cursorTrailIndex].createTime = createTime;

    cursorTrailIndex++;

    if (cursorTrailIndex >= MAX_CURSOR_TRAIL)
    {
        cursorTrailIndex = 0;
    }
}

void RhythmMiniGame::AddCombo()
{
    combo++;
    comboEffectStartTime = GetTickCount();
}

void RhythmMiniGame::ResetCombo()
{
    combo = 0;
    comboEffectStartTime = GetTickCount();
}