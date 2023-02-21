 #pragma once
#include <memory>
#include <string>
#include "utils/BaseTypeUtil.h"
class cScene : public std::enable_shared_from_this<cScene>
{
public:
    explicit cScene();
    virtual ~cScene();
    virtual void Init(const std::string &conf_path) = 0;
    virtual void Update(_FLOAT dt);
    virtual void Reset();

protected:
    _FLOAT mCurdt;
    _FLOAT mCurTime;
};
