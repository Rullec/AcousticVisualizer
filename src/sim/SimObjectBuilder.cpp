#include "SimObjectBuilder.h"
#include "sim/AcousticBody.h"
#include "sim/BaseObject.h"
#include "sim/KinematicBodyBuilder.h"
#include "sim/AcousticManager.h"
#include "sim/SNISR_DebugDraw.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"

cBaseObjectPtr BuildSimObj(const Json::Value &conf, int id_)
{
    eObjectType type = cBaseObject::BuildObjectType(
        cJsonUtil::ParseAsString("object_type", conf));
    cBaseObjectPtr object = nullptr;
    switch (type)
    {
    case eObjectType::KINEMATICBODY_TYPE:
    {
        object = BuildKinematicBody(conf, id_);
        break;
    }
    case eObjectType::ACOUSTICBODY_TYPE:
    {
        object = std::make_shared<cAcousticBody>(id_);
        break;
    }
    case eObjectType::SNISR_DEBUG_DRAW_TYPE:
    {
        object = std::make_shared<cSNISRDebugDrawBall>(id_);
        break;
    }
    case eObjectType::ACOUSTICMANAGER_TYPE:
    {
        object = std::make_shared<cAcousticManager>(id_);
        break;
    }
    default:
        SIM_ERROR("unrecognized object type {}", type);
        break;
    };
    if (object)
        object->Init(conf);
    return object;
}