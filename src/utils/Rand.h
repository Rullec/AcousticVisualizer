#pragma once
#include "utils/BaseTypeUtil.h"
#include <random>

class cRand
{
public:
    cRand();
    cRand(unsigned long int seed);
    virtual ~cRand();

    virtual FLOAT RandFloat();
    virtual FLOAT RandFloat(FLOAT min, FLOAT max);
    virtual FLOAT RandFloatExp(FLOAT lambda);
    virtual FLOAT RandFloatNorm(FLOAT mean, FLOAT stdev);
    virtual int RandInt();
    virtual int RandInt(int min, int max);
    virtual int RandUint();
    virtual int RandUint(unsigned int min, unsigned int max);
    virtual int RandIntExclude(int min, int max, int exc);
    virtual void Seed(unsigned long int seed);
    virtual int RandSign();
    virtual bool FlipCoin(FLOAT p = 0.5);

private:
    std::default_random_engine mRandGen;
    std::uniform_real_distribution<FLOAT> mRandFloatDist;
    std::normal_distribution<FLOAT> mRandFloatDistNorm;
    std::uniform_int_distribution<int> mRandIntDist;
    std::uniform_int_distribution<unsigned int> mRandUintDist;
};