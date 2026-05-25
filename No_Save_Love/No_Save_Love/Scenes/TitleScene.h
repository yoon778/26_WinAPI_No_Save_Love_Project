#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <atlimage.h>

class TitleScene
{
public:
	
	void Initialize(); // 초기화

	void Shutdown(); // 종료시 호출함.

	void OnMouseClick(int x, int y); // 마우스 클릭 담당

	bool IsStartClicked() const; // start를 눌렀는지 판단. gamemanager한테 반환

	void Render(HDC hDC); // 그리기 담당.

public:

	bool IsClickStart(int x, int y) const; // START 버튼 영역 클릭 확인

private:
	
	// 포토샵/포토피아 등에서 미리 블러 처리해 둔 타이틀 배경 이미지다.
	ATL::CImage titleBackgroundImage;

	// 투명 배경의 게임 제목 로고 이미지다.
	ATL::CImage titleLogoImage;

	RECT startButtonRect = { 760, 680, 1160, 770 }; // 시작 버튼 위치 

	// START 버튼이 눌렸는지 저장
	bool startClicked = false;

private:
	// 타이틀 배경 위에 아주 약한 밝은 반투명 레이어를 얹는다.
	void DrawSoftOverlay(HDC hDC);
};

