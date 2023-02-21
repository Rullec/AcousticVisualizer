#include "Rand.h"
#include <algorithm>
#include <assert.h>
#include <time.h>

cRand::cRand()
{
    unsigned long int seed = static_cast<unsigned long int>(time(NULL));
    mRandGen = std::default_random_engine(seed);
    mRandFloatDist = std::uniform_real_distribution<_FLOAT>(0, 1);
    mRandFloatDistNorm = std::normal_distribution<_FLOAT>(0, 1);
    mRandIntDist = std::uniform_int_distribution<int>(
        std::numeric_limits<int>::min() + 1,
        std::numeric_limits<int>::max()); // + 1 since there is one more neg int
                                          // than pos int
    mRandUintDist = std::uniform_int_distribution<unsigned int>(
        std::numeric_limits<unsigned int>::min(),
        std::numeric_limits<unsigned int>::max());
}

cRand::cRand(unsigned long int seed) : cRand() { Seed(seed); }

cRand::~cRand() {}

_FLOAT cRand::RandFloat() { return mRandFloatDist(mRandGen); }

_FLOAT cRand::RandFloat(_FLOAT min, _FLOAT max)
{
    if (min == max)
    {
        return min;
    }

    // generate random _FLOAT in [min, max]
    _FLOAT rand_FLOAT = mRandFloatDist(mRandGen);
    rand_FLOAT = min + (rand_FLOAT * (max - min));
    return rand_FLOAT;
}

_FLOAT cRand::RandFloatExp(_FLOAT lambda)
{
    std::exponential_distribution<_FLOAT> dist(lambda);
    _FLOAT rand_FLOAT = dist(mRandGen);
    return rand_FLOAT;
}

_FLOAT cRand::RandFloatNorm(_FLOAT mean, _FLOAT stdev)
{
    _FLOAT rand_FLOAT = mRandFloatDistNorm(mRandGen);
    rand_FLOAT = mean + stdev * rand_FLOAT;
    return rand_FLOAT;
}

int cRand::RandInt() { return mRandIntDist(mRandGen); }

int cRand::RandInt(int min, int max)
{
    if (min == max)
    {
        return min;
    }

    // generate random _FLOAT in [min, max)
    int delta = max - min;
    int rand_int = std::abs(RandInt());
    rand_int = min + rand_int % delta;

    return rand_int;
}

int cRand::RandUint() { return mRandUintDist(mRandGen); }

int cRand::RandUint(unsigned int min, unsigned int max)
{
    if (min == max)
    {
        return min;
    }

    // generate random _FLOAT in [min, max)
    int delta = max - min;
    int rand_int = RandUint();
    rand_int = min + rand_int % delta;

    return rand_int;
}

int cRand::RandIntExclude(int min, int max, int exc)
{
    int rand_int = 0;
    if (exc < min || exc >= max)
    {
        rand_int = RandInt(min, max);
    }
    else
    {
        int new_max = max - 1;
        if (new_max <= min)
        {
            rand_int = min;
        }
        else
        {
            rand_int = RandInt(min, new_max);
            if (rand_int >= exc)
            {
                ++rand_int;
            }
        }
    }
    return rand_int;
}

void cRand::Seed(unsigned long int seed)
{
    mRandGen.seed(seed);
    mRandFloatDist.reset();
    mRandFloatDistNorm.reset();
    mRandIntDist.reset();
    mRandUintDist.reset();
}

int cRand::RandSign() { return FlipCoin() ? -1 : 1; }

bool cRand::FlipCoin(_FLOAT p) { return (RandFloat(0, 1) < p); }
