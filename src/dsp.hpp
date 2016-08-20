//
//  dsp.hpp
//  butt
//
//  Created by Melchor Garau Madrigal on 16/2/16.
//  Copyright © 2016 Daniel Nöthen. All rights reserved.
//

#ifndef dsp_hpp
#define dsp_hpp

#include <stdint.h>

class DSPEffects {
private:
    float* dsp_buff;
    uint32_t dsp_size;
    uint32_t samplerate;
    class Biquad* band1, *band2, *band3, *band4, *band5;

public:
    DSPEffects(uint32_t frames, uint8_t channels, uint32_t sampleRate);
    ~DSPEffects();

    bool hasToProcessSamples();
    void processSamples(short* samples);
};

#endif /* dsp_hpp */
