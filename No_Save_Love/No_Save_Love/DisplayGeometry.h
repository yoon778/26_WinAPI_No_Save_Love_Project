#pragma once
#include <windows.h>
#include <algorithm>

namespace DisplayGeometry
{
    constexpr int Width = 1920;
    constexpr int Height = 1080;

    inline RECT Fit(int clientWidth, int clientHeight)
    {
        if (clientWidth <= 0 || clientHeight <= 0) return {};
        const double scale = (std::min)(clientWidth / double(Width), clientHeight / double(Height));
        const int width = (std::max)(1, static_cast<int>(Width * scale));
        const int height = (std::max)(1, static_cast<int>(Height * scale));
        const int left = (clientWidth - width) / 2;
        const int top = (clientHeight - height) / 2;
        return { left, top, left + width, top + height };
    }

    inline POINT ToLogical(POINT client, const RECT& viewport)
    {
        if (viewport.right <= viewport.left || viewport.bottom <= viewport.top) return { -1, -1 };
        // Letterbox margins stay outside the game, including negative captured drags.
        if (client.x < viewport.left || client.x >= viewport.right ||
            client.y < viewport.top || client.y >= viewport.bottom) return { -1, -1 };
        return {
            static_cast<LONG>((client.x - viewport.left) * static_cast<long long>(Width) / (viewport.right - viewport.left)),
            static_cast<LONG>((client.y - viewport.top) * static_cast<long long>(Height) / (viewport.bottom - viewport.top))
        };
    }
}
