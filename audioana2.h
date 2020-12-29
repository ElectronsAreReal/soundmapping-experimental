/* Author: Robert Oetting, Mail: robo.oe [ at ] gmail.com
 * Audio-analysis class for generating static audio-data from raw audio. Not build for realtime-use, it needs adjustment for that.
 * There is absolutely no warranty that this software performs as expected. The author takes no responsibility for and financial or physical harm caused by this software.
 * Please note that the values of the spectrograms are not weighted properly. The logarithmic scaling is chosen for aesthetic appeal in last application of the class.
 * Developed for usage in soundmapping software.
 * It provides:
 * - Spectrograms (overlapping and different sizes possible) (uses kiss-fft - just powers of two)
 * - averaged spectrum over whole track
 * - time series (from averaged frequency bins for each spectrum)
 * - function for retrieving average of custom intervals of the spectrogram (good for visualization purposes)
 * - implements experimental algorithms for calculiating the box-dimension of 2d and 1d data. Used for calculating fractal dimension of spectrograms.
 *   Mind that the box-count algorithms executin time depends strongly on the input-levels or the chosen multiplication factor. The execution time is also not proportional to the amount of data-points and depends on the data-values.
 */

#include <stdio.h>
#include <string>
#include <vector>
#include "kiss_fftr.h"
#include "kiss_fft.h"
#include <limits>       // numeric_limits

#ifndef AUDIOANA2_H
#define AUDIOANA2_H
using namespace std;

class audioAna2
{

public:
    unsigned sR;
    unsigned long len;
    unsigned infftLen;
    unsigned outfftLen;
    unsigned drawWaveLen;
    float sumWinGauss;
    float maxDBexpected;
    float * speco;
    float * spec;
    float * specfa;
    float * specta;
    float * specfal;
    float * specfalm;
    float * spectal;
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
    float maxSpecoValue;
    float maxSpectal;
    float maxSpecta;
    float maxSpecfal;
    float maxSpecfalm;
    float maxSpecfa;
    unsigned winlenF;
    unsigned winlenT;
    float sumWinGfa;
    float sumWinGta;
    float * winta;
    float * winfa;
    float * tmpf;
    float * tmpt;
    audioAna2(float *, unsigned, unsigned, unsigned , unsigned long , unsigned , unsigned long , unsigned long , float );
    audioAna2(){}
    float getBoxDimensionSpectrogram(float tmin, float tmax, float fmin, float fmax, bool normalized);
    float getSpectrumAverage(float tmin, float tmax, float fmin, float fmax, bool normalized);
    float getDimT(bool logarithmized);//returns 1D Dimension of power series specta
    float getDimF(bool logarithmized);//returns 1D Dimension of spectrum average specfa
    float getDimFm();//returns 1D Dimension of spectrum average specfalm
    float getAverage2D(float *data2d, unsigned dataWidth, unsigned dataHeight, unsigned startPosX, unsigned startPosY, unsigned sizeX, unsigned sizeY);
    float getMinFrequency();
    float getMaxFrequency();
    float getStepFrequency();
    float getSpecAvT(float time);
    float getSpecAvF(float freq);
    void convGauss(float * in, unsigned len, unsigned flen, float *win, float wsum, float * tmp);
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
    void fftr(float *in, float *out, float *win, unsigned inl, unsigned outl);
    void ifftr(float *in, float *out, unsigned inl, unsigned outl);
    //dimension-calculation-functions
    void logarr(float *in, float *out, unsigned len);
    float linReg(float *ina, float *inb, unsigned len);
    float getDimension2D(float *data2d, unsigned dataWidth, unsigned dataHeight, unsigned startPosX, unsigned startPosY, unsigned sizeX, unsigned sizeY, float strtch);
    float getDimension1D(float *data1d, unsigned dataLen, unsigned startPos, unsigned size);
    void boxCount2D(float *data, float *bcnt, float *scf, unsigned scs, unsigned datW, unsigned datH, unsigned startX, unsigned startY, unsigned sizeX, unsigned sizeY, unsigned stretch, float boxsizeBegin, unsigned mindim);
    void boxCount1D(float *data, float *bcnt, float *scf, unsigned scs, unsigned start, unsigned sl, unsigned stretch, float boxsizeBegin);
    void winsymfilter(unsigned len, float * windowHammingSymbolifft);
    void getDrawWave(float * drawwave, unsigned length);
    float getVolume();
    float getCenterFreq();
    float getFvar();

    float getTvar();
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

#endif // AUDIOANA2_H
