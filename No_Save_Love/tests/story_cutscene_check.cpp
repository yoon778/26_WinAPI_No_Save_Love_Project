// 실제 StoryData/StoryScene을 사용한 Win32 오프스크린 회귀 검사.
// 테스트에서만 상태를 들여다본다. 게임에 디버그 조작이나 테스트 API를 추가하지 않는다.
#include <windows.h>
#include <string>
#include <vector>
#include <atlimage.h>
#include <gdiplus.h>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#define private public
#include "../No_Save_Love/StoryData.h"
#undef private
// MSVC는 접근 지정자를 심벌에 포함하므로 테스트는 한 번역 단위로 컴파일한다.
#include "../No_Save_Love/StoryData.cpp"
#include "../No_Save_Love/Scenes/StoryScene.cpp"

void FinishFade(StoryScene& scene)
{
    for (int i = 0; i < 60 && scene.fadeState != FadeState::None; ++i) scene.Update();
    assert(scene.fadeState == FadeState::None);
}

std::uint64_t Digest(CImage& frame, int top, int bottom, int left = 0, int right = 1920)
{
    std::uint64_t hash = 1469598103934665603ULL;
    for (int y = top; y < bottom; y += 4)
    {
        auto row = static_cast<const unsigned char*>(frame.GetPixelAddress(0, y));
        for (int x = left * 3; x < right * 3; x += 11) hash = (hash ^ row[x]) * 1099511628211ULL;
    }
    return hash;
}

#include "reading_check.h"

void CheckHeroineAnimation(StoryScene& scene, CImage& frame, HDC dc,
    const std::filesystem::path& output, bool preview)
{
    const wchar_t* keys[] = {L"hansea", L"yuharin", L"seoirin"};
    const wchar_t* names[] = {L"한세아", L"유하린", L"서이린"};
    const wchar_t* backgrounds[] = {L"hansea_room", L"yuharin_classroom", L"library_day"};
    const wchar_t* actions[] = {L"idle", L"nod", L"tilt", L"shy", L"laugh", L"surprise"};
    double renderMs = 0.0;
    int rendered = 0;
    for (int heroine = 0; heroine < 3; ++heroine)
    {
        for (int action = 0; action < 6; ++action)
        {
            const auto motion = static_cast<HeroineMotion>(action);
            const std::wstring character = std::wstring(keys[heroine]) + (motion == HeroineMotion::Laugh ? L"_smile" : L"_normal");
            scene.SetDialogues({{names[heroine], L"네가 있어서, 오늘은 조금 달라졌어.", backgrounds[heroine], character, L"", L"", motion}});
            FinishFade(scene);
            for (int tick = 0; tick < 12; ++tick) scene.UpdateObjectFades();
            scene.OnMouseClick(960, 950);
            assert(scene.currentHeroineMotion == motion && scene.characterAlpha == 255);
            scene.heroineSeconds = 0.0;
            scene.reactionSeconds = 0.0;
            scene.Render(dc);
            const auto first = Digest(frame, 20, 780, 690, 1290);
            const auto text = Digest(frame, 835, 1035, 100, 1820);
            const auto skip = Digest(frame, 40, 100, 1700, 1860);
            scene.heroineSeconds = action == 0 ? 1.18 : 0.0; // 반응은 기본 호흡 시간을 고정해 검사한다.
            scene.reactionSeconds = 0.8;
            scene.Render(dc);
            assert(first != Digest(frame, 20, 780, 690, 1290));
            assert(text == Digest(frame, 835, 1035, 100, 1820));
            assert(skip == Digest(frame, 40, 100, 1700, 1860));
            assert(scene.currentDialogueIndex == 0 && !scene.IsFinished());
            const auto stem = std::wstring(L"heroine_") + keys[heroine] + L"_" + actions[action];
            assert(SUCCEEDED(frame.Save((output / (stem + L".png")).c_str())));
            scene.heroineSeconds = scene.reactionSeconds = 3600.0;
            scene.Render(dc);
            assert(text == Digest(frame, 835, 1035, 100, 1820));
            assert(!scene.IsFinished());
            if (!preview) continue;
            const auto directory = output / L"heroine_preview" / stem;
            std::filesystem::create_directories(directory);
            CImage crop;
            crop.Create(350, 400, 24);
            HDC cropDC = crop.GetDC();
            SetStretchBltMode(cropDC, HALFTONE);
            for (int sample = 0; sample < 48; ++sample)
            {
                scene.heroineSeconds = scene.reactionSeconds = sample / 12.0;
                const auto start = std::chrono::steady_clock::now();
                scene.Render(dc);
                renderMs += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
                ++rendered;
                // 실제 대화 장면의 히로인 부분을 확대해 눈과 고개 움직임을 확인한다.
                StretchBlt(cropDC, 0, 0, 350, 400, dc, 640, 0, 700, 800, SRCCOPY);
                wchar_t fileName[32];
                swprintf_s(fileName, L"%04d.png", sample);
                assert(SUCCEEDED(crop.Save((directory / fileName).c_str())));
            }
            crop.ReleaseDC();
        }
    }
    // 같은 인물의 표정·소품 변경은 호흡 시간을 유지한다. 새 반응만 처음부터 시작한다.
    scene.SetDialogues({
        {L"한세아", L"시작", L"hansea_room", L"hansea_normal", L"", L"", HeroineMotion::Nod},
        {L"한세아", L"웃음", L"hansea_room", L"hansea_smile", L"anim_label", L"", HeroineMotion::Laugh},
        {L"한세아", L"계속", L"hansea_room", L"hansea_smile", L"anim_label", L"", HeroineMotion::Laugh},
        {L"서이린", L"다른 인물", L"library_day", L"seoirin_normal", L"", L"", HeroineMotion::Tilt}
    });
    FinishFade(scene);
    scene.heroineSeconds = 3.0;
    scene.reactionSeconds = 0.8;
    scene.OnMouseClick(960, 950);
    scene.OnMouseClick(960, 950);
    assert(scene.heroineSeconds == 3.0 && scene.reactionSeconds == 0.0 && scene.currentHeroineMotion == HeroineMotion::Laugh);
    scene.reactionSeconds = 0.6;
    scene.OnMouseClick(960, 950);
    scene.OnMouseClick(960, 950);
    assert(scene.reactionSeconds == 0.6);
    scene.OnMouseClick(960, 950);
    scene.OnMouseClick(960, 950);
    scene.lastCutsceneTick = GetTickCount64() - 5000;
    scene.Update();
    assert(scene.heroineSeconds == 3.0 && scene.reactionSeconds == 0.6); // 페이드 중 정지.
    FinishFade(scene);
    assert(scene.animatedHeroineKey == L"seoirin" && scene.heroineSeconds == 0.0 && scene.reactionSeconds == 0.0);
    scene.lastCutsceneTick = GetTickCount64() - 5000;
    scene.Update();
    assert(scene.heroineSeconds > 0.0 && scene.heroineSeconds <= 0.100001);
    scene.OnMouseClick(1780, 65);
    assert(!scene.IsFinished() && scene.fastForward); // 배속은 현재 대사부터 차례로 진행한다.
    scene.SetDialogues({});
    assert(scene.IsFinished() && scene.animatedHeroineKey.empty());
    std::cout << "HEROINE PASS: 3 heroines, 6 motions, blink/pose pixels, UI, timing and fast forward";
    if (rendered) std::cout << ", " << rendered << " preview frames, mean full render " << renderMs / rendered << " ms";
    std::cout << ".\n";
}

int wmain(int argc, wchar_t** argv)
{
    assert(argc == 2 || (argc == 3 && (std::wstring(argv[2]) == L"--preview" || std::wstring(argv[2]) == L"--heroine-preview")));
    const bool preview = argc == 3 && std::wstring(argv[2]) == L"--preview";
    const bool heroinePreview = argc == 3 && std::wstring(argv[2]) == L"--heroine-preview";
    const std::filesystem::path output(argv[1]);
    std::filesystem::create_directories(output);
    StoryData data;
    data.Initialize();
    StoryScene scene;
    scene.Initialize();
    CImage frame;
    frame.Create(1920, 1080, 24);
    HDC dc = frame.GetDC();
    std::vector<const std::vector<DialogueLineInfo>*> scripts = { &data.GetIntroStory() };
    for (int round = 0; round < 4; ++round)
        for (int heroine = 0; heroine < 3; ++heroine)
            scripts.push_back(&data.GetBranchStory(round, heroine));
    for (int heroine = 0; heroine < 4; ++heroine)
        scripts.push_back(&data.GetFinalChoiceIntroStory(heroine));
    for (int type = 0; type < 3; ++type)
        for (int heroine = 0; heroine < 3; ++heroine)
            scripts.push_back(&data.GetEndingStory(type, heroine));

    for (int heroine = 0; heroine < 3; ++heroine)
    {
        scripts.push_back(&data.GetPerformanceStory(heroine, false));
        scripts.push_back(&data.GetPerformanceStory(heroine, true));
    }

    const std::set<std::wstring> effects = {L"", L"rewind", L"rain", L"petals", L"memory", L"glitch",
        L"anim_letter", L"anim_photo", L"anim_map", L"anim_voice", L"anim_bookmark", L"anim_ink",
        L"anim_bread", L"anim_label", L"anim_bandage", L"anim_milk", L"anim_projector", L"anim_message",
        L"anim_window", L"anim_umbrella", L"anim_ribbon", L"anim_clock", L"anim_reading", L"anim_promise"};
    const std::set<std::wstring> music = {L"", L"stop", L"story1", L"story2", L"story3", L"story4", L"story5",
        L"story2.1", L"story3.1", L"story4.1", L"story5.1"};
    std::set<std::wstring> photographed;
    std::set<std::wstring> animated;
    std::set<std::wstring> renderedAnimations;
    std::ofstream manifest(output / L"animation_manifest.txt");
    assert(manifest.good());
    int animationSegments = 0;
    int heroineReactions = 0;
    int previewIndex = 0;
    int lineCount = 0;
    int maxTextHeight = 0;
    HFONT font = CreateFontW(36, 0, 0, 0, 600, FALSE, FALSE, FALSE, HANGEUL_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");
    HGDIOBJ oldFont = SelectObject(dc, font);
    for (const auto* script : scripts)
    {
        assert(!script->empty());
        scene.SetDialogues(*script);
        for (size_t i = 0; i < script->size(); ++i)
        {
            FinishFade(scene);
            const auto& line = (*script)[i];
            assert(scene.currentDialogueIndex == static_cast<int>(i));
            assert(!line.text.empty());
            assert(effects.contains(line.effectKey));
            assert(music.contains(line.bgmKey));
            auto* background = scene.GetBackgroundImage(scene.currentBackgroundKey);
            assert(background && !background->IsNull());
            assert(scene.currentEffectKey == line.effectKey);
            assert(line.heroineMotion >= HeroineMotion::Idle && line.heroineMotion <= HeroineMotion::Surprise);
            if (line.heroineMotion != HeroineMotion::Idle)
            {
                ++heroineReactions;
                assert(!scene.currentCharacterKey.empty() && (!scene.IsCutscene() || scene.IsStoryAnimation()));
                assert(scene.currentHeroineMotion == line.heroineMotion);
            }
            if (!scene.currentCharacterKey.empty())
            {
                auto* character = scene.GetCharacterImage(scene.currentCharacterKey);
                assert(character && character->GetLastStatus() == Gdiplus::Ok);
            }
            assert(scene.ConsumePendingBgmKey() == line.bgmKey);
            assert(scene.ConsumePendingBgmKey().empty());
            // 받침 유무, 최대 이름 길이로 실제 Win32 글꼴의 줄바꿈 높이를 측정한다.
            for (const auto& name : {L"윤서", L"김철수", L"가나다라마바"})
            {
                scene.SetPlayerName(name);
                std::wstring text = scene.GetCurrentDisplayText();
                assert(text.find(L"{PLAYER}") == std::wstring::npos);
                RECT bounds = scene.textRect;
                const int height = DrawTextW(dc, text.c_str(), -1, &bounds, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_CALCRECT);
                if (height > scene.textRect.bottom - scene.textRect.top)
                {
                    std::wcerr << L"Text overflow (" << height << L"): " << text << L'\n';
                    return 2;
                }
                maxTextHeight = (std::max)(maxTextHeight, height);
            }
            scene.SetPlayerName(L"윤서");
            // 첫 클릭은 글자 전체 표시, 다음 클릭은 정확히 한 줄 진행.
            scene.OnMouseClick(960, 950);
            assert(scene.isTypingFinished);
            if (scene.IsStoryAnimation())
            {
                animated.insert(line.effectKey);
                if (i == 0 || (*script)[i - 1].effectKey != line.effectKey) ++animationSegments;
            }
            const auto animationName = line.effectKey + L"_" + scene.currentCharacterKey.substr(0, scene.currentCharacterKey.find(L'_'));
            if (scene.IsStoryAnimation() && renderedAnimations.insert(animationName).second)
            {
                for (wchar_t ch : animationName)
                {
                    assert(ch <= 127); // 효과/캐릭터 키는 ASCII 파일명이다.
                    manifest.put(static_cast<char>(ch));
                }
                manifest.put('\n');
                assert(!scene.currentCharacterKey.empty());
                // 테스트는 시간을 직접 지정하므로 실제 프레임에서 진행되는 캐릭터 페이드도 마친다.
                for (int tick = 0; tick < 12; ++tick) scene.UpdateObjectFades();
                assert(scene.characterAlpha == 255);
                scene.cutsceneSeconds = 0.3;
                scene.Render(dc);
                // 히로인의 호흡 움직임이 아니라 왼쪽 소품 자체의 변화를 검사한다.
                const auto first = Digest(frame, 120, 780, 100, 1190);
                const auto textPixels = Digest(frame, 835, 1035, 100, 1820);
                const auto skipPixels = Digest(frame, 40, 100, 1700, 1860);
                assert(SUCCEEDED(frame.Save((output / (animationName + L"_start.png")).c_str())));
                scene.cutsceneSeconds = 3.0;
                scene.Render(dc);
                assert(first != Digest(frame, 120, 780, 100, 1190));
                assert(textPixels == Digest(frame, 835, 1035, 100, 1820));
                assert(skipPixels == Digest(frame, 40, 100, 1700, 1860));
                assert(SUCCEEDED(frame.Save((output / (animationName + L".png")).c_str())));
                scene.cutsceneSeconds = 6.0;
                scene.Render(dc);
                assert(textPixels == Digest(frame, 835, 1035, 100, 1820));
                assert(scene.currentDialogueIndex == static_cast<int>(i)); // 시간 경과로 대사를 넘기지 않음.
                assert(SUCCEEDED(frame.Save((output / (animationName + L"_end.png")).c_str())));
                scene.cutsceneSeconds = 3600.0;
                scene.Render(dc);
                assert(textPixels == Digest(frame, 835, 1035, 100, 1820));
                assert(skipPixels == Digest(frame, 40, 100, 1700, 1860));
                if (preview)
                {
                    const auto previewPath = output / L"animation_preview" / animationName;
                    std::filesystem::create_directories(previewPath);
                    CImage thumbnail;
                    thumbnail.Create(640, 360, 24);
                    HDC previewDC = thumbnail.GetDC();
                    SetStretchBltMode(previewDC, HALFTONE);
                    for (int sample = 0; sample < 72; ++sample)
                    {
                        scene.cutsceneSeconds = sample / 12.0;
                        scene.Render(dc);
                        StretchBlt(previewDC, 0, 0, 640, 360, dc, 0, 0, 1920, 1080, SRCCOPY);
                        wchar_t fileName[32];
                        swprintf_s(fileName, L"%04d.png", sample);
                        ++previewIndex;
                        assert(SUCCEEDED(thumbnail.Save((previewPath / fileName).c_str())));
                    }
                    thumbnail.ReleaseDC();
                }
            }
            if ((line.backgroundKey.rfind(L"cut_", 0) == 0 || line.backgroundKey == L"hidden_end")
                && photographed.insert(line.backgroundKey).second)
            {
                scene.cutsceneSeconds = 2.5;
                scene.Render(dc);
                const auto first = Digest(frame, 40, 790);
                const auto textPixels = Digest(frame, 850, 1010, 125, 1795);
                scene.cutsceneSeconds = 6.0;
                scene.Render(dc);
                assert(first != Digest(frame, 40, 790)); // 이미지/효과가 시간에 따라 변화.
                assert(textPixels == Digest(frame, 850, 1010, 125, 1795)); // 대사창은 효과에 흔들리지 않음.
                assert(SUCCEEDED(frame.Save((output / (line.backgroundKey + L".png")).c_str())));
            }
            scene.OnMouseClick(960, 950);
            ++lineCount;
        }
        assert(scene.IsFinished());
    }
    assert(photographed.size() == 7);
    assert(animated.size() == 18);
    assert(renderedAnimations.size() == 22);
    assert(animationSegments == 23);
    assert(heroineReactions == 50);
    for (int round = 0; round < 4; ++round)
        for (int heroine = 0; heroine < 3; ++heroine)
        {
            const auto& branch = data.GetBranchStory(round, heroine);
            assert(std::any_of(branch.begin(), branch.end(), [](const auto& line) { return line.effectKey.rfind(L"anim_", 0) == 0; }));
            assert(std::count_if(branch.begin(), branch.end(), [](const auto& line) { return line.heroineMotion != HeroineMotion::Idle; }) >= 3);
        }
    assert(data.GetBranchStory(-1, 0).empty() && data.GetBranchStory(4, 0).empty());
    assert(data.GetBranchStory(0, -1).empty() && data.GetBranchStory(0, 3).empty());
    assert(data.GetEndingStory(-1, 0).empty() && data.GetEndingStory(3, 0).empty());
    assert(data.GetEndingStory(0, -1).empty() && data.GetEndingStory(0, 3).empty());
    assert(data.GetFinalChoiceIntroStory(-1).empty() && data.GetFinalChoiceIntroStory(4).empty());
    assert(data.GetPerformanceStory(-1, false).empty() && data.GetPerformanceStory(3, true).empty());
    for (int i = 0; i < 3; ++i)
    {
        assert(data.GetEndingStory(0, i).size() >= 60);
        assert(data.GetEndingStory(1, i).size() >= 35);
        assert(data.GetEndingStory(2, i).size() == data.GetEndingStory(2, 0).size());
        for (size_t j = 0; j < data.GetEndingStory(2, 0).size(); ++j)
            assert(data.GetEndingStory(2, i)[j].text == data.GetEndingStory(2, 0)[j].text);
    }

    // 같은 장면의 대사 진행으로 효과가 재시작되지 않으며, 평범한 장면에서는 해제된다.
    scene.SetDialogues({
        {L"", L"시작", L"cut_hansea", L"none", L"petals"},
        {L"한세아", L"계속", L"cut_hansea", L"none", L"petals"},
        {L"", L"일상", L"room", L"none", L""}
    });
    FinishFade(scene);
    scene.cutsceneSeconds = 4.0;
    scene.OnMouseClick(960, 950);
    scene.OnMouseClick(960, 950);
    assert(scene.cutsceneSeconds == 4.0);
    scene.OnMouseClick(960, 950);
    scene.OnMouseClick(960, 950);
    FinishFade(scene);
    assert(!scene.IsCutscene() && scene.cutsceneSeconds < 0.1);

    // 일반 배경에서 소품 애니메이션과 표정 변경을 이어가고 다음 줄에서 해제한다.
    scene.SetDialogues({
        {L"유하린", L"지도 시작", L"school_hallway", L"yuharin_normal", L"anim_map"},
        {L"유하린", L"표정 변경", L"school_hallway", L"yuharin_smile", L"anim_map"},
        {L"유하린", L"일상", L"school_hallway", L"yuharin_smile", L""}
    });
    FinishFade(scene);
    assert(scene.IsStoryAnimation() && !scene.IsCutscene());
    scene.cutsceneSeconds = 4.0;
    scene.OnMouseClick(960, 950);
    scene.OnMouseClick(960, 950);
    assert(scene.cutsceneSeconds == 4.0 && scene.currentCharacterKey == L"yuharin_smile");
    // 긴 프레임 간격은 0.1초로 제한한다. 페이드 중에는 효과 시간을 멈춘다.
    scene.lastCutsceneTick = GetTickCount64() - 5000;
    scene.Update();
    assert(scene.cutsceneSeconds > 4.0 && scene.cutsceneSeconds <= 4.100001);
    const double beforeFade = scene.cutsceneSeconds;
    scene.fadeState = FadeState::FadeIn;
    scene.fadeAlpha = 100;
    scene.lastCutsceneTick = GetTickCount64() - 5000;
    scene.Update();
    assert(scene.cutsceneSeconds == beforeFade);
    FinishFade(scene);
    scene.OnMouseClick(960, 950);
    scene.OnMouseClick(960, 950);
    assert(!scene.IsStoryAnimation() && scene.currentEffectKey.empty());
    scene.SetDialogues({{L"서이린", L"책갈피", L"library_day", L"seoirin_normal", L"anim_bookmark"}});
    scene.OnMouseClick(1780, 65);
    assert(!scene.IsFinished() && scene.fastForward); // 페이드 중에도 건너뛰지 않는다.
    scene.SetDialogues({{L"유하린", L"사진", L"school_hallway", L"yuharin_normal", L"anim_photo"}});
    FinishFade(scene);
    assert(scene.cutsceneSeconds == 0.0 && scene.IsStoryAnimation());

    // 배속을 켜도 묶음을 즉시 끝내지 않는다. 다음 묶음에서 배속은 해제된다.
    scene.SetDialogues({ {L"", L"건너뛰기", L"cut_rewind", L"none", L"rewind"} });
    scene.OnMouseClick(1780, 65);
    assert(!scene.IsFinished() && scene.fastForward);
    scene.SetDialogues({});
    assert(scene.IsFinished() && scene.currentBackgroundKey.empty() && !scene.IsCutscene());

    CheckReading(scene, data, frame, dc, output);
    CheckHeroineAnimation(scene, frame, dc, output, heroinePreview);
    SelectObject(dc, oldFont);
    DeleteObject(font);
    frame.ReleaseDC();
    frame.Destroy();
    scene.Shutdown();
    std::cout << "PASS: " << scripts.size() << " script arrays, " << lineCount
        << " lines, " << photographed.size() << " rendered cutscenes, " << animated.size()
        << " story animations, " << renderedAnimations.size() << " visual variants, " << animationSegments
        << " segments, max text height " << maxTextHeight
        << "/130, input/reset/effects checked, preview frames " << previewIndex << ".\n";
}
