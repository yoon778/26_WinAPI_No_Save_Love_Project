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

	int score;		// pc방 게임 점수
	double timer;		// pc방 게임시간
	bool finished;	// pc방 게임의 종료 상태 / 게임이 진행중이면 false, 끝났다면  true
	CImage background;

	RECT resetArea;	// 재료 초기화 버튼 위치
	RECT ingredientArea[6];		// 라면 재료 위치
	RECT seatArea[6];			// 1~6번 좌석 위치
	POINT deliveryPos[6];		// 좌석 중앙 좌표

	ramen seatorder[6];			// 손님이 주문하는 라면
	ramen curramen;				// 현재 플레이어가 만들고 있는 라면

	int moveSpeed = 30;
	POINT playerPos;			// 현재 캐릭터 위치
	POINT targetPos;			// 목표 위치
	POINT homePos;				// 초기 위치
	BOOL ismoving;				// 이동 여부
	BOOL isreturn;				// 배달 후 돌아는 여부
	int seatindex;

	bool InsideRect(RECT rect, int x, int y);

	void DrawTextW(HDC hDC, int x, int y, std::wstring text);
	std::wstring RamenToString(ramen r);

	void DeliverToSeat(int seatIndex);
	void StartDelivery(int index);

public:
	PCroomgame();

	void Init();	// pc방 게임이 시작되면 실행하는 함수 
	void MOUSE(int x, int y);
	void PAINT(HDC hDC);
	void KEYDOWN(WPARAM wParam);
	void Update();		// 게임이 진행되면 자동으로 바뀌는 것을 처리하는 함수(ex: timer)
	bool Finishedgame();
	int getscore();
	bool isfinished();
};