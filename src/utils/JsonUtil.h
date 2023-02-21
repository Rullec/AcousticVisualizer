#pragma once
#include "utils/EigenUtil.h"
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
    static tVectorX ReadVectorJson(std::string key, const Json::Value &root,
                                   int requested_size = -1);
    static tVectorX ReadVectorJsonFromFile(const std::string &path,
                                   const Json::Value &root);
    static Json::Value BuildVectorJson(const tVectorX &vec);
    static std::string BuildVectorString(const tVectorX &vec);
    // static bool ReadVectorJson(const Json::Value &root,
    //                            tVectorX &out_vec);
    // static bool ReadVectorJson(const Json::Value &root, tVector4 &out_vec);
    // static bool ReadMatrixJson(const Json::Value &root, tMatrixX &out_mat);
    static bool LoadJson(const std::string &path, Json::Value &value);
    static bool WriteJson(const std::string &path, Json::Value &value,
                          bool indent = true, int precision = 5);

    static int ParseAsInt(const std::string &data_field_name,
                          const Json::Value &root);
    static std::string ParseAsString(const std::string &data_field_name,
                                     const Json::Value &root);
    static _FLOAT ParseAsfloat(const std::string &data_field_name,
                               const Json::Value &root);
    static _FLOAT ParseAsFloat(const std::string &data_field_name,
                               const Json::Value &root);
    static bool ParseAsBool(const std::string &data_field_name,
                            const Json::Value &root);
    static Json::Value ParseAsValue(const std::string &data_field_name,
                                    const Json::Value &root);
    static bool HasValue(const std::string &name, const Json::Value &root);
};
