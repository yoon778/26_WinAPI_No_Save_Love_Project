#include "PCroomgame.h"


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

int PCroomgame::makerandomracipe() {
	int order = NOODLE + SOUP + WATER;
	

}

void PCroomgame::Init() {
	score = 0;
	timer = 60;
	finished = false;

	currentRecipe = 0;

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
	{ 160, 200, 393, 317 }, // 1행 1열 - 라면 
	{ 393, 200, 626, 317 }, // 1행 2열 - 스프
	{ 626, 200, 860, 317 }, // 1행 3열 - 물

	{ 160, 317, 393, 435 }, // 2행 1열 - 계란
	{ 393, 317, 626, 435 }, // 2행 2열 - 치즈 
	{ 626, 317, 860, 435 }  // 2행 3열 - 만두
	};

	background.Load(L"pcroom_1920x1080.png");
}

void PCroomgame::MOUSE(int x, int y) {
	if (finished == true)		// 게임이 종료된 상태면 클릭 x
	{
		return;
	}

	if (INsideRect(ingredientArea[0],x,y))
	{
		currentRecipe |= NOODLE;
		// 면 버튼을 클릭한 것
		return;
	}
	if (INsideRect(ingredientArea[1], x, y))
	{
		currentRecipe |= SOUP;
		// 스프 버튼을 클릭한 것
		return;
	}
	if (INsideRect(ingredientArea[2], x, y))
	{
		currentRecipe |= WATER;
		// 물 버튼을 클릭한 것
		return;
	}
	if (INsideRect(ingredientArea[3], x, y))
	{
		currentRecipe |= EGG;
		// 계란 버튼을 클릭한 것
		return;
	}
	if (INsideRect(ingredientArea[4], x, y))
	{
		currentRecipe |= CHEESE;
		// 치즈 버튼을 클릭한 것
		return;
	}
	if (INsideRect(ingredientArea[5], x, y))
	{
		currentRecipe |= DUMPLING;
		// 만두 버튼을 클릭한 것
		return;
	}


	for (int i = 0; i < 6; ++i) {
		if (INsideRect(seatArea[i], x, y)) {		// 좌석 클릭시
			if (currentRecipe == seatorder[i]) {	// 주문한 메뉴와 만든 레시피가 같을 경우
				score += 100;
			}
			else {
				score -= 10;
			}

			currentRecipe = 0;						// 만든 레시피 초기화
			return;
		}
	}

}

void PCroomgame::PAINT(HDC hDC) {

}

int PCroomgame::getscore() {

	return score;
}
