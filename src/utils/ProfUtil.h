#pragma once
#include "utils/DefUtil.h"
#include <string>
#include <vector>

SIM_DECLARE_STRUCT_AND_PTR(tProfNode);

class cProfUtil
{
public:
    static void Begin(std::string name);
    static void End(std::string name);
    static std::string GetTreeDesc(std::string name);
    static void ClearAll();
    static void ClearRoot(std::string name);

protected:
    static std::vector<tProfNodePtr> mRootArray;
};