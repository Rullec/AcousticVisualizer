#pragma once

#include "utils/BaseTypeUtil.h"
#include "LogUtil.h"
#include "MathUtil.h"
#include "json/json.h"
#include <string>

namespace Json
{
class Value;
};
class cJsonUtil
{
public:
    // static Json::Value BuildVectorJson(const tVector &vec);
    static tVectorX ReadVectorJson(const Json::Value &root);
    static Json::Value BuildVectorJson(const tVectorX &vec);
    static std::string BuildVectorString(const tVectorX &vec);
    static bool ReadVectorJson(const Json::Value &root,
                               tVectorX &out_vec);
    static bool ReadVectorJson(const Json::Value &root, tVector4 &out_vec);
    static bool ReadMatrixJson(const Json::Value &root, tMatrixX &out_mat);
    static bool LoadJson(const std::string &path, Json::Value &value);
    static bool WriteJson(const std::string &path, Json::Value &value,
                          bool indent = true);

    static int ParseAsInt(const std::string &data_field_name,
                          const Json::Value &root);
    static std::string ParseAsString(const std::string &data_field_name,
                                     const Json::Value &root);
    static FLOAT ParseAsfloat(const std::string &data_field_name,
                                const Json::Value &root);
    static FLOAT ParseAsFloat(const std::string &data_field_name,
                              const Json::Value &root);
    static bool ParseAsBool(const std::string &data_field_name,
                            const Json::Value &root);
    static Json::Value ParseAsValue(const std::string &data_field_name,
                                    const Json::Value &root);
    static bool HasValue(const std::string & name, const Json::Value & root);
};
