#pragma once
#include "utils/BaseTypeUtil.h"
#include <random>

class cRand
{
public:
    cRand();
    cRand(unsigned long int seed);
    virtual ~cRand();

    virtual _FLOAT RandFloat();
    virtual _FLOAT RandFloat(_FLOAT min, _FLOAT max);
    virtual _FLOAT RandFloatExp(_FLOAT lambda);
    virtual _FLOAT RandFloatNorm(_FLOAT mean, _FLOAT stdev);
    virtual int RandInt();
    virtual int RandInt(int min, int max);
    virtual int RandUint();
    virtual int RandUint(unsigned int min, unsigned int max);
    virtual int RandIntExclude(int min, int max, int exc);
    virtual void Seed(unsigned long int seed);
    virtual int RandSign();
    virtual bool FlipCoin(_FLOAT p = 0.5);

private:
    std::default_random_engine mRandGen;
    std::uniform_real_distribution<_FLOAT> mRandFloatDist;
    std::normal_distribution<_FLOAT> mRandFloatDistNorm;
    std::uniform_int_distribution<int> mRandIntDist;
    std::uniform_int_distribution<unsigned int> mRandUintDist;
};