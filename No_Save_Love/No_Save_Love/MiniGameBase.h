#pragma once
#include <Windows.h>

class MiniGameBase
{
public:
	virtual void Init() = 0;		// 미니게임 초기 상태로 만듬(초기화) / ex: 처음 시작좌표, 초기 시간 등 등
	virtual void Update() = 0;		// 게임 상태를 갱신하는 함수 / ex: 시간 감소, 캐릭터 이동 등
	virtual void Render(HDC hdc) = 0;		// 미니게임 화면 그리기
	virtual void OneKeyDown(WPARAM wParam) = 0; // 키보드 입력 처리하는 함수
	virtual void OnMouse(int x, int y) = 0;		// 마우스 입력 처리하는 함수
	virtual bool IsFinished() = 0;				// 미니게임이 끝남을 알려주는 함수(ture면 미니게임 종료, false면 미니게임 진행중)
	virtual int GetScore() = 0;			// 미니게임에서 획득한 점수를 반환하는 함수

};

