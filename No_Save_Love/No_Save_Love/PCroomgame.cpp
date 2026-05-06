#include "PCroomgame.h"
#include "ramen.h"

PCroomgame::PCroomgame() {
	score = 0;
	timer = 60;
	finished = false;
}

bool INsideRect(RECT rect, int x, int y) {			// 마우스로 클릭한 좌표 안에 들어가는지 검사하는 함수
	if (x >= rect.left && x <= rect.right &&
		y >= rect.top && y <= rect.bottom)
	{
		return true;
	}

	return false;
}



void PCroomgame::Init() {
	score = 0;
	timer = 60;
	finished = false;


	RECT seatArea[6] = {		// 마우스 클릭 판정을 위한 좌석 좌표
	{ 1155, 150, 1420, 395 }, // 1번 좌석
	{ 1155, 405, 1420, 665 }, // 2번 좌석
	{ 1155, 675, 1420, 930 }, // 3번 좌석

	{ 1548, 150, 1810, 395 }, // 4번 좌석
	{ 1548, 405, 1810, 665 }, // 5번 좌석
	{ 1548, 675, 1810, 930 }  // 6번 좌석
	};

	POINT seatCenter[6] = {		// 캐릭터 배달할 위치 좌표
	{ 1287, 272 }, // 1번
	{ 1287, 535 }, // 2번
	{ 1287, 802 }, // 3번

	{ 1679, 272 }, // 4번
	{ 1679, 535 }, // 5번
	{ 1679, 802 }  // 6번
	};

	RECT ingredientArea[6] = {		// 라면 재료 마우스 클릭 판정을 위한 좌표
	{ 160, 200, 393, 317 }, // 1행 1열 - 면 
	{ 393, 200, 626, 317 }, // 1행 2열 - 스프
	{ 626, 200, 860, 317 }, // 1행 3열 - 물

	{ 160, 317, 393, 435 }, // 2행 1열 - 계란
	{ 393, 317, 626, 435 }, // 2행 2열 - 치즈 
	{ 626, 317, 860, 435 }  // 2행 3열 - 만두
	};

	curramen.clear();
	for (int i = 0; i < 6; i++) {
		seatorder[i].makerandramen();
	}

	background.Load(L"pcroom_1920x1080.png");
}

void PCroomgame::MOUSE(int x, int y) {
	if (finished == true)		// 게임이 종료된 상태면 클릭 x
	{
		return;
	}
	if (INsideRect(ingredientArea[0], x, y)) {
		curramen.noodle = true;
	}
	if (INsideRect(ingredientArea[1], x, y)) {
		curramen.soup = true;
	}
	if (INsideRect(ingredientArea[2], x, y)) {
		curramen.water = true;
	}
	if (INsideRect(ingredientArea[3], x, y)) {
		curramen.egg = true;
	}
	if (INsideRect(ingredientArea[4], x, y)) {
		curramen.cheese = true;
	}
	if (INsideRect(ingredientArea[5], x, y)) {
		curramen.dumpling = true;
	}

}

bool PCroomgame::INsideRect(RECT rect, int x, int y) {
	if (x >= rect.left && x <= rect.right &&
		y >= rect.top && y <= rect.bottom)
	{
		return true;
	}

	return false;
}

void PCroomgame::PAINT(HDC hDC) {

}

int PCroomgame::getscore() {

	return score;
}
