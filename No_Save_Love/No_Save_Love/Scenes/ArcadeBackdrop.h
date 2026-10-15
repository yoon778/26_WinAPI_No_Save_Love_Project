#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <string>
namespace ArcadeArt
{
using namespace Gdiplus;
inline const Color White(255,236,242,255), Muted(255,157,178,207), Blue(255,104,216,237), Pink(255,244,147,184);
    inline void Text(Graphics& g, const std::wstring& value, RectF rect, float size, Color color = White, bool bold = false)
    {
        Font font(L"맑은 고딕", size, bold ? FontStyleBold : FontStyleRegular, UnitPixel);
        SolidBrush ink(color); StringFormat format;
        g.DrawString(value.c_str(), -1, &font, rect, &format, &ink);
    }
    inline void Panel(Graphics& g, RectF rect, Color fill, Color border)
    {
        GraphicsPath path; constexpr float r = 22;
        path.AddArc(rect.X, rect.Y, r, r, 180, 90);
        path.AddArc(rect.GetRight()-r, rect.Y, r, r, 270, 90);
        path.AddArc(rect.GetRight()-r, rect.GetBottom()-r, r, r, 0, 90);
        path.AddArc(rect.X, rect.GetBottom()-r, r, r, 90, 90); path.CloseFigure();
        SolidBrush brush(fill); Pen pen(border, 1.5f);
        g.FillPath(&brush, &path); g.DrawPath(&pen, &path);
    }
inline void DrawArcadeBackdrop(HDC dc)
{
    Graphics g(dc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    LinearGradientBrush wall(Point(0,0),Point(1920,1080),Color(255,18,24,49),Color(255,79,44,75));
    g.FillRectangle(&wall,0,0,1920,1080);
    SolidBrush floor(Color(255,29,26,44)); g.FillRectangle(&floor,0,700,1920,380);
    Pen grid(Color(75,185,145,198),2);
    for (int i=-4;i<15;++i) g.DrawLine(&grid,960.f+(i-5)*94,700.f,960.f+(i-5)*280,1080.f);
    for(int y=735;y<1080;y+=65) g.DrawLine(&grid,0,y,1920,y);
    for (int i=0;i<6;++i)
    {
        const float x = 55.f+i*320;
        const Color neon = i%2 ? Pink : Blue;
        Panel(g,{x,235,285,520},Color(255,24,31,50),Color(255,70,79,103));
        Panel(g,{x+18,260,249,64},Color(255,43,44,73),neon);
        Text(g,i%2?L"STAR / DUO":L"AFTER SCHOOL",{x+33,278,220,38},23,neon,true);
        Panel(g,{x+30,348,225,235},Color(255,10,21,36),Color(255,80,94,117));
        for (int star=0;star<16;++star)
        {
            SolidBrush pixel(Color(110,neon.GetR(),neon.GetG(),neon.GetB()));
            g.FillRectangle(&pixel,x+50+(star*47)%185,370.f+(star*37)%190,5.f,5.f);
        }
        SolidBrush light(neon); g.FillEllipse(&light,x+67,615.f,22.f,22.f);
        g.FillEllipse(&light,x+174,612.f,16.f,16.f); g.FillEllipse(&light,x+207,628.f,16.f,16.f);
        Pen stick(neon,7); g.DrawLine(&stick,x+78,626.f,x+83,588.f);
        Text(g,L"2 PLAYERS",{x+72,682,180,35},23,Muted,true);
    }
    Text(g,L"AFTER SCHOOL ARCADE",{465,86,1000,100},64,Color(255,242,193,223),true);
    Pen light(Color(160,132,219,241),4); g.DrawLine(&light,440,182,1470,182);
}
}
