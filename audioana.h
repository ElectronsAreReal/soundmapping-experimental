#include <stdio.h>
#include <string>
#include <vector>
#include "kiss_fftr.h"
#include "kiss_fft.h"
#include <limits>       // numeric_limits

#ifndef AUDIOANA_H
#define AUDIOANA_H
using namespace std;

class audioAna
{

public:
    unsigned sR;
    unsigned long len;
    unsigned infftLen;
    unsigned outfftLen;
    bool logarithmized;
    bool powerSpectrum;
    float sumWinGauss;
    float * spec;
    float * specn;
    float * specta;
    float * specfa;
    float * windowGaussInfftlen;
    float * data;
    float * frequencies;
    float frequencyStep;
    unsigned long dataLenOriginal;
    unsigned long dataLen;
    unsigned long specLen;
    unsigned step; //stepsize in samples in which fft-analysis is performed on the audio data
    unsigned overlap; //overlap in samples of fft-analysis windows
    unsigned specT; //spectrums n spectrogram
    unsigned specF; //frequencies in spectrum
    float maxSpecValue;
    audioAna(float *, unsigned, unsigned, unsigned , unsigned long , unsigned , unsigned long , unsigned long , bool , bool );
    audioAna(){}
    float getBoxDimensionSpectrogram(float tmin, float tmax, float fmin, float fmax, bool normalized);
    float getSpectrumAverage(float tmin, float tmax, float fmin, float fmax, bool normalized);
    float getAverage2D(float *data2d, unsigned dataWidth, unsigned dataHeight, unsigned startPosX, unsigned startPosY, unsigned sizeX, unsigned sizeY);
    float getMinFrequency();
    float getMaxFrequency();
    float getStepFrequency();
    float getSpecAvT(float time);
    float getSpecAvF(float freq);
    //general / spectrogram / frequencies / etc.
    void calcSpecAvT();
    void calcSpecAvF();
    void copyData(float * inData, unsigned long inl, unsigned long start, unsigned channels);
    void normalizeArr(float *in, float *out, unsigned long len);
    unsigned todB(float *in, float *out, unsigned long len, float f);
    float getMaxValArr(float *in, unsigned long len);
    void cutBelow(float *in, float *out, unsigned long len, float th);
    void calcSpectro(float * src, float * dest);
    void calcFrequencies();
    float calcWinGauss(unsigned len, float * wd);
    void fftr(float *in, float *out, float *win, unsigned inl, unsigned outl, bool power);
    void ifftr(float *in, float *out, float *win, unsigned inl, unsigned outl);
    //dimension-calculation-functions
    void logarr(float *in, float *out, unsigned len);
    float linReg(float *ina, float *inb, unsigned len);
    float getDimension2D(float *data2d, unsigned dataWidth, unsigned dataHeight, unsigned startPosX, unsigned startPosY, unsigned sizeX, unsigned sizeY, float strtch);
    float getDimension1D(float *data1d, unsigned dataLen, unsigned startPos, unsigned size);
    void boxCount2D(float *data, float *bcnt, float *scf, unsigned scs, unsigned datW, unsigned datH, unsigned startX, unsigned startY, unsigned sizeX, unsigned sizeY, unsigned stretch, float boxsizeBegin, unsigned mindim);
    void boxCount1D(float *data, float *bcnt, float *scf, unsigned scs, unsigned start, unsigned sl, unsigned stretch, float boxsizeBegin);
    bool is_infinite_pos(float v){
        float max_v=std::numeric_limits<float>::max();
        return ! (v <= max_v);
    }bool is_infinite_neg(float v){
        float min_v=-std::numeric_limits<float>::max();
        return ! (v >= min_v);
    }
    bool is_nan(float v){
        return v != v;
    }
};

#endif // AUDIOANA_H
