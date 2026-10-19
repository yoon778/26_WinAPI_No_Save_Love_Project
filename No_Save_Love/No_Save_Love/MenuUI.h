#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace MenuUI
{
    inline constexpr COLORREF Paper = RGB(250, 247, 242);
    inline constexpr COLORREF Ink = RGB(53, 43, 57);
    inline constexpr COLORREF Muted = RGB(112, 102, 114);
    inline constexpr COLORREF Rose = RGB(159, 70, 103);
    inline constexpr COLORREF Border = RGB(225, 216, 218);
    inline constexpr COLORREF SoftRose = RGB(245, 233, 237);
    inline constexpr COLORREF Plum = RGB(69, 49, 67);
    inline POINT Pointer = {-1, -1};
    inline std::vector<RECT> HitAreas;
    inline bool Hit(RECT r, int x, int y) { return PtInRect(&r, POINT{x, y}) != FALSE; }
    inline void BeginFrame() { HitAreas.clear(); }
    inline bool Interactive(RECT r)
    {
        HitAreas.push_back(r);
        return Hit(r, Pointer.x, Pointer.y);
    }
    inline bool MovePointer(int x, int y)
    {
        bool changed = false;
        for (const RECT& r : HitAreas)
            changed |= Hit(r, Pointer.x, Pointer.y) != Hit(r, x, y);
        Pointer = {x, y};
        return changed;
    }
    inline void Fill(HDC dc, RECT r, COLORREF color)
    {
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(dc, &r, brush);
        DeleteObject(brush);
    }
    inline void Line(HDC dc, int x1, int y1, int x2, int y2, COLORREF color = Border)
    {
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        auto old = SelectObject(dc, pen);
        POINT previous{}; MoveToEx(dc, x1, y1, &previous); LineTo(dc, x2, y2);
        MoveToEx(dc, previous.x, previous.y, nullptr);
        SelectObject(dc, old); DeleteObject(pen);
    }
    inline void Box(HDC dc, RECT r, COLORREF fill = Paper, COLORREF edge = Border)
    {
        HBRUSH brush = CreateSolidBrush(fill);
        HPEN pen = CreatePen(PS_SOLID, 1, edge);
        auto oldBrush = SelectObject(dc, brush); auto oldPen = SelectObject(dc, pen);
        RoundRect(dc, r.left, r.top, r.right, r.bottom, 20, 20);
        SelectObject(dc, oldBrush); SelectObject(dc, oldPen);
        DeleteObject(brush); DeleteObject(pen);
    }
    inline void Text(HDC dc, const std::wstring& value, RECT r, int size = 28,
        COLORREF color = Ink, UINT format = DT_LEFT | DT_WORDBREAK | DT_NOPREFIX, int weight = FW_NORMAL)
    {
        HFONT font = CreateFontW(-size, 0, 0, 0, weight, FALSE, FALSE, FALSE,
            HANGEUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH, L"맑은 고딕");
        auto old = SelectObject(dc, font);
        const int oldMode = SetBkMode(dc, TRANSPARENT);
        const COLORREF oldColor = SetTextColor(dc, color);
        DrawTextW(dc, value.c_str(), -1, &r, format);
        SetTextColor(dc, oldColor); SetBkMode(dc, oldMode);
        SelectObject(dc, old); DeleteObject(font);
    }
    inline void Button(HDC dc, RECT r, const std::wstring& label, bool selected = false, bool enabled = true)
    {
        const bool hovered = enabled && Interactive(r);
        const COLORREF fill = selected ? (hovered ? Plum : Rose) :
            enabled ? (hovered ? SoftRose : RGB(255, 253, 250)) : RGB(238, 234, 232);
        Box(dc, r, fill, selected || hovered ? Rose : Border);
        Text(dc, label, r, r.bottom - r.top <= 44 ? 20 : 24, selected ? RGB(255, 255, 255) : enabled ? Ink : Muted,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX, FW_SEMIBOLD);
    }
    inline void Badge(HDC dc, RECT r, const std::wstring& label, COLORREF fill, COLORREF color)
    {
        Box(dc, r, fill, fill);
        Text(dc, label, r, 20, color, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX, FW_SEMIBOLD);
    }
    inline void Meter(HDC dc, RECT r, int value, int maxValue = 100, COLORREF color = Rose)
    {
        Fill(dc, r, Border);
        if (value <= 0 || maxValue <= 0) return;
        r.right = r.left + MulDiv(r.right - r.left, value < maxValue ? value : maxValue, maxValue);
        Fill(dc, r, color);
    }
    inline void Background(HDC dc)
    {
        TRIVERTEX colors[] = {{0, 0, 0xf500, 0xef00, 0xea00, 0}, {1920, 1080, 0xe900, 0xe000, 0xe700, 0}};
        GRADIENT_RECT gradient{0, 1};
        GradientFill(dc, colors, 2, &gradient, 1, GRADIENT_FILL_RECT_V);
        Line(dc, 84, 70, 84, 1010, RGB(212, 196, 202));
        Line(dc, 1836, 70, 1836, 1010, RGB(212, 196, 202));
    }
}
