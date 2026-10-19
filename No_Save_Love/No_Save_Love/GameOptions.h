#pragma once

// 실행 중 공통 설정. 저장과 메뉴 입력은 GameManager에서 관리한다.
namespace GameOptions
{
    inline int difficulty = 1; // 0: 스토리, 1: 보통, 2: 도전
    inline int musicVolume = 100;
    inline int effectsVolume = 100;
    inline int textSpeed = 1; // 0: 느리게, 1: 보통, 2: 빠르게
    inline bool screenShake = true;
    inline bool fullscreen = true;

    inline const wchar_t* DifficultyName()
    {
        return difficulty == 0 ? L"스토리" : difficulty == 2 ? L"도전" : L"보통";
    }
}
