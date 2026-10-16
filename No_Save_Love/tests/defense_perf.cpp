#include <windows.h>
#include <atlimage.h>
#include <gdiplus.h>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <cassert>
#define private public
#include "../No_Save_Love/Scenes/PromiseScene.h"
#undef private
#include "../No_Save_Love/Scenes/PromiseScene.cpp"
AudioManager audioManager;

template<class Draw> void Measure(const char* name, Draw draw)
{
    std::vector<double> times;
    for(int frame=0;frame<65;++frame)
    {
        const auto start=std::chrono::steady_clock::now();
        draw(frame); GdiFlush();
        const auto end=std::chrono::steady_clock::now();
        if(frame>=5) times.push_back(std::chrono::duration<double,std::milli>(end-start).count());
    }
    double total=0; for(auto ms:times) total+=ms;
    std::sort(times.begin(),times.end());
    std::cout<<name<<" mean_ms="<<total/times.size()<<" p95_ms="<<times[times.size()*95/100]<<std::endl;
}

int main()
{
    PromiseScene game; game.Initialize(2); game.BeginBattle();
    for(int slot=0;slot<15;++slot) game.units[slot]={slot%4,100,0};
    for(int i=0;i<36;++i) game.enemies.push_back({i%3,i%8,760.f+i/3*58,100,180});
    game.time=130; game.slow[1]=5;
    CImage frame; frame.Create(1920,1080,32); HDC dc=frame.GetDC();
    Measure("full_15_units_36_enemies",[&](int tick) {
        game.visualTime=tick/60.f;
        for(auto& enemy:game.enemies) enemy.walk=tick*.15f;
        game.Render(dc);
    });
    Measure("background",[&](int) { PromiseScene::DrawArcade(dc); });
    Measure("48_sprites",[&](int tick) {
        Gdiplus::Graphics g(dc); g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        for(int i=0;i<48;++i) game.DrawUnit(g,i%8,150.f+i%12*135,150.f+i/12*200,.9f,true,tick%4);
    });
    frame.ReleaseDC(); game.Shutdown();
}
