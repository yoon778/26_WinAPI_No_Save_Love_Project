#pragma once

inline const wchar_t* GetResultGradeText(int score)
{
    if (score >= 90)
    {
        return L"S";
    }

    if (score >= 80)
    {
        return L"A";
    }

    if (score >= 60)
    {
        return L"B";
    }

    if (score >= 40)
    {
        return L"C";
    }

    if (score >= 20)
    {
        return L"D";
    }

    return L"F";
}
