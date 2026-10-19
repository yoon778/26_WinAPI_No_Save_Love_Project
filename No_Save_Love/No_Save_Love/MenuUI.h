#pragma once
#include <windows.h>
#include <string>

namespace MenuUI
{
    inline constexpr COLORREF Paper = RGB(255, 250, 242);
    inline constexpr COLORREF Ink = RGB(65, 48, 65);
    inline constexpr COLORREF Muted = RGB(117, 99, 115);
    inline constexpr COLORREF Rose = RGB(184, 74, 111);
    inline void Box(HDC dc, RECT r, COLORREF fill = Paper, COLORREF edge = RGB(224, 203, 208))
    {
        HBRUSH brush = CreateSolidBrush(fill);
        HPEN pen = CreatePen(PS_SOLID, 2, edge);
        auto oldBrush = SelectObject(dc, brush); auto oldPen = SelectObject(dc, pen);
        RoundRect(dc, r.left, r.top, r.right, r.bottom, 22, 22);
        SelectObject(dc, oldBrush); SelectObject(dc, oldPen);
        DeleteObject(brush); DeleteObject(pen);
    }
    inline void Text(HDC dc, const std::wstring& value, RECT r, int size = 28,
        COLORREF color = Ink, UINT format = DT_LEFT | DT_WORDBREAK | DT_NOPREFIX)
    {
        HFONT font = CreateFontW(size, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
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
        Box(dc, r, selected ? Rose : enabled ? Paper : RGB(235, 230, 226));
        Text(dc, label, r, 26, selected ? RGB(255, 255, 255) : enabled ? Ink : Muted,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }
    inline bool Hit(RECT r, int x, int y) { return PtInRect(&r, POINT{x, y}) != FALSE; }
}
