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

#include "audioana2.h"
#include <iostream>
using namespace std;

audioAna2::audioAna2(float * rawData, unsigned infftlen, unsigned overlapSamples, unsigned sr, unsigned long len, unsigned chs,
                   unsigned long analysisLength, unsigned long start, float dbForVal1)
{
    sR=sr;
    dataLenOriginal=len;
    overlap=overlapSamples;
    infftLen=infftlen;
    if (analysisLength+start+infftlen>dataLenOriginal-1){
        std::cout << "warning: required analysis length plus start position exceed length of audio file! analysis length will be recuced to match length of audio data!" << endl;
        dataLen=dataLenOriginal-start-1;
    }
    else {
        dataLen=analysisLength-1;
    }
    outfftLen=(infftlen/2)-2;
    specF=outfftLen-1;//the DC-content is not going to be part of the spectrum.
    step=infftlen-overlap;
    specT=(dataLen-infftlen)/unsigned(step);
    specLen=unsigned(specT)*unsigned(specF);

    cout << winlenF << ", " << winlenT << endl;
    data = new float[dataLen];
    spec = new float[specLen];
    speco = new float[specLen];
    specfa = new float[specF];
    specta = new float[specT];
    specfal = new float[specF];
    specfalm = new float[specF];
    spectal = new float[specT];
    frequencies = new float[specF];
    windowGaussInfftlen = new float[infftlen];
    sumWinGauss=calcWinGauss(infftLen, windowGaussInfftlen);
    unsigned maxCorrCountp=0;
    unsigned maxCorrCountm=0;
    unsigned invalidCountNan=0;
    unsigned invalidCountInf=0;

    //std::cout << "Sample Data Statistics: tolow: " << maxCorrCountm << " tobig: " << maxCorrCountp << " Nan: " << invalidCountNan << " Inf: " << invalidCountInf << endl;
    calcFrequencies();
    frequencyStep=frequencies[1]-frequencies[0];
    copyData(rawData, dataLen, start, chs);
    for (unsigned i=0; i<dataLen; i++){
        if (data[i]>1.f){
            data[i]=1.f;
            maxCorrCountp++;
        }
        if (data[i]<-1.f){
            data[i]=-1.f;
            maxCorrCountm++;
        }
        if (is_nan(data[i])){
            data[i]=0.f;
            invalidCountNan++;
        }
        if (is_infinite_neg(data[i])){
            data[i]=-1.f;
            invalidCountInf++;
        }
        if (is_infinite_pos(data[i])){
            data[i]=1.f;
            invalidCountInf++;
        }
    }
    //cout << maxCorrCountp << " " << maxCorrCountm << endl;
    calcSpectro(data, speco);
    unsigned zerosCount=0;
    maxDBexpected=dbForVal1;
    float hearingThresholdValue=pow(10, -maxDBexpected/20.f);
    zerosCount=todB(speco, spec, specLen, hearingThresholdValue);
    maxSpecValue=getMaxValArr(spec,specLen);
    maxSpecoValue=getMaxValArr(speco,specLen);
    calcSpecAvF();
    calcSpecAvT();
    /*
    winlenF=specF/100;
    winlenT=(sR/step)*2;
    winfa=new float[winlenF];
    winta=new float[winlenT];
    sumWinGta=calcWinGauss(winlenT, winta);
    sumWinGfa=calcWinGauss(winlenF, winfa);
    tmpf=new float[winlenF+specF];
    convGauss(specfa, specF, winlenF, winfa, sumWinGfa, tmpf);
    convGauss(specfal, specF, winlenF, winfa, sumWinGfa, tmpf);
    convGauss(specfalm, specF, winlenF, winfa, sumWinGfa, tmpf);
    delete [] tmpf;
    tmpt=new float[winlenT+specT];
    convGauss(specta, specT, winlenT, winta, sumWinGta, tmpt);
    convGauss(spectal, specT, winlenT, winta, sumWinGta, tmpt);
    delete [] tmpt;
    */
    maxSpecfa=getMaxValArr(specfa,specF);
    maxSpecfal=getMaxValArr(specfal,specF);
    maxSpecfalm=getMaxValArr(specfalm,specF);
    maxSpecta=getMaxValArr(specta,specT);
    maxSpectal=getMaxValArr(spectal,specT);

    cout << "Audio-Analysis-Info-Data:" << " sampleRate: " << sR << " | samples per Channel analyzed: " <<  dataLen << endl
         << " | STFT-stepsize: " << step << " | spectrum-length(time): " << specT << " | spectrum-widht(frequency): " << specF << " | STFT-overlap: " << overlap << endl
         << " | fmin: " << frequencies[0] << " | fmax: " << frequencies[specF-1] << " | maxSpecVal Log: " << maxSpecValue  << " | maxSpecVal: " << maxSpecoValue << endl
         << " | HearingThreshold(cutoff)- value: " << hearingThresholdValue << " | spectrum values below hearing-threshold fraction: " << float(zerosCount)/float(specLen) << endl;
}



//_________________________________here are the functions for box-dimension calculation___________________________


float audioAna2::getSpecAvF(float freq){
    return specfa[unsigned(freq*float(specF)/float(sR))];
}
float audioAna2::getSpecAvT(float time){
    return specta[unsigned(time*float(sR)/float(step))];
}

float audioAna2::getSpectrumAverage(float tmin, float tmax, float fmin, float fmax, bool logarithmic){
    //std::cout << tmin << " " << tmax << " " << fmin << " " << fmax << " " << sR << endl;
    unsigned startPosY=unsigned(tmin*float(sR)/float(step));
    unsigned sizeY=unsigned((tmax-tmin)*float(sR)/float(step));
    unsigned startPosX=unsigned(fmin*float(specF)/float(sR));
    unsigned stopPosX=unsigned(fmax*float(specF)/float(sR));
    unsigned sizeX=stopPosX+1-startPosX;
    //std::cout << startPosY << " " << sizeY << " " << startPosX << " " << sizeX << endl;
    if (logarithmic){
        return getAverage2D(spec,specF,specT,startPosX, startPosY, sizeX, sizeY);
    }
    else {
        return getAverage2D(speco,specF,specT,startPosX, startPosY, sizeX, sizeY);
    }
}

float audioAna2::getMinFrequency(){
    return frequencies[0];
}

float audioAna2::getMaxFrequency(){
    return frequencies[specF-1];
}

float audioAna2::getStepFrequency(){
    return frequencyStep;
}
float audioAna2::getAverage2D(float *data2d, unsigned dataWidth, unsigned dataHeight, unsigned startPosX, unsigned startPosY, unsigned sizeX, unsigned sizeY){
    float av=0.f;
    for (unsigned x=startPosX; x<sizeX+startPosX; x++){
        for (unsigned y=startPosY; y<sizeY+startPosY; y++){
            av+=data2d[x+y*dataWidth];
        }
    }
    av=av/float(sizeX*sizeY);
    return av;
}

float audioAna2::getBoxDimensionSpectrogram(float tmin, float tmax, float fmin, float fmax, bool logarithmic){
    //returns dimension to user.
    //std::cout << tmin << " " << tmax << " " << fmin << " " << fmax << " " << sR << endl;
    unsigned startPosY=unsigned(tmin*float(sR)/float(step));
    unsigned sizeY=unsigned((tmax-tmin)*float(sR)/float(step));
    unsigned startPosX=unsigned(fmin*float(specF)/sR);
    unsigned stopPosX=unsigned(fmax*float(specF)/sR);
    unsigned sizeX=stopPosX+1-startPosX;
    //std::cout << startPosY << " " << sizeY << " " << startPosX << " " << sizeX << endl;
    if (logarithmic){
        return getDimension2D(spec,specF,specT,startPosX, startPosY, sizeX, sizeY, float(min(sizeX,sizeY)));
    }
    else {
        return getDimension2D(speco,specF,specT,startPosX, startPosY, sizeX, sizeY, float(min(sizeX,sizeY)));
    }
}

float audioAna2::getDimF(bool logarithmized){
    if (logarithmized){
        return getDimension1D(specfal, specF, 0, specF);
    }
    else{
        return getDimension1D(specfa, specF, 0, specF);
    }
}
float audioAna2::getDimFm(){
    return getDimension1D(specfalm, specF, 0, specF);
}
float audioAna2::getDimT(bool logarithmized){
    if (logarithmized){
        return getDimension1D(spectal, specT, 0, specT);
    }
    else{
        return getDimension1D(specta, specT, 0, specT);
    }

}
void audioAna2::convGauss(float *in, unsigned int len, unsigned int flen, float *win, float wsum, float *tmp){
    float av=0.f;
    int flenh=flen/2;

    for (unsigned i=0; i<flenh; i++){
        tmp[i]=in[0];
    }
    for (unsigned i=len+flenh; i<len+flen; i++){
        tmp[i]=in[len-1];
    }
    for (unsigned i=0; i<len; i++){
        tmp[i+flenh]=in[i];
    }
    for (int i=0; i<int(len); i++){
        av=0.f;
        for (int j=-flenh; j<int(flen)/2; j++){
            av+=tmp[unsigned(i+(flenh)+j)]*win[j+flenh];
        }
        in[unsigned(i)]=av/wsum;
    }
}
void audioAna2::calcSpecAvF(){
    for (unsigned i=0; i<specF; i++){
        specfa[i]=0.f;
        specfal[i]=0.f;
        specfalm[i]=0.f;
    }
    for (unsigned t=0; t<specT; t++){
        for (unsigned f=0; f<specF; f++){
            specfal[f]+=spec[f+t*specF];
            specfa[f]+=speco[f+t*specF];
            if (spec[f+t*specF]>specfalm[f]){
                specfalm[f]=spec[f+t*specF];
            }
        }
    }
    for (unsigned i=0; i<specF; i++){
        specfa[i]=specfa[i]/float(specT);
        specfal[i]=specfal[i]/float(specT);
       // std::cout << specfa[i] <<  ", "<< specfal[i] <<  "| ";
    }

}

void audioAna2::calcSpecAvT(){
    for (unsigned i=0; i<specT; i++){
        specta[i]=0.f;
        spectal[i]=0.f;
    }
    for (unsigned f=0; f<specF; f++){
        for (unsigned t=0; t<specT; t++){
            specta[t]+=speco[f+t*specF];
            spectal[t]+=spec[f+t*specF];
        }
    }
    for (unsigned i=0; i<specT; i++){
        specta[i]=specta[i]/float(specF);
        spectal[i]=spectal[i]/float(specF);
    }


}

float audioAna2::getVolume(){
    float vol=0.f;
    for (unsigned i=0; i<specF; i++){
        vol+=specfal[i];
    }
    vol/=float(specF);
    return vol;
}

float audioAna2::getCenterFreq(){
    //as suggested here: https://en.wikipedia.org/wiki/Spectral_centroid
    float sumfx=0.f;
    float sumx=0.f;
    for(unsigned f=0; f<specF; f++){
        sumfx+=frequencies[f]*specfa[f];
        sumx+=specfa[f];
    }
    return sumfx/sumx;
}

float audioAna2::getDimension2D(float *data2d, unsigned int dataWidth, unsigned int dataHeight, unsigned int startPosX, unsigned int startPosY, unsigned int sizeX, unsigned int sizeY, float strtch){
    unsigned scaleBegin=16;
    unsigned mindim=min(sizeX, sizeY);
    unsigned scalesteps=unsigned(log2(mindim))-unsigned(log2(scaleBegin));
    unsigned stretch=strtch*40.f;
    float boxsizeBegin=float(mindim)/float(scaleBegin);
    float dimension=-1.f;
    if (scalesteps<4){
        cout<<"getDimension2D: size of section to analyze is to small" << endl;
    }
    else{
        float *scales =new float[scalesteps];
        float *boxcounts =new float[scalesteps];
        float *scalesLog =new float[scalesteps];
        float *boxcountsLog =new float[scalesteps];
        boxCount2D(data2d, boxcounts, scales, scalesteps, dataWidth, dataHeight, startPosX, startPosY, sizeX, sizeY, stretch, boxsizeBegin, mindim);
        logarr(scales, scalesLog, scalesteps);
        logarr(boxcounts, boxcountsLog, scalesteps);
        dimension=linReg(scalesLog, boxcountsLog, scalesteps);
    }
    return dimension;
}

float audioAna2::getDimension1D(float *data1d, unsigned int dataLen, unsigned int startPos, unsigned int size){
    unsigned scaleBegin=16;
    unsigned scalesteps=unsigned(log2(size))-unsigned(log2(scaleBegin));
    unsigned stretch=size;
    float boxsizeBegin=float(size)/float(scaleBegin);
    float dimension=-1.f;
    if (startPos+size>dataLen){
        cout<<"getDimension2D: ERROR: size plus startPos exceeds length of given array! memory error is expected!" << endl;
    }
    if (scalesteps<2){
        cout<<"getDimension2D: size of section to analyze is to small" << endl;
    }
    else{
        float *scales =new float[scalesteps];
        float *boxcounts =new float[scalesteps];
        float *scalesLog =new float[scalesteps];
        float *boxcountsLog =new float[scalesteps];
        boxCount1D(data1d, boxcounts, scales, scalesteps, startPos, size, stretch, boxsizeBegin);
        logarr(scales, scalesLog, scalesteps);
        logarr(boxcounts, boxcountsLog, scalesteps);
        dimension=linReg(scalesLog, boxcountsLog, scalesteps);
    }
    return dimension;
}


void audioAna2::boxCount2D(float *data, float *bcnt, float *scf, unsigned int scs, unsigned int datW, unsigned int datH, unsigned int startX, unsigned int startY, unsigned int sizeX, unsigned int sizeY, unsigned int stretch, float boxsizeBegin, unsigned int mindim){
    //all dimensions must be power of two
    float upscale=float(stretch);
    float boxs=boxsizeBegin;
    int scsc=0;
    int boxcount;
    float bxu, bxo, byu, byo, bzu, bzo, xb, yb, x, y, z;
    bool allunderbox, alloverbox, openbordercross, top;
    while (boxs>=2){ //loop for each scaling factor
        x=float(startX);
        y=float(startY);
        z=0.;
        boxcount=0;
        while (x<=float(sizeX+startX)-boxs){
            bxu=x;
            bxo=x+boxs;
            y=0.;
            while (y<=float(sizeY+startY)-boxs){
                byu=y;
                byo=y+boxs;
                top=false; //stop lifting the box if all points are under the box
                z=0.;
                while (top==false){
                    if (z>stretch){
                        top=true;
                       // std::cout << " error: maximum height of bosstack reached->check for invalid spectrum data!" << endl;
                    }
                    bzu=z;
                    bzo=z+boxs;
                    allunderbox=true;
                    alloverbox=true;
                    openbordercross=false;
                    xb=bxu;
                    while (xb<bxo && alloverbox==true){//check if all points are over the box
                        yb=byu;
                        while (yb<byo && alloverbox==true){
                            if (data[datW*unsigned(yb)+unsigned(xb)]*upscale<bzo){
                                alloverbox=false;
                            }
                            yb++;
                        }
                        xb++;
                    }

                    if (alloverbox==true){ //if all points are over the box, there cant be one under the box
                        allunderbox=false;
                    }
                    else {
                        xb=bxu;
                        while (xb<bxo && allunderbox==true){//check if all points are under the box
                            yb=byu;
                             while (yb<byo && allunderbox==true){
                                if (data[datW*unsigned(yb)+unsigned(xb)]*upscale>=bzu){
                                    allunderbox=false;
                                }
                                yb++;
                            }
                            xb++;
                        }
                    }

                    if (allunderbox && byo<sizeY+startY && bxo<sizeX+startX){ //every box has three closed sides, where the ponts on the border count, and three open sides where the points do not count. if all points are under or over the box, there is the possibility left that a point at an open border (x or y) is under/over the box. if so, the box needs to be counted, because if one point under the box is next to a point over the box and on the open edge of the box, the plane intersects the box.
                        //if all points are under the box, but at least one point at the open x or y border is higher than the bottom of the box, the plane intersects the box.
                        xb=bxu;
                        while (xb<=bxo && openbordercross==false){
                            if (data[datW*unsigned(byo)+unsigned(xb)]*upscale>=bzu){ //check if one point at the border is higher than the bottom of the box
                                openbordercross=true;
                            }
                            xb++;
                        }
                        yb=byu;
                        while (yb<=byo && openbordercross==false){
                            if (data[datW*unsigned(yb)+unsigned(bxo)]*upscale>=bzu){ //check if one point at the border is higher than the bottom of the box
                                openbordercross=true;
                            }
                            yb++;
                        }
                    }
                    if (alloverbox && byo<sizeY+startY && bxo<sizeX+startX){
                        //if all points are over the box, but at least one point at the open x or y border is lower than the top of the box, the plane intersects the box.
                        xb=bxu;
                        while (xb<=bxo && openbordercross==false){ //check open y-border
                            if (data[datW*unsigned(byo)+unsigned(xb)]*upscale<bzo){ //check if one point at the border is lower than the top of the box
                                openbordercross=true;
                            }
                            xb++;
                        }
                        yb=byu;
                        while (yb<=byo && openbordercross==false){//check open x-border
                            if (data[datW*unsigned(yb)+unsigned(bxo)]*upscale<bzo){ //check if one point at the border is lower than the top of the box
                                openbordercross=true;
                            }
                            yb++;
                        }
                    }
                    if (!(allunderbox || alloverbox) || openbordercross){ //if not all points are under or over the box the box intersects the plane. Or one point at the open x or y border is: lower then the top if all are over the box. or higher than the bottom if all are under the box. Then the plane intersects the box to, even all in the range are under or all are over the box.
                        boxcount++;
                    }

                    if (allunderbox && openbordercross==false){ //if all points are under the box, and the openborderchedk ist negative, the box will not intersect the plane at any position higher than the current one.
                        top=true;
                    }
                    z=z+boxs;
                }
                y=y+boxs;
            }
            x=x+boxs;
        }
        bcnt[scsc]=float(boxcount);
        scf[scsc]=float(mindim)/float(boxs);
        boxs=boxs/2.f;
        //boxs=boxs-2.f;
        scsc++;
    }
}



void audioAna2::boxCount1D(float *data, float *bcnt, float *scf, unsigned int scs, unsigned int start, unsigned int sl, unsigned int stretch, float boxsizeBegin){
    unsigned height=stretch; //zahl mit der die kleinen float-werte des signals multipliziert werden, dadurch können die boxen als quadratisch betrachtet werden
    int scsc=0; //zähler für die skalierschritte, dient als index für die arrays scf und bcnt.
    float boxs=boxsizeBegin; //boxgröße, beginnend mit dem ersten sklierungsfaktor, eine globale variable namens boxCount1Dbegscale=16. 16 hat sich als ein guter startwert herausgestellt.
    int boxcount; //zähler für die boxen
    float bxu, bxo, byu, byo, xb, x, y, vnow, vnowb; //bxu: untergrenze der aktuellen box, bxo: obergrenze, byu: untergrenze in y-dimension, bxo: obergrenze in y-dim, xb: index für iteration über die box, x,y:position der aktuellen box
    bool allunderbox, alloverbox, openbordercross, top; //allunderobox: hilfsvariable, die auf false gesetzt wird, sobald ein wert under der box liegt, alloverbox: selbiges,
    //openbordercross: hilfsvariable, die immer dann wenn alle unter der box sind auf true gesetzt wird, falls der wert am oberen (nicht zur box gehörigen) rand der box (bxo) über der untergrenze (byu) der box liegt.
    //auch wenn alle werte über der box liegen, aber der wert bei bxo im signal kleiner als byo ist.

    while (boxs>=2){ //loop for each scaling factor
        x=start; //am anfang beginnen
        y=0;
        boxcount=0; //reset
        while (x<=sl+start-boxs){ //loop über die x-achse. verschiebt die box um eine boxbreite nach links bei jeder iteration
            bxu=x; //untergrenze der box auf der x-skala
            bxo=x+boxs; //obergrenze der box auf der y-skala
            y=0; //reset
            top=false; //reset, top wir dann true, wenn alle
            while (top==false){ //loop über die y-achse. läuft so lange wie alle Boxen unter den werten liegen.
                    byu=y;
                    byo=y+boxs;
                    allunderbox=true;
                    alloverbox=true;
                    openbordercross=false;
                    xb=bxu;
                    vnow=data[int(floor(xb))]*float(height);
                    vnowb=data[int(floor(bxo))]*float(height);
                    while (xb<bxo && alloverbox==true){//check if all points are under the box
                        if (vnow<byo){
                            alloverbox=false;
                        }
                        xb++;
                    }
                    if (alloverbox==true){ //if all points are over the box, there cant be one under the box
                        allunderbox=false;
                    }
                    else {
                        xb=bxu;
                        while (xb<bxo && allunderbox==true){//check if all points are under the box
                            if (vnow>=byu){
                                allunderbox=false;
                            }
                            xb++;
                        }
                    }
                    if (allunderbox && bxo<start+sl){ //every box has three closed sides, where the ponts on the border count, and three open sides where the points do not count. if all points are under or over the box, there is the possibility left that a point at an open border (x or y) is under/over the box. if so, the box needs to be counted, because if one point under the box is next to a point over the box and on the open edge of the box, the plane intersects the box.
                        //if all points are under the box, but at least one point at the open x or y border is higher than the bottom of the box, the plane intersects the box.

                        if (vnowb>=byu){ //check if one point at the border is higher than the bottom of the box
                            openbordercross=true;
                        }
                    }
                    if (alloverbox && bxo<start+sl){
                        //if all points are over the box, but at least one point at the open x or y border is lower than the top of the box, the plane intersects the box.
                        if (vnowb<byo){ //check if one point at the border is lower than the top of the box
                            openbordercross=true;
                        }
                    }
                    if (!(allunderbox || alloverbox) || openbordercross){ //if not all points are under or over the box the box intersects the plane. Or: one point at the open border is: - lower then the top if all are over the box. - higher than the bottom if all are under the box. Then the plane intersects the box to, even all in the range are under or all are over the box.
                        boxcount++;
                    }

                    if (allunderbox && openbordercross==false){ //if all points are under the box, and the openborderchedk ist negative, the box will not intersect the plane at any position higher than the current one.
                        top=true;
                    }
                    y=y+boxs; //box anheben
            }
            x=x+boxs; //box nach links verschieben
        }
        bcnt[scsc]=float(boxcount);
        scf[scsc]=float(sl)/float(boxs);
        boxs=boxs/2.f;
        //boxs=boxs-2.f;
        scsc++;
    }
}

float audioAna2::linReg(float *inx, float *iny, unsigned int len){
    float xyq=0.f, xq=0.f, yq=0.f, xsq=0.f;
    for (unsigned i=0; i<len; i++){
        xyq+=iny[i]*inx[i];
        yq+=iny[i];
        xq+=inx[i];
        xsq+=inx[i]*inx[i];
    }
    xyq/=float(len);
    xq/=float(len);
    yq/=float(len);
    xsq/=float(len);
    return (xyq-xq*yq)/(xsq-xq*xq); //calculate slope of linear best fit function
}


void audioAna2::copyData(float *inData, unsigned long inl, unsigned long start, unsigned int channels){
    float val;
    for (unsigned long i = 0; i< inl; i++){
        val=0.f;
        for (unsigned long c=0; c<channels; c++){
            val+=inData[i*channels+c+start];
        }
        data[i]=val;
    }
}

float audioAna2::getMaxValArr(float *in, unsigned long len){
    float val=in[0];
    for (unsigned long i=1; i<len; i++){
        if (in[i]>val){
            val=in[i];
        }
    }
    return val;
}


void audioAna2::normalizeArr(float *in, float *out, unsigned long len){
    float mv=in[0];
    for (unsigned i=1; i<len; i++){
        if (in[i]>mv){
            mv=in[i];
        }
    }
    for (unsigned i=0; i<len; i++){
        out[i]=in[i]/mv;
    }
}

void audioAna2::logarr(float *in, float *out, unsigned int len){
    for (unsigned i=0; i<len; i++){
        out[i]=log(in[i]);
    }
}

unsigned audioAna2::todB(float *in, float *out, unsigned long len, float f){
    for (unsigned i=0; i<len; i++){
        out[i]=(20.f*log(in[i]/f))/maxDBexpected;
    }
    unsigned zc=0;
    for (unsigned i=0; i<len; i++){
        if (out[i]<0.f){
            out[i]=0.f;
            zc++;
        }
    }
    return zc;
}

void audioAna2::calcFrequencies(){ //self expanatory
    for (unsigned i=0; i<specF; i++){
        frequencies[i]=float(i+1)*float(sR)/float(infftLen); //i+1 because the dc-content gets dumped.
    }
}

void audioAna2::calcSpectro(float * src, float* dest){
    for (unsigned i=0; i<specT; i++){
        fftr(src+i*step, dest+i*specF, windowGaussInfftlen, infftLen, outfftLen);
        if (specF*specT>specLen){std::cout<< " spectumOverload" << endl;}
        for (unsigned t=0; t<specF; t++){
            if (is_infinite_neg(dest[i*specF+t]) || is_infinite_pos(dest[i*specF+t])){
                std::cout << "infinite value in spectum: t: " << t << " i: " << i << endl;
            }
        }
    }
}

float audioAna2::calcWinGauss(unsigned int len, float *wd){
        int hs=len/2;
        // set standard deviation
        float sigma = float(hs)/3.5f;
        float s = 2.0f * sigma * sigma;
        float sum=0.f;
        for (int x = -hs; x <= hs; x++){
                wd[x + hs] = (exp(-(x*x)/s))/(float(M_PI) * s);
        }
        //normalize to 1. as maxsize
        float maxv=0.;
        for (unsigned x = 0; x < len; x++){
            if (wd[x]>maxv){
                maxv=wd[x];
            }
        }
        //cout << endl;
        for (unsigned x = 0; x < len; x++){
            wd[x]/=maxv;
            sum+=wd[x];
            //cout<<wd[x]<<"|";
        }
        //cout << endl;
        return sum;
}

void audioAna2::fftr(float *in, float *out, float *win, unsigned inl, unsigned outl){
    /*
     * This function takes N input values and returns (N/2)-2 real output values as magintude of the complex values.
     * So the DC-part and the sampling-rate frequency are not part of the output spectrum
    */

    kiss_fftr_cfg cfg = kiss_fftr_alloc(inl,0,0,0);
    kiss_fft_scalar *cx_in = new kiss_fft_scalar[inl];
    kiss_fft_cpx *cx_out = new kiss_fft_cpx[outl+3]; //out-array of size (N/2)-1
    for (unsigned i=0; i<inl;i++){
        cx_in[i]=in[i]*win[i];
    }
    kiss_fftr(cfg, cx_in, cx_out);
    unsigned i=0;
    for (i=0; i<outl-1; i++){
        unsigned io=i+1; //increase index by one for not copying the dc-content of the signal.
        out[i]=2.f*sqrt(cx_out[io].r*cx_out[io].r + cx_out[io].i*cx_out[io].i); //get the magnitude
    }
    free(cfg);
    delete[] cx_in;
    delete[] cx_out;
}

void audioAna2::getDrawWave(float * drawwave, unsigned int length){
    //unsigned fftinlen=(specF+0);
    unsigned fftinlen=(specF+0);
    unsigned fftoutlen=(fftinlen-1)*2;
    if (fftoutlen<=length) {
        std::cout << "Draw Wave length to long!"<<endl;
    }
    float * specfaf=new float[fftinlen];
    float * wave=new float[fftoutlen];

    unsigned zeroLowNum=5;
    unsigned start=0;
    //std::cout<<endl<< "specfa: " <<endl;
    for (unsigned i=0; i<fftinlen; i++){
        if (i<zeroLowNum+start || i>=start+specF){
            specfaf[i]=0.f;
        }
        else{
            specfaf[i]=specfa[i-start];
      //      std::cout << specfa[i-start] << ", ";
        }
    }
    //std::cout << endl;
    ifftr(specfaf, wave, fftinlen, fftoutlen);
    unsigned cnt=0;
    unsigned i=(fftoutlen-length)/2;
    while (cnt<length){
        drawwave[cnt]=wave[i];
        cnt++;
        i++;
    }

    delete [] specfaf;
    delete [] wave;
}

void audioAna2::ifftr(float *in, float *out, unsigned inl, unsigned outl){
    kiss_fftr_cfg cfg = kiss_fftr_alloc(outl, 1, NULL, NULL);
    kiss_fft_cpx *cx_in = new kiss_fft_cpx[inl];
    kiss_fft_scalar *cx_out = new kiss_fft_scalar[outl];
    float iv;
    //std::cout<<endl<< "fftin: " <<endl;
    for (int i=0; i<inl;i++){
        iv=in[i];
        cx_in[i].r= iv;
        cx_in[i].i= iv;
      //  std::cout << iv << ", ";
    }
    //std::cout<<endl<< "fftout: " <<endl;
    kiss_fftri(cfg, cx_in, cx_out);
    for (int i=0; i<outl;i++){
        out[i]= cx_out[i];
      // std::cout << out[i] << ", ";

    }
    //std::cout<<endl;    std::cout<<endl;
    free(cfg);
    delete[] cx_in;
    delete[] cx_out;
}
