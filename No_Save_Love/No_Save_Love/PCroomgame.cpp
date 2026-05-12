#include "PCroomgame.h"
#include "ramen.h"
#include <ctime>

#define DIR_FRONT 0
#define DIR_BACK  1
#define DIR_RIGHT 2
#define DIR_LEFT  3

PCroomgame::PCroomgame() {
	srand((unsigned int)time(NULL));

	// 새 플레이어 이미지 기준
	// 전체 이미지: 1086 x 1448
	// 3열 x 4행 구조
	frameWidth = 362;
	frameHeight = 362;

	currentFrame = 1;	// 정지 기본 프레임은 가운데 이미지
	maxFrame = 3;		// 방향당 3프레임
	animTick = 0;
	animDelay = 3;

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
	currentDir = DIR_FRONT;
}

bool PCroomgame::InsideRect(RECT rect, int x, int y) {
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

void PCroomgame::DrawTextW(HDC hDC, int x, int y, std::wstring text)
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

	currentFrame = 1;
	animTick = 0;
	currentDir = DIR_FRONT;

	// 캐릭터 배달할 위치 좌표
	seatArea[0] = { 1155, 150, 1420, 395 };		// 1번 자리
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
	ingredientArea[2] = { 626, 200, 860, 317 }; // 스프

	ingredientArea[3] = { 160, 317, 393, 435 }; // 계란
	ingredientArea[4] = { 393, 317, 626, 435 }; // 치즈
	ingredientArea[5] = { 626, 317, 860, 435 }; // 만두

	curramen.clear();

	for (int i = 0; i < 6; i++) {
		seatorder[i].makerandramen();
	}

	background.Load(L"pcroom_1920x1080.png");
	player.Load(L"ramen_player.png");
	playerReturn.Load(L"playerreturn.png");

	customerImg[0].Load(L"order1.png"); // 주문하는 모습
	customerImg[1].Load(L"order2.png"); // 라면 먹는 모습
	customerImg[2].Load(L"order3.png"); // 화내는 모습

	for (int i = 0; i < 6; i++)
	{
		customerState[i] = ORDER1;
		customerFrame[i] = 0;
		customerAnimTick[i] = 0;
		customerStateTimer[i] = 0;
	}
}

void PCroomgame::MOUSE(int x, int y) {
	if (finished == true)
	{
		return;
	}

	// 초기화 버튼 클릭
	if (InsideRect(resetArea, x, y))
	{
		curramen.clear();
		return;
	}

	// 재료 버튼 클릭 시 현재 라면에 추가
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

	// 좌석 클릭 시
	for (int i = 0; i < 6; i++)
	{
		if (InsideRect(seatArea[i], x, y))
		{
			StartDelivery(i);
			return;
		}
	}
}

void PCroomgame::Update()
{
	// =========================
	// 1. 이미 게임이 끝났다면 아무것도 하지 않음
	// =========================
	if (finished == true)
	{
		return;
	}

	// =========================
	// 2. 제한시간 감소
	// SetTimer가 100ms라면 Update() 한 번마다 0.1초 감소
	// =========================
	timer -= 0.1;

	if (timer <= 0.0)
	{
		timer = 0.0;
		finished = true;

		ismoving = false;
		isreturn = false;

		currentFrame = 1;
		animTick = 0;

		return;
	}

	// 손님 애니메이션은 플레이어 이동과 상관없이 항상 갱신
	UpdateCustomers();

	if (ismoving == false)
	{
		currentFrame = 1;
		animTick = 0;
		return;
	}

	// =========================
	// 3. 현재 이동 중이 아니라면
	// 캐릭터는 정지 프레임으로 유지
	// =========================
	if (ismoving == false)
	{
		currentFrame = 1;
		animTick = 0;
		return;
	}
	UpdateAnimation();

	// ==================================================
	// 4. 좌석으로 배달하러 가는 중
	//    이동 순서: y 먼저 이동 → x 이동
	// ==================================================
	if (isreturn == false)
	{
		// -------------------------
		// 4-1. y 방향 먼저 이동
		// -------------------------
		if (playerPos.y < targetPos.y)
		{
			// 아래쪽으로 이동
			currentDir = DIR_FRONT;

			playerPos.y += moveSpeed;

			if (playerPos.y > targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}
		else if (playerPos.y > targetPos.y)
		{
			// 위쪽으로 이동
			currentDir = DIR_BACK;


			playerPos.y -= moveSpeed;

			if (playerPos.y < targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}

		// -------------------------
		// 4-2. y가 같아졌다면 x 이동
		// -------------------------
		if (playerPos.x < targetPos.x)
		{
			// 오른쪽으로 이동
			currentDir = DIR_RIGHT;


			playerPos.x += moveSpeed;

			if (playerPos.x > targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}
		else if (playerPos.x > targetPos.x)
		{
			// 왼쪽으로 이동
			currentDir = DIR_LEFT;


			playerPos.x -= moveSpeed;

			if (playerPos.x < targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}

		// -------------------------
		// 4-3. 좌석 앞 배달 위치에 도착
		// -------------------------
		if (playerPos.x == targetPos.x && playerPos.y == targetPos.y)
		{
			if (seatindex != -1)
			{
				DeliverToSeat(seatindex);
				seatindex = -1;
			}

			// 초기 위치로 복귀 시작
			targetPos = homePos;
			isreturn = true;
			ismoving = true;

			return;
		}
	}

	// ==================================================
	// 5. 초기 위치로 돌아오는 중
	//    이동 순서: x 먼저 이동 → y 이동
	// ==================================================
	else if (isreturn == true)
	{
		// -------------------------
		// 5-1. x 방향 먼저 이동
		// -------------------------
		if (playerPos.x < targetPos.x)
		{
			// 오른쪽으로 이동
			currentDir = DIR_RIGHT;


			playerPos.x += moveSpeed;

			if (playerPos.x > targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}
		else if (playerPos.x > targetPos.x)
		{
			// 왼쪽으로 이동
			currentDir = DIR_LEFT;


			playerPos.x -= moveSpeed;

			if (playerPos.x < targetPos.x)
			{
				playerPos.x = targetPos.x;
			}

			return;
		}

		// -------------------------
		// 5-2. x가 같아졌다면 y 이동
		// -------------------------
		if (playerPos.y < targetPos.y)
		{
			// 아래쪽으로 이동
			currentDir = DIR_FRONT;


			playerPos.y += moveSpeed;

			if (playerPos.y > targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}
		else if (playerPos.y > targetPos.y)
		{
			// 위쪽으로 이동
			currentDir = DIR_BACK;


			playerPos.y -= moveSpeed;

			if (playerPos.y < targetPos.y)
			{
				playerPos.y = targetPos.y;
			}

			return;
		}

		// -------------------------
		// 5-3. 초기 위치까지 복귀 완료
		// -------------------------
		if (playerPos.x == targetPos.x && playerPos.y == targetPos.y)
		{
			ismoving = false;
			isreturn = false;

			targetPos = playerPos;

			currentFrame = 1;
			animTick = 0;

			return;
		}
	}
}

void PCroomgame::UpdateAnimation()
{
	animTick++;

	if (animTick >= animDelay)
	{
		currentFrame++;

		if (currentFrame >= maxFrame)
		{
			currentFrame = 0;
		}

		animTick = 0;
	}
}

void PCroomgame::DeliverToSeat(int seatIndex)
{
	if (seatIndex < 0 || seatIndex >= 6)
	{
		return;
	}

	if (curramen.ramenSame(seatorder[seatIndex]))
	{
		score += 100;
		resultMessage = L"주문 성공! +100";

		// 손님이 라면 먹는 상태로 30틱(약 3초) 전환
		SetCustomerState(seatIndex, ORDER2, 30);
	}
	else
	{
		score -= 50;
		resultMessage = L"주문 실패! -50";

		// 손님이 화내는 상태로 20틱(약 2초) 전환
		SetCustomerState(seatIndex, ORDER3, 20);
	}

	resultTimer = 15;
	curramen.clear();
}

void PCroomgame::DrawPlayer(HDC hDC)
{
	CImage* drawImage = nullptr;

	// 복귀 중이면 빈손 이미지, 아니면 라면 든 이미지
	if (isreturn == true)
		drawImage = &playerReturn;
	else
		drawImage = &player;

	if (drawImage == nullptr || drawImage->IsNull())
	{
		Ellipse(hDC,
			playerPos.x - 25,
			playerPos.y - 25,
			playerPos.x + 25,
			playerPos.y + 25);
		return;
	}

	// --------------------------------------------------
	// [방향][프레임]
	// 방향 순서:
	// 0 = 앞
	// 1 = 뒤
	// 2 = 오른쪽
	// 3 = 왼쪽
	// --------------------------------------------------
	static const RECT playerSrc[4][3] =
	{
		// 앞
		{
			{171,  33, 333, 342},
			{457,  33, 620, 342},
			{743,  33, 907, 342}
		},

		// 뒤
		{
			{176, 379, 329, 724},
			{460, 381, 617, 724},
			{748, 380, 904, 724}
		},

		// 오른쪽
		{
			{167, 724, 340, 1086},
			{456, 724, 630, 1086},
			{742, 724, 914, 1086}
		},

		// 왼쪽
		{
			{161, 1086, 331, 1383},
			{451, 1086, 621, 1385},
			{738, 1086, 909, 1382}
		}
	};

	RECT src = playerSrc[currentDir][currentFrame];

	int srcW = src.right - src.left;
	int srcH = src.bottom - src.top;

	int drawH = 120;
	int drawW = srcW * drawH / srcH;

	int drawX = playerPos.x - drawW / 2;
	int drawY = playerPos.y - drawH / 2;

	drawImage->Draw(
		hDC,
		drawX, drawY, drawW, drawH,
		src.left, src.top, srcW, srcH
	);
}

void PCroomgame::PAINT(HDC hDC)
{
	background.Draw(hDC, 0, 0, 1920, 1080);

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
	wchar_t timerBuffer[100];
	swprintf_s(timerBuffer, L"남은 시간: %.1f", timer);
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
// 손님 그리기
	DrawCustomers(hDC);

	// 주문 중인 손님 위에 말풍선 그리기
	for (int i = 0; i < 6; i++)
	{
		DrawOrderBubble(hDC, i);
	}

	// 플레이어 그리기
	DrawPlayer(hDC);

	// 손님 그리기
	DrawCustomers(hDC);

	// 플레이어 그리기
	DrawPlayer(hDC);

	// 게임 종료 출력
	if (finished == true)
	{
		DrawTextW(hDC, 800, 500, L"게임 종료!");
		DrawTextW(hDC, 800, 540, L"최종 점수: " + std::to_wstring(score));
	}
}

int PCroomgame::getscore() {
	return score;
}

bool PCroomgame::isfinished() {
	return finished;
}

void PCroomgame::StartDelivery(int index)
{
	if (index < 0 || index >= 6)
	{
		return;
	}

	// 식사 중이면 배달하지 않음
	if (customerState[index] == ORDER2)
	{
		return;
	}

	targetPos = deliveryPos[index];
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

void PCroomgame::SetCustomerState(int index, CustomerState state, int duration)
{
	if (index < 0 || index >= 6)
		return;

	customerState[index] = state;
	customerStateTimer[index] = duration;

	customerFrame[index] = 0;
	customerAnimTick[index] = 0;
}

void PCroomgame::UpdateCustomers()
{
	for (int i = 0; i < 6; i++)
	{
		// 손님 애니메이션 프레임 갱신
		customerAnimTick[i]++;

		if (customerAnimTick[i] >= customerAnimDelay)
		{
			customerFrame[i]++;
			if (customerFrame[i] >= 3)
				customerFrame[i] = 0;

			customerAnimTick[i] = 0;
		}

		// 상태 지속시간 감소
		if (customerStateTimer[i] > 0)
		{
			customerStateTimer[i]--;

			if (customerStateTimer[i] == 0)
			{
				// 먹기 애니메이션이 끝났으면 새 주문 생성
				if (customerState[i] == ORDER2)
				{
					seatorder[i].makerandramen();
				}

				// 다시 주문 상태로 복귀
				customerState[i] = ORDER1;
				customerFrame[i] = 0;
				customerAnimTick[i] = 0;
			}
		}
	}
}

void PCroomgame::DrawOneCustomer(HDC hDC, int index)
{
	if (index < 0 || index >= 6)
		return;

	CImage& img = customerImg[(int)customerState[index]];

	if (img.IsNull())
		return;

	int frameW = img.GetWidth() / 3;
	int frameH = img.GetHeight();

	int srcX = customerFrame[index] * frameW;
	int srcY = 0;

	int drawW = 150;
	int drawH = 150;

	int centerX = (seatArea[index].left + seatArea[index].right) / 2;
	int drawX = centerX - drawW / 2;
	int drawY = seatArea[index].top + 80;

	img.Draw(
		hDC,
		drawX, drawY, drawW, drawH,
		srcX, srcY, frameW, frameH
	);
}

void PCroomgame::DrawCustomers(HDC hDC)
{
	for (int i = 0; i < 6; i++)
	{
		DrawOneCustomer(hDC, i);
	}
}

void PCroomgame::DrawOrderBubble(HDC hDC, int index)
{
	if (index < 0 || index >= 6)
	{
		return;
	}

	// 주문 중인 손님에게만 말풍선 표시
	if (customerState[index] != ORDER1)
	{
		return;
	}

	std::wstring orderText = RamenToString(seatorder[index]);

	// 글자 크기 측정
	SIZE textSize;
	GetTextExtentPoint32W(
		hDC,
		orderText.c_str(),
		(int)orderText.length(),
		&textSize
	);

	int paddingX = 15;
	int paddingY = 8;

	int bubbleW = textSize.cx + paddingX * 2;
	int bubbleH = textSize.cy + paddingY * 2;

	// 말풍선 최소 크기
	if (bubbleW < 120)
	{
		bubbleW = 120;
	}

	// 손님 중심 x좌표
	int centerX = (seatArea[index].left + seatArea[index].right) / 2;

	// DrawOneCustomer()의 손님 위치 기준
	// 손님 drawY = seatArea[index].top + 80
	int customerTopY = seatArea[index].top + 80;

	// 말풍선 위치
	int bubbleBottom = customerTopY - 15;
	int bubbleTop = bubbleBottom - bubbleH;
	int bubbleLeft = centerX - bubbleW / 2;
	int bubbleRight = centerX + bubbleW / 2;

	// 말풍선 색상
	HBRUSH bubbleBrush = CreateSolidBrush(RGB(255, 255, 255));
	HPEN bubblePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));

	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, bubbleBrush);
	HPEN oldPen = (HPEN)SelectObject(hDC, bubblePen);

	// 둥근 말풍선 본체
	RoundRect(
		hDC,
		bubbleLeft,
		bubbleTop,
		bubbleRight,
		bubbleBottom,
		18,
		18
	);

	// 아래쪽 꼬리
	POINT tail[3] =
	{
		{ centerX - 10, bubbleBottom - 1 },
		{ centerX + 10, bubbleBottom - 1 },
		{ centerX, bubbleBottom + 12 }
	};

	Polygon(hDC, tail, 3);

	// 주문 텍스트 중앙 배치
	int textX = centerX - textSize.cx / 2;
	int textY = bubbleTop + paddingY;

	DrawTextW(hDC, textX, textY, orderText);

	// GDI 자원 정리
	SelectObject(hDC, oldBrush);
	SelectObject(hDC, oldPen);

	DeleteObject(bubbleBrush);
	DeleteObject(bubblePen);
}