#pragma once
#include <windows.h>
#include <atlimage.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// 화면에 동시에 존재할 수 있는 최대 개수
#define MAX_HIT_CIRCLES 30
#define MAX_SLIDERS 10

#define MAX_CURSOR_TRAIL 80
#define CURSOR_TRAIL_LIFETIME 450

#define WINDOW_GIMMICK_TRIGGER_TIME 15000   // 게임 시작 15초 뒤 발동
#define WINDOW_GIMMICK_DURATION 8000         // 8초 동안 유지

#define MIN_GIMMICK_CLIENT_WIDTH 640
#define MIN_GIMMICK_CLIENT_HEIGHT 360

#define GIMMICK_OBJECT_PADDING 220
#define MAX_GIMMICK_CLIENT_WIDTH 800
#define MAX_GIMMICK_CLIENT_HEIGHT 450

#define SMALL_WINDOW_HITCIRCLE_SCALE 0.75
#define SMALL_WINDOW_SPAWN_INTERVAL 800

#define WINDOW_JUMP_GIMMICK_TRIGGER_TIME 35000
#define WINDOW_JUMP_INTERVAL (BEAT_INTERVAL * 4)
#define WINDOW_JUMP_COUNT 5

#define WINDOW_SLIDER_FOLLOW_TRIGGER_TIME 55000

#define WINDOW_SLIDER_FOLLOW_CLIENT_WIDTH 760
#define WINDOW_SLIDER_FOLLOW_CLIENT_HEIGHT 460

#define WINDOW_SLIDER_FOLLOW_POWER 0.8

#define WINDOW_SLIDER_FOLLOW_PATTERN_COUNT 5

#define SECOND_WINDOW_GIMMICK_DELAY 10000

#define CURSOR_TRAIL_POINT_SPACING 1

#define COMBO_ANIMATION_FRAME_COUNT 6
#define COMBO_ANIMATION_FRAME_INTERVAL 180
#define COMBO_ANIMATION_DURATION 3000

#define BGM_BPM 131
#define BEAT_INTERVAL 458

#define BGM_VOLUME 250

#define BGM_BPM 131
#define BEAT_INTERVAL 458

#define NOTE_BEAT_STEP 2

#define WINDOW_NOTE_SPAWN_BLOCK_TIME 700

#define BEAT_INTERVAL 458
#define HIT_APPROACH_TIME (BEAT_INTERVAL * 2)

#define WINDOW_GIMMICK_TRIGGER_BEAT 32          // 32박자  ≈ 14.6초  → 작은 창 기믹
#define WINDOW_JUMP_GIMMICK_TRIGGER_BEAT 76     // 76박자  ≈ 34.8초  → 연속 창 점프 기믹
#define WINDOW_SLIDER_FOLLOW_TRIGGER_BEAT 120   // 120박자 ≈ 54.9초  → 슬라이더 창 이동 기믹

#define SECOND_WINDOW_GIMMICK_DELAY_BEAT 12
#define PRE_GIMMICK_NOTE_SKIP_TIME (NOTE_BEAT_STEP * BEAT_INTERVAL)

#define WINDOW_JUMP_NOTE_DELAY 450

#define RHYTHM_GAME_DURATION 108000

#define MISS_REACTION_FRAME_COUNT 3
#define MISS_REACTION_FRAME_INTERVAL 120
#define MISS_REACTION_DURATION 1400

#define COMBO_FLASH_DURATION 180
#define COMBO_FLASH_ALPHA 90

#define GIMMICK_FLASH_DURATION 160
#define GIMMICK_FLASH_ALPHA 120

#define WINDOW_SHAKE_DURATION 220
#define WINDOW_SHAKE_POWER 10

#define FINAL_EFFECT_START_TIME (RHYTHM_GAME_DURATION - 15000)

#define FINAL_HITCIRCLE_MOVE_RANGE 260
#define FINAL_HITCIRCLE_MARGIN_X 160
#define FINAL_HITCIRCLE_MARGIN_Y 130

#define FINAL_ANIMATION_FRAME_COUNT 8
#define FINAL_ANIMATION_FRAME_INTERVAL 150

#define FINAL_PLAY_AREA_LEFT_RATIO 32
#define FINAL_PLAY_AREA_RIGHT_RATIO 68

#define FINAL_PLAY_AREA_TOP_MARGIN 170
#define FINAL_PLAY_AREA_BOTTOM_MARGIN 140

#define MAX_COMBO_TARGET 112
#define MAX_COMBO_BONUS_SCORE 2000

#define GRADE_S_SCORE 28000
#define GRADE_A_SCORE 24000
#define GRADE_B_SCORE 20000
#define GRADE_C_SCORE 16000

#define FINAL_SPARKLE_COUNT 80
#define FINAL_SPARKLE_SPAWN_INTERVAL 35
#define FINAL_SPARKLE_LIFETIME 650

#define FINAL_COUNTDOWN_STRONG_TIME 3000


enum ComboCharacterType
{
    COMBO_CHARACTER_NONE,
    COMBO_CHARACTER_10,
    COMBO_CHARACTER_20,
    COMBO_CHARACTER_30
};

struct CursorTrailPoint     // 마우스 커서 구조체
{
    int x;
    int y;
    bool isActive;
    DWORD createTime;
};

struct FinalSparkle
{
    int x;
    int y;
    int size;
    DWORD createTime;
    DWORD lifeTime;
    bool isActive;
};

enum HitJudge           // 클릭판정
{
    JUDGE_NONE,
    JUDGE_PERFECT,
    JUDGE_GOOD,
    JUDGE_BAD,
    JUDGE_MISS
};

// =========================
// 히트서클 정보
// =========================
struct HitCircle
{
    bool isActive;      // 현재 화면에 존재하는가?
    bool isJudged;      // 이미 클릭 판정이 끝났는가?

    int x;              // 원 중심 x좌표
    int y;              // 원 중심 y좌표

    DWORD spawnTime;    // 화면에 생성된 시간
    DWORD hitTime;      // 정확히 눌러야 하는 시간
};


// =========================
// 슬라이더 정보
// =========================
struct Slider
{
    bool isActive;      // 현재 화면에 존재하는가?
    bool isStarted;     // 시작 원을 클릭해서 슬라이더가 시작되었는가?
    bool isFinished;    // 슬라이더 진행이 끝났는가?
    bool isFailed;      // 도중에 실패했는가?
    bool isTrackingSuccess;   // 끝까지 제대로 따라가고 있는가?

    int startX;         // 슬라이더 시작점 x좌표
    int startY;         // 슬라이더 시작점 y좌표
    int endX;           // 슬라이더 끝점 x좌표
    int endY;           // 슬라이더 끝점 y좌표
    bool isWindowFollowTarget;

    DWORD spawnTime;    // 슬라이더가 화면에 생성된 시간
    DWORD hitTime;      // 시작 원을 눌러야 하는 정확한 시간
    DWORD duration;     // 슬라이더 볼이 시작점에서 끝점까지 이동하는 시간
    DWORD slideStartTime;   // 슬라이더 공이 실제로 움직이기 시작한 시간
};


// =========================
// 리듬 미니게임 클래스
// =========================
class RhythmMiniGame
{
private:
    // =========================
    // 게임 기본 상태
    // =========================
    int score;
    bool isGameOver;

    // =========================
    // 마우스 입력 상태
    // =========================
    int mouseX;
    int mouseY;
    bool isMouseDown;

    // =========================
    // 화면 크기
    // =========================
    int screenWidth;
    int screenHeight;
    CImage back;

    // =========================
    // 히트서클 이미지
    // =========================
    CImage hitCircleImg;
    CImage hitCircleOverlayImg;
    CImage approachCircleImg;
    
    // =========================
    // 판정 이미지
    // =========================
    CImage hit300Img;
    CImage hit100Img;
    CImage hit50Img;
    CImage hit0Img;

    // =========================
    // 슬라이더 이미지
    // =========================
    CImage sliderStartCircleImg;
    CImage sliderStartCircleOverlayImg;
    CImage sliderBallImg;
    CImage sliderFollowCircleImg;

    // =========================
    // 커서 이미지
    // =========================
    CImage cursorImg;
    CImage cursorTrailImg;

    // =========================
    // 최근 판정 표시용
    // =========================
    HitJudge lastJudge;
    DWORD judgeDisplayStartTime;
    
    int judgeX;
    int judgeY;

    // =========================
    // 연속 창 점프 기믹
    // =========================
    bool hasWindowJumpGimmickTriggered;
    bool isWindowJumpGimmickActive;

    DWORD windowJumpLastMoveTime;
    int windowJumpStep;
    
    // =========================
    // 히트서클 자동 생성
    // =========================
    DWORD lastHitCircleSpawnTime;
    DWORD hitCircleSpawnInterval;

    // =========================
    // 히트서클 / 슬라이더 저장 배열
    // =========================
    HitCircle hitCircles[MAX_HIT_CIRCLES];
    Slider sliders[MAX_SLIDERS];

    // =========================
    // BGM / 박자 기반 생성
    // =========================
    DWORD bgmStartTime;
    int currentBeatIndex;
    DWORD noteSpawnBlockUntilTime;

    // =========================
    // 창 크기 변화 기믹
    // =========================
    HWND gameHwnd;

    DWORD gameStartTime;
    DWORD windowGimmickStartTime;

    bool hasWindowGimmickTriggered;
    bool isWindowGimmickActive;

    RECT originalWindowRect;

    int originalClientWidth;
    int originalClientHeight;

    // =========================
    // 슬라이더 볼 따라 창 이동 기믹
    // =========================
    bool hasSliderFollowWindowGimmickTriggered;
    bool isSliderFollowWindowGimmickActive;
    int sliderFollowPatternStep;

    RECT sliderFollowBaseWindowRect;

    // =========================
    // 두 번째 작은 창 기믹 예약
    // =========================
    bool isSecondWindowGimmickWaiting;
    DWORD secondWindowGimmickReserveTime;
   
    // =========================
    // 회전 커서 장식
    // =========================
    ULONG_PTR gdiplusToken;
    Gdiplus::Image* cursorMiddleImg;
    float cursorRotationAngle;
    Gdiplus::Image* cursorRotateImg;

    // =========================
    // 콤보 시스템
    // =========================
    int combo;
    DWORD comboEffectStartTime;

    // =========================
    // 콤보 캐릭터 애니메이션
    // =========================
    CImage combo10Frames[COMBO_ANIMATION_FRAME_COUNT];
    CImage combo20Frames[COMBO_ANIMATION_FRAME_COUNT];
    CImage combo30Frames[COMBO_ANIMATION_FRAME_COUNT];

    bool isComboAnimationActive;
    ComboCharacterType currentComboCharacterType;
    DWORD comboAnimationStartTime;
    int comboAnimationFrameIndex;

    // =========================
    // 2번째 창 점프 기믹 노트 지연 생성
    // =========================
    bool isWindowJumpNotePending;
    DWORD windowJumpNoteSpawnTime;

    // =========================
    // Miss 캐릭터 반응
    // =========================
    CImage missReactionFrames[MISS_REACTION_FRAME_COUNT];

    bool isMissReactionActive;
    DWORD missReactionStartTime;
    int missReactionFrameIndex;


    // =========================
    // 화면 플래시
    // =========================
    bool isScreenFlashActive;
    DWORD screenFlashStartTime;
    DWORD screenFlashDuration;
    int screenFlashMaxAlpha;


    // =========================
    // 창 흔들림
    // =========================
    bool isWindowShakeActive;
    DWORD windowShakeStartTime;
    DWORD windowShakeDuration;
    int windowShakePower;
    RECT windowShakeBaseRect;


    // =========================
    // 마지막 5초 연출
    // =========================
    bool hasFinalEffectStarted;
    bool isFinalEffectActive;
    DWORD finalEffectStartTime;
    int finalLastHitX;
    int finalLastHitY;
    bool hasFinalLastHitPosition;

    // =========================
    // 마지막 5초 전용 응원 애니메이션
    // =========================
    CImage finalLeftAnimationFrames[FINAL_ANIMATION_FRAME_COUNT];
    CImage finalRightAnimationFrames[FINAL_ANIMATION_FRAME_COUNT];
    bool isFinalAnimationActive;
    DWORD finalAnimationStartTime;
    int finalAnimationFrameIndex;

    // =========================
    // 게임 종료 / 클리어 연출
    // =========================
    bool isClearEffectActive;
    DWORD clearEffectStartTime;

    // =========================
    // 결과 / 등급
    // =========================
    int maxCombo;
    int finalScore;
    int resultScore100;
    bool isResultCalculated;

    // =========================
    // 마지막 15초 반짝이 효과
    // =========================
    FinalSparkle finalSparkles[FINAL_SPARKLE_COUNT];
    DWORD lastFinalSparkleSpawnTime;
    int finalSparkleIndex;

    // =========================
    // 마지막 3초 카운트다운 강조
    // =========================
    int lastFinalCountdownSecond;



    void LoadEffectSounds();
    void CloseEffectSounds();

    void PlayEffectSound(const wchar_t* aliasName);

    void PlayWindowSound();
    void PlayImpactSound();

    void InitFinalSparkles();
    void SpawnFinalSparkle(DWORD currentTime);
    void UpdateFinalSparkles(DWORD currentTime);
    void RenderFinalSparkles(HDC hDC);

    void UpdateFinalCountdownEffect(DWORD currentTime);

    void CalculateResult();
    const wchar_t* GetGradeText() const;
    void RenderClearEffect(HDC hDC);

    void RenderFinalTimer(HDC hDC);
    void UpdateFinalAnimation(DWORD currentTime);
    void RenderFinalAnimation(HDC hDC);

    void CreateFinalHitCircle();
    void TriggerMissReaction();
    void UpdateMissReaction(DWORD currentTime);
    void RenderMissReaction(HDC hDC);

    void TriggerScreenFlash(int maxAlpha, DWORD duration);
    void UpdateScreenFlash(DWORD currentTime);
    void RenderScreenFlash(HDC hDC);

    void StartWindowShake(DWORD currentTime, DWORD duration, int power);
    void UpdateWindowShake(DWORD currentTime);

    void TriggerGimmickImpact(DWORD currentTime);

    void StartFinalEffect(DWORD currentTime);

    void TriggerComboAnimation();
    void UpdateComboAnimation(DWORD currentTime);
    void RenderComboAnimation(HDC hDC);

    void AddCombo();
    void ResetCombo();

    void UpdateSliderFollowWindowGimmick(DWORD currentTime);
    void TriggerSliderFollowWindowGimmick(DWORD currentTime);
    void MoveWindowWithSliderBall(DWORD currentTime);

    void UpdateWindowGimmick(DWORD currentTime);
    void TriggerWindowGimmick(DWORD currentTime, bool isSecondRun = false);
    void RestoreOriginalWindow(DWORD currentTime);
    void MoveWindowJumpStep(int step, DWORD currentTime);

    void CalculateGimmickClientSize(int& outWidth, int& outHeight);

    int hitCircleCount;     // 현재까지 사용 중인 히트서클 개수
    int sliderCount;        // 현재까지 사용 중인 슬라이더 개수
    int spawnedObjectCount;
    CursorTrailPoint cursorTrail[MAX_CURSOR_TRAIL];
    int cursorTrailIndex;

    void CreateHitCircle(int x, int y);     // 히트서클 1개 생성하는 함수
    void PremultiplyAlpha(CImage& image);   // 이미지 알파 처리 함수(이미지에 있는 빛번짐 같은거 제거)
    void CreateSlider(int startX, int startY, int endX, int endY, bool isWindowFollowTarget = false);  // 슬라이더 생성 함수

    void ClearAllNotes();
    void CreateImmediateHitCircle();

    void UpdateWindowJumpGimmick(DWORD currentTime);
    void TriggerWindowJumpGimmick(DWORD currentTime);
    void CreateSliderFollowPattern(int patternIndex, DWORD currentTime);

    void MoveSliderFollowWindowToRandomPosition();
    void AddCursorTrailPoint(int x, int y, DWORD createTime);

    void PlayHitSound();
    void PlayMissSound();

    void PlayBGM();
    void StopBGM();
    void UpdateBeatSpawn(DWORD currentTime);
    bool ShouldSkipNoteBeforeGimmick(DWORD currentTime, int beatIndex);
    void UpdateWindowJumpDelayedNote(DWORD currentTime);

public:
    RhythmMiniGame();

    void Init(HWND hWnd);
    void Update();
    void Render(HDC hDC);
    void Release();

    void OnMouseDown(int x, int y);
    void OnMouseUp(int x, int y);
    void OnMouseMove(int x, int y);

    bool IsGameOver() const;
    int GetResultScore100() const;
    int GetFinalScore() const;
};