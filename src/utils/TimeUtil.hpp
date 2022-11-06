#pragma once
#include "utils/BaseTypeUtil.h"
#include <chrono>
#include <ctime>
#include <map>
#include <string>
typedef std::chrono::system_clock::time_point cTimePoint;
class cTimeUtil
{
public:
    // calculate a continuous segment of time
    static void Begin(const std::string &name);
    static bool HasBegin(const std::string &name);
    static FLOAT End(const std::string &name, bool silent = false);

    // calculate a discrete segment of time, lazy calculation until the final
    static void BeginLazy(const std::string &name);
    static void EndLazy(const std::string &name);
    static void ClearLazy(const std::string &name);

    static std::string GetSystemTime();
    static cTimePoint GetCurrentTime_chrono();
    static FLOAT CalcTimeElaspedms(const cTimePoint &st, const cTimePoint &ed);

private:
    inline static std::map<const std::string,
                           std::chrono::high_resolution_clock::time_point>
        mTimeTable; // record current time
    inline static std::map<
        const std::string,
        std::chrono::high_resolution_clock::time_point>::iterator time_it;

    inline static std::map<const std::string, FLOAT>
        mLazyTimeTable; // record lazy accumulated time
};