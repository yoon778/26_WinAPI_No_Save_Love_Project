#include "StoryData.h"

void StoryData::Initialize()
{
    // 인트로 대사를 초기화한다.
    introStory =
    {
        { L"{PLAYER}", L"이상한 노트를 발견한 뒤로, 내 봄은 조금씩 반복되고 있었다."},
        { L"{PLAYER}", L"그리고 그 반복의 중심에는 세 명의 소녀가 있었다." },
        { L"시스템", L"미니게임 성적에 따라 스탯이 상승합니다." },
        { L"시스템", L"스탯과 선택 기록에 따라 마지막 엔딩이 달라집니다." }
    };
    // =========================
    // 1회차 선택 이후 스토리
    // =========================

    storyScripts[0][0] =
    {
        { L"한세아", L"네가 도와준 덕분에 일이 조금 정리됐어." , L"hansea_room", L"hansea_normal", },
        { L"{PLAYER}", L"그냥... 내가 할 수 있는 걸 한 것뿐이야.", L"hansea_room", L"hansea_normal" },
        { L"한세아", L"그런 말을 아무렇지 않게 하는 게 더 이상하네.", L"hansea_room", L"hansea_normal" }
    };

    storyScripts[0][1] =
    {
        { L"유하린", L"오, {PLAYER}! 방금 선택 나였지?", L"yuharin_classroom", L"yuharin_normal" },
        { L"{PLAYER}", L"그걸 그렇게 바로 물어봐?", L"yuharin_classroom", L"yuharin_normal" },
        { L"유하린", L"당연하지. 이런 건 확인이 중요하거든.", L"yuharin_classroom", L"yuharin_normal" }
    };

    storyScripts[0][2] =
    {
        { L"서이린", L"나를 선택했구나." , L"seoirin_library", L"seoirin_normal"},
        { L"{PLAYER}", L"응. 뭔가 네가 신경 쓰여서." , L"seoirin_library", L"seoirin_normal"},
        { L"서이린", L"그 말... 기억해둘게.", L"seoirin_library", L"seoirin_normal" }
    };

    // =========================
    // 2회차 선택 이후 스토리
    // =========================

    storyScripts[1][0] =
    {
        { L"한세아", L"또 나를 도와주는 거야?" , L"hansea_room", L"hansea_normal"},
        { L"{PLAYER}", L"싫으면 안 할게." , L"hansea_room", L"hansea_normal"},
        { L"한세아", L"싫다고는 안 했어." , L"hansea_room", L"hansea_normal"}
    };

    storyScripts[1][1] =
    {
        { L"유하린", L"{PLAYER}, 너 은근히 나랑 잘 맞는 거 알아?" , L"yuharin_classroom", L"yuharin_normal"},
        { L"{PLAYER}", L"갑자기 그런 말을 하면 반응하기 어렵잖아." , L"yuharin_classroom", L"yuharin_normal"},
        { L"유하린", L"그 반응 보려고 한 건데?" }
    };

    storyScripts[1][2] =
    {
        { L"서이린", L"오늘도 왔네." , L"seoirin_library", L"seoirin_normal"},
        { L"{PLAYER}", L"오면 안 되는 거였어?" , L"seoirin_library", L"seoirin_normal"},
        { L"서이린", L"아니. 기다렸어." , L"seoirin_library", L"seoirin_normal"}
    };

    // =========================
    // 3회차 선택 이후 스토리
    // =========================

    storyScripts[2][0] =
    {
        { L"한세아", L"나는 항상 실수하면 안 된다고 생각했어.", L"hansea_room", L"hansea_normal" },
        { L"{PLAYER}", L"그래도 사람은 실수할 수 있잖아.", L"hansea_room", L"hansea_normal" },
        { L"한세아", L"그 말을 듣고 싶었던 걸지도 몰라." , L"hansea_room", L"hansea_normal"}
    };

    storyScripts[2][1] =
    {
        { L"유하린", L"나는 웃고 있으면 괜찮아 보이잖아." , L"yuharin_classroom", L"yuharin_normal"},
        { L"{PLAYER}", L"괜찮아 보이는 거랑 진짜 괜찮은 건 다르지.", L"yuharin_classroom", L"yuharin_normal" },
        { L"유하린", L"...오늘은 장난으로 못 넘기겠네." , L"yuharin_classroom", L"yuharin_normal"}
    };

    storyScripts[2][2] =
    {
        { L"서이린", L"{PLAYER}야, 넌 반복이 무섭지 않아?" , L"seoirin_library", L"seoirin_normal"},
        { L"{PLAYER}", L"무섭지. 그런데 혼자 있는 게 더 무서워." , L"seoirin_library", L"seoirin_normal"},
        { L"서이린", L"그럼 내 옆에 있어." , L"seoirin_library", L"seoirin_normal"}
    };

    // =========================
    // 4회차 선택 이후 스토리
    // =========================

    storyScripts[3][0] =
    {
        { L"한세아", L"이번 봄이 끝나면, 나는 조금 달라질 수 있을까?" , L"hansea_room", L"hansea_normal"},
        { L"{PLAYER}", L"혼자서 다 버티지 않아도 된다면 가능하지 않을까." , L"hansea_room", L"hansea_normal"},
        { L"한세아", L"그럼... 조금만 더 옆에 있어줘.", L"hansea_room", L"hansea_normal" }
    };

    storyScripts[3][1] =
    {
        { L"유하린", L"나 사실 겁났어. 진지해지면 망가질까 봐.", L"yuharin_classroom", L"yuharin_normal" },
        { L"{PLAYER}", L"그래도 말해줘서 고마워." , L"yuharin_classroom", L"yuharin_normal"},
        { L"유하린", L"이번엔 웃으면서 도망치지 않을게." , L"yuharin_classroom", L"yuharin_normal"}
    };

    storyScripts[3][2] =
    {
        { L"서이린", L"반복이 끝나도, 넌 나를 기억할까?", L"seoirin_library", L"seoirin_normal" },
        { L"{PLAYER}", L"잊지 않을게.", L"seoirin_library", L"seoirin_normal" },
        { L"서이린", L"그럼 됐어. 나는 그 말이면 충분해.", L"seoirin_library", L"seoirin_normal" }
    };


  // =========================
  // 해피엔딩
  // =========================
    endingStories[0][0] =
    {
        { L"한세아", L" 한세아 해피엔딩", L"hansea_room", L"hansea_normal" },
        { L"{PLAYER}", L"한세아 해피엔딩" , L"hansea_room", L"hansea_normal"},
        { L"한세아", L"한세아 해피엔딩", L"hansea_room", L"hansea_normal" }
    };
    endingStories[0][1] =
    {
        { L"유하린", L" 유하린 해피엔딩" , L"yuharin_classroom", L"yuharin_normal"},
        { L"{PLAYER}", L"유하린 해피엔딩" , L"yuharin_classroom", L"yuharin_normal"},
        { L"유하린", L"유하린 해피엔딩" , L"yuharin_classroom", L"yuharin_normal"}
    };
    endingStories[0][2] =
    {
        { L"서이린", L" 서이린 해피엔딩" , L"seoirin_library", L"seoirin_normal"},
        { L"{PLAYER}", L"서이린 해피엔딩", L"seoirin_library", L"seoirin_normal" },
        { L"서이린", L"서이린 해피엔딩" , L"seoirin_library", L"seoirin_normal"}
    };

    // =========================
  // 배드엔딩
  // =========================
    endingStories[1][0] =
    {
        { L"한세아", L" 한세아 배드엔딩", L"hansea_room", L"hansea_normal" },
        { L"{PLAYER}", L"한세아 배드엔딩", L"hansea_room", L"hansea_normal" },
        { L"한세아", L"한세아 배드엔딩" , L"hansea_room", L"hansea_normal"}
    };
    endingStories[1][1] =
    {
        { L"유하린", L" 유하린 배드엔딩", L"yuharin_classroom", L"yuharin_normal" },
        { L"{PLAYER}", L"유하린 배드엔딩" , L"yuharin_classroom", L"yuharin_normal"},
        { L"유하린", L"유하린 배드엔딩", L"yuharin_classroom", L"yuharin_normal" }
    };
    endingStories[1][2] =
    {
        { L"서이린", L" 서이린 배드엔딩" , L"seoirin_library", L"seoirin_normal"},
        { L"{PLAYER}", L"서이린 배드엔딩" , L"seoirin_library", L"seoirin_normal"},
        { L"서이린", L"서이린 배드엔딩", L"seoirin_library", L"seoirin_normal" }
    };
    // =========================
// 히든엔딩
// =========================
    endingStories[2][0] =
    {
        { L"한세아", L" 한세아 히든엔딩", L"hansea_room", L"hansea_normal" },
        { L"{PLAYER}", L"한세아 히든엔딩" , L"hansea_room", L"hansea_normal"},
        { L"한세아", L"한세아 히든엔딩" , L"hansea_room", L"hansea_normal"}
    };
    endingStories[2][1] =
    {
        { L"유하린", L" 유하린 히든엔딩", L"yuharin_classroom", L"yuharin_normal" },
        { L"{PLAYER}", L"유하린 히든엔딩", L"yuharin_classroom", L"yuharin_normal" },
        { L"유하린", L"유하린 히든엔딩" , L"yuharin_classroom", L"yuharin_normal"}
    };
    endingStories[2][2] =
    {
        { L"서이린", L" 서이린 히든엔딩", L"seoirin_library", L"seoirin_normal" },
        { L"{PLAYER}", L"서이린 히든엔딩" , L"seoirin_library", L"seoirin_normal"},
        { L"서이린", L"서이린 히든엔딩" , L"seoirin_library", L"seoirin_normal"}
    };

}

const std::vector<DialogueLineInfo>& StoryData::GetBranchStory(int roundIndex, int heroineIndex) const
{
    // 회차가 범위를 벗어나면 빈 대사를 반환한다.
    if (roundIndex < 0 || roundIndex >= STORY_ROUND_COUNT)
    {
        return emptyStory;
    }

    // 히로인 번호가 범위를 벗어나면 빈 대사를 반환한다.
    if (heroineIndex < 0 || heroineIndex >= HEROINE_COUNT)
    {
        return emptyStory;
    }

    // 정상 범위라면 해당 회차, 해당 히로인의 스토리를 반환한다.
    return storyScripts[roundIndex][heroineIndex];
}

const std::vector<DialogueLineInfo>& StoryData::GetIntroStory() const
{
    return introStory;
}

const std::vector<DialogueLineInfo>& StoryData::GetEndingStory(int endingType, int heroineIndex) const
{
    if (endingType < 0 || endingType >= ENDING_TYPE_COUNT)
    {
        return emptyStory;
    }

    if (heroineIndex < 0 || heroineIndex >= HEROINE_COUNT)
    {
        return emptyStory;
    }

    return endingStories[endingType][heroineIndex];
}

