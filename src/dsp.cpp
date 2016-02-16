//
//  dsp.cpp
//  butt
//
//  Created by Melchor Garau Madrigal on 16/2/16.
//  Copyright © 2016 Daniel Nöthen. All rights reserved.
//

#include "dsp.hpp"
#include <samplerate.h>
#include <cmath>
#include "cfg.h"

static float logdrc(float x, float t);

DSPEffects::DSPEffects(uint32_t frames, uint8_t channels) {
    dsp_size = frames * channels;
    dsp_buff = new float[dsp_size];
}

bool DSPEffects::hasToProcessSamples() {
    return cfg.main.gain != 1 || cfg.dsp.compressor;
}

void DSPEffects::processSamples(short *samples) {
    if(dsp_buff == NULL) return;

    src_short_to_float_array(samples, dsp_buff, dsp_size);
    for(uint32_t sample = 0; sample < dsp_size; sample++) {
        if(cfg.main.gain != 1)
            dsp_buff[sample] *= cfg.main.gain;
        if(cfg.dsp.compressor)
            dsp_buff[sample] = logdrc(dsp_buff[sample], cfg.dsp.compQuantity);
    }
    src_float_to_short_array(dsp_buff, samples, dsp_size);
}

DSPEffects::~DSPEffects() {
    delete[] dsp_buff;
    dsp_buff = NULL;
}

//Logaritmic Dynamic Range Compresor from
// http://www.voegler.eu/pub/audio/digital-audio-mixing-and-normalization.html
static float fa(float n) {
    if(std::abs(n - 0.000f) < 0.0125) return 2.51286f;
    if(std::abs(n - 0.025f) < 0.0125) return 2.58788f;
    if(std::abs(n - 0.050f) < 0.0125) return 2.66731f;
    if(std::abs(n - 0.075f) < 0.0125) return 2.75153f;
    if(std::abs(n - 0.100f) < 0.0125) return 2.84098f;
    if(std::abs(n - 0.125f) < 0.0125) return 2.93615f;
    if(std::abs(n - 0.150f) < 0.0125) return 3.03758f;
    if(std::abs(n - 0.175f) < 0.0125) return 3.14590f;
    if(std::abs(n - 0.200f) < 0.0125) return 3.26181f;
    if(std::abs(n - 0.225f) < 0.0125) return 3.38611f;
    if(std::abs(n - 0.250f) < 0.0125) return 3.51971f;
    if(std::abs(n - 0.275f) < 0.0125) return 3.66367f;
    if(std::abs(n - 0.300f) < 0.0125) return 3.81918f;
    if(std::abs(n - 0.325f) < 0.0125) return 3.98766f;
    if(std::abs(n - 0.350f) < 0.0125) return 4.17073f;
    if(std::abs(n - 0.375f) < 0.0125) return 4.37030f;
    if(std::abs(n - 0.400f) < 0.0125) return 4.58862f;
    if(std::abs(n - 0.425f) < 0.0125) return 4.82837f;
    if(std::abs(n - 0.450f) < 0.0125) return 5.09272f;
    if(std::abs(n - 0.475f) < 0.0125) return 5.38553f;
    if(std::abs(n - 0.500f) < 0.0125) return 5.71144f;
    if(std::abs(n - 0.525f) < 0.0125) return 6.07617f;
    if(std::abs(n - 0.550f) < 0.0125) return 6.48678f;
    if(std::abs(n - 0.575f) < 0.0125) return 6.95212f;
    if(std::abs(n - 0.600f) < 0.0125) return 7.48338f;
    if(std::abs(n - 0.625f) < 0.0125) return 8.09498f;
    if(std::abs(n - 0.650f) < 0.0125) return 8.80573f;
    if(std::abs(n - 0.675f) < 0.0125) return 9.64061f;
    if(std::abs(n - 0.700f) < 0.0125) return 10.63353f;
    if(std::abs(n - 0.725f) < 0.0125) return 11.83158f;
    if(std::abs(n - 0.750f) < 0.0125) return 13.30200f;
    if(std::abs(n - 0.775f) < 0.0125) return 15.14398f;
    if(std::abs(n - 0.800f) < 0.0125) return 17.50980f;
    if(std::abs(n - 0.825f) < 0.0125) return 20.64488f;
    if(std::abs(n - 0.850f) < 0.0125) return 24.96984f;
    if(std::abs(n - 0.875f) < 0.0125) return 31.26618f;
    if(std::abs(n - 0.900f) < 0.0125) return 41.15485f;
    if(std::abs(n - 0.925f) < 0.0125) return 58.58648f;
    if(std::abs(n - 0.950f) < 0.0125) return 96.08797f;
    if(std::abs(n - 0.975f) < 0.0125) return 221.62505f;
    return 300;
}

static float logdrc(float x, float t) {
    if(std::abs(x) > t)
        return (float) (x / std::abs(x) * (t + (1 - t) * (std::log(1 + fa(t) * (std::abs(x) - t) / (2 - t))) / (1 + fa(t))));
    else return x;
}