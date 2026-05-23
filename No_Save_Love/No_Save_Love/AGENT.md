# AGENTS.md

## Project Overview

이 프로젝트는 Win32 API / C++ 기반의 2D 미연시 + 미니게임 최종 과제이다.

게임 제목:
- 윤서의 고백은 세이브 로드가 없다

핵심 컨셉:
- 선택에 따라 관계와 엔딩이 달라지는 연애 시뮬레이션
- 스토리, 선택지, 미니게임, 결과 화면이 연결되는 구조

기본 진행 구조:
- TitleScene
- NameInputScene
- Intro Story
- ResultScene
- minigames 4종류
- ChoiceScene
- Branch Story
- ResultScene / ChoiceScene 반복
- FinalChoiceScene
- EndingDialogue
- EndingScene

기획서 기준으로 원래 게임은 미연시 + 4종 미니게임이며, 기본 구조는 “스토리 진행 → 선택지 → 미니게임 → 결과 반영 → 다음 스토리” 흐름이다.

## Architecture Rules

1. GameManager is the only class that changes the current game mode.
   - Scene classes must not directly change the global game mode.
   - Scene classes should only handle their own input, update, render, and state.

2. Scene responsibilities:
   - TitleScene: title screen input and rendering only.
   - NameInputScene: player name input only.
   - StoryScene: dialogue rendering, typing effect, background/character/effect keys.
   - ChoiceScene: heroine choice only.
   - ResultScene: stat result display only.
   - MiniGameScene: minigame execution and score/result delivery only.
   - EndingScene: fade out, credit scroll, END rendering.

3. Do not merge unrelated scene logic into one file.
4. Prefer small changes over large rewrites.
5. Before editing, read the related .h and .cpp files first.
6. Preserve existing naming style unless the task explicitly asks for renaming.

## Win32 / GDI / CImage Rules

1. Do not load images inside Render or WM_PAINT.
   - Load images in Initialize or WM_CREATE.
   - Draw images only in Render or WM_PAINT.
   - Destroy or release images in Shutdown or WM_DESTROY.

2. Rendering rules:
   - WM_PAINT or Render should only draw the current state.
   - Game state should be updated in Update, WM_TIMER, or input handlers.
   - Avoid changing scene flow inside Render.

3. Double buffering:
   - If flickering occurs, use memory DC double buffering.
   - Draw everything to memory DC first, then copy to screen DC with BitBlt.

4. GDI object rules:
   - Every CreatePen, CreateBrush, CreateCompatibleBitmap, CreateCompatibleDC must be paired with proper cleanup.
   - Always restore old GDI objects after SelectObject.
   - Delete created GDI objects after use.

5. Mouse and keyboard:
   - Use WM_KEYDOWN for non-character keys such as arrows, Enter, ESC, function keys.
   - Use WM_CHAR for text input.
   - Use LOWORD(lParam), HIWORD(lParam) for mouse coordinates.

## Collaboration Rules

1. Do not modify teammate-owned files unless the task explicitly says so.
2. Before editing shared files, explain why the shared file must be changed.
3. Avoid broad formatting changes.
4. Do not rename files, classes, or functions unless necessary.
5. Do not move asset files without permission.
6. Keep changes minimal and easy to review.
7. At the end of every task, summarize:
   - changed files
   - reason for each change
   - possible conflict risk
   - manual test checklist

## Validation Rules

After making code changes:

1. If possible, check for C++ compile errors.
2. If Visual Studio build cannot be run in the current environment, do not pretend it passed.
3. Instead, provide a manual test checklist.
4. Always check for:
   - missing includes
   - unresolved external symbols risk
   - header/source declaration mismatch
   - wrong scene transition ownership
   - image loading inside Render
   - GDI object leaks
   - invalid pointer access
   - array index out of range


## Response Style

When explaining code changes:

1. Explain in Korean.
2. Include comments in newly written code.
3. Explain why the change is needed.
4. Show the exact file and function that changed.
5. If there are multiple possible approaches, recommend the safest one for the current project.
6. Do not over-engineer the solution.
7. Prefer beginner-friendly Win32 API code over advanced modern C++ tricks when both are possible.
8. 코드를 작성할 때 연관되어 있는 코드 끼리 같이 두고 엔터로 구분하기
9. 주석은 최대한 짧게 하지만 알아보기는 쉽게
