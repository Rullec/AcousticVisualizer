#pragma once
#include "utils/RenderUtil.h"

class cRenderBaseObject
{
public:
    virtual std::vector<cRenderResourcePtr> GetRenderingResource() const = 0;

protected:
};