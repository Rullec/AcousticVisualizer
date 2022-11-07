#ifndef AUDIO_WAVE_H_
#define AUDIO_WAVE_H_
#include "utils/DefUtil.h"
#include "utils/MathUtil.h"

class tDiscretedWave
{
public:
    tDiscretedWave(float dt);
    tVectorXf data;
    void Allocate();
    float GetDuration() const;
    int GetNumOfData() const;
    void SetData(const tVectorXf &data);
    template <typename T>
    void SetData(const std::vector<T> &data_)
    {
        data.resize(data_.size());
        for (int i = 0; i < data.size(); i++)
            data[i] = data_[i];
        duration = dt * data.size();
    }
    void ChangeFrequency(int tar_freq);
    int GetFrequency() const;
    bool LoadFromFile(std::string path);
    void DumpToFile(std::string path);
    bool LoadFromWAV(std::string path);
    void DumpToWAV(std::string path);

protected:
    float dt;
    float duration;
};
SIM_DECLARE_PTR(tDiscretedWave);

class tAnalyticWave
{
public:
    tAnalyticWave();
    virtual void Clear();
    virtual void AddWave(double strength, double freq_hz);
    virtual double Evaluate(double t) const;

protected:
    std::vector<double> mStrength; // meter, amplitude
    std::vector<double> mFreqHZ;   // hz
};

SIM_DECLARE_PTR(tAnalyticWave);
tDiscretedWavePtr DiscretizeWave(tAnalyticWavePtr ana_wave, double duration,
                                 double dt);
#endif