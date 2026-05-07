#include "PCroomgame.h"
#include "ramen.h"
#include <ctime>

PCroomgame::PCroomgame() {
	srand((unsigned int)time(NULL));

	score = 0;
	timer = 180.0;
	finished = false;

	playerPos = { 1495, 1000 };
	targetPos = playerPos;
	ismoving = false;
	seatindex = -1;
	isreturn = false;
	homePos = { 1495, 1000 };

	resultMessage = L"";
	resultTimer = 0;
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

	// 1. 토핑 먼저 출력
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

	// 2. 면 + 스프 + 물이 모두 있으면 마지막에 "라면" 출력
	if (r.noodle == true && r.soup == true && r.water == true)
	{
		result += L"라면 ";
	}
	else
	{
		// 아직 기본 라면이 완성되지 않은 경우에는 재료 그대로 출력
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
	}

	// 3. 아무것도 없으면 없음 출력
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
	timer = 180.0;
	finished = false;

	playerPos = { 1495, 1000 };
	targetPos = playerPos;
	ismoving = false;
	seatindex = -1;
	isreturn = false;
	homePos = { 1495, 1000 };

	resultMessage = L"";
	resultTimer = 0;

	// 캐릭터 배달할 위치 좌표
	seatArea[0] = { 1155, 150, 1420, 395 };		// 1번자리
	seatArea[1] = { 1155, 405, 1420, 665 };		// 2번 자리
	seatArea[2] = { 1155, 675, 1420, 930 };		// 3번 자리		

	seatArea[3] = { 1548, 150, 1810, 395 };		// 4번 자리
	seatArea[4] = { 1548, 405, 1810, 665 };		// 5번 자리
	seatArea[5] = { 1548, 675, 1810, 930 };		// 6번 자리

	deliveryPos[0] = { 1445, 272 }; // 1번 좌석 앞 통로
	deliveryPos[1] = { 1445, 535 }; // 2번 좌석 앞 통로
	deliveryPos[2] = { 1445, 802 }; // 3번 좌석 앞 통로

	deliveryPos[3] = { 1523, 272 }; // 4번 좌석 앞 통로
	deliveryPos[4] = { 1523, 535 }; // 5번 좌석 앞 통로
	deliveryPos[5] = { 1523, 802 }; // 6번 좌석 앞 통로
	
	// 재료 초기화 버튼 위치
	resetArea = { 160, 200, 393, 317 };

	// 재료 버튼 위치
	ingredientArea[0] = { 70, 540, 325, 670 };  // 물
	ingredientArea[1] = { 393, 200, 626, 317 }; // 면
	ingredientArea[2] = { 626, 200, 860, 317 }; // 스푸

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

	// 초기화 버튼 클릭
	if (InsideRect(resetArea, x, y))
	{
		curramen.clear();
		return;
	}

	// 재료 버튼 클릭시 현재 라면에 추가
	if (InsideRect(ingredientArea[0], x, y)) {
		curramen.water = true;
		return;
	}
	if (InsideRect(ingredientArea[1], x, y)) {
		curramen.noodle = true;
		return;
	}
	if (InsideRect(ingredientArea[2], x, y)) {
		curramen.soup = true;
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

	// 좌석 클릭시
	for (int i = 0; i < 6; i++)
	{
		if (InsideRect(seatArea[i], x, y))
		{
			StartDelivery(i);
			return;
		}
	}
}

void PCroomgame::Update() {
	if (finished == true)
	{
		return;
	}

	timer -= 0.1;

	if (timer <= 0.0)
	{
		timer = 0.0;
		finished = true;
		ismoving = false;
		isreturn = false;
		return;
	}

	if (resultTimer > 0)
	{
		resultTimer--;

		if (resultTimer == 0)
		{
			resultMessage = L"";
		}
	}


	if (ismoving == false)
	{
		return;
	}

	// =========================
	// 1. 좌석으로 가는 중
	// y 먼저 이동, y가 같으면 x 이동
	// =========================
	if (isreturn == false)
	{
		// y 방향 먼저 이동
		if (playerPos.y < targetPos.y)
		{
			playerPos.y += moveSpeed;

			if (playerPos.y > targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}
		else if (playerPos.y > targetPos.y)
		{
			playerPos.y -= moveSpeed;

			if (playerPos.y < targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}

		// y가 같아진 후 x 이동
		if (playerPos.x < targetPos.x)
		{
			playerPos.x += moveSpeed;

			if (playerPos.x > targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}
		else if (playerPos.x > targetPos.x)
		{
			playerPos.x -= moveSpeed;

			if (playerPos.x < targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}

		// 좌석 도착
		if (playerPos.x == targetPos.x && playerPos.y == targetPos.y)
		{
			if (seatindex != -1)
			{
				DeliverToSeat(seatindex);
				seatindex = -1;
			}

			// 이제 집으로 돌아가기 시작
			targetPos = homePos;
			isreturn = true;
			ismoving = true;

			return;
		}
	}

	// =========================
	// 2. 초기 위치로 돌아오는 중
	// x 먼저 이동, x가 같으면 y 이동
	// =========================
	else if (isreturn == true)
	{
		// x 방향 먼저 이동
		if (playerPos.x < targetPos.x)
		{
			playerPos.x += moveSpeed;

			if (playerPos.x > targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}
		else if (playerPos.x > targetPos.x)
		{
			playerPos.x -= moveSpeed;

			if (playerPos.x < targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}

		// x가 같아진 후 y 이동
		if (playerPos.y < targetPos.y)
		{
			playerPos.y += moveSpeed;

			if (playerPos.y > targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}
		else if (playerPos.y > targetPos.y)
		{
			playerPos.y -= moveSpeed;

			if (playerPos.y < targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}

		// 초기 위치 도착
		if (playerPos.x == targetPos.x && playerPos.y == targetPos.y)
		{
			ismoving = false;
			isreturn = false;
			targetPos = playerPos;

			return;
		}
	}


}

void PCroomgame::DeliverToSeat(int seatIndex)		// 배달한 좌석 검사
{
	if (seatIndex < 0 || seatIndex >= 6)
	{
		return;
	}

	if (curramen.ramenSame(seatorder[seatIndex]))
	{
		score += 100;
		resultMessage = L"주문 성공! +100";
	}
	else
	{
		score -= 50;
		resultMessage = L"주문 실패! -50";
	}
	resultTimer = 15; // WM_TIMER가 0.1초라면 약 1.5초 표시

	curramen.clear();
	seatorder[seatIndex].makerandramen();
}

void PCroomgame::PAINT(HDC hDC) {
	background.Draw(hDC, 0, 0, 1920, 1080);		// 배경 그리기

	// 초기화 버튼 그리기
	Rectangle(hDC, resetArea.left, resetArea.top,
		resetArea.right, resetArea.bottom);
	DrawTextW(hDC, resetArea.left + 70, resetArea.top + 45, L"초기화");

	// 재료 버튼 그리기
	Rectangle(hDC, ingredientArea[0].left, ingredientArea[0].top,
		ingredientArea[0].right, ingredientArea[0].bottom);
	DrawTextW(hDC, ingredientArea[0].left + 100, ingredientArea[0].top + 50, L"물");

	Rectangle(hDC, ingredientArea[1].left, ingredientArea[1].top,
		ingredientArea[1].right, ingredientArea[1].bottom);
	DrawTextW(hDC, ingredientArea[1].left + 80, ingredientArea[1].top + 45, L"면");

	Rectangle(hDC, ingredientArea[2].left, ingredientArea[2].top,
		ingredientArea[2].right, ingredientArea[2].bottom);
	DrawTextW(hDC, ingredientArea[2].left + 80, ingredientArea[2].top + 45, L"스프");

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
	wchar_t timerBuffer[100];								// 소수점 너무 길게 보이지 않기 위해 사용
	swprintf_s(timerBuffer, L"남은 시간: %.1f", timer);		// 소수점 너무 길게 보이지 않기 위해 사용
	DrawTextW(hDC, 50, 80, timerBuffer);

	// 현재 만든 라면 출력
	std::wstring currentText = L"현재 라면: " + RamenToString(curramen);
	DrawTextW(hDC, 50, 120, currentText);

	// 성공/실패 메시지 출력
	if (resultMessage != L"")
	{
		DrawTextW(hDC, 50, 160, resultMessage);
	}

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

	// 플레이어 그리기
	Ellipse(hDC, playerPos.x - 25, playerPos.y - 25, playerPos.x + 25, playerPos.y + 25);
	DrawTextW(hDC, playerPos.x - 20, playerPos.y - 10, L"윤서");
}

int PCroomgame::getscore() {

	return score;
}
void PCroomgame::StartDelivery(int index)
{
	if (index < 0 || index >= 6)
	{
		return;
	}

	targetPos = deliveryPos[index];   // 또는 deliveryPos[index]를 쓰고 있다면 deliveryPos[index]
	ismoving = true;
	isreturn = false;
	seatindex = index;
}

void PCroomgame::KEYDOWN(WPARAM wParam) {
	if (finished == true)
	{
		return;
	}

	if (wParam >= '1' && wParam <= '6')
	{
		int index = (int)(wParam - '1');
		StartDelivery(index);
		return;
	}

}
