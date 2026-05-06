#pragma once
#include <Windows.h>
#include <string>
#include <atlImage.h>

class PCroomgame {
private:
	const int NOODLE = 1;
	const int SOUP = 2;
	const int WATER = 4;
	const int EGG = 8;
	const int CHEESE = 16;
	const int DUMPLING = 32;

	int score;		// pc방 게임 점수
	int timer;		// pc방 게임시간
	bool finished;	// pc방 게임의 종료 상태 / 게임이 진행중이면 false, 끝났다면  true
	CImage background;
	
	int currentRecipe;	// 현재 플레이어가 만든 라면조합을 저장하는 변수
	int seatorder[6];	// 손님 주문을 저장하는 배열
	RECT ingredientArea[6];		// 라면 재료 위치
	RECT seatArea[6];			// 1~6번 좌석 위치
	POINT seatCenter[6];		// 좌석 중앙 좌표


	bool INsideRect(RECT rect, int x, int y);
	int makerandomracipe();
	

public:
	void Init();	// pc방 게임이 시작되면 실행하는 함수 
	void MOUSE(int x, int y);		
	void PAINT(HDC hDC);
	void Update();		// 게임이 진행되면 자동으로 바뀌는 것을 처리하는 함수(ex: timer)
	bool Finishedgame();
	int getscore();

};