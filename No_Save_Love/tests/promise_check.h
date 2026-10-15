#pragma once

void SavePromiseFrame(PromiseScene& game, const std::filesystem::path& path)
{
    CImage image;
    image.Create(1920, 1080, 24);
    HDC dc = image.GetDC(); game.Render(dc);
    GdiFlush(); // 메모리 비트맵을 인코딩하기 전에 남은 글꼴 그리기를 완료한다.
    image.ReleaseDC();
    assert(SUCCEEDED(image.Save(path.c_str())));
}

// 카드 클릭 → 목적지 클릭과 드래그를 번갈아 사용한다.
void PlacePromiseItem(PromiseScene& game, int item, bool drag)
{
    const RECT r = game.ItemRect(item);
    game.OnMouseDown(r.left + 40, r.top + 45);
    assert(game.selected == item && game.IsDragging());
    const int target = game.TargetFor(item);
    const RECT slot = game.SlotRect(target);
    const int x = game.stage < 2 ? slot.left + 40 : 470 + target % 4 * 125;
    const int y = game.stage < 2 ? slot.top + 40 : 310 + target / 4 * 125;
    if (drag) { game.OnMouseMove(x, y); game.OnMouseUp(x, y); }
    else { game.OnMouseUp(r.left + 40, r.top + 45); game.OnMouseDown(x, y); game.OnMouseUp(x, y); }
    assert(game.positions[item] == target && game.selected == -1 && !game.IsDragging());
}

void CheckPromises(const std::filesystem::path& output)
{
    PromiseScene game;
    for (int heroine = 0; heroine < 3; ++heroine)
    {
        for (bool useHints : {false, true})
        {
            game.Initialize(heroine, true);
            assert(game.background->GetLastStatus() == Gdiplus::Ok);
            assert(game.portrait->GetLastStatus() == Gdiplus::Ok);
            assert(game.GetScore() == 0 && game.IsTutorial() && !game.IsFinished());
            game.OnMouseDown(200, 900); assert(game.IsTutorial() && game.selected == -1);
            if (!useHints) SavePromiseFrame(game, output / (L"promise_intro_" + std::to_wstring(heroine) + L".png"));
            game.OnMouseDown(960, 838); assert(!game.IsTutorial());
            for (int stage = 0; stage < 3; ++stage)
            {
                assert(game.stage == stage && game.PlacedCount() == 0);
                game.OnMouseDown(1650, 880); assert(game.stage == stage && !game.IsFinished());
                const auto card = game.ItemRect(0);
                game.OnMouseDown(card.left + 40, card.top + 45);
                game.OnMouseMove(-200, -300); game.OnMouseUp(-200, -300);
                assert(game.positions[0] == -1 && !game.IsDragging());
                game.CancelDrag(); assert(game.selected == -1);
                if (useHints)
                {
                    game.OnMouseDown(1650, 975); const int score = game.GetScore();
                    game.OnMouseDown(1650, 975); assert(game.GetScore() == score);
                }
                if (stage < 2)
                {
                    const auto wrong = game.SlotRect((game.TargetFor(0) + 1) % 6);
                    game.OnMouseDown(card.left + 40, card.top + 45);
                    game.OnMouseMove(wrong.left + 30, wrong.top + 30);
                    game.OnMouseUp(wrong.left + 30, wrong.top + 30);
                    assert(game.positions[0] == -1);
                    game.CancelDrag();
                }
                else
                {
                    PlacePromiseItem(game, 3, true);
                    // 큰 문집과 겹치거나 오른쪽 경계를 넘는 배치는 거절한다.
                    assert(!game.TryPlace(0, 470, 310) && !game.TryPlace(0, 950, 560));
                    assert(game.positions[0] == -1 && game.PlacedCount() == 1);
                    const auto before = game.positions[3];
                    assert(!game.TryPlace(3, 950, 670) && game.positions[3] == before);
                    // 자기 자리를 제외하고 충돌을 계산하므로, 놓인 물건도 이동할 수 있다.
                    assert(game.TryPlace(3, 720, 310) && game.positions[3] == 2);
                    game.OnMouseDown(1250, 225); assert(game.PlacedCount() == 0);
                }
                for (int item = 0; item < 6; ++item) PlacePromiseItem(game, item, item % 2 == 0);
                assert(game.PlacedCount() == 6);
                SavePromiseFrame(game, output / (L"promise_" + std::to_wstring(heroine) + L"_" + std::to_wstring(stage) + (useHints ? L"_hint.png" : L".png")));
                game.OnMouseDown(1650, 880);
            }
            assert(game.IsFinished() && game.GetScore() == (useHints ? 85 : 100));
            game.OnMouseDown(1250, 225); assert(game.PlacedCount() == 6);
        }
    }
    game.Initialize(0); game.FinishImmediately(); assert(game.IsFinished() && game.GetScore() == 0);
    game.Shutdown();
    std::cout << "PASS: promise puzzle three routes, 18 placements, drag/click, wrong/overlap/bounds, repack, hint 85/perfect 100, restart\n";
}
