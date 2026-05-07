#pragma once
#include <Windows.h>
#include <string>
#include <atlImage.h>
#include "ramen.h"
#include <atlimage.h>

class PCroomgame {
private:


	int score;		// pc방 게임 점수
	int timer;		// pc방 게임시간
	bool finished;	// pc방 게임의 종료 상태 / 게임이 진행중이면 false, 끝났다면  true
	CImage background;

	RECT ingredientArea[6];		// 라면 재료 위치
	RECT seatArea[6];			// 1~6번 좌석 위치
	POINT seatCenter[6];		// 좌석 중앙 좌표

	ramen seatorder[6];			// 손님이 주문하는 라면
	ramen curramen;				// 현재 플레이어가 만들고 있는 라면

	int moveSpeed = 30;
	POINT playerPos;			// 현재 캐릭터 위치
	POINT targetPos;			// 목표 위치
	POINT homePos;				// 초기 위치
	BOOL ismoving;				// 이동 여부
	BOOL isretrun;				// 배달 후 돌아는 여부
	int seatindex;

	bool InsideRect(RECT rect, int x, int y);

	void DrawTextW(HDC hDC, int x, int y, std::wstring text);
	std::wstring RamenToString(ramen r);

	void DeliverToSeat(int seatIndex);

public:
	PCroomgame();

	void Init();	// pc방 게임이 시작되면 실행하는 함수 
	void MOUSE(int x, int y);
	void PAINT(HDC hDC);
	void Update();		// 게임이 진행되면 자동으로 바뀌는 것을 처리하는 함수(ex: timer)
	bool Finishedgame();
	int getscore();

};