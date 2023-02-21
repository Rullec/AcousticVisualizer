#include "ProfUtil.h"
#include "utils/LogUtil.h"
#include "utils/StringUtil.h"
#include "utils/TimeUtil.hpp"

// ================= Prof Node ===================

struct tProfNode
{
    tProfNode(std::string name)
    {
        mName = name;

        mChildArray.clear();
    }

    bool StartChild(std::string name)
    {
        auto layers = cStringUtil::SplitString(name, "/");
        SIM_ASSERT(layers[0] == this->mName);
        if (layers.size() == 1)
        {
            mSt = cTimeUtil::GetCurrentTime_chrono();
        }
        else if (layers.size() == 2)
        {
            for (auto &x : mChildArray)
            {
                printf("%s\n", x->mName.c_str());
                SIM_ASSERT(x->mName != layers[1] &&
                           "do not add the same node duplicately");
            }
            // insert
            auto child = std::make_shared<tProfNode>(layers[1]);

            child->StartChild(layers[1]);
            mChildArray.push_back(child);
            mChildCosts.push_back(0);
        }
        else if (layers.size() > 2)
        {
            // insert into next level
            for (auto &x : mChildArray)
            {
                if (layers[1] == x->mName)
                {
                    x->StartChild(cStringUtil::ConcatenateString(
                        layers.begin() + 1, layers.end(), "/"));
                }
            }
        }
        else
        {
            SIM_ERROR("cannot reach here\n");
            exit(1);
        }
        return true;
    }
    bool EndChild(std::string name)
    {
        // printf("[debug] begin to end child %s\n", name.c_str());
        auto layers = cStringUtil::SplitString(name, "/");
        SIM_ASSERT(layers[0] == this->mName);
        if (IsLeafNode() == true)
        {
            // printf("[debug] begin to end leaf %s\n", this->mName.c_str());
            mEd = cTimeUtil::GetCurrentTime_chrono();
            return true;
        }
        else
        {
            if (layers.size() == 1)
            {
                // end myself
                mEd = cTimeUtil::GetCurrentTime_chrono();
            }
            else
            {
                bool updated = false;
                for (int i = 0; i < mChildArray.size(); i++)
                {
                    auto &x = mChildArray[i];
                    if (layers[1] == x->mName)
                    {
                        updated = true;

                        x->EndChild(cStringUtil::ConcatenateString(
                            layers.begin() + 1, layers.end(), "/"));
                        mChildCosts[i] = x->GetCost();
                    }
                }
                SIM_ASSERT(updated = true);
            }
        }

        return true;
    }

    float GetCost() const { return cTimeUtil::CalcTimeElaspedms(mSt, mEd); }
    bool IsValidRecord() const
    {
        return cTimeUtil::CalcTimeElaspedms(mSt, mEd) > 0;
    }

    bool IsLeafNode() const { return mChildArray.size() == 0; }

    std::string mName;
    cTimePoint mSt, mEd;
    std::vector<float> mChildCosts;
    std::vector<tProfNodePtr> mChildArray;
};

std::vector<tProfNodePtr> cProfUtil::mRootArray = {};

// ================= Prof Util ===================

void cProfUtil::Begin(std::string name)
{
    // 1. get root name
    auto layers = cStringUtil::SplitString(name, "/");
    std::string root_name = layers[0];
    bool visited = false;
    for (auto &x : mRootArray)
    {
        if (x->mName == root_name)
        {
            x->StartChild(name);
            visited = true;
        }
    }
    // create new root
    if (visited == false)
    {
        auto node = std::make_shared<tProfNode>(root_name);
        mRootArray.push_back(node);
        node->StartChild(name);
    }
}
void cProfUtil::End(std::string name)
{
    auto layers = cStringUtil::SplitString(name, "/");
    std::string root_name = layers[0];
    bool visited = false;
    for (int i = 0; i < mRootArray.size(); i++)
    {
        auto x = mRootArray[i];
        if (x->mName == root_name)
        {
            // printf("[debug] end root %s\n", x->mName.c_str());
            x->EndChild(name);
            visited = true;
        }
    }
    SIM_ASSERT(visited);
}

// void cProfUtil::Clear() { mRootArray.clear(); }
#include <iostream>
std::string PrintProfTree(const std::string &prefix, const tProfNodePtr node,
                          bool is_first, float total_cost_ms)
{
    std::string ret_str = "";
    if (node != nullptr)
    {
        ret_str += prefix;

        ret_str += (is_first ? "\u251C\u2500\u2500" : "\u2514\u2500\u2500");

        // print the value of the node
        char output[200] = {};
        sprintf(output, "%s %.1fms(%.1f\%)\n", node->mName.c_str(),
                node->GetCost(), node->GetCost() / total_cost_ms * 100);
        ret_str += std::string(output);
        // enter the next tree level - left and right branch
        for (int i = 0; i < node->mChildArray.size(); i++)
        {
            ret_str +=
                PrintProfTree(prefix + (is_first ? "\u2502  " : "   "),
                              node->mChildArray[i], i == 0, total_cost_ms);
        }
    }
    return ret_str;
}
std::string cProfUtil::GetTreeDesc(std::string name)
{
    std::string output = "";
    for (auto &x : mRootArray)
    {
        if (x->mName == name)
        {
            output = PrintProfTree("", x, false, x->GetCost());
            break;
        }
    }
    return output;
}

void cProfUtil::ClearAll() { mRootArray.clear(); }

void cProfUtil::ClearRoot(std::string name)
{
    auto it = mRootArray.begin();
    while (it != mRootArray.end())
    {
        if (name == (*it)->mName)
        {
            mRootArray.erase(it);
            return;
        }
        it++;
    }
}