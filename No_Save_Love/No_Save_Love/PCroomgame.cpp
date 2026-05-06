#include "PCroomgame.h"
#include "ramen.h"

PCroomgame::PCroomgame() {
	score = 0;
	timer = 60;
	finished = false;
}

bool PCroomgame::InsideRect(RECT rect, int x, int y) {			// 마우스로 클릭한 좌표 안에 들어가는지 검사하는 함수
	if (x >= rect.left && x <= rect.right &&
		y >= rect.top && y <= rect.bottom)
	{
		return true;
	}

	return false;
}

std::wstring PCroomgame::RamenToString(ramen r)
{
	std::wstring result = L"";
	if (r.noodle == true)
	{
		result += L"면 ";
	}
	if (r.soup == true)
	{
		result += L"스프 ";
	}
	if (r.water == true)
	{
		result += L"물 ";
	}
	if (r.egg == true)
	{
		result += L"계란 ";
	}
	if (r.cheese == true)
	{
		result += L"치즈 ";
	}
	if (r.dumpling == true)
	{
		result += L"만두 ";
	}
	if (result == L"")
	{
		result = L"없음";
	}
	return result;
}

void PCroomgame::DrawTextW(HDC hDC, int x, int y, std::wstring text)	// 글 쓰는 함수
{
	TextOutW(hDC, x, y, text.c_str(), (int)text.length());
}

void PCroomgame::Init() {
	score = 0;
	timer = 60;
	finished = false;

	// 캐릭터 배달할 위치 좌표
	seatArea[0] = { 1155, 150, 1420, 395 };		// 1번자리
	seatArea[1] = { 1155, 405, 1420, 665 };		// 2번 자리
	seatArea[2] = { 1155, 675, 1420, 930 };		// 3번 자리		

	seatArea[3] = { 1548, 150, 1810, 395 };		// 4번 자리
	seatArea[4] = { 1548, 405, 1810, 665 };		// 5번 자리
	seatArea[5] = { 1548, 675, 1810, 930 };		// 6번 자리

	// 자리 중앙 위치
	seatCenter[0] = { 1287, 272 }; // 1번
	seatCenter[0] = { 1287, 535 }; // 2번
	seatCenter[0] = { 1287, 802 }; // 3번

	seatCenter[0] = { 1679, 272 }; // 4번
	seatCenter[0] = { 1679, 535 }; // 5번
	seatCenter[0] = { 1679, 802 };  // 6번
	

	// 재료 버튼 위치
	ingredientArea[0] = { 160, 200, 393, 317 }; // 면
	ingredientArea[1] = { 393, 200, 626, 317 }; // 스프
	ingredientArea[2] = { 626, 200, 860, 317 }; // 물

	ingredientArea[3] = { 160, 317, 393, 435 }; // 계란
	ingredientArea[4] = { 393, 317, 626, 435 }; // 치즈
	ingredientArea[5] = { 626, 317, 860, 435 }; // 만두

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
	if (InsideRect(ingredientArea[0], x, y)) {
		curramen.noodle = true;
		return;
	}
	if (InsideRect(ingredientArea[1], x, y)) {
		curramen.soup = true;
		return;
	}
	if (InsideRect(ingredientArea[2], x, y)) {
		curramen.water = true;
		return;
	}
	if (InsideRect(ingredientArea[3], x, y)) {
		curramen.egg = true;
		return;
	}
	if (InsideRect(ingredientArea[4], x, y)) {
		curramen.cheese = true;
		return;
	}
	if (InsideRect(ingredientArea[5], x, y)) {
		curramen.dumpling = true;
		return;
	}

	for (int i = 0; i < 6; i++)
	{
		if (InsideRect(seatArea[i], x, y))
		{
			if (curramen.ramenSame(seatorder[i]))
			{
				score += 100;
			}
			else
			{
				score -= 50;
			}

			curramen.clear();
			seatorder[i].makerandramen();

			return;
		}
	}
}

void PCroomgame::PAINT(HDC hDC) {
	background.Draw(hDC, 0, 0, 1920, 1080);		// 배경 그리기

	// 재료 버튼 그리기
	Rectangle(hDC, ingredientArea[0].left, ingredientArea[0].top,
		ingredientArea[0].right, ingredientArea[0].bottom);
	DrawTextW(hDC, ingredientArea[0].left + 80, ingredientArea[0].top + 45, L"면");

	Rectangle(hDC, ingredientArea[1].left, ingredientArea[1].top,
		ingredientArea[1].right, ingredientArea[1].bottom);
	DrawTextW(hDC, ingredientArea[1].left + 80, ingredientArea[1].top + 45, L"스프");

	Rectangle(hDC, ingredientArea[2].left, ingredientArea[2].top,
		ingredientArea[2].right, ingredientArea[2].bottom);
	DrawTextW(hDC, ingredientArea[2].left + 80, ingredientArea[2].top + 45, L"물");

	Rectangle(hDC, ingredientArea[3].left, ingredientArea[3].top,
		ingredientArea[3].right, ingredientArea[3].bottom);
	DrawTextW(hDC, ingredientArea[3].left + 80, ingredientArea[3].top + 45, L"계란");

	Rectangle(hDC, ingredientArea[4].left, ingredientArea[4].top,
		ingredientArea[4].right, ingredientArea[4].bottom);
	DrawTextW(hDC, ingredientArea[4].left + 80, ingredientArea[4].top + 45, L"치즈");

	Rectangle(hDC, ingredientArea[5].left, ingredientArea[5].top,
		ingredientArea[5].right, ingredientArea[5].bottom);
	DrawTextW(hDC, ingredientArea[5].left + 80, ingredientArea[5].top + 45, L"만두");

	// 점수 출력
	std::wstring scoreText = L"점수: " + std::to_wstring(score);
	DrawTextW(hDC, 50, 50, scoreText);

	// 남은 시간 출력
	std::wstring timerText = L"남은 시간: " + std::to_wstring(timer);
	DrawTextW(hDC, 50, 80, timerText);

	// 현재 만든 라면 출력
	std::wstring currentText = L"현재 라면: " + RamenToString(curramen);
	DrawTextW(hDC, 50, 120, currentText);

	// 좌석별 주문 출력
	for (int i = 0; i < 6; i++)
	{
		Rectangle(hDC, seatArea[i].left, seatArea[i].top,
			seatArea[i].right, seatArea[i].bottom);

		std::wstring seatText = L"좌석 " + std::to_wstring(i + 1);
		DrawTextW(hDC, seatArea[i].left + 20, seatArea[i].top + 20, seatText);

		std::wstring orderText = L"주문: " + RamenToString(seatorder[i]);
		DrawTextW(hDC, seatArea[i].left + 20, seatArea[i].top + 55, orderText);
	}

	// 게임 종료 출력
	// 7. 게임 종료 상태 출력
	if (finished == true)
	{
		DrawTextW(hDC, 800, 500, L"게임 종료!");
		DrawTextW(hDC, 800, 540, L"최종 점수: " + std::to_wstring(score));
	}
}

int PCroomgame::getscore() {

	return score;
}
