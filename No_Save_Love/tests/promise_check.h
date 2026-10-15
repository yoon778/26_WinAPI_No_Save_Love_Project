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
    // 0.75초마다 한 번 의사결정한다. 자원/체력을 조작하지 않고 실제 클릭으로 배치한다.
    const int tick=static_cast<int>(game.time*60+.5f);
    if(tick%45==0)
    {
        int dangerLane=-1; float danger=0;
        for(int lane=0;lane<3;++lane)
        {
            float pressure=0;
            for(const auto& e:game.enemies) if(e.lane==lane && e.x<1180)
                pressure += (e.maxHp/100)*(1180-e.x)/180;
            const auto& front=game.units[lane*5+4];
            if(front.type>=0 && front.hp<100) pressure+=4;
            if(pressure>danger) { danger=pressure; dangerLane=lane; }
        }
        if(dangerLane>=0 && danger>4 && game.support>=100)
        { game.OnMouseDown(1670,755); game.OnMouseDown(1100,295+194*dangerLane); }
        else
        {
            const int eco=strategy==0?0:2, attack=strategy==0?2:0;
            const int order[]={3,8,13,eco,eco+5,eco+10,4,9,14,attack,attack+5,attack+10,1,6,11};
            const int formation[2][5]={{3,1,2,1,0},{1,2,3,1,0}};
            bool acted=false;
            for(int slot:order)
            {
                const int type=formation[strategy][slot%5];
                if(game.units[slot].type<0 && game.energy>=game.Costs[type])
                {
                    game.OnMouseDown(130+type*344,880); ClickDefenseSlot(game,slot); acted=true; break;
                }
            }
            if(!acted && game.time>65 && game.energy>=20)
                for(int slot:order) if(game.units[slot].type>=0 && game.units[slot].level==1)
                { ClickDefenseSlot(game,slot); game.OnMouseDown(1580,968); break; }
        }
    }
    game.Step(1.f/60);
}
void CheckPromises(const std::filesystem::path& output)
{
    PromiseScene game;
    game.Initialize(0);
    {
        CImage gallery; gallery.Create(1920,1080,24); HDC dc=gallery.GetDC();
        {
            Gdiplus::Graphics g(dc); g.Clear(Gdiplus::Color(255,18,28,47));
            g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            Text(g,L"UNIT ANIMATION / 대기 · 걸음 · 공격 · 피격",{50,25,1800,70},36);
            for(int unit=0;unit<12;++unit) {
                const bool enemy=unit>=4; const int type=enemy?unit-4:unit;
                const float x=60.f+unit%4*470, y=120.f+unit/4*300;
                Text(g,enemy?EnemyNames[type]:UnitNames[type],{x,y,420,50},28);
                for(int frame=0;frame<4;++frame) game.DrawUnit(g,type,x+40+frame*105,y+135,.9f,enemy,frame);
            }
        }
        gallery.ReleaseDC(); assert(SUCCEEDED(gallery.Save((output/L"defense_animations.png").c_str())));
    }
    for(int heroine=0;heroine<3;++heroine)
    {
        game.Initialize(heroine,true);
        assert(game.portrait->GetLastStatus()==Gdiplus::Ok && game.smile->GetLastStatus()==Gdiplus::Ok);
        assert(game.units.size()==15);
        for(auto* sheet:game.spriteSheets) {
            assert(sheet && sheet->GetLastStatus()==Gdiplus::Ok && sheet->GetWidth()>=1000 && sheet->GetWidth()==sheet->GetHeight());
            assert(sheet->GetFlags() & Gdiplus::ImageFlagsHasAlpha);
        }
        for(int slot=0;slot<15;++slot) {
            const auto r=game.SlotRect(slot);
            assert(r.left>195 && r.right<1100 && r.top>=198 && r.bottom<780);
            if(slot%5) assert(game.SlotRect(slot-1).right<r.left);
        }
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
        assert(!game.PlaceUnit(-1,0) && !game.PlaceUnit(15,0) && !game.PlaceUnit(0,4) && game.energy==before);
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
            std::cout<<"Defense route="<<heroine<<" strategy="<<strategy<<" time="<<game.time<<" hp="<<game.baseHp<<" kills="<<game.kills<<" score="<<game.GetScore()<<std::endl;
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
    fresh(); game.units[2]={1,110,0}; game.enemies={{0,0,1000,100,100},{0,2,1100,200,200},{0,0,1500,100,100}};
    game.upgrades[1]=true; game.Step(1.f/60);
    assert(game.enemies[0].hp==56 && std::abs(game.enemies[1].hp-169.2f)<.01f && game.enemies[2].hp==100);
    fresh(); game.units[2]={2,150,0}; game.enemies={{0,0,900,100,100},{0,0,960,100,100},{0,0,1100,100,100}};
    game.upgrades[2]=true; game.Step(1.f/60);
    assert(game.enemies[0].hp==66 && game.enemies[1].hp==44 && game.enemies[2].hp==78);
    fresh(); game.units[4]={0,240,100}; game.enemies={{0,0,1030,100,100}};
    game.upgrades[3]=true; game.Step(1.f/60);
    assert(game.units[4].hp==222 && game.enemies[0].hp==81);
    fresh(); game.units[2]={0,120,100}; game.enemies={{0,0,1000,-1,100}};
    game.upgrades[0]=game.upgrades[5]=true; game.RemoveDefeated();
    assert(game.units[2].hp==144 && game.energy==93 && game.enemies.empty());
    game.RecallUnit(2); assert(std::abs(game.energy-110.6f)<.01f);
    fresh(); game.units[0]={3,100,100}; game.units[1]={1,110,0}; game.enemies={{0,0,1000,100,100}};
    game.upgrades[4]=true; game.Step(1.f/60);
    assert(std::abs(game.units[1].cooldown-.84f)<.01f);
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
    // 전열과 후열 인접 판정은 행 끝에서 다음 행으로 이어지지 않는다.
    fresh(); game.units[4]={0,240,100}; assert(!game.Linked(5)); assert(game.Linked(9));
    // 새 칸 끝까지 실제 마우스로 배치/선택/회수된다.
    fresh(); game.OnMouseDown(130,880); ClickDefenseSlot(game,14);
    assert(game.units[14].type==0); ClickDefenseSlot(game,14); game.OnMouseDown(1740,968); assert(game.units[14].type==-1);
    // 방어구는 폭죽 피해를 줄이지만 저격/지원은 정상적으로 관통한다.
    fresh(); PromiseScene::Enemy armor{0,2,1200,360,360};
    game.Damage(armor,100,2); assert(armor.hp==305); game.Damage(armor,100,1); assert(armor.hp==205);
    // 저격수는 가까운 호위보다 사거리 안의 특수 적을 먼저 공격한다.
    fresh(); game.units[3]={1,110,0}; game.enemies={{0,0,1000,135,135},{0,5,1250,180,180}};
    game.Step(1.f/60); assert(game.enemies[0].hp==135 && game.enemies[1].hp==136 && game.units[3].action>0);
    // 치유병은 살아 있는 주변 동료만 회복한다. 자가 회복/부활은 없다.
    fresh(); game.enemies={{0,5,1200,90,180},{0,0,1100,30,135},{0,0,1150,0,135},{1,0,1100,30,135}};
    game.enemies[0].ability=0; game.Step(1.f/60);
    assert(game.enemies.size()==3 && game.enemies[0].hp==90 && game.enemies[1].hp==60 && game.enemies[2].hp==30);
    assert(game.enemies[0].action>0);
    // 포격은 예고 후 발사. 조준 도중 회수하면 빈 칸에 피해를 주지 않는다.
    fresh(); game.units[2]={2,150,100}; game.enemies={{0,6,1100,210,210}}; game.enemies[0].ability=0;
    game.Step(1.f/60); assert(game.enemies[0].aim==2 && game.units[2].hp==150);
    SavePromiseFrame(game,output/L"defense_mortar_warning.png");
    game.Step(.5f); assert(game.units[2].hp==150); game.Step(.45f); assert(game.units[2].hp==113 && game.units[2].flash>0);
    game.enemies[0].ability=0; game.Step(1.f/60); game.RecallUnit(2); game.Step(1); assert(game.units[2].type==-1);
    // 방해는 생산을 멈추고 공격 간격을 늦춘다. 시간이 지나면 자동 회복한다.
    fresh(); game.units[3]={3,100,100}; game.enemies={{0,7,1150,165,165}}; game.enemies[0].ability=0;
    game.Step(1.f/60); assert(game.enemies[0].aim==3); game.Step(1);
    assert(game.units[3].jam>0 && std::abs(game.ChargingRate()-2.2f)<.01f);
    SavePromiseFrame(game,output/L"defense_jammed.png");
    game.enemies.clear(); game.Step(4); assert(game.units[3].jam==0 && game.ChargingRate()>2.8f);
    game.units[3]={1,110,1}; game.units[3].jam=2; game.Step(.1f); assert(std::abs(game.units[3].cooldown-.955f)<.001f);
    game.Initialize(2); game.BeginBattle(); game.spawnTimer=100;
    game.units[3]={2,150,0}; game.enemies={{0,0,1200,135,135}}; game.Step(1.f/60);
    assert(game.enemies[0].chill>1.5f && game.enemies[0].x>1199.4f);
    game.units.fill(PromiseScene::Unit{}); game.Step(2); assert(game.enemies[0].chill==0);
    // 이동은 거리 기반 프레임, 공격/피격은 사건 기반 프레임을 사용한다.
    fresh(); game.enemies={{0,0,1300,135,135}}; game.Step(.25f);
    assert(game.enemies[0].x<1300 && game.enemies[0].walk>0);
    game.units[4]={0,240,0}; game.HitUnit(4,10); assert(game.units[4].flash>0);
    // 모든 적 유형은 실제 웨이브에서 등장한다.
    fresh(); std::array<bool,8> seen{};
    for(int i=0;i<11000 && !game.finished;++i) {
        PlayDefenseStep(game,1);
        for(const auto& e:game.enemies) seen[e.type]=true;
    }
    for(bool present:seen) assert(present);
    // 이전 세 칸식 배치를 한 번 채운 뒤 방치하면 무피해 클리어가 불가능하다.
    fresh(); int occupied=0;
    for(int i=0;i<11000 && !game.finished;++i) {
        if(game.choosing) game.ChooseUpgrade(0);
        if(occupied<9) {
            const int order[]={3,8,13,4,9,14,2,7,12}; const int slot=order[occupied];
            const int type=slot%5==4?0:slot%5==3?1:2;
            if(game.PlaceUnit(slot,type)) ++occupied;
        }
        game.Step(1.f/60);
    }
    std::cout<<"Passive defense hp="<<game.baseHp<<" time="<<game.time<<" score="<<game.GetScore()<<std::endl;
    assert(game.baseHp<75);
    // 아무 배치도 하지 않으면 실제로 거점이 무너진다.
    game.Initialize(0); game.BeginBattle();
    for(int i=0;i<11000 && !game.finished;++i)
    { if(game.choosing) game.ChooseUpgrade(0); game.Step(1.f/60); }
    assert(game.finished && !game.victory && game.GetScore()<80);
    game.Initialize(0); game.FinishImmediately(); assert(game.GetScore()==0);
    game.Shutdown();
    std::cout<<"PASS: defense 15 slots, 8 enemies, sprite alpha, telegraphs, armor, healing, jamming, 3 partners x 2 strategies, passive damage, integration controls\n";
}
