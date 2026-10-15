#pragma once

void SavePromiseFrame(PromiseScene& game, const std::filesystem::path& path)
{
    CImage image; image.Create(1920,1080,24);
    HDC dc=image.GetDC(); game.Render(dc); GdiFlush(); image.ReleaseDC();
    assert(SUCCEEDED(image.Save(path.c_str())));
}
void ClickDefenseSlot(PromiseScene& game, int slot)
{
    const auto r=game.SlotRect(slot); game.OnMouseDown((r.left+r.right)/2,(r.top+r.bottom)/2);
}
void PlayDefenseStep(PromiseScene& game, int strategy)
{
    if(game.choosing) { game.ChooseUpgrade(game.upgradeRound==0 ? strategy : 0); return; }
    // 두 작전: 방패+범위 공격, 충전기+관통 사격. 자원과 체력은 조작하지 않는다.
    const int order[]={2,5,8,1,4,7,0,3,6};
    for(int slot:order)
    {
        const int type=slot%3==2 ? 0 : slot%3==1 ? 1 : strategy==0 ? 2 : 3;
        if(game.units[slot].type<0 && game.energy>=game.Costs[type])
        {
            game.OnMouseDown(130+type*344,880); ClickDefenseSlot(game,slot); break;
        }
    }
    int dangerLane=-1; float nearest=1100;
    for(const auto& e:game.enemies) if(e.x<nearest) { nearest=e.x; dangerLane=e.lane; }
    if(dangerLane>=0 && game.support>=100)
    {
        game.OnMouseDown(1670,755); game.OnMouseDown(1100,295+194*dangerLane);
    }
    game.Step(1.f/60);
}
void CheckPromises(const std::filesystem::path& output)
{
    PromiseScene game;
    for(int heroine=0;heroine<3;++heroine)
    {
        game.Initialize(heroine,true);
        assert(game.portrait->GetLastStatus()==Gdiplus::Ok && game.smile->GetLastStatus()==Gdiplus::Ok);
        SavePromiseFrame(game,output/(L"defense_intro_"+std::to_wstring(heroine)+L".png"));
        game.OnMouseDown(200,900); assert(game.IsTutorial());
        game.OnMouseDown(960,838); assert(!game.IsTutorial() && game.practice);
        game.OnMouseDown(130,880); ClickDefenseSlot(game,2);
        assert(game.units[2].type==0 && game.energy==68);
        ClickDefenseSlot(game,2); game.OnMouseDown(1740,968);
        assert(game.units[2].type==-1 && game.practicedRecall && game.energy>82 && game.energy<83);
        game.enemies.push_back({0,0,800,100,100});
        game.OnMouseDown(1670,755); game.OnMouseDown(1000,295);
        assert(game.support==0 && game.practicedSupport);
        if(heroine==0) assert(game.barrier[0]==8);
        if(heroine==1) assert(game.enemies[0].x==980 && game.enemies[0].hp==35);
        if(heroine==2) assert(game.slow[0]==10);
        game.OnMouseDown(1670,755); assert(game.support==0); // recharge cannot be bypassed
        const auto before=game.energy;
        assert(!game.PlaceUnit(-1,0) && !game.PlaceUnit(9,0) && !game.PlaceUnit(0,4) && game.energy==before);
        for(int i=0;i<1201;++i) game.Step(1.f/60);
        assert(!game.practice && game.time<.1f && game.kills==0 && game.energy>=90 && game.baseHp==100);
        for(const auto& unit:game.units) assert(unit.type==-1);
        for(int strategy=0;strategy<2;++strategy)
        {
            game.Initialize(heroine); game.BeginBattle();
            bool captured=false, firstOffer=false, secondOffer=false;
            for(int tick=0;tick<11000 && !game.IsFinished();++tick)
            {
                if(game.choosing)
                {
                    const auto time=game.time, energy=game.energy;
                    game.Step(10); assert(game.time==time && game.energy==energy);
                    assert(game.offers[0]!=game.offers[1] && game.offers[0]!=game.offers[2] && game.offers[1]!=game.offers[2]);
                    if(game.upgradeRound==0) firstOffer=true; else secondOffer=true;
                    if(strategy==0 && game.upgradeRound==1) SavePromiseFrame(game,output/(L"defense_upgrade_"+std::to_wstring(heroine)+L".png"));
                }
                if(!captured && game.time>130 && strategy==0)
                {
                    SavePromiseFrame(game,output/(L"defense_battle_"+std::to_wstring(heroine)+L".png")); captured=true;
                }
                PlayDefenseStep(game,strategy);
            }
            std::cout<<"Defense route="<<heroine<<" strategy="<<strategy<<" time="<<game.time<<" hp="<<game.baseHp<<" kills="<<game.kills<<" score="<<game.GetScore()<<"\n";
            assert(game.IsFinished() && game.victory && firstOffer && secondOffer && game.time==160);
            assert(game.GetScore()>=80);
            const auto hp=game.baseHp;
            game.Step(10); assert(game.baseHp==hp && game.time==160);
            game.baseHp=100; assert(game.GetScore()==100);
            game.baseHp=75; assert(game.GetScore()==90);
            game.baseHp=74; assert(game.GetScore()>=80 && game.GetScore()<90);
        }
    }
    // 강화가 실제 전투 수치에 영향을 주는지 production Step으로 검사한다.
    const auto fresh = [&]() { game.Initialize(1); game.BeginBattle(); game.spawnTimer=100; };
    fresh(); game.units[1]={1,110,0}; game.enemies={{0,0,1000,100,100},{0,2,1100,200,200}};
    game.upgrades[1]=true; game.Step(1.f/60);
    assert(game.enemies[0].hp==58 && std::abs(game.enemies[1].hp-169.2f)<.01f);
    fresh(); game.units[1]={2,150,0}; game.enemies={{0,0,900,100,100},{0,0,960,100,100},{0,0,1100,100,100}};
    game.upgrades[2]=true; game.Step(1.f/60);
    assert(game.enemies[0].hp==71 && game.enemies[1].hp==49 && game.enemies[2].hp==78);
    fresh(); game.units[2]={0,240,100}; game.enemies={{0,0,900,100,100}};
    game.upgrades[3]=true; game.Step(1.f/60);
    assert(game.units[2].hp==227 && game.enemies[0].hp==81);
    fresh(); game.units[2]={0,120,100}; game.enemies={{0,0,1000,-1,100}};
    game.upgrades[0]=game.upgrades[5]=true; game.RemoveDefeated();
    assert(game.units[2].hp==144 && game.energy==93 && game.enemies.empty());
    game.RecallUnit(2); assert(std::abs(game.energy-110.6f)<.01f);
    fresh(); game.units[0]={3,100,100}; game.units[1]={1,110,0}; game.enemies={{0,0,1100,100,100}};
    game.upgrades[4]=true; game.Step(1.f/60);
    assert(std::abs(game.units[1].cooldown-.77f)<.01f);
    fresh(); game.units[2]={0,100,100}; game.energy=30;
    ClickDefenseSlot(game,2); game.OnMouseDown(1580,968);
    assert(game.units[2].level==2 && game.units[2].hp==220 && game.energy==10);
    assert(!game.UpgradeUnit(2));
    game.OnMouseDown(1740,968); assert(game.units[2].type==-1 && std::abs(game.energy-37.3f)<.01f);
    fresh(); game.units[0]={3,100,100}; game.units[1]={1,110,0};
    // 저자원, 차 있는 칸, 화면 밖 클릭은 배치를 바꾸지 않는다.
    game.energy=0; const auto before=game.units;
    assert(!game.PlaceUnit(2,2) && !game.PlaceUnit(0,0));
    game.OnMouseDown(-100,-100); assert(game.units[0].type==before[0].type && game.units[2].type==-1);
    // 아무 배치도 하지 않으면 실제로 거점이 무너진다.
    game.Initialize(0); game.BeginBattle();
    for(int i=0;i<11000 && !game.finished;++i)
    { if(game.choosing) game.ChooseUpgrade(0); game.Step(1.f/60); }
    assert(game.finished && !game.victory && game.GetScore()<80);
    game.Initialize(0); game.FinishImmediately(); assert(game.GetScore()==0);
    game.Shutdown();
    std::cout<<"PASS: defense 3 partners x 2 strategies, practice reset, upgrade pause, support, recall, invalid input, grade boundaries, defeat, restart\n";
}
