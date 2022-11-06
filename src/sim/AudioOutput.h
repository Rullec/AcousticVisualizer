#ifndef AUDIO_OUTPUT_H_
#pragma once
#include "sim/AudioWave.h"
#include "utils/DefUtil.h"
#include "utils/TimeUtil.hpp"

enum eAudioPlayMode
{
    REPEAT_MODE = 0,
    ONESHOOT_MODE,
    NUM_OF_AUDIO_PLAY_MODE
};
SIM_DECLARE_CLASS_AND_PTR(cAudioOutput);
class cAudioOutput : std::enable_shared_from_this<cAudioOutput>
{

public:
    static std::shared_ptr<cAudioOutput> getInstance();
    void SetContent(unsigned int frame_count, float *buf);
    void SetWave(const tDiscretedWavePtr &wave);
    bool GetEnableAudioFlag() const;
    void SetEnableAudioFlag(bool);

private:
    eAudioPlayMode mAudioPlayMode = eAudioPlayMode::ONESHOOT_MODE;
    bool mEnableAudio;
    virtual void Init();
    cAudioOutput();
    cAudioOutput(const cAudioOutput &);
    cAudioOutput &operator=(const cAudioOutput &);
    ~cAudioOutput();
    static void DestroyInstance(cAudioOutput *); // define a deleter

    static std::shared_ptr<cAudioOutput> instance; // singleton, a shared ptr
    tDiscretedWavePtr mCurWave;
};

#endif