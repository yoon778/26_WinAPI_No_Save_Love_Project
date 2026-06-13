#pragma once
#include <Windows.h>
#include <string>
#include <atlImage.h>
#include "ramen.h"
#include <atlimage.h>

class PCroomgame {
private:
	std::wstring resultMessage; // 성공/실패 메시지
	int resultTimer;            // 메시지를 보여줄 시간
	bool lastDeliverySucceeded; // 마지막 배달 결과

	int score;		// pc방 게임 점수
	double timer;		// pc방 게임시간
	bool finished;	// pc방 게임의 종료 상태 / 게임이 진행중이면 false, 끝났다면  true
	CImage background;
	CImage player;
	CImage playerReturn;
	CImage ingredientImg;
	CImage waterImg;
	CImage resetBtnImg;
	CImage boilingWaterImg;
	CImage successFeedbackImg;
	CImage failFeedbackImg;

	HFONT hudFont;
	CImage hudImg;
	RECT hudArea;

	void DrawHudNumber(HDC hDC, RECT rc, std::wstring text, COLORREF color);

	int frameWidth;       // 한 프레임 너비
	int frameHeight;      // 한 프레임 높이

	int currentFrame;     // 현재 프레임 번호
	int maxFrame;         // 한 행의 프레임 수

	int animTick;         // 애니메이션 속도 조절용
	int animDelay;        // 몇 번 Update마다 프레임 바꿀지

	int currentDir;       // 현재 방향

	RECT resetArea;	// 재료 초기화 버튼 위치
	RECT ingredientArea[6];		// 라면 재료 위치
	RECT seatArea[6];			// 1~6번 좌석 위치
	POINT deliveryPos[6];		// 좌석 중앙 좌표

	ramen seatorder[6];			// 손님이 주문하는 라면
	ramen curramen;				// 현재 플레이어가 만들고 있는 라면
	RECT bowlArea[3];


	int moveSpeed = 30;
	POINT playerPos;			// 현재 캐릭터 위치
	POINT targetPos;			// 목표 위치
	POINT homePos;				// 초기 위치
	bool ismoving;				// 이동 여부
	bool isreturn;				// 배달 후 돌아는 여부
	int seatindex;
	RECT waterArea[2];

	bool InsideRect(RECT rect, int x, int y);

	void DrawTextW(HDC hDC, int x, int y, std::wstring text);
	std::wstring RamenToString(ramen r);

	void DeliverToSeat(int seatIndex);
	void StartDelivery(int index);

	void UpdateAnimation();
	void DrawPlayer(HDC hDC);
	void DrawDeliveryFeedback(HDC hDC);

	enum CustomerState
	{
		ORDER1 = 0,   // 주문
		ORDER2 = 1,   // 먹는 중
		ORDER3 = 2    // 화남
	};

	CImage customerImg[3];     // 0=order1, 1=order2, 2=order3

	CustomerState customerState[6];
	int customerFrame[6];
	int customerAnimTick[6];
	int customerStateTimer[6];

	int customerAnimDelay = 4;



	int boilingWaterFrame;
	int boilingWaterAnimTick;
	int boilingWaterAnimDelay;

	void UpdateBoilingWaterAnimation();

	void UpdateCustomers();
	void DrawCustomers(HDC hDC);
	void DrawOneCustomer(HDC hDC, int index);
	void SetCustomerState(int index, CustomerState state, int duration);

	void DrawOrderBubble(HDC hDC, int index);

	void DrawIngredientImage(HDC hDC, RECT rc, int imageIndex, bool selected);

	int waterFrame;
	int waterAnimTick;
	int waterAnimDelay;

	void DrawWaterButton(HDC hDC, RECT rc);
	void UpdateWaterAnimation();
	void DrawBowls(HDC hDC);

	enum CenterPotState
	{
		POT_BOILING_WATER = 0,

		POT_ADD_WATER,
		POT_ADD_NOODLE,
		POT_BOILING_NOODLE,

		POT_ADD_SOUP,

		POT_ADD_CHEESE,
		POT_BOILING_CHEESE,

		POT_ADD_DUMPLING,
		POT_BOILING_DUMPLING,

		POT_ADD_EGG,
		POT_BOILING_EGG
	};

	// 재료 넣기 애니메이션
	CImage addWaterImg;
	CImage addNoodleImg;
	CImage addSoupImg;
	CImage addCheeseImg;
	CImage addDumplingImg;
	CImage addEggImg;

	// 넣은 뒤 유지할 끓는 애니메이션
	CImage boilingNoodleImg;
	CImage boilingCheeseImg;
	CImage boilingDumplingImg;
	CImage boilingEggImg;

	CenterPotState centerPotState;

	int centerPotFrame;
	int centerPotAnimTick;
	int centerPotAnimDelay;

	bool centerPotAnimating;

	void StartCenterPotAnimation(CenterPotState state);
	void UpdateCenterPotAnimation();
	void ResetCenterPotState();

	void DrawThreeFrameImage(HDC hDC, CImage& img, RECT rc, int frame);
public:
	PCroomgame();

	void Init();	// pc방 게임이 시작되면 실행하는 함수 
	void MOUSE(int x, int y);
	void PAINT(HDC hDC);
	void KEYDOWN(WPARAM wParam);
	void Update();		// 게임이 진행되면 자동으로 바뀌는 것을 처리하는 함수(ex: timer)
	int getscore();
	bool isfinished();
	void Release();		// 미니게임에서 사용한 리소스 자원 정리하는 함수(WM_DESTORY에 들어가는거)
};
