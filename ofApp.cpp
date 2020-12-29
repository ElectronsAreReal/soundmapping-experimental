/* Author: Robert Oetting, Mail: robo.oe [ at ] gmail.com
 * This code is used for soundmapping.
 * It takes a set of wav-files and one GPS-Logfile in csv-format.
 * It compares the recording-times of the audio-files with the gps-dataset and determines the recording positoin of the wav-files. The recordin time of the wav-files needs to be in the filename (required format: YYYY.MM.DD_HH:MM:SS.wav)
 * The audio gets analyzed using the audioana2-class and is then saved as one array of audio-objects and time and location-information.
 * This dataset is then the basis for audiomapping.
 * Functions included for Audio-Mapping:
 * - Inverse-Distance-Interpolation (can be used to interpolate many layers. spectrograms can be created for every point in the map)
 * - several sound-representing-symbols, diagrams and other graphic elements that represent sound-properties.
 * - further audio-analysis like spectral centroids
 * - isoline-generator for usage on interpolated data. generates lines among ponts of equal values, like noise-levels, central frequency, ....
 * - outputs svg-files.
 * - draws markers for georeferencing into the map. this is useful for placing the audio map ontop of geographical maps. This way the maps can be imported into GIS-Software. (If converted into a pixel-format like png)
 * This code is highly experimental. Usage requires manipulation of the code.
 * I should re-order this file and make it more usable.
 * Things to do:
 * - Reogranize the functions in this file and order them in clases.
 * - Implement a preview-window.
 * - Implement a Interface for combining the functions.
 *
 *
 * If you want to play with this start in the setup() function and follow the commands there.
 * All the map-making happens in the drawMap(filepath)- function.
 * From there, different map-making options can be chosen by uncommenting the code.
 * There is no warranty anything will work. There are errors and ugly things in there.

 */

#include "ofApp.h"
#include <stdio.h>
#include <vector>
#include <string>
#include "ofxAudioFile.h"
#include <math.h>
using namespace std;
//--------------------------------------------------------------
void ofApp::setup(){
  //data preparation and audio analysis instanciation
  float analysisLengthSeconds=150.f; //length of audio-analysis: all files must be longer than that or they will not be includet in the data-vector.
  float startTimeSeconds=3.f; //delay at beginning of file (in case the recording has unwanted noises form pressing record)
  globAudio.length=analysisLengthSeconds-startTimeSeconds;
  //path to gps-log file in csv-format
  std::string gf("/home/robert/PublicSoundData/MAP-8.12.19/gpslog/20191208.csv");
  //std::string af("/home/robert/OF/apps/myApps/sound-to-map-0_1/bin/data/afiles/test/"); //folder containing audio files
  //std::string af("/home/robert/OF/apps/myApps/sound-to-map-0_1/bin/data/afiles/"); //folder containing audio files
  //std::string af("/home/robert/PublicSoundData/MAP-8.12.19/audio-work/"); //folder containing audio files
  std::string af("/home/robert/PublicSoundData/MAP-12.4.20/audio-work/"); //folder containing audio files: all files need to be longer than analysisLengthSeconds. all samples after that time will not be analyzed.
  //std::string af("/home/robert/PublicSoundData/recordings/Breg-25.02.19/"); //folder containing audio files

//  std::string af("/home/robert/PublicSoundData/MAP-8.12.19/audio-work/testfiles/"); //folder containing audio files
/*
  combineData(af,gf,startTimeSeconds,analysisLengthSeconds,false,false);
  //map drawing
  string mapPath1("map-idi-p3-nolog.svg");
  drawMap(mapPath1);
*/
  //project name / filename for map and logfile
  //path of output file
  string mapPath2("poster-8.12.19_31.9-paths-4a.svg");


  //logfile stuff
  logfile.open("logfile_"+mapPath2+".txt", ios::out);
  if (!logfile){
      cout<<"error creating file!"<<endl;
  }
  else {
      cout<<"logfile created!"<<endl;
  }

  //find positions for recording intervals
  float dbForMaxVal=50.f; //the db-Value representing the maximum Amplitude in the Audiofile (eg at 255 or 1.f) if this value is correctly given as specified by the recording device, then the resulting ones get physical meaning)
  combineData(af,gf,startTimeSeconds,analysisLengthSeconds, dbForMaxVal);
  //now data-vector is prepared for mapping :-)
  /*
   * this should in future save the precalculated data in a .dat file for saving time and power
  bool collectData=true;
  if (collectData){
      combineData(af,gf,startTimeSeconds,analysisLengthSeconds,true,false);
      fileData.open ("File.dat", ios::out | ios::binary | ios::trunc);
      fileData.write((char *) &data, sizeof(data));
  }
  */
  //draw the map
  drawMap(mapPath2);
  //drawPoster1(mapPath2);

}
//--------------------------------------------------------------
void ofApp::update(){
    //
    logfile.close();
    ofExit();
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//not in use, probably broken
void ofApp::drawWave(unsigned int i, float x, float y, float xs, float ys){
    unsigned dsize=xs;
    float * dat= new float[dsize];
    data[i].audio.getDrawWave(dat, dsize);
    mapData01(dat,dsize);
    for (unsigned k=0; k<xs-1; k++){
        ofSetColor(0);
        ofSetLineWidth(1.);
        std::cout << dat[k] << ", ";
        ofDrawLine(k, y+dat[k]*ys, k+1, y+dat[k+1]*ys);
    }
    std::cout << endl;
    delete [] dat;
}


void ofApp::drawSymbol(unsigned int i, float x, float y, float xs, float ys){
    unsigned dsize=xs;
    unsigned interlen=dsize/4;
    float * dat= new float[dsize+interlen];
    float * dat0= new float[dsize+interlen];

    data[i].audio.getDrawWave(dat0, dsize+interlen);

    for (unsigned i=0; i<dsize+interlen; i++) {
        dat[i]=dat0[i];
    }
    for (unsigned i=0; i<interlen+1; i++) {
        float f=float(i)/float(interlen);
        float fm=1.f-f;
        dat[i]=dat0[dsize+i]*fm+dat0[i]*f;
      //  dat[dsize-interlen+i]=dat0[dsize-interlen+i]*f+dat0[i]*fm;
    }
    mapData01(dat,dsize);
    unsigned step=dsize/5;
    //ofNoFill();
    //ofSetLineWidth(2.);
    ofBeginShape();
    for (unsigned k=0; k<xs+1; k++){
        ofSetColor(0);
        ofSetLineWidth(1.);
        //std::cout << dat[k] << ", ";
        unsigned k2=k+step;
        unsigned k1=k;
        if (k2>xs-1){
            k2=k2-xs;
        }
        if (k1>xs-1){
            k1=k1-xs;
        }
        ofVertex(x+dat[k1]*xs, y+dat[k2]*ys);//, x+dat[k+1]*xs, y+dat[k+step+1]*ys);
        //ofDrawLine(x+dat[k]*xs, y+dat[k+step]*ys, x+dat[k+1]*xs, y+dat[k+step+1]*ys);
    }
    ofEndShape();
    //std::cout << endl;
    delete [] dat;
}

void ofApp::drawPoster(string path){
    ofVec2f sizePerSample=ofVec2f((800-1)*2,(800-1)*2);
    ofVec2f size=ofVec2f(sizePerSample.x,sizePerSample.y);

    ofBackground(255,0);
    for (unsigned i=0; i<data.size(); i++){
        ofBeginSaveScreenAsSVG(names[i].erase(names[i].length()-4,names[i].length())+".svg",false, false, ofRectangle(ofVec2f(0.,0.), size));
        float y=0;
        float x=0;
        float xs=size.x;
        float ys=size.y;
        drawSymbol(i,x,y,xs,ys);
        ofEndSaveScreenAsSVG();
    }

}

void ofApp::drawPoster1(string path){
    ofBackground(255,255);
    for (unsigned i=0; i<1; i++){
        float fs=70.f*1.3;
        float ts=500.f*1.3;
        ofVec2f size=ofVec2f(800,4500);
        ofBeginSaveScreenAsSVG(names[i].erase(names[i].length()-4,names[i].length())+"notime4.svg",false, false, ofRectangle(ofVec2f(0.,0.), size));
        cout << data[i].audio.getStepFrequency() << endl;
        //ofDrawCircle(100,1000,10);
        cout << data[i].audio.getSpecAvT(100.f) << ", " << data[i].audio.getSpecAvF(500.f) << endl;
        float tstep=data[i].time.length/ts;
        float fstep=(data[i].audio.getMaxFrequency()-data[i].audio.getMinFrequency())/fs;
        float * avfs = new float[unsigned(fs)];
        float * frequs = new float[unsigned(fs)];
        cout << endl;
        for (unsigned f=0; f<unsigned(fs); f++){
            float m=3000.f*float(f)/fs;
            frequs[f]=700.f*(exp(m/1127.f) - 1.f);
            //cout << "f-number: "<< f << ", freq: " << frequs[f] << " \t";
        }
        cout << endl;
        for (unsigned f=0; f<unsigned(fs); f++){
            avfs[f]=data[i].audio.getSpectrumAverage(0.f,data[i].time.length,frequs[f],frequs[f+1],false);
            //cout << "average : "<< f << ", avamng: " << avfs[f] << " \t";
        }
        int cnt=0;
        for (float f=0; f<fs; f++){
            for (float t=0; t<ts; t++){
                if (cnt==7){
                    cnt=0;
                }
                else{
                    cnt=cnt+1;
                }

  //              ofEnableAlphaBlending();
                float tu=data[i].time.length*t/ts;
                float to=tu+tstep;
                float fu=frequs[int(f)];
                float fo=frequs[int(f)+1];
                float av=data[i].audio.getSpectrumAverage(tu,to,fu,fo,false);
                int x=int(size.x*.95*f/fs)+(size.x*.05);
                int y=int((size.y-10)*t/ts)+5;
                /*
                float col=200.f*av/avfs[int(f)];
                if (av>avfs[int(f)]){
                    //ofSetColor(255, 255, 20, 20);
                    //ofDrawCircle(x,y,10.f*av/avfs[int(f)]);
                    //ofDrawCircle(x,y,4.f);
                    ofSetColor(0);
                    //ofDrawCircle(x,y,4.f);
                    //ofDrawCircle(x,y,10.f*av/avfs[int(f)]);
                    ofDrawCircle(x,y,3.f);
                }
                */
                float d=av/avfs[int(f)];
                if (d>1){
                    int col=255-ofMap(d,1,3,0,255);
                    float size=ofMap(d,1,3,0.,10.)+2;

                    ofSetColor(0);
                    ofNoFill();
                    ofSetLineWidth(.4);
                    ofDrawCircle(x,y,size);
                    ofFill();
                    ofSetLineWidth(0);
                    ofSetColor(0,10);
                    ofDrawCircle(x,y,size);
                }
/*
                if (f==0 && cnt==0){
                    ofSetHexColor(0x000000);
                    //ofRotateDeg(90);
                    int min=int(tu)/60;
                    int sec=int(tu)%60;
                    ofDrawBitmapString("0"+ofToString(min)+":"+ofToString(sec)+"."+ofToString(tu-(float(min*60)+float(sec))).substr(2,2), 4, y);

                }

                if (t==0){
                    //ofPushView();
                    ofSetHexColor(0x000000);
                    //ofRotateZDeg(90.f);
                    ofDrawBitmapString(ofToString((fo-fu)/2.f) +" Hz", x, 75);
                    //ofPopView();
                }
                */

            }
        }
        ofEndSaveScreenAsSVG();
    }
}
void ofApp::mapDcol(vector<ofVec2f> pos, ofVec2f size){

    vector<vector<float>> indices=getIndices(); //getIndices returns: dimension of average spectrum, dimension of volume series, volume, and the spectral centroid.
    vector<vector<float>> bands(indices.size());
    for (unsigned i=0; i<bands.size(); i++){
        //bands[i]=prepVector(indices[i]);
        bands[i]=indices[i];
    }
    float * fieldx=new float[(unsigned long)(size.x*size.y)];
    float * fieldy=new float[(unsigned long)(size.x*size.y)];
    float * fieldc=new float[(unsigned long)(size.x*size.y)];
    float interpolationPowerParameter=5.f;
    inverseDistanceInterpolation(bands[4],pos,fieldx,size, interpolationPowerParameter);
    inverseDistanceInterpolation(bands[1],pos,fieldy,size, interpolationPowerParameter);
    inverseDistanceInterpolation(bands[2],pos,fieldc,size, interpolationPowerParameter);
    image.allocate(size.x, size.y, OF_IMAGE_COLOR);
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            //unsigned val=unsigned(field[x+unsigned(size.x)*y]/255.);
            char valr=char(ofMap(fieldx[x+unsigned(size.x)*y] ,1.f,2.f,0,255)); //freq-diversity
            char valg=char(ofMap(fieldy[x+unsigned(size.x)*y] ,1.f,2.f,0,255)); //time-diversity
            char valb=char(ofMap(fieldc[x+unsigned(size.x)*y] ,0.f,1.f,0,255)); //volume
            image.setColor(x,y,ofColor(valr,valg,valb));
        }
    }
    delete [] fieldx;
    delete [] fieldy;
    delete [] fieldc;
    image.draw(0,0);
    /*
    float radius=size.length()/300.f;
    for (unsigned i=0; i<pos.size(); i++){
        ofNoFill();
        ofSetLineWidth(radius/10.f);
        ofSetColor(255);
        ofDrawCircle(pos[i],radius);
        ofSetColor(0);
        ofDrawCircle(pos[i],radius+radius/10.f);
    }
    */
    ofSetColor(5,10,230);
   // ofScale(15.,15.);
    float boxSize=size.x/30.f;
    ofVec2f gsize(boxSize,boxSize*.3f);
    float globalMaxf=globAudio.maxSpecfalm;
    float globalMaxt=globAudio.maxSpectal;
    for (unsigned i=0; i<pos.size(); i++){
        ofPushMatrix();
        float * datf=data[i].audio.specfalm;
        float * datt=data[i].audio.spectal;
        //drawGraph3(datf,data[i].audio.specF, pos[i], gsize, globalMaxf);
        //drawGraph3(datt,data[i].audio.specT, pos[i]+ofVec2f(0,gsize.y), gsize, globalMaxt);
        ofSetColor(255);
        ofTranslate(pos[i]-ofVec2f(gsize.x,gsize.y));
        ofScale(0.5);
        ofDrawBitmapString(names[i], 0, 0);
        ofPopMatrix();
    }
}
void ofApp::mapFcol(vector<ofVec2f> pos, ofVec2f size){
    vector<vector<float>> bands0=getFreqBands(350, 1000, true);
    vector<vector<float>> bands(3);
    for (unsigned i=0; i<3; i++){
        bands[i]=prepVector(bands0[i]);
    }
    float * fieldx=new float[(unsigned long)(size.x*size.y)];
    float * fieldy=new float[(unsigned long)(size.x*size.y)];
    float * fieldz=new float[(unsigned long)(size.x*size.y)];
    float interpolationPowerParameter=5.f;
    inverseDistanceInterpolation(bands[0],pos,fieldx,size, interpolationPowerParameter);
    inverseDistanceInterpolation(bands[1],pos,fieldy,size, interpolationPowerParameter);
    inverseDistanceInterpolation(bands[2],pos,fieldz,size, interpolationPowerParameter);
    image.allocate(size.x, size.y, OF_IMAGE_COLOR);
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            //unsigned val=unsigned(field[x+unsigned(size.x)*y]/255.);
            char valr=char(ofMap(fieldx[x+unsigned(size.x)*y] ,0.f,1.f,0,255));
            char valg=char(ofMap(fieldy[x+unsigned(size.x)*y] ,0.f,1.f,0,255));
            char valb=char(ofMap(fieldz[x+unsigned(size.x)*y] ,0.f,1.f,0,255));
            image.setColor(x,y,ofColor(valr,valg,valb));
        }
    }
  //  delete [] fieldx;
  //  delete [] fieldy;
  //  delete [] fieldz;
    image.draw(0,0);

    float radius=size.length()/300.f;
    for (unsigned i=0; i<pos.size(); i++){
        ofFill();

        ofDrawCircle(pos[i],radius);
        ofSetColor(0);
        ofSetLineWidth(radius/2.f);
        ofSetColor(255);
        ofDrawCircle(pos[i],radius);
    }
    /*
    ofPushMatrix();
    ofSetColor(5,10,230);
   // ofScale(15.,15.);
    for (unsigned i=0; i<pos.size(); i++){
        cout << dimensions_measured[i] << endl;
        ofDrawCir
        ofDrawBitmapString(ofToString(dimensions_measured[i]), pos[i]);
    }
    ofPopMatrix();
    */
}

void ofApp::mapLine(vector<ofVec2f> pos, ofVec2f size){
    unsigned bandNum=64;
    vector<vector<float>> bands0=getFreqBands(350, 1000, true);
    vector<vector<float>> bands(bandNum);
    for (unsigned i=0; i<bandNum; i++){
        bands[i]=prepVector(bands0[i]);
    }
    float * fieldx=new float[(unsigned long)(size.x*size.y)];
    float * fieldy=new float[(unsigned long)(size.x*size.y)];
    float * fieldz=new float[(unsigned long)(size.x*size.y)];
    float interpolationPowerParameter=4.f;
    inverseDistanceInterpolation(bands[0],pos,fieldx,size, interpolationPowerParameter);
    inverseDistanceInterpolation(bands[1],pos,fieldy,size, interpolationPowerParameter);
    inverseDistanceInterpolation(bands[2],pos,fieldz,size, interpolationPowerParameter);
    image.allocate(size.x, size.y, OF_IMAGE_COLOR);
    for (unsigned x=0; x<size.x; x++){
        for (unsigned y=0; y<size.y; y++){
            //unsigned val=unsigned(field[x+unsigned(size.x)*y]/255.);
            char valr=char(ofMap(fieldx[x+unsigned(size.x)*y] ,0.f,1.f,0,255));
            char valg=char(ofMap(fieldy[x+unsigned(size.x)*y] ,0.f,1.f,0,255));
            char valb=char(ofMap(fieldz[x+unsigned(size.x)*y] ,0.f,1.f,0,255));
            image.setColor(x,y,ofColor(valr,valg,valb));
        }
    }
  //  delete [] fieldx;
  //  delete [] fieldy;
  //  delete [] fieldz;
    image.draw(0,0);
    float radius=size.length()/100.f;
    for (unsigned i=0; i<pos.size(); i++){
        ofNoFill();
        ofSetLineWidth(5);
        ofSetColor(255);

        ofDrawCircle(pos[i],radius);
        ofSetColor(0);
        ofDrawCircle(pos[i],radius+3.f);
    }
    /*
    ofPushMatrix();
    ofSetColor(5,10,230);
   // ofScale(15.,15.);
    for (unsigned i=0; i<pos.size(); i++){
        cout << dimensions_measured[i] << endl;
        ofDrawBitmapString(ofToString(dimensions_measured[i]), pos[i]);
    }
    ofPopMatrix();
    */
}
void ofApp::drawGraph(float* data, unsigned dl, ofVec2f pos, ofVec2f size){
    ofNoFill();
    ofVec2f center(pos+.5*size);
    ofVec2f point;
    ofVec2f dir;
    float astep=2.f*3.14156f/float(dl);
    float r0=size.length()*.5f;
    int idx=0;
    for (float a=0.f; a<2.f*3.14156f; a+=astep){
        dir=ofVec2f(cos(a), sin(a));
        point=center+dir*r0*data[unsigned(idx)];
        ofSetLineWidth(size.x/70.f);
        ofSetColor(0,0,0,200);
        ofDrawLine(center, point);
        idx++;
    }
}
void ofApp::drawGraph2(float* data, unsigned dl, ofVec2f pos, ofVec2f size){
    ofFill();
    ofVec2f center(pos+.5*size);
    ofVec2f point;
    ofVec2f dir;
    float astep=2.f*M_PIf32/float(dl-1);
    float r0=size.length()*.5f;
    unsigned idx=0;
    ofSetLineWidth(.0f);
    ofSetColor(0,0,0,200);
    ofBeginShape();
    float a=0.f;
    for (a=0.f; a<=2.f*M_PIf32; a+=astep){
        dir=ofVec2f(cos(a), sin(a));
        point=center+dir*r0*data[idx];
        ofVertex(point);
        idx++;
    }
    dir=ofVec2f(cos(0.f), sin(0.f));
    point=center+dir*r0*data[0];
    ofVertex(point);
    ofEndShape();
}
void ofApp::drawGraph3(float* data, unsigned dl, ofVec2f pos0, ofVec2f size, float maxValue){
    ofVec2f pos=pos0-size/2.f;
    float lineW=size.x/300.f;
    ofFill();
    ofSetLineWidth(lineW);
    ofSetColor(255,255,255);
    ofDrawRectangle(pos, size.x, size.y);
    ofFill();
    ofVec2f point;
    ofVec2f dir;
    float astep=size.x/float(dl);
    unsigned idx=0;
    ofSetLineWidth(.0f);
    ofSetColor(0,0,0);
    ofBeginShape();
    for (float a=0.f; a<size.x; a+=astep){
        float d=ofMap(data[idx], 0.f, maxValue, 0.f, size.y);
        point=pos+ofVec2f(a, size.y-d);
        ofVertex(point);
        idx++;
    }
    float d=ofMap(data[dl-1], 0.f, maxValue, 0.f, size.y);
    ofVertex(pos.x+size.x, pos.y+size.y-d);
    ofVertex(pos.x+size.x, pos.y+size.y);
    ofVertex(pos.x, pos.y+size.y);
    ofEndShape();
}
void ofApp::drawGraph5(float* data, unsigned dl, ofVec2f pos, ofVec2f size, float sta, float sto){

    ofVec2f center(pos);
    ofVec2f point;
    ofVec2f dir;
    float astep=(sto-sta)/float(dl);
    float r00=size.length()*.1f;
    float r0=size.length()*.5f;
    float ll=r0-r00;
    int idx=0;
    ofSetLineWidth(size.x/60.f);
    float av=data[0];
    for (unsigned i=0; i<dl; i++){
        av+=data[i];
    }
    /*
    ofFill();
    ofSetColor(50+int(200.f*av/float(dl)));
    ofDrawCircle(pos, r00);
    */
    ofFill();
    //ofSetColor(0,0,0,255);
    //ofSetLineWidth(size.x/40.f);
    ofBeginShape();
    ofVertex(center+ofVec2f(cos(sta), sin(sta))*r00);
    for (float a=sta; a<sto; a+=astep){
        dir=ofVec2f(cos(a), sin(a));
        point=center+dir*(r00+ll*data[unsigned(idx)]*.95);
        ofVertex(point);
        idx++;
    }
    ofVertex(center+ofVec2f(cos(sto), sin(sto))*r00);
    ofEndShape();
}
void ofApp::drawGraph4(float* data, unsigned dl, ofVec2f pos, ofVec2f size){

    ofVec2f center(pos);
    ofVec2f point;
    ofVec2f dir;
    float astep=2.f*3.14156f/float(dl);
    float r00=size.length()*.1f;
    float r0=size.length()*.5f;
    float ll=r0-r00;
    int idx=0;
    ofSetLineWidth(size.x/60.f);
    float av=data[0];
    for (unsigned i=0; i<dl; i++){
        av+=data[i];
    }
    /*
    ofFill();
    ofSetColor(50+int(200.f*av/float(dl)));
    ofDrawCircle(pos, r00);
    */
    ofFill();
    //ofSetColor(0,0,0,255);
    //ofSetLineWidth(size.x/40.f);
    ofBeginShape();
    ofVertex(center+ofVec2f(cos(0), sin(0))*r00);
    for (float a=0.f; a<2.f*3.14156f; a+=astep){
        dir=ofVec2f(cos(a), sin(a));
        point=center+dir*(r00+ll*data[unsigned(idx)]*.95);
        ofVertex(point);
        idx++;
    }
    ofVertex(center+ofVec2f(cos(2.f*3.14156f), sin(2.f*3.14156f))*r00);
    ofEndShape();
}
float ofApp::triwave(float x){
    float pih=(3.14156f/2.f);
    float xx=fmod(x, (3.14156f));
    float out;
    if (xx<pih){
        out=xx;
    }
    else{
        out=3.141156f-xx;
    }
    return out/pih;
}

void ofApp::drawSymbol2(ofVec2f pos0, ofVec2f size, vector<float> bands, vector<float> indices){
    ofVec2f pos=pos0-size/2.f;
    int tdiv=int(ofMap(indices[1],1.2f, 2.f, 1, 8));
    int fdiv=int(ofMap(indices[4],1.2f, 2.f, 1, 8));
    float volume=ofMap(indices[2],0.f, .6f, 0.0f, size.x*.8);
    int freq=int(ofMap(indices[3],600.f, 1350.f, 40.f, 254.f));

    ofNoFill();
    ofSetColor(0,255);
    ofSetLineWidth(size.x*.05);
    //time-diversity
    float step=(size.x)/float(tdiv);

    for (float s=0.f; s<size.x; s+=step){
        ofDrawLine(pos.x+s, pos.y, pos.x+s, pos.y-size.y);
    }

    step=(size.y)/float(fdiv);
    for (float s=0.f; s<size.y; s+=step){
        ofDrawLine(pos.x,pos.y-s, pos.x+size.x, pos.y-s);
    }
    ofVec2f center=pos+ofVec2f(size.x*.5f, -size.y*.5f);
    ofFill();
    ofSetColor(freq,255);
    ofSetLineWidth(.0f);
    ofDrawCircle(center, volume*.3f);
}

void ofApp::drawSymbol1(ofVec2f pos0, ofVec2f size, vector<float> bands, vector<float> indices){
    ofVec2f pos=pos0-size/2.f;
    ofVec2f size1(size.x/2.f, size.y/2.f);
    int tdiv=int(ofMap(indices[1],1.2f, 2.f, 1, 8));
    int fdiv=int(ofMap(indices[4],1.2f, 2.f, 1, 8));
    float volume=ofMap(indices[2],0.f, .6f, 0.0f, 1.f);
    float freq=ofMap(indices[3],600.f, 1300.f, 0.2f, 1.f);
    ofFill();
    ofSetLineWidth(.5);
    ofSetColor(255);
    float border=size.x/10.f;
    ofVec2f posr=pos-border;
    ofDrawRectangle(posr,size.x+border*2,size.y+border*2);
    ofNoFill();
    ofSetColor(0);
    //time-diversity
    ofVec3f size1l=size1*1.;
    ofVec2f sdif=(size1-size1l);
    float step=size1l.x/float(tdiv);
    float steph=step/2.f;

    for (float s=sdif.x*.5f+steph; s<size1.x-sdif.x*.5f; s+=step){
        ofSetLineWidth(.7);
        ofDrawLine(pos.x+s, pos.y, pos.x+s, pos.y+size1l.y);
    }

    step=size1l.x/float(fdiv);
    for (float s=sdif.y*.5f+steph; s<size1.y-sdif.y*.5f; s+=step){
        ofSetLineWidth(.7);
        ofDrawLine(pos.x+size1l.x,pos.y+s, pos.x+size1l.x*2.f, pos.y+s);
    }
    ofVec2f center=pos+ofVec2f(size1.x*0.5f, size1.y*1.5f);
    ofFill();
    ofSetColor(20);
    ofSetLineWidth(0.f);
    ofDrawCircle(center, volume*size1.x*.45f);
    center=pos+ofVec2f(size1.x*1.5f, size1.y*1.5f);
    ofFill();
    ofSetColor(20);
    ofSetLineWidth(0.f);
    ofDrawCircle(center, freq*size1.x*.45f);
}

void ofApp::drawSymbolSimple1(ofVec2f pos, ofVec2f size, vector<float> bands){
    float * as=new float[3];

    for (unsigned b=0; b<3; b++){
        float val=bands[b];
        as[b]=val;
        //float val=bands[b][i];
        /*
        as[b]=log10(val/0.005)*.5;
        if (as[b]<0.f){
            as[b]=0.f;
        }
        */
    }
    int xs=size.x;
    int ys=size.y;

    int pts=(xs+ys)*4;
    float * formL=new float[pts];
    float * formM=new float[pts];
    float * formH=new float[pts];
    float * form=new float[pts];
    float r0=.5;
    float my=pos.x+ys/2.f;
    float mx=pos.y+xs/2.f;

    //LOW
    for (int j=0; j<pts; j++){
        float angle=2.f*3.14156f*float(j)/float(pts);
        float w=cos(3.f*angle)*1.f+1.f;
        float r=w;
        formL[j]=.2f+r*as[0]*.5f;
    }
    //MID
    for (int j=0; j<pts; j++){
        float angle=2.f*3.14156f*float(j)/float(pts);
        float w=cos(9.f*angle)*1.f+1.f;
        float r=w;
        formM[j]=.2f+r*as[1]*.5f;
    }
    //HI
    for (int j=0; j<pts; j++){
        float angle=1.5f+2.f*3.14156f*float(j)/float(pts);
        float w=triwave(15.f*angle)*1.f;
        float r=w;
        formH[j]=0.2f+r*as[2]*.9f;
    }
    for (int j=0; j<pts; j++){
        form[j]=max(formL[j],max(formM[j],formH[j]));
//        form[j]=(formL[j]*.33+formM[j]*.33+formH[j]*.33);
    }
    //draw form
    ofNoFill();
    ofSetColor(50);

    ofSetLineWidth(.1);
    ofBeginShape();

    for (int j=0; j<pts; j++){
        float angle=2.f*3.14156f*float(j)/float(pts);
        float yn=sin(angle)*(formL[j])*float(ys/2);
        float xn=cos(angle)*(formL[j])*float(ys/2);
        ofVertex(mx+xn,my+yn);
    }
    float angle=0.f;
    float yn=sin(angle)*(formL[0])*float(ys/2);
    float xn=cos(angle)*(formL[0])*float(ys/2);
    ofVertex(mx+xn,my+yn);
    ofEndShape();

    ofNoFill();
    ofSetColor(50);
    ofSetLineWidth(.1);
    ofBeginShape();
    for (int j=0; j<pts; j++){
        float angle=2.f*3.14156f*float(j)/float(pts);
        float yn=sin(angle)*(formM[j])*float(ys/2);
        float xn=cos(angle)*(formM[j])*float(ys/2);
        ofVertex(mx+xn,my+yn);
    }
    angle=0.f;
    yn=sin(angle)*(formM[0])*float(ys/2);
    xn=cos(angle)*(formM[0])*float(ys/2);
    ofVertex(mx+xn,my+yn);
    ofEndShape();

    ofNoFill();
    ofSetColor(50);
    ofSetLineWidth(.2);
    ofBeginShape();
    for (int j=0; j<pts; j++){
        float angle=2.f*3.14156f*float(j)/float(pts);
        float yn=sin(angle)*(formH[j])*float(ys/2);
        float xn=cos(angle)*(formH[j])*float(ys/2);
        ofVertex(mx+xn,my+yn);
    }
    angle=0.f;
    yn=sin(angle)*(formH[0])*float(ys/2);
    xn=cos(angle)*(formH[0])*float(ys/2);
    ofVertex(mx+xn,my+yn);
    ofEndShape();

    ofSetColor(0);
    ofNoFill();
    ofSetLineWidth(.1);
    ofBeginShape();
    for (int j=0; j<pts; j++){
        float angle=2.f*3.14156f*float(j)/float(pts);
        float yn=sin(angle)*(form[j])*float(ys/2);
        float xn=cos(angle)*(form[j])*float(ys/2);
        ofVertex(mx+xn,my+yn);
    }
    angle=0.f;
    yn=sin(angle)*(form[0])*float(ys/2);
    xn=cos(angle)*(form[0])*float(ys/2);
    ofVertex(mx+xn,my+yn);
    ofEndShape();
    delete [] form;
}


void ofApp::mapFLine(vector<ofVec2f> pos, ofVec2f size){
    unsigned chs0=24;
    float * freqsMel = new float[chs0+1];
    vector<vector<float>> bands0=getFreqBandsMel(chs0,3600.f,freqsMel, true);
    vector<vector<float>> bands=prepFreqsVecs(bands0);
    float maxv=0;
    for (unsigned i=0; i<chs0; i++){
        for (unsigned j=0; j<bands[i].size(); j++){
            if (bands[i][j]>maxv){
                maxv=bands[i][j];
            }
        }
    }
    for (unsigned i=0; i<chs0; i++){
        for (unsigned j=0; j<bands[i].size(); j++){
            bands[i][j]/=maxv;
        }
        for (unsigned j=0; j<bands[i].size(); j++){
            cout << bands[i][j] << ", ";
        }
        cout << endl;
    }
    cout << endl;
    unsigned long fieldsize=(unsigned long)(size.x*size.y);
    float * field0=new float[fieldsize*chs0];
    int * field=new int[fieldsize];
    float * field2=new float[fieldsize];

    float interpolationPowerParameter=5.f;
    for (unsigned i=0; i<chs0; i++){
        inverseDistanceInterpolation(bands[i],pos,field0+i*fieldsize,size, interpolationPowerParameter); //power parameter gets calculated from point density! (not anymore!, wont work properly :-/)
    }
    int lineNum=12;
    float mag=0.f, magmax=0.f, magmin=99999999.f;;;
    char val;
    image.allocate(size.x, size.y, OF_IMAGE_COLOR);
    //image.setColor(255);
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            mag=0.f;
            for (unsigned i=0; i<chs0; i++){
                mag+=field0[x+unsigned(size.x)*y+fieldsize*i];
            }
            if (mag<magmin){
                magmin=mag;
            }
            field2[x+unsigned(size.x)*y]=mag;
        }
    }
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            mag=field2[x+unsigned(size.x)*y];
            mag=mag-magmin;
            if (mag>magmax){
                magmax=mag;
            }
            field2[x+unsigned(size.x)*y]=mag;
        }
    }
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            /*find points where the difference to neighbours is smaller than a small threshold,
             * Then write a number at that border. all other points get the number zero. In the next step this line of pixels will be traced by creating a vector of points on that line.
             *(and the average intensity ist equal to discrete values representing the desired number of lines)
            */
            mag=field2[x+unsigned(size.x)*y];
            int valm=int(ofMap(mag, 0.f, magmax, 1, lineNum+1));
            field[x+unsigned(size.x)*y]=valm;
            image.setColor(x,y,ofColor(unsigned((float(valm)/float(lineNum+2))*254.f),unsigned(mag*254.f),0,0));

        }
    }
    delete [] field2;
    int xn, yn, grad, diff, valn, maxval;
    unsigned idx;
    int * field1=new int[fieldsize];
    int * check=new int[fieldsize];
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            idx=x+unsigned(size.x)*y;
            grad=0;
            maxval=0;
            bool found=false;

            for (int xk=-1; xk<=0; xk++){
                for (int yk=-1; yk<=0; yk++){
                    xn=x+xk;
                    yn=y+yk;
                    if (found==false && xn>0 && yn>0 && xn<unsigned(size.x)-1 && yn<unsigned(size.y)-1 && (xk!=0 || yk!=0)){
                        valn=field[unsigned(xn)+unsigned(size.x)*unsigned(yn)];
                        diff=field[idx]-valn;
                        grad+=abs(diff);
                        if (grad>0){
                            if (maxval<valn){
                                maxval=valn;
                            }
                        }

                    }
                }
            }
            if (grad>0){
                val=255;
                field1[idx]=2;
            }
            else{
                val=0;
                field1[x+unsigned(size.x)*y]=0;
            }
            //image.setColor(x,y,ofColor(val,val,val));
        }
    }
   // image.draw(0,0);
    for (int i=0; i<fieldsize; i++){
        check[i]=0;
    }

    int xa, ya, xab, yab, linecolor, idxs, xk, yk;
    bool foundNextPoint;
    const int searchNum=4;
    //int sidxX [8]={-1,-1, 0, 1, 1, 1, 0,-1}; //arrays containing indices for searching pixel neighbourhood clockwise. this is necessary in order not to get to a dead end
    //int sidxY [8]={ 0,-1,-1,-1, 0, 1, 1, 1};
    int sidxX [searchNum]={-1, 0, 1, 0}; //arrays containing indices for searching pixel neighbourhood clockwise. this is necessary in order not to get to a dead end
    int sidxY [searchNum]={ 0,-1, 0, 1};
    vector<vector<ofVec2f>> linePaths;
    vector<bool> closedLoop;
    for (int x=1; x<int(size.x)-1; x++){
        for (int y=1; y<int(size.y)-1; y++){
            //in this loop over all pixels, every time a point in field1 other than zero is encountered, a line goes through this point.
            //if the line has not been followed before (we could have encountered it earlyer) the line gets traced in two steps:
            //1) follow the line in an arbitrary direction until the starting point is found again (loop) or the line ends (open line)
            //2) from that end point / point on loop, we follow the line again until the end is reached. this time the points on the way get recorded in "vector<vector<ofVec2f>> linePaths".
            //every line gets one subvector of linePaths
            unsigned long idx=x+unsigned(size.x)*y;
            int pathPointCount=0;
            if (field1[idx]>0 && check[idx]==0){ //if this is true, a point on a unfollowed line has been found which is not part of any line traced before
                //first this line will be followed until the end is found, or we get back to the starting point
                bool isClosedLoop=false;
                bool foundTaleOrLoop=false;
                check[idx]=-1; //-1 is the marker for the pathfinding to locate the starting point (one end or for identifiying a loop-structure)
                xa=x; //coordinates for following path of same valued array elements. if the path end ins found or the path is a closed loop, the path gets followed back and recorded for later line rendering :-)
                ya=y;
               // image.setColor(xa,ya,ofColor(255,0,255,150));
                xab=x;
                yab=y;
                linecolor=field1[idx]; //set the value of the line to be followed in field1. (this is necessary to avoid getting lost at encounters with another line, aka to stay on the track)
                //in this while loop the End of the pixel-path is found, and if the path is closed, its closedness will be confirmed
                while (foundTaleOrLoop==false){
                    foundNextPoint=false; //set check variable false
                    for (int n=0; n<searchNum; n++){ //iterate through array with neighbouring points; clockwise around current pixel [xa, ya]
                            xk=sidxX[n];
                            yk=sidxY[n];
                            xn=xa+xk;
                            yn=ya+yk;
                            if (xn>0 && yn>0 && xn<int(size.x) && yn<int(size.y) && foundNextPoint==false){ //search the neighbourhood clockwise if we are not at the border of the pixel array
                                idxs=int(xn)+int(size.x)*int(yn); //neighbourhood index
                                valn=field1[idxs]; //value of current neighbourhood pixel
                                if (valn==linecolor && check[idxs]==0){
                                    xa=xn;
                                    ya=yn;
                                    check[idxs]=-1; //mark point as found in path
                                    foundNextPoint=true; //marker set, so we go on. if false the end of line is found or the loop is closed (if isClosedLoop is also true)
                            }
                        }
                    }
                    pathPointCount++;
                   // image.setColor(xa,ya,ofColor(15,250,2,150));
                    if (foundNextPoint==false){
                        foundTaleOrLoop=true;

                    }
                }
                //cout<< "p1: " << pathPointCount << " xa: " << xa << " ya: " << ya <<endl;
                pathPointCount=0;
                vector<ofVec2f> newPath; //add new line vector to lines vector
                bool pathRecorded=false;
                int startX=xa;
                int startY=ya;
                check[idxs=int(xa)+int(size.x)*int(ya)]=linecolor;;
                isClosedLoop==false;
                while (pathRecorded==false){
                    foundNextPoint=false;
                    for (int n=0; n<searchNum; n++){
                            xk=sidxX[n];
                            yk=sidxY[n];
                            xn=xa+xk;
                            yn=ya+yk;
                            if (xn>0 && yn>0 && xn<int(size.x) && yn<int(size.y) && foundNextPoint==false){
                                idxs=int(xn)+int(size.x)*int(yn);
                                valn=field1[idxs];
                                if (valn==linecolor && check[idxs]!=linecolor){
                                    xab=xa;
                                    yab=ya;
                                    xa=xn;
                                    ya=yn;
                                    check[idxs]=linecolor;
                                    foundNextPoint=true;
                                    //break;
                            }
                        }
                    }
                    if (pathPointCount>10 && abs(xa-startX)<4 && (abs(ya-startY)<4)){ //if we find the start point again the loop is closed and we are finished
                        isClosedLoop=true;
                    }
                    if (foundNextPoint==false || isClosedLoop==true){
                        pathRecorded=true;
                    }
                    newPath.push_back(ofVec2f((float(xa)+float(xab))/2.f, (float(ya)+float(yab))/2.f));
                    pathPointCount++;
                    if (pathPointCount>1){
                     //   ofDrawLine(newPath[newPath.size()-1],newPath[newPath.size()-2]);
                    }
                }
                if (isClosedLoop){
                    newPath.push_back(newPath[0]);
                 //   cout<<"closed loop: " << isClosedLoop << endl;
                }
                if (newPath.size()>10){
                    linePaths.push_back(newPath); //add the newly found path to the vector of paths
                    closedLoop.push_back(isClosedLoop);
                }
               // cout<< "p2: " << pathPointCount <<endl;
               // cout<<endl;
             }

        }
    }

    cout << linePaths.size() << endl;

    image.save("map-1.bmp");
    //now, equalize distance between points



    vector<vector<ofVec2f>> linePathsResampled;
    float distBetNewPts=1.5f;
    for (unsigned i=0; i<linePaths.size(); i++) {
        vector<ofVec2f> path=linePaths[i];
        vector<ofVec2f> pathResampled;
        ofVec2f p0, p0b, dir, dirn, pdraw;
        float distBetNewPts=1.f;
        float mindist=distBetNewPts;
        pathResampled.push_back(path[0]);
        cout<<"i: "<< i << endl;
        if (closedLoop[i]){
            for (unsigned p=1; p<path.size(); p++){
                p0b=pathResampled.back();
                p0=path[p];
                dir=p0-p0b;
                dirn=dir;
                dirn.normalize();
                while ((pathResampled.back()-p0b).length()<dir.length()){
                    pdraw=pathResampled.back()+distBetNewPts*dirn;
                    pathResampled.push_back(pdraw);
                }
            }
            /*
            p0b=pathResampled.back();
            p0=pathResampled[0];
            dir=p0-p0b;
            dirn=dir;
            dirn.normalize();
            cout << "p:" << p0b <<", dir:" << dir.x <<", " <<dir.y<<endl;
            while ((pathResampled.back()-p0b).length()<dir.length()){
                pdraw=pathResampled.back()+distBetNewPts*dirn;
                pathResampled.push_back(pdraw);
            }
            */
        }
        else{
            for (unsigned p=1; p<path.size(); p++){
                p0b=pathResampled.back();
                p0=path[p];
                dir=p0-p0b;
                dirn=dir;
                dirn.normalize();
                while ((pathResampled.back()-p0b).length()<dir.length()){
                    pdraw=pathResampled.back()+distBetNewPts*dirn;
                    pathResampled.push_back(pdraw);
                }
           }
        }

        linePathsResampled.push_back(pathResampled);
        linePaths[i].clear();
    }
    linePaths.clear();

    //now! smooth the paths!

    vector<vector<ofVec2f>> linePathsSmooth;
    vector<bool> closedLoopSmooth;
    for (unsigned i=0; i<linePathsResampled.size(); i++) {
        vector<ofVec2f> path=linePathsResampled[i];
        vector<ofVec2f> pathSmooth;
        float slen=15.f;
        if (path.size()>=100/distBetNewPts){
            for (unsigned p=0; p<path.size(); p++){
                //image.setColor(path[p].x,path[p].y,ofColor(0,255,0,0));
                ofVec2f av=path[p];
                float slenn=0.f;
                int s=1;
                while (slenn<slen && p+s<path.size()){
                    av+=path[p+s-1];
                    slenn+=(path[p+s-1]-path[p+s]).length();
                    s++;
                }
                if (s>2){
                    pathSmooth.push_back(av/float(s));
                }

            }

            if (closedLoop[i]){
                pathSmooth.push_back(pathSmooth[0]); //close the loop (smoothing made it open again)
                closedLoopSmooth.push_back(true);
              //  pathSmooth.push_back(pathSmooth[1]);
            }
            else{
                closedLoopSmooth.push_back(false);
            }
            linePaths.push_back(pathSmooth);
        }
    }

    //draw the frequency content distortetd lines of equal loudness. yeah!!!
 //   linePaths=linePathsResampled;
    ofNoFill();
    ofSetLineWidth(.5);
    vector<ofVec2f> pathdraw;
    for (unsigned i=0; i<linePaths.size(); i++) {
        vector<ofVec2f> path=linePaths[i];
        //ofSetColor((255*i)/linePaths.size(),255,0,255);
        ofSetColor(255,255,255);
        ofVec2f p0, p0b, dir, dirn, dirorthon, pdraw;

        float len=0.f;

        ofBeginShape();

        cout<<"i: " <<i<<", cl: " << closedLoopSmooth[i]<<endl;
        unsigned p,pb;
        for (unsigned p=1; p<path.size();p++){
            p0b=path[p-1];
            p0=path[p];
            dir=p0-p0b;
            dirn=dir;
            dirorthon=dirn.normalize().rotate(90.f);
            ofVertex(p0b);
            pathdraw.push_back(pdraw);
           // ofVertex(p0+dirorthon*5.f*sin(len/period));
            len+=dir.length(); //integrate length of path

            //cout<<"dir: "<<dir<<", dirn: "<<dirn<<", ortho: "<<dirorthon<<", ampl: "<<amplitude<<", pdraw: "<<pdraw<<", omega: "<<omega<<endl;
        }
        pathdraw.clear();
        //cout<<endl;
        ofEndShape();
    }
}

void ofApp::mapWaveLine(vector<ofVec2f> pos, ofVec2f size){
    unsigned chs0=24;
    float * freqsMel = new float[chs0+1];
    vector<vector<float>> bands0=getFreqBandsMel(chs0,3600.f,freqsMel, true);
    vector<vector<float>> bands=prepFreqsVecs(bands0);
    float maxv=0;
    for (unsigned i=0; i<chs0; i++){
        for (unsigned j=0; j<bands[i].size(); j++){
            if (bands[i][j]>maxv){
                maxv=bands[i][j];
            }
        }
    }
    for (unsigned i=0; i<chs0; i++){
        for (unsigned j=0; j<bands[i].size(); j++){
            bands[i][j]/=maxv;
        }
        for (unsigned j=0; j<bands[i].size(); j++){
            cout << bands[i][j] << ", ";
        }
        cout << endl;
    }
    cout << endl;
    unsigned long fieldsize=(unsigned long)(size.x*size.y);
    float * field0=new float[fieldsize*chs0];
    int * field=new int[fieldsize];
    float * field2=new float[fieldsize];

    float interpolationPowerParameter=5.f;
    for (unsigned i=0; i<chs0; i++){
        inverseDistanceInterpolation(bands[i],pos,field0+i*fieldsize,size, interpolationPowerParameter); //power parameter gets calculated from point density! (not anymore!, wont work properly :-/)
    }
    int lineNum=12;
    float mag=0.f, magmax=0.f, magmin=99999999.f;;;
    char val;
    image.allocate(size.x, size.y, OF_IMAGE_COLOR);
    //image.setColor(255);
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            mag=0.f;
            for (unsigned i=0; i<chs0; i++){
                mag+=field0[x+unsigned(size.x)*y+fieldsize*i];
            }
            if (mag<magmin){
                magmin=mag;
            }
            field2[x+unsigned(size.x)*y]=mag;
        }
    }
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            mag=field2[x+unsigned(size.x)*y];
            mag=mag-magmin;
            if (mag>magmax){
                magmax=mag;
            }
            field2[x+unsigned(size.x)*y]=mag;
        }
    }
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            /*find points where the difference to neighbours is smaller than a small threshold,
             * Then write a number at that border. all other points get the number zero. In the next step this line of pixels will be traced by creating a vector of points on that line.
             *(and the average intensity ist equal to discrete values representing the desired number of lines)
            */
            mag=field2[x+unsigned(size.x)*y];
            int valm=int(ofMap(mag, 0.f, magmax, 1, lineNum+1));
            field[x+unsigned(size.x)*y]=valm;
            image.setColor(x,y,ofColor(unsigned((float(valm)/float(lineNum+2))*254.f),unsigned(mag*254.f),0,0));

        }
    }
    delete [] field2;
    int xn, yn, grad, diff, valn, maxval;
    unsigned idx;
    int * field1=new int[fieldsize];
    int * check=new int[fieldsize];
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            idx=x+unsigned(size.x)*y;
            grad=0;
            maxval=0;
            bool found=false;

            for (int xk=-1; xk<=0; xk++){
                for (int yk=-1; yk<=0; yk++){
                    xn=x+xk;
                    yn=y+yk;
                    if (found==false && xn>0 && yn>0 && xn<unsigned(size.x)-1 && yn<unsigned(size.y)-1 && (xk!=0 || yk!=0)){
                        valn=field[unsigned(xn)+unsigned(size.x)*unsigned(yn)];
                        diff=field[idx]-valn;
                        grad+=abs(diff);
                        if (grad>0){
                            if (maxval<valn){
                                maxval=valn;
                            }
                        }

                    }
                }
            }
            if (grad>0){
                val=255;
                field1[idx]=2;
            }
            else{
                val=0;
                field1[x+unsigned(size.x)*y]=0;
            }
            //image.setColor(x,y,ofColor(val,val,val));
        }
    }
   // image.draw(0,0);
    for (int i=0; i<fieldsize; i++){
        check[i]=0;
    }

    int xa, ya, xab, yab, linecolor, idxs, xk, yk;
    bool foundNextPoint;
    const int searchNum=4;
    //int sidxX [8]={-1,-1, 0, 1, 1, 1, 0,-1}; //arrays containing indices for searching pixel neighbourhood clockwise. this is necessary in order not to get to a dead end
    //int sidxY [8]={ 0,-1,-1,-1, 0, 1, 1, 1};
    int sidxX [searchNum]={-1, 0, 1, 0}; //arrays containing indices for searching pixel neighbourhood clockwise. this is necessary in order not to get to a dead end
    int sidxY [searchNum]={ 0,-1, 0, 1};
    vector<vector<ofVec2f>> linePaths;
    vector<bool> closedLoop;
    for (int x=1; x<int(size.x)-1; x++){
        for (int y=1; y<int(size.y)-1; y++){
            //in this loop over all pixels, every time a point in field1 other than zero is encountered, a line goes through this point.
            //if the line has not been followed before (we could have encountered it earlyer) the line gets traced in two steps:
            //1) follow the line in an arbitrary direction until the starting point is found again (loop) or the line ends (open line)
            //2) from that end point / point on loop, we follow the line again until the end is reached. this time the points on the way get recorded in "vector<vector<ofVec2f>> linePaths".
            //every line gets one subvector of linePaths
            unsigned long idx=x+unsigned(size.x)*y;
            int pathPointCount=0;
            if (field1[idx]>0 && check[idx]==0){ //if this is true, a point on a unfollowed line has been found which is not part of any line traced before
                //first this line will be followed until the end is found, or we get back to the starting point
                bool isClosedLoop=false;
                bool foundTaleOrLoop=false;
                check[idx]=-1; //-1 is the marker for the pathfinding to locate the starting point (one end or for identifiying a loop-structure)
                xa=x; //coordinates for following path of same valued array elements. if the path end ins found or the path is a closed loop, the path gets followed back and recorded for later line rendering :-)
                ya=y;
               // image.setColor(xa,ya,ofColor(255,0,255,150));
                xab=x;
                yab=y;
                linecolor=field1[idx]; //set the value of the line to be followed in field1. (this is necessary to avoid getting lost at encounters with another line, aka to stay on the track)
                //in this while loop the End of the pixel-path is found, and if the path is closed, its closedness will be confirmed
                while (foundTaleOrLoop==false){
                    foundNextPoint=false; //set check variable false
                    for (int n=0; n<searchNum; n++){ //iterate through array with neighbouring points; clockwise around current pixel [xa, ya]
                            xk=sidxX[n];
                            yk=sidxY[n];
                            xn=xa+xk;
                            yn=ya+yk;
                            if (xn>0 && yn>0 && xn<int(size.x) && yn<int(size.y) && foundNextPoint==false){ //search the neighbourhood clockwise if we are not at the border of the pixel array
                                idxs=int(xn)+int(size.x)*int(yn); //neighbourhood index
                                valn=field1[idxs]; //value of current neighbourhood pixel
                                if (valn==linecolor && check[idxs]==0){
                                    xa=xn;
                                    ya=yn;
                                    check[idxs]=-1; //mark point as found in path
                                    foundNextPoint=true; //marker set, so we go on. if false the end of line is found or the loop is closed (if isClosedLoop is also true)
                            }
                        }
                    }
                    pathPointCount++;
                   // image.setColor(xa,ya,ofColor(15,250,2,150));
                    if (foundNextPoint==false){
                        foundTaleOrLoop=true;

                    }
                }
                //cout<< "p1: " << pathPointCount << " xa: " << xa << " ya: " << ya <<endl;
                pathPointCount=0;
                vector<ofVec2f> newPath; //add new line vector to lines vector
                bool pathRecorded=false;
                int startX=xa;
                int startY=ya;
                check[idxs=int(xa)+int(size.x)*int(ya)]=linecolor;;
                isClosedLoop==false;
                while (pathRecorded==false){
                    foundNextPoint=false;
                    for (int n=0; n<searchNum; n++){
                            xk=sidxX[n];
                            yk=sidxY[n];
                            xn=xa+xk;
                            yn=ya+yk;
                            if (xn>0 && yn>0 && xn<int(size.x) && yn<int(size.y) && foundNextPoint==false){
                                idxs=int(xn)+int(size.x)*int(yn);
                                valn=field1[idxs];
                                if (valn==linecolor && check[idxs]!=linecolor){
                                    xab=xa;
                                    yab=ya;
                                    xa=xn;
                                    ya=yn;
                                    check[idxs]=linecolor;
                                    foundNextPoint=true;
                                    //break;
                            }
                        }
                    }
                    if (pathPointCount>10 && abs(xa-startX)<4 && (abs(ya-startY)<4)){ //if we find the start point again the loop is closed and we are finished
                        isClosedLoop=true;
                    }
                    if (foundNextPoint==false || isClosedLoop==true){
                        pathRecorded=true;
                    }
                    newPath.push_back(ofVec2f((float(xa)+float(xab))/2.f, (float(ya)+float(yab))/2.f));
                    pathPointCount++;
                    if (pathPointCount>1){
                     //   ofDrawLine(newPath[newPath.size()-1],newPath[newPath.size()-2]);
                    }
                }
                if (isClosedLoop){
                    newPath.push_back(newPath[0]);
                 //   cout<<"closed loop: " << isClosedLoop << endl;
                }
                if (newPath.size()>10){
                    linePaths.push_back(newPath); //add the newly found path to the vector of paths
                    closedLoop.push_back(isClosedLoop);
                }
               // cout<< "p2: " << pathPointCount <<endl;
               // cout<<endl;
             }

        }
    }

    cout << linePaths.size() << endl;

    image.save("map-1.bmp");
    //now, equalize distance between points



    vector<vector<ofVec2f>> linePathsResampled;
    float distBetNewPts=1.5f;
    for (unsigned i=0; i<linePaths.size(); i++) {
        vector<ofVec2f> path=linePaths[i];
        vector<ofVec2f> pathResampled;
        ofVec2f p0, p0b, dir, dirn, pdraw;
        float distBetNewPts=1.f;
        float mindist=distBetNewPts;
        pathResampled.push_back(path[0]);
        cout<<"i: "<< i << endl;
        if (closedLoop[i]){
            for (unsigned p=1; p<path.size(); p++){
                p0b=pathResampled.back();
                p0=path[p];
                dir=p0-p0b;
                dirn=dir;
                dirn.normalize();
                while ((pathResampled.back()-p0b).length()<dir.length()){
                    pdraw=pathResampled.back()+distBetNewPts*dirn;
                    pathResampled.push_back(pdraw);
                }
            }
            /*
            p0b=pathResampled.back();
            p0=pathResampled[0];
            dir=p0-p0b;
            dirn=dir;
            dirn.normalize();
            cout << "p:" << p0b <<", dir:" << dir.x <<", " <<dir.y<<endl;
            while ((pathResampled.back()-p0b).length()<dir.length()){
                pdraw=pathResampled.back()+distBetNewPts*dirn;
                pathResampled.push_back(pdraw);
            }
            */
        }
        else{
            for (unsigned p=1; p<path.size(); p++){
                p0b=pathResampled.back();
                p0=path[p];
                dir=p0-p0b;
                dirn=dir;
                dirn.normalize();
                while ((pathResampled.back()-p0b).length()<dir.length()){
                    pdraw=pathResampled.back()+distBetNewPts*dirn;
                    pathResampled.push_back(pdraw);
                }
           }
        }

        linePathsResampled.push_back(pathResampled);
        linePaths[i].clear();
    }
    linePaths.clear();

    //now! smooth the paths!

    vector<vector<ofVec2f>> linePathsSmooth;
    vector<bool> closedLoopSmooth;
    for (unsigned i=0; i<linePathsResampled.size(); i++) {
        vector<ofVec2f> path=linePathsResampled[i];
        vector<ofVec2f> pathSmooth;
        float slen=15.f;
        if (path.size()>=100/distBetNewPts){
            for (unsigned p=0; p<path.size(); p++){
                //image.setColor(path[p].x,path[p].y,ofColor(0,255,0,0));
                ofVec2f av=path[p];
                float slenn=0.f;
                int s=1;
                while (slenn<slen && p+s<path.size()){
                    av+=path[p+s-1];
                    slenn+=(path[p+s-1]-path[p+s]).length();
                    s++;
                }
                if (s>2){
                    pathSmooth.push_back(av/float(s));
                }

            }

            if (closedLoop[i]){
                pathSmooth.push_back(pathSmooth[0]); //close the loop (smoothing made it open again)
                closedLoopSmooth.push_back(true);
              //  pathSmooth.push_back(pathSmooth[1]);
            }
            else{
                closedLoopSmooth.push_back(false);
            }
            linePaths.push_back(pathSmooth);
        }
    }

    //draw the frequency content distortetd lines of equal loudness. yeah!!!
 //   linePaths=linePathsResampled;
    ofNoFill();
    ofSetLineWidth(.5);
    float period=size.x/20.f;
    float amplitudeMax=size.x/30.f;
    vector<float> randPhaseShift(chs0); //for avoidance of ugly uppsumming of starting sine waves
    vector<ofVec2f> pathdraw;
    for (unsigned i=0; i<linePaths.size(); i++) {
        vector<ofVec2f> path=linePaths[i];
        //ofSetColor((255*i)/linePaths.size(),255,0,255);
        ofSetColor(0,0,0);
        ofVec2f p0, p0b, dir, dirn, dirorthon, pdraw;
        float freq, amplitude, omega;
        float len=0.f;
        for (unsigned f=0; f<chs0; f++){
            randPhaseShift[f]=3.14f*2.f*ofRandomf();
        }
        ofBeginShape();
        unsigned overlap=0;
        if (closedLoopSmooth[i]){
            overlap=period*2;
        }
        cout<<"i: " <<i<<", cl: " << closedLoopSmooth[i]<<endl;
        unsigned p,pb;
        for (unsigned pp=1; pp<path.size()+overlap;pp++){
            if (pp>=path.size()){
                p=pp-path.size();
            }
            else {
                p=pp;
            }
            if ((pp-1)>=path.size()){
                pb=(pp-1)-path.size();
            }
            else {
                pb=pp-1;
            }
            cout << "| p: " << p <<", pb: "<< pb ;
            p0b=path[pb];
            p0=path[p];
            dir=p0-p0b;
            dirn=dir;
            dirorthon=dirn.normalize().rotate(90.f);

            amplitude=0.f;
            omega=10.f*3.1415f*len/period;
            //cout<<"frequency-loop:"<<endl;
            for (unsigned f=0; f<chs0; f++){
                freq=field0[unsigned(p0.x) + unsigned(p0.y)*unsigned(size.x) + fieldsize*f];
                //amplitude+=sin(float(f+1)*omega+randPhaseShift[f])*freq;
                amplitude+=sin(float(f)*3.1415*.000001f*len/period+randPhaseShift[f])*freq;
              //  cout<<", ampl: "<<amplitude<<", freq: "<<freq<<", omega: "<<omega<<endl;
            }

         //   amplitude=(amplitude*amplitudeMax)/float(chs0);
            pdraw=p0+amplitude*dirorthon;

            if (pp>path.size()-1){
                float w=float(p)/float(overlap);
                float wi=1.-w;
                pdraw=pdraw*wi+pathdraw[p]*w;
            }
            ofVertex(pdraw);
            pathdraw.push_back(pdraw);
            ofVertex(p0+dirorthon*5.f*sin(len/period));
            len+=dir.length(); //integrate length of path

            //cout<<"dir: "<<dir<<", dirn: "<<dirn<<", ortho: "<<dirorthon<<", ampl: "<<amplitude<<", pdraw: "<<pdraw<<", omega: "<<omega<<endl;
        }
        pathdraw.clear();
        //cout<<endl;
        ofEndShape();
    }
}

void ofApp::mapRaster(vector<ofVec2f> pos, ofVec2f size){
    unsigned chs0=24;
    float * freqsMel = new float[chs0+1];
    vector<vector<float>> bands0=getFreqBandsMel(chs0,3600.f,freqsMel, true);
    vector<vector<float>> bands=prepFreqsVecs(bands0);
    float maxv=0;

    for (unsigned i=0; i<chs0; i++){
        bands[i]=prepVector(bands[i]);
        //cout << bands[i] << endl;
    }

    cout << endl;
    unsigned long fieldsize=(unsigned long)(size.x*size.y);
    float * field0=new float[fieldsize*chs0];
    int * field=new int[fieldsize];
    float * field2=new float[fieldsize];

    float interpolationPowerParameter=5.f;
    for (unsigned i=0; i<chs0; i++){
        inverseDistanceInterpolation(bands[i],pos,field0+i*fieldsize,size, interpolationPowerParameter); //power parameter gets calculated from point density! (not anymore!, wont work properly :-/)
    }
    float * pointSpec=new float[chs0];
    ofVec2f dpos;
    ofVec2f dsize;
    float rastersizeX=(size.x/60);
    float rastersizeY=rastersizeX;
    unsigned stepX=unsigned(rastersizeX);
    unsigned stepY=unsigned(rastersizeY);
    dsize=ofVec2f(rastersizeX, rastersizeY)*.78f;

    for (unsigned x=0; x<unsigned(size.x); x+=stepX){
        for (unsigned y=0; y<unsigned(size.y); y+=stepY){
            for (unsigned c=0; c<chs0; c++){
                pointSpec[c]=field0[c*fieldsize+x+unsigned(size.x)*y];
            }
            dpos=ofVec2f(float(x), float(y));
            drawGraph4(pointSpec, chs0, dpos, dsize);
           // drawGraph3(pointSpec, chs0, dpos, dsize);
        }
    }

}
vector<vector<float>> ofApp::getFreqBandsNorm(float f1, float f2){
    vector<vector<float>> bs(3);
    vector<float> bsav(3);
    for (unsigned i=0; i<data.size(); i++){
        bs[0].push_back(data[i].audio.getSpectrumAverage(0, globAudio.length, globAudio.minFreq, f1, true));
        bs[1].push_back(data[i].audio.getSpectrumAverage(0, globAudio.length, f1, f2, true));
        bs[2].push_back(data[i].audio.getSpectrumAverage(0, globAudio.length, f2, globAudio.maxFreq, true));
    }
    return bs;
}
vector<vector<vector<float>>> ofApp::getFreqBandsTime(float f1, float f2, unsigned num){
    float tstep=float(globAudio.length)/float(num);
    vector<vector<vector<float>>> bs(3);
    vector<float> bsav(3);
    for (unsigned i=0; i<data.size(); i++){
        float tpos=0.f;
        vector<float> a;
        vector<float> b;
        vector<float> c;
        for (unsigned t=0; t<num; t++){
            a.push_back(data[i].audio.getSpectrumAverage(tpos, tpos+tstep, globAudio.minFreq, f1, true));
            b.push_back(data[i].audio.getSpectrumAverage(tpos, tpos+tstep, f1, f2, true));
            c.push_back(data[i].audio.getSpectrumAverage(tpos, tpos+tstep, f2, globAudio.maxFreq, true));
            tpos+=tstep;
        }
        bs[0].push_back(a);
        bs[1].push_back(b);
        bs[2].push_back(c);
    }
    return bs;
}


float ofApp::compareVec(vector<float> a, vector<float> b){
    unsigned minl=(min(a.size(), b.size()));
    float d=0.f;
    for (unsigned i=0; i<minl; i++){
        d+=((a[i]-b[i])*(a[i]-b[i]));
    }
    return d/float(minl);
}

void ofApp::similarityLines(vector<ofVec2f> pos, ofVec2f size){
    vector<vector<float>> indices0=getIndices();
    vector<vector<float>> bands0=getFreqBandsNorm(250, 1500);
    vector<vector<float>> indices=indices0;
    //vector<vector<float>> bands=bands0;

    vector<vector<float>> bands=prepFreqsVecs(bands0);
    for (unsigned i=0; i<bands.size(); i++){
        bands[i]=prepVector(bands0[i]);
    }

    for (unsigned i=0; i<pos.size(); i++){
        vector<ofVec3f> nrstp0=findNearestPtsf(pos, pos[i], pos.size()-2);
        vector<ofVec3f> nrstp(0);
        for (unsigned p=0; p<nrstp0.size(); p++){
            ofVec2f nrstpn;
            nrstpn.x=nrstp0[p].x; nrstpn.y=nrstp0[p].y;
            float angle=pos[i].angle(nrstpn);
            float dist=pos[i].distance(nrstpn);
            nrstp.push_back(nrstp0[p]);
        }

        vector<float> a(3,0),b(3,0);
        for (unsigned p=0; p<nrstp.size(); p++){
            ofVec2f nrstpn;
            nrstpn.x=nrstp[p].x; nrstpn.y=nrstp[p].y;
            unsigned nrsti=unsigned(nrstp[p].z);
            cout<<nrsti<<endl;
            for (unsigned c=0; c<3; c++){
                a[c]=(bands[c][i]);
                b[c]=(bands[c][nrsti]);
            }
            float diff=compareVec(a,b);
            if (diff<10.0f){
                cout<<diff<<endl;
                ofSetLineWidth(diff*10.f+.2f);
                ofSetColor(0,0,0,100);

                if (pos[i].x>0.f && pos[i].y>0.f && nrstpn.y>0.f && nrstpn.x>0.f){
                    ofDrawLine(pos[i].x, pos[i].y, nrstpn.x, nrstpn.y);
                }
            }
        }
    }
}
void ofApp::similarityLines1(vector<ofVec2f> pos, ofVec2f size){
    vector<vector<float>> indices0=getIndices();
    vector<vector<float>> bands0=getFreqBandsNorm(250, 1500);
    vector<vector<float>> indices=indices0;
    //vector<vector<float>> bands=bands0;

    vector<vector<float>> bands=prepFreqsVecs(bands0);
    for (unsigned i=0; i<bands.size(); i++){
        bands[i]=prepVector(bands0[i]);
    }

    for (unsigned i=0; i<pos.size(); i++){
        vector<ofVec3f> nrstp0=findNearestPtsf(pos, pos[i], pos.size()-2);
        vector<ofVec3f> nrstp(0);


        vector<float> a(7,0),b(7,0);
        vector<float> diffv(pos.size());
        for (unsigned p=0; p<pos.size(); p++){

            for (unsigned c=0; c<3; c++){
                a[c]=(bands[c][i]);
                b[c]=(bands[c][p]);
            }
            for (unsigned c=3; c<7; c++){
                a[c]=(indices[c-3][i]);
                b[c]=(indices[c-3][p]);
            }
            diffv[p]=compareVec(a,b);
        }
        diffv=prepVector(diffv);

        float th=.005;
        for (unsigned p=0; p<pos.size(); p++){
            float diff=diffv[p];

            if (diff<th && i!=p){
                cout<<diff<<endl;
                ofSetLineWidth((th-diff)*(16.f/th)+2.f);
                ofSetColor(int(indices[2][i]*1.9*255),int(indices[3][i]*.2),int((indices[1][i]-1.f)*1.2*255),255);
                if (pos[i].x>0.f && pos[i].y>0.f && pos[p].y>0.f && pos[p].x>0.f){
                    ofDrawLine(pos[i].x, pos[i].y, pos[p].x, pos[p].y);
                }
            }
        }
    }
}



void ofApp::mapRasterSymbol(vector<ofVec2f> pos, ofVec2f size,int symbol){
    vector<vector<float>> indices0=getIndices();
    vector<vector<float>> bands0=getFreqBandsNorm(250, 1500);
    vector<vector<float>> indices=indices0;
    vector<vector<float>> bands=prepFreqsVecs(bands0);
    for (unsigned i=0; i<bands.size(); i++){
        bands[i]=prepVector(bands0[i]);
    }

    cout << endl;
    unsigned long fieldsize=(unsigned long)(size.x*size.y);
    float * fieldInd=new float[fieldsize*indices.size()];
    float * fieldBnd=new float[fieldsize*bands.size()];
    int * field=new int[fieldsize];
    float * field2=new float[fieldsize];

    float interpolationPowerParameter=3.f;
    for (unsigned i=0; i<indices.size(); i++){
        inverseDistanceInterpolation(indices[i],pos,fieldInd+i*fieldsize,size, interpolationPowerParameter); //power parameter gets calculated from point density! (not anymore!, wont work properly :-/)
    }
    for (unsigned i=0; i<bands.size(); i++){
        inverseDistanceInterpolation(bands[i],pos,fieldBnd+i*fieldsize,size, interpolationPowerParameter); //power parameter gets calculated from point density! (not anymore!, wont work properly :-/)
    }

    ofVec2f dpos;
    ofVec2f dsize;
    float rastersizeX=(size.x/30);
    float rastersizeY=rastersizeX;
    unsigned stepX=unsigned(rastersizeX);
    unsigned stepY=unsigned(rastersizeY);
    dsize=ofVec2f(rastersizeX, rastersizeY)*.6f;
    vector<float> bandsnow(bands.size());
    vector<float> indicesnow(indices.size());

    for (unsigned x=0; x<unsigned(size.x); x+=stepX){
        for (unsigned y=0; y<unsigned(size.y); y+=stepY){
            for (unsigned c=0; c<bands.size(); c++){
                bandsnow[c]=fieldBnd[x+unsigned(size.x)*y+c*fieldsize];
            }
            for (unsigned c=0; c<indices.size(); c++){
                indicesnow[c]=fieldInd[x+unsigned(size.x)*y+c*fieldsize];
            }
            dpos=ofVec2f(float(x), float(y));
            //drawSymbolSimple1(dpos, dsize, bandsnow);
            vector<unsigned> ndist=findNearestPts(pos, dpos, 1);
            if (ndist[0]<unsigned(size.x/8)){
                if (symbol==0){
                    drawSymbolSimple1(dpos, dsize, bandsnow);
                }
                if (symbol==1){
                    drawSymbol1(dpos, dsize, bandsnow, indicesnow);
                }
                if (symbol==2){
                    drawSymbol2(dpos, dsize, bandsnow, indicesnow);
                }
            }

        }
    }

    vector<ofVec2f> pos1;

    float radius=size.length()/300.f;
    /*
    for (unsigned i=0; i<pos.size(); i++){
        ofFill();
        ofSetLineWidth(radius/10.f);
        ofSetColor(254);
        //ofDrawCircle(pos[i],radius);
        ofSetColor(0);
        int x=pos[i][0]; int y=pos[i][1];
        for (unsigned c=0; c<bands.size(); c++){
            bandsnow[c]=fieldBnd[x+unsigned(size.x)*y+c*fieldsize];
        }
        for (unsigned c=0; c<indices.size(); c++){
            indicesnow[c]=fieldInd[x+unsigned(size.x)*y+c*fieldsize];
        }
        dpos=ofVec2f(float(x), float(y));
        //drawSymbolSimple1(dpos, dsize, bandsnow);
        drawSymbol1(dpos, dsize, bandsnow, indicesnow);
        //drawSymbol2(dpos, dsize, bandsnow, indicesnow);
        //ofDrawCircle(pos[i],radius+radius/10.f);
    }
    */
}

void ofApp::mapPointsSymbol(vector<ofVec2f> pos, ofVec2f size){
    vector<vector<float>> indices0=getIndices();
    vector<vector<float>> bands0=getFreqBandsNorm(250, 1500);
    vector<vector<float>> indices=indices0;
    vector<vector<float>> bands=prepFreqsVecs(bands0);
    for (unsigned i=0; i<bands.size(); i++){
        bands[i]=prepVector(bands0[i]);
    }

    cout << endl;
    unsigned long fieldsize=(unsigned long)(size.x*size.y);
    float * fieldInd=new float[fieldsize*indices.size()];
    float * fieldBnd=new float[fieldsize*bands.size()];
    int * field=new int[fieldsize];
    float * field2=new float[fieldsize];

    float interpolationPowerParameter=3.f;
    for (unsigned i=0; i<indices.size(); i++){
        inverseDistanceInterpolation(indices[i],pos,fieldInd+i*fieldsize,size, interpolationPowerParameter); //power parameter gets calculated from point density! (not anymore!, wont work properly :-/)
    }
    for (unsigned i=0; i<bands.size(); i++){
        inverseDistanceInterpolation(bands[i],pos,fieldBnd+i*fieldsize,size, interpolationPowerParameter); //power parameter gets calculated from point density! (not anymore!, wont work properly :-/)
    }

    ofVec2f dpos;
    ofVec2f dsize;
    float rastersizeX=(size.x/45);
    float rastersizeY=rastersizeX;
    unsigned stepX=unsigned(rastersizeX);
    unsigned stepY=unsigned(rastersizeY);
    dsize=ofVec2f(rastersizeX, rastersizeY)*.6f;
    vector<float> bandsnow(bands.size());
    vector<float> indicesnow(indices.size());
/*
    for (unsigned x=0; x<unsigned(size.x); x+=stepX){
        for (unsigned y=0; y<unsigned(size.y); y+=stepY){
            for (unsigned c=0; c<bands.size(); c++){
                bandsnow[c]=fieldBnd[x+unsigned(size.x)*y+c*fieldsize];
            }
            for (unsigned c=0; c<indices.size(); c++){
                indicesnow[c]=fieldInd[x+unsigned(size.x)*y+c*fieldsize];
            }
            dpos=ofVec2f(float(x), float(y));
            //drawSymbolSimple1(dpos, dsize, bandsnow);
            vector<unsigned> ndist=findNearestPts(pos, dpos, 1);
            if (ndist[0]<unsigned(size.x/8)){
                drawSymbol1(dpos, dsize, bandsnow, indicesnow);
            }
            //drawSymbol2(dpos, dsize, bandsnow, indicesnow);
        }
    }
*/
    vector<ofVec2f> pos1;

    float radius=size.length()/300.f;

    for (unsigned i=0; i<pos.size(); i++){
        ofFill();
        ofSetLineWidth(radius/10.f);
        ofSetColor(254);
        //ofDrawCircle(pos[i],radius);
        ofSetColor(0);
        int x=pos[i][0]; int y=pos[i][1];
        for (unsigned c=0; c<bands.size(); c++){
            bandsnow[c]=fieldBnd[x+unsigned(size.x)*y+c*fieldsize];
        }
        for (unsigned c=0; c<indices.size(); c++){
            indicesnow[c]=fieldInd[x+unsigned(size.x)*y+c*fieldsize];
        }
        dpos=ofVec2f(float(x), float(y));
        //drawSymbolSimple1(dpos, dsize, bandsnow);
        drawSymbol1(dpos, dsize, bandsnow, indicesnow);
        //drawSymbol2(dpos, dsize, bandsnow, indicesnow);
        //ofDrawCircle(pos[i],radius+radius/10.f);
    }
}

vector<vector<float>> ofApp::getFreqBandsMel(unsigned N, float melmax, float * freqsApp, bool logarithmic){
    vector<vector<float>> bs(N);
    vector<float> bsav(N);
    vector<float> freqs(N+1);
    float minfreq=30.f;
    float melmin=2595.f*log10(1.f+minfreq/700.f);
    for (unsigned c=0; c<N+1; c++){
        float mel=melmin+(melmax-melmin)*float(c)/float(N+1);
        freqs[c]=700.f*(pow(10.f,mel/2595)-1.f);
        freqsApp[c]=(freqs[c]);
    }
    cout<<"Used Frequency range form Drawing the Map: minfreq: " << freqs[0] << " | maxfreq: " << freqs[N];
    cout<<endl;
    for (unsigned i=0; i<data.size(); i++){
        for (unsigned c=0; c<N; c++){
            bs[c].push_back(data[i].audio.getSpectrumAverage(0, globAudio.length, freqs[c], freqs[c+1], logarithmic));
        }
    }
    return bs;
}

void ofApp::drawMap(string path){
    /*
     * lon -> X-Axis [0]
     * lat -> Y-Axis [1]
    */
    image.clear();
    double scale=500000.;
    vector<vector<double>> positions;
    vector<ofVec2f> posVec;
    vector<double> smallest=getSmallest();
    vector<double> biggest=getBiggest();
    double border=(biggest[0]-smallest[0])*scale*0.1;
    double sizeX=(biggest[0]-smallest[0])*scale+border*2.;
    double sizeY=(biggest[1]-smallest[1])*scale+border*2.;
    ofVec2f size=ofVec2f(sizeX, sizeY);
    ofVec2f sizeall=ofVec2f(sizeX,sizeY*2.);
    for (unsigned i=0; i<data.size();i++){
        vector<double> now(2);
        now[0]=data[i].pos.lon;
        now[1]=data[i].pos.lat;
        vector<double> newpos(2);
        newpos[0]=(now[0]-smallest[0])*scale+border;
        newpos[1]=(biggest[1]-smallest[1])*scale-(now[1]-smallest[1])*scale+border;
        positions.push_back(newpos);
        posVec.push_back(ofVec2f(float(newpos[0]),float(newpos[1])));
       // ofLog() << " i: " << i << " pos x: " << positions[i][0] << " pos y: " << positions[i][1] << endl;
    }
/*
    ofBeginSaveScreenAsSVG("simlines_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    similarityLines1(posVec, size);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();

    ofBeginSaveScreenAsSVG("symbol-pos_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    mapPointsSymbol(posVec, size);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();

    ofBeginSaveScreenAsSVG("fcol-pos_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    mapFcol(posVec, size);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();

    ofBeginSaveScreenAsSVG("names-_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    mapDcol(posVec, size);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();

    ofBeginSaveScreenAsSVG("raster0_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    mapRasterSymbol(posVec, size, 0);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();

    ofBeginSaveScreenAsSVG("raster1_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    mapRasterSymbol(posVec, size, 1);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();

    ofBeginSaveScreenAsSVG("raster2_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    mapRasterSymbol(posVec, size, 2);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();

    ofBeginSaveScreenAsSVG("wline-pos_"+path,false, false, ofRectangle(ofVec2f(0.,0.), size));
    ofDisableAntiAliasing();
    ofBackground(255,255);//transparent background
    mapWaveLine(posVec, size);
    setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    ofEndSaveScreenAsSVG();
    */
    //ofBeginSaveScreenAsSVG("diagram-pos_"+path,false, false, ofRectangle(ofVec2f(0.,0.), sizeall));
    //ofBeginSaveScreenAsPDF("diagram-pos_"+path+".pdf",false, false, ofRectangle(ofVec2f(0.,0.), sizeall));
    ofDisableAntiAliasing();
    //ofBackground(0,0);//transparent background
    ofBackground(255);
    //mapWaveLine(posVec, size);
    //diagram(posVec, size, sizeall);
    //paths(posVec, size, sizeall);
    ofVec2f sizeposter(2000.f,2000.f);
    ofBeginSaveScreenAsSVG("diagram-pos_"+path,false, false, ofRectangle(ofVec2f(0.,0.), sizeposter));
    pathsPoster(posVec, sizeposter, sizeposter);
    //setMarkers(border, (biggest[1]-smallest[1])*scale+border, (biggest[0]-smallest[0])*scale+border, border);
    //ofEndSaveScreenAsPDF();
    ofEndSaveScreenAsSVG();

    //map1(positions, size);
    //map3(posVec, size);
    //mapFcol(posVec, size);
    //mapFLine(posVec, size);
    //mapRaster(posVec, size);
    //mapDcol(posVec, size);
    //print marker coordinates for georeferencing

    printf("\nMarker Positioins for Georeferencing:\n"
           "south west lat: %15f\t lon: %15f\nnorth east lat: %15f\t lon: %15f\n"
           "north west lat: %15f\t lon: %15f\nsouth east lat: %15f\t lon: %15f\n"
           "Scaling factor from degrees : %10f\n",
           smallest[0], smallest[1], biggest[0], biggest[1],
           smallest[0], biggest[1], biggest[0], smallest[1], scale);

    logfile     <<endl<<"Marker Positioins for Georeferencing (The outer ends of the 3-pixel lines, pointing to the corner of the map, mark the following positions:) :"<<endl
                <<"south west lat: "<<smallest[0]<<"\t lon: "<<smallest[1]<<"\nnorth east lat: "<<biggest[0]<<"\t lon: "<<biggest[1]<<endl
                <<"north west lat: "<<smallest[0]<<"\t lon: "<<biggest[1]<<"\nsouth east lat: "<<biggest[0]<<"\t lon: "<<smallest[1]<<endl
                <<"Scaling factor from degrees : "<<scale<<endl<<endl;

}

void ofApp::paths(vector<ofVec2f> pos, ofVec2f size, ofVec2f sizeall){
    unsigned points=1200;
    vector<vector<vector<float>>> bandst=getFreqBandsTime(250,1500,points);
    vector<vector<float>> indices0=getIndices();
    vector<vector<float>> bands0=getFreqBandsNorm(250, 1500);
    vector<vector<float>> indices=indices0;
    vector<vector<float>> bands=prepFreqsVecs(bands0);

    for (unsigned i=0; i<bandst.size(); i++){
        float min=bandst[i][0][0];
        float max=bandst[i][0][0];
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                if (min>bandst[i][j][k]){
                    min=bandst[i][j][k];
                }
            }
        }
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                bandst[i][j][k]=bandst[i][j][k]-min;
            }
        }
        max=bandst[i][0][0];
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                if (max<bandst[i][j][k]){
                    max=bandst[i][j][k];
                }
            }
        }
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                bandst[i][j][k]=bandst[i][j][k]/max;
            }
        }
    }
    for (unsigned i=0; i<bands0.size(); i++){
        bands[i]=prepVector(bands0[i]);
    }
    for (unsigned i=0; i<indices0.size(); i++){
        indices[i]=prepVector(indices0[i]);
    }
    cout << endl;

    float radius0=size.length()/200.f;
    vector<float> bandsnow(bands.size());
    vector<float> indicesnow(indices.size());

    for (unsigned i=0; i<pos.size(); i++){

        ofSetColor(0);


        ofVec3f ptav=ofVec3f(bandst[0][i][0],bandst[1][i][0],bandst[2][i][0]);
        ofVec3f ptd=ptav;
        unsigned avlen=10;
        float nx=7, ny=7;
        ofVec2f sizen=size/float(nx);
        ofVec2f posn=ofVec2f(sizen.x*(i%7), sizen.y*int(i/7));
        //ofVec2f posn=pos[i]-sizen/2.f;

        ofSetColor(0);
        ofNoFill();
        ofSetLineWidth(3);
        ofRect(posn.x, posn.y,sizen.x, sizen.y);


        float borf=0.01f;
        float bor=posn.x*borf/2.;
        ofPushMatrix();
        ofSetColor(0);
        ofTranslate(posn+ofVec2f(15,sizen.y-20));
        ofScale(9);
        ofDrawBitmapString(ofToString(i+1),0,0);
        ofPopMatrix();


        ofFill();
        ofSetLineWidth(0);
        ofVec2f av(2,0);
        ofVec2f pt(2,0);
        float cav=0.f;
        for (unsigned t=0; t<points-avlen; t++){
            ofVec3f ptnew=ofVec3f(bandst[0][i][t],bandst[1][i][t],bandst[2][i][t]);
            ptav=ofVec3f(0.);
            for (unsigned m=1; m<avlen; m++){
                ptnew=ofVec3f(bandst[0][i][t+m],bandst[1][i][t+m],bandst[2][i][t+m]);
                ptav=ptnew+ptav;
            }
            ptav=ptav/float(avlen);
            ptd=ptav;
            pt=ofVec2f(posn.x+bor+ptd.x*sizen.x*(1.-borf)*1.15, posn.y+bor+ptd.y*sizen.y*(1.-borf)*1.15);
            av+=pt;
            cav+=ptd.z;

        }
        av/=float(points-avlen);
        cav/=float(points-avlen);
        ofSetLineWidth(3);
        ofSetColor((20.+25.*cav),(20.+225.*cav),(20.+125.*cav), 255);
        ofDrawLine(posn.x+sizen.x/2,posn.y,av.x,av.y);
        ofDrawLine(posn.x+sizen.x/2,posn.y+sizen.y,av.x,av.y);
        ofDrawLine(posn.x,posn.y+sizen.y/2,av.x,av.y);
        ofDrawLine(posn.x+sizen.x,posn.y+sizen.y/2,av.x,av.y);
        ofSetLineWidth(4);
        ofBeginShape();
        for (unsigned t=0; t<points-avlen; t++){

            ofVec3f ptnew=ofVec3f(bandst[0][i][t],bandst[1][i][t],bandst[2][i][t]);
            ptav=ofVec3f(0.);
            for (unsigned m=1; m<avlen; m++){
                ptnew=ofVec3f(bandst[0][i][t+m],bandst[1][i][t+m],bandst[2][i][t+m]);
                ptav=ptnew+ptav;
            }
            ptav=ptav/float(avlen);
            ptd=ptav;

            ofSetColor((20.+25.*ptd.z), (20.+225.*ptd.z),(20.+125.*ptd.z), 100);
            pt=ofVec2f(posn.x+bor+ptd.x*sizen.x*(1.-borf)*1.12, posn.y+bor+ptd.y*sizen.y*(1.-borf)*1.12);

            ofDrawCircle(pt,6.f+(ptd.z)*40.f);
            //ofVertex(pt);
        }
        ofEndShape();

        ofVec2f geopos=ofVec2f(pos[i].x,pos[i].y+size.y);
        ofPushMatrix();

        ofTranslate(geopos);
        ofSetColor(80);

        ofScale(11);
        ofDrawCircle(6,-5,10);
        ofSetColor(255);
        ofDrawBitmapString(ofToString(i+1),0,0);

        ofPopMatrix();

    }
}
void ofApp::pathsPoster(vector<ofVec2f> pos, ofVec2f size, ofVec2f sizeall){
    unsigned points=12000;
    vector<vector<vector<float>>> bandst=getFreqBandsTime(250,1500,points);
    vector<vector<float>> indices0=getIndices();
    vector<vector<float>> bands0=getFreqBandsNorm(250, 1500);
    vector<vector<float>> indices=indices0;
    vector<vector<float>> bands=prepFreqsVecs(bands0);

    for (unsigned i=0; i<bandst.size(); i++){
        float min=bandst[i][0][0];
        float max=bandst[i][0][0];
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                if (min>bandst[i][j][k]){
                    min=bandst[i][j][k];
                }
            }
        }
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                bandst[i][j][k]=bandst[i][j][k]-min;
            }
        }
        max=bandst[i][0][0];
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                if (max<bandst[i][j][k]){
                    max=bandst[i][j][k];
                }
            }
        }
        for (unsigned j=0; j<pos.size(); j++){
            for (unsigned k=0; k<points; k++){
                bandst[i][j][k]=bandst[i][j][k]/max;
            }
        }
    }
    for (unsigned i=0; i<bands0.size(); i++){
        bands[i]=prepVector(bands0[i]);
    }
    for (unsigned i=0; i<indices0.size(); i++){
        indices[i]=prepVector(indices0[i]);
    }
    cout << endl;

    float radius0=size.length()/200.f;
    vector<float> bandsnow(bands.size());
    vector<float> indicesnow(indices.size());

    for (unsigned i=0; i<pos.size(); i++){

        ofSetColor(0);

        ofVec3f ptav=ofVec3f(bandst[0][i][0],bandst[1][i][0],bandst[2][i][0]);
        ofVec3f ptd=ptav;
        unsigned avlen=10;
        float nx=7, ny=7;
        ofVec2f sizen=size/float(nx);
        ofVec2f posn=ofVec2f(sizen.x*(i%7), sizen.y*int(i/7));
        //ofVec2f posn=pos[i]-sizen/2.f;

        ofSetColor(0);
        ofNoFill();
        ofSetLineWidth(3);
        ofRect(posn.x, posn.y,sizen.x, sizen.y);

        float borf=0.01f;
        float bor=posn.x*borf/2.;
        ofPushMatrix();
        ofSetColor(0);
        ofTranslate(posn+ofVec2f(15,sizen.y-20));
        ofScale(9);
      //  ofDrawBitmapString(ofToString(i+1),0,0);
        ofPopMatrix();

        ofFill();
        ofSetLineWidth(0);
        ofVec2f av(2,0);
        ofVec2f pt(2,0);
        float cav=0.f;
        for (unsigned t=0; t<points-avlen; t++){
            ofVec3f ptnew=ofVec3f(bandst[0][i][t],bandst[1][i][t],bandst[2][i][t]);
            ptav=ofVec3f(0.);
            for (unsigned m=1; m<avlen; m++){
                ptnew=ofVec3f(bandst[0][i][t+m],bandst[1][i][t+m],bandst[2][i][t+m]);
                ptav=ptnew+ptav;
            }
            ptav=ptav/float(avlen);
            ptd=ptav;
            pt=ofVec2f(posn.x+bor+ptd.x*sizen.x*(1.-borf)*1.15, posn.y+bor+ptd.y*sizen.y*(1.-borf)*1.15);
            av+=pt;
            cav+=ptd.z;

        }
        av/=float(points-avlen);
        cav/=float(points-avlen);
        ofSetLineWidth(3);
        ofSetColor((20.+25.*cav),(20.+225.*cav),(20.+125.*cav), 255);
        ofDrawLine(posn.x+sizen.x/2,posn.y,av.x,av.y);
        ofDrawLine(posn.x+sizen.x/2,posn.y+sizen.y,av.x,av.y);
        ofDrawLine(posn.x,posn.y+sizen.y/2,av.x,av.y);
        ofDrawLine(posn.x+sizen.x,posn.y+sizen.y/2,av.x,av.y);
        ofSetLineWidth(4);
        ofBeginShape();
        for (unsigned t=0; t<points-avlen; t++){

            ofVec3f ptnew=ofVec3f(bandst[0][i][t],bandst[1][i][t],bandst[2][i][t]);
            ptav=ofVec3f(0.);
            for (unsigned m=1; m<avlen; m++){
                ptnew=ofVec3f(bandst[0][i][t+m],bandst[1][i][t+m],bandst[2][i][t+m]);
                ptav=ptnew+ptav;
            }
            ptav=ptav/float(avlen);
            ptd=ptav;

            ofSetColor((20.+25.*ptd.z), (20.+225.*ptd.z),(20.+125.*ptd.z), 100);
            pt=ofVec2f(posn.x+bor+ptd.x*sizen.x*(1.-borf)*1.12, posn.y+bor+ptd.y*sizen.y*(1.-borf)*1.12);

            ofDrawCircle(pt,6.f+(ptd.z)*40.f);
            //ofVertex(pt);
        }
        ofEndShape();
/*
        ofVec2f geopos=ofVec2f(pos[i].x,pos[i].y+size.y);
        ofPushMatrix();

        ofTranslate(geopos);
        ofSetColor(80);

        ofScale(11);
        ofDrawCircle(6,-5,10);
        ofSetColor(255);
      //  ofDrawBitmapString(ofToString(i+1),0,0);

        ofPopMatrix();
        */
    }
}

void ofApp::diagram(vector<ofVec2f> pos, ofVec2f size, ofVec2f sizeall){
    vector<vector<float>> indices0=getIndices();
    vector<vector<float>> bands0=getFreqBandsNorm(250, 1500);
    vector<vector<float>> indices=indices0;
    vector<vector<float>> bands=prepFreqsVecs(bands0);

    for (unsigned i=0; i<bands0.size(); i++){
        bands[i]=prepVector(bands0[i]);
    }
    for (unsigned i=0; i<indices0.size(); i++){
        indices[i]=prepVector(indices0[i]);
    }
    cout << endl;

    float radius0=size.length()/200.f;
    vector<float> bandsnow(bands.size());
    vector<float> indicesnow(indices.size());

    float border=size.x*.1;


/*
    for (unsigned i=0; i<pos.size(); i++){
        float volume=indices[2][i];
        float frequency=indices[3][i];
        ofVec2f dpos=ofVec2f(.05f*size.x+volume*size.x*.9f, .05f*size.y+ frequency*size.y*.9f);
        vector<unsigned int> nrst=findNearestPts(pos, pos[i], 3);
        //ofScale(30.f);
        ofSetColor(0);
        ofSetLineWidth(1);
        for (unsigned k=0; k<nrst.size(); k++){

            float volume=indices[2][nrst[k]];
            float frequency=indices[3][nrst[k]];
            ofVec2f dposn=ofVec2f(.05f*size.x+volume*size.x*.9f, .05f*size.y+ frequency*size.y*.9f+size.y);
            ofDrawLine(dpos,dposn);
        }
    }
*/

    for (unsigned i=0; i<pos.size(); i++){

        //ofDrawCircle(pos[i],radius);
        ofSetColor(0);
        ofVec2f geopos=pos[i];
        float volume=indices[2][i];
        float frequency=indices[3][i];
        ofVec2f dpos=ofVec2f(border+volume*size.x*.8f, border+ frequency*size.y*.8f+size.y);
        ofVec2f dsize=size/40.f;
        cout<<"freq: "<<frequency<<",\t\tvolume: "<<volume<<endl;
        //drawSymbolSimple1(dpos, dsize, bandsnow);
        vector<float> bandsnow(bands.size());
        vector<float> indicesnow(indices.size());
        for (unsigned j=0; j<bands.size(); j++){
            bandsnow[j]=bands[j][i];
        }
        for (unsigned j=0; j<indices.size(); j++){
            indicesnow[j]=indices[j][i];
        }
        float radius=indices[0][i]*10.f;
        float rad=radius0;

        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(0);
        ofDrawCircle(dpos,rad);
        ofFill();
        ofSetColor(int(bands[0][i]*255.),int(bands[1][i]*255.),int(bands[2][i]*255.));
        ofDrawCircle(dpos,max(dsize.x,dsize.y)*1.1f);
        float * datf=data[i].audio.specfalm;
        float * datt=data[i].audio.spectal;
        //drawGraph3(datf,data[i].audio.specF, pos[i], gsize, globalMaxf);
        //drawGraph3(datt,data[i].audio.specT, pos[i]+ofVec2f(0,gsize.y), gsize, globalMaxt);
        float pi=3.14156f;
        float pih=3.14156f/2.f;
        //ofSetColor(150,255,20);

        ofSetColor(125-int(125-bands[0][i]*255.)/2,255-int(bands[1][i]*255.),255-int(bands[2][i]*255.));
        //drawGraph5(datf, data[i].audio.specF, dpos, dsize/2.f,-pih,2.f*pi-pih);
        drawGraph5(datt, data[i].audio.specT, dpos, dsize*1.5f,-pih,2.f*pi-pih);
       // drawGraph3(pointSpec, chs0, dpos, dsize);
        ofSetLineWidth(2);
        ofDrawLine(dpos, geopos);
        ofSetColor(0);
        ofFill();
        ofDrawCircle(geopos,6.);

        //ofScale(0.5f);
        //ofDrawBitmapString(names[i], dpos+ofVec2f(rad*2.+rad*.2, 0.));
    }
    for (unsigned i=0; i<pos.size(); i++){
        float volume=indices[2][i];
        float frequency=indices[3][i];
        ofVec2f geopos=pos[i];
        ofVec2f dpos=ofVec2f(border+volume*size.x*.8f, border+ frequency*size.y*.8f+size.y);
        //ofSetColor(125-int(125-bands[0][i]*255.)/2,255-int(bands[1][i]*255.),255-int(bands[2][i]*255.));
        ofSetColor(0);
        ofSetLineWidth(4);
        ofDrawLine(dpos, geopos);
        ofSetColor(0);
        ofFill();
        ofDrawCircle(geopos,12.);
    }

}


void ofApp::map1(vector<vector<double>> pos, ofVec2f size){
    for (unsigned i=0; i<data.size();i++){
        ofSetColor(5,100,255);
        ofSetLineWidth(8.);
        ofNoFill();
        float dimension=data[i].audio.getBoxDimensionSpectrogram(0.f, 58.f, 0.f, 22000.f,false);
        dimension=dimension-2.f;
        dimension=dimension*dimension*2.f;
        cout << "dim: " << dimension << endl;
        ofVec2f posnow=ofVec2f(pos[i][0],pos[i][1]);
        ofDrawCircle(posnow, (dimension)*50.+9.);
    }
}

void ofApp::map3(vector<ofVec2f> pos, ofVec2f size){
    vector<float> dimensions_measured=getDimensions_measured(true);
    vector<float> dimensions=prepVector(dimensions_measured);
    float * field=new float[(unsigned long)(size.x*size.y)];
    float interpolationPowerParameter=4.f;
    inverseDistanceInterpolation(dimensions,pos,field,size, interpolationPowerParameter);
    image.allocate(unsigned(size.x),unsigned(size.y),OF_IMAGE_COLOR);
    for (unsigned x=0; x<unsigned(size.x); x++){
        for (unsigned y=0; y<unsigned(size.y); y++){
            //unsigned val=unsigned(field[x+unsigned(size.x)*y]/255.);
            float val=field[x+unsigned(size.x)*y];
            char valr=char(ofMap(val*val*val,0.f,1.f,0,255));
            char valg=char(ofMap(val,0.f,1.f,0,255));
            char valb=char(ofMap(val*val,0.f,1.f,0,255));
            image.setColor(x,y,ofColor(valr,valg,valb));
        }
    }
    delete [] field;
    image.draw(0,0);
    /*
    for (unsigned i=0; i<pos.size(); i++){
        ofDrawBitmapString("e) ellipse(); \n", 520, 140);
        ofSetColor(255);
        ofDrawCircle(pos[i],10.);
        ofSetColor(0);
        ofDrawCircle(pos[i],7.);
        //cout << " p: " << nrst[i] << " d: " << ofVec2f(3000,3000).distance(pos[nrst[i]]) << " pt: " << pos[nrst[i]] <<endl;
    }
    */
    ofPushMatrix();
    ofSetColor(5,10,230);
   // ofScale(15.,15.);
    for (unsigned i=0; i<pos.size(); i++){
        cout << dimensions_measured[i] << endl;
        ofDrawBitmapString(ofToString(dimensions_measured[i]), pos[i]);
    }
    ofPopMatrix();
}

vector<vector<float>> ofApp::getFreqBands(float f1, float f2, bool logarithmic){
    vector<vector<float>> bs(3);
    vector<float> bsav(3);
    for (unsigned i=0; i<data.size(); i++){
        bs[0].push_back(data[i].audio.getSpectrumAverage(0, globAudio.length, globAudio.minFreq, f1, logarithmic));
        bs[1].push_back(data[i].audio.getSpectrumAverage(0, globAudio.length, f1, f2, logarithmic));
        bs[2].push_back(data[i].audio.getSpectrumAverage(0, globAudio.length, f2, globAudio.maxFreq, logarithmic));
    }
    return bs;
}
vector<vector<float>> ofApp::getIndices(){
    vector<vector<float>> bs(5);
    for (unsigned i=0; i<data.size(); i++){
        bs[0].push_back(data[i].audio.getDimF(true));
        bs[1].push_back(data[i].audio.getDimT(true));
        bs[2].push_back(data[i].audio.getVolume());
        bs[3].push_back(data[i].audio.getCenterFreq());
        bs[4].push_back(data[i].audio.getDimFm());
    }
    cout << "\t\t\tfreqDim" << ", \t " << "timeDim"<< ", \t " << "Volume"<<", \t " << "CenterFreq"<<", \t " << "FreqMax-Dim"<<endl;
    for (unsigned i=0; i<data.size(); i++){
        cout << names[i] << ":\t " << bs[0][i] << ", \t " << bs[1][i] << ", \t " << bs[2][i]<< ", \t " << bs[3][i] << ", \t\t " << bs[4][i] <<endl;
    }
    cout << endl;
    return bs;
}
vector<float> ofApp::getDimensions_measured(bool logarithmic){
    vector<float> dimensions(data.size());
    for (unsigned i=0; i<data.size(); i++){
        dimensions[i]=data[i].audio.getBoxDimensionSpectrogram(0.f, 58.f, 0.f, 22000.f,logarithmic);
    }
    return dimensions;
}
vector<vector<float>> ofApp::prepFreqsVecs(vector<vector<float>> in){
    vector<vector<float>> out(in.size());
    float maxv=in[0][0];
    for (unsigned i=0; i<in.size(); i++){
        for (unsigned j=0; j<in[i].size(); j++){
            if (maxv<in[i][j]){
                maxv=in[i][j];
            }
        }
    }
    for (unsigned i=0; i<in.size(); i++){
    //    cout << endl;
        for (unsigned j=0; j<in[i].size(); j++){
            out[i].push_back(in[i][j]/maxv);
           // cout << in[i][j]/maxv;
        }
    }
    return out;
}
vector<float> ofApp::prepVector(vector<float> in){
    vector<float> out(in.size());
    float minv=in[0];

    for (unsigned i=0; i<in.size(); i++){
        out[i]=in[i];
        if (minv>out[i]){
            minv=out[i];
        }
    }
    for (unsigned i=0; i<in.size(); i++){
        out[i]=out[i]-minv;
    }

    float maxv=out[0];
    for (unsigned i=1; i<in.size(); i++){
        if (maxv<out[i]){
            maxv=out[i];
        }
    }
    for (unsigned i=0; i<in.size(); i++){
        out[i]=out[i]/maxv;
    }
    return out;
}

vector<float> ofApp::prepDimensions(vector<float> dims_real){
    vector<float> dimensions(data.size());
    float mindim=50.f;
    for (unsigned i=0; i<data.size(); i++){
        float dimension=dims_real[i];
        dimension=dimension-2.f;
        if (dimension<0.f){
            dimension=0.f;
        }
        //dimension=dimension*dimension*2.f;
        dimensions[i]=dimension;
        if (mindim>dimensions[i]){
            mindim=dimensions[i];
        }
    }
    for (unsigned i=0; i<data.size(); i++){
        dimensions[i]=dimensions[i]-mindim;
    }
    float maxdim=dimensions[0];
    for (unsigned i=1; i<data.size(); i++){
        if (maxdim<dimensions[i]){
            maxdim=dimensions[i];
        }
    }
    for (unsigned i=0; i<data.size(); i++){
        dimensions[i]=dimensions[i]/maxdim;
    }
    return dimensions;
}

void ofApp::setMarkers(double mminx, double mminy, double mmaxx, double mmaxy){
    //sets markers for georeferencing the soundmap onto a geographic map

    float length=20.f;
    ofSetLineWidth(2.);
    ofSetColor(0);
    ofDrawLine(mminx, mminy, mminx+length, mminy-length);
    ofDrawLine(mmaxx, mmaxy, mmaxx-length, mmaxy+length);
    ofDrawLine(mminx, mmaxy, mminx+length, mmaxy+length);
    ofDrawLine(mmaxx, mminy, mmaxx-length, mminy-length);
    ofSetLineWidth(1.);
    ofSetColor(255); //making sure the marker can be found in every environment
    ofDrawLine(mminx, mminy, mminx+length, mminy-length);
    ofDrawLine(mmaxx, mmaxy, mmaxx-length, mmaxy+length);
    ofDrawLine(mminx, mmaxy, mminx+length, mmaxy+length);
    ofDrawLine(mmaxx, mminy, mmaxx-length, mminy-length);
}

void ofApp::inverseDistanceInterpolation(vector<float> vals, vector<ofVec2f> pos, float *field, ofVec2f size, float powerParameter){
    /*Using Shepard's Method as described on Wikipedia:
     *https://en.wikipedia.org/wiki/Inverse_distance_weighting
     *scaling of power parameter was added by robert oetting to improve accuracy in uneven distributed fields
     */
    unsigned numP=pos.size();
    unsigned numPpow=3;
    vector<unsigned> nrst(numP,0);
    vector<unsigned> nrstp(numPpow,0);
    vector<float> dists(numP,0.f);
    float sum1;
    float sum2;
    float wi;
    bool onedistzero;
    unsigned zeroindex=0;
    float value;
    float power=powerParameter;
    for (unsigned x=0; x<unsigned(size.x);x++){
        for (unsigned y=0; y<unsigned(size.y);y++){
            ofVec2f pt=ofVec2f(x,y);
            nrst=findNearestPts(pos,pt,numP);
            /*
            nrstp=findNearestPts(pos,pt,numPpow);
            float avdist=0.f;

            for (unsigned n=0; n<numPpow; n++){
                avdist+=pt.distance(pos[nrstp[n]]);
            }
            avdist=avdist/float(numPpow);
            */
            onedistzero=false;
            //float avdistall=0.f;
            for (unsigned n=0; n<numP; n++){
                dists[n]=pt.distance(pos[nrst[n]]);
              //  avdistall+=dists[n];
                if (dists[n]==0.f){
                    onedistzero=true;
                    zeroindex=n;
                    break;
                }
            }
            /*
            avdistall=avdistall/float(numP);
            float distfraction=avdist/avdistall;
            power=(ofMap(distfraction,0.0f,.5f,2.f,5.f)); //scale power factor for different average positions to nearest numPpow point in relation to over all average distance. this should improve interpolation accuracy. this is added to the algorithm by robert oettin.
            */
            if (onedistzero){
                value=vals[nrst[zeroindex]];
            }
            else{
                sum1=0.f;
                sum2=0.f;
                for (unsigned i=0; i<numP; i++){
                    wi=(1.f/pow(dists[i],power));
                    sum1+=wi*vals[nrst[i]];
                    sum2+=wi;
                }
                value=sum1/sum2;
            }
            field[x+unsigned(size.x)*y]=value;
        }
    }
}

vector<double> ofApp::getSmallest(){
    vector<double> pos(2);
    pos[0]=data[0].pos.lon;
    pos[1]=data[0].pos.lat;
    for (unsigned i=1; i<data.size();i++){
        if(data[i].pos.lon<pos[0]){
            pos[0]=data[i].pos.lon;
        }
        if(data[i].pos.lat<pos[1]){
            pos[1]=data[i].pos.lat;
        }
    }
    return pos;
}

vector<double> ofApp::getBiggest(){
    vector<double> pos(2);
    pos[0]=data[0].pos.lon;
    pos[1]=data[0].pos.lat;
    for (unsigned i=1; i<data.size();i++){
        if(data[i].pos.lon>pos[0]){
            pos[0]=data[i].pos.lon;
        }
        if(data[i].pos.lat>pos[1]){
            pos[1]=data[i].pos.lat;
        }
    }
    return pos;
}

vector<unsigned> ofApp::findNearestPts(vector<ofVec2f> pts, ofVec2f pt, unsigned num){
    vector<unsigned> nrst(num,0);
    vector<float> dists(num,0);
    for (unsigned i=0; i<num;i++){
        dists[i]=pt.distance(pts[i]);
        nrst[i]=i;
    }
    for (unsigned n=0; n<num; n++){
    for (unsigned i=num; i<pts.size(); i++){
        float dist=pt.distance(pts[i]);
        bool notinlist=true;
        for (unsigned k=0; k<n; k++){
            if (i==nrst[k]){
                notinlist=false;
            }
        }
        if (dist<dists[n] && notinlist){
            nrst[n]=i;
            dists[n]=dist;
        }
    }
    }
    return nrst;
}

vector<ofVec3f> ofApp::findNearestPtsf(vector<ofVec2f> pts, ofVec2f pt, unsigned num){
    vector<float> nrst(num,0);
    vector<float> dists(num,0);
    vector<ofVec3f> nrstp(num,ofVec3f(0,0,0));
    for (unsigned i=0; i<num;i++){
        dists[i]=pt.distance(pts[i]);
        nrst[i]=i;
    }
    for (unsigned n=0; n<num; n++){
    for (unsigned i=num; i<pts.size(); i++){
        float dist=pt.distance(pts[i]);
        bool notinlist=true;
        for (unsigned k=0; k<n; k++){
            if (i==nrst[k]){
                notinlist=false;
            }
        }
        if (dist<dists[n] && notinlist){
            nrst[n]=i;
            dists[n]=dist;
            nrstp[n].x=pts[i].x;
            nrstp[n].y=pts[i].y;
            nrstp[n].z=float(i);
        }
    }
    }
    return nrstp;
}
void ofApp::mapData01(float *p, unsigned long size){
    float minval=p[0];
    for (unsigned long i=1; i<size; i++){
        if (minval>p[i]){
            minval=p[i];
        }
    }
    for (unsigned long i=0; i<size; i++){
        p[i]=p[i]-minval;
    }
    float maxval=p[0];
    for (unsigned long i=1; i<size; i++){
        if (maxval<p[i]){
            maxval=p[i];
        }
    }
    for (unsigned long i=0; i<size; i++){
        p[i]/=maxval;
    }
}

void ofApp::combineData(string wavFileFolder, string gpsCsvFilePath, float startTimeSeconds, float analysisLenSeconds, float dbAtMax){
    data.clear();
    GpsPosition gpsPos=GpsPosition(gpsCsvFilePath);
    DataRead fileReaderWav=DataRead(wavFileFolder);

    vector<std::string> wavFileList;
    vector<unsigned long> wavFileCreationTimeList;
    wavFileList = fileReaderWav.returnFileList(); //returns a vector of strings containing all wav-files in specified folder
    wavFileCreationTimeList = fileReaderWav.returnFileCreationTimeList(); //returns a vector of ints contianing the creation times of the files since midnight in seconds (data read from filename, not os-creation time data)

    //fileReaderWav.print_wavFiles();
    /*read the audio files and crate a audio analysis Object
    for every recording. position and time data get extracted from the csv gps-logfile and filenames (recording time). each set contains
    one instance of the audio analysis class, a struct for position data, and a struct
    for recording time data. additinal data could be textual descriptions of the soundscapes.
    those tags could be compared for equality and counted, or viewed on the map.
    */

    std::string file;
    int dataSetCounter=0;
    int creationTimeErrorCount=0;
    int fileLengthErrorCount=0;
    int loadErrorCount=0;
    int noGPSDataSetCount=0;
    float globalSpecMax=0.f;
    float globalSpecLMax=0.f;
    float globalSpecfaMax=0.f;
    float globalSpecfalMax=0.f;
    float globalSpecfalMaxMax=0.f;
    float globalSpectaMax=0.f;
    float globalSpectalMax=0.f;
    logfile << "Audio and GPS Data" << endl << endl;
    for (unsigned i=0; i<wavFileList.size(); i++){
      /*In this loop,
       *
      */
        file.clear();
        file.insert(0,wavFileFolder);
        file.insert(wavFileFolder.size(),wavFileList[i]); //current file path
        ofxAudioFile audiofile;
        audiofile.setVerbose(true);
        ofSetLogLevel(OF_LOG_VERBOSE);
        cout << wavFileList[i] << endl;
        if( ofFile::doesFileExist( file ) ){
            cout << endl;
            audiofile.load( file );
            unsigned long startPosition=unsigned(float(audiofile.samplerate())*startTimeSeconds);
            unsigned long analysisLengthSamples=unsigned(analysisLenSeconds*float(audiofile.samplerate()));
            if (!audiofile.loaded()){
                ofLogError()<<"error loading file, double check the file path:";
                cout << "error loading file number " << i << ", double check the file path: " << file << endl;
                logfile << "error loading file number " << i << ", double check the file path: " << file << endl;
                cout << endl;
                loadErrorCount++;
            }
            else if (analysisLengthSamples+startPosition>audiofile.length()) {
                ofLog()<<"problem: File " << wavFileList[i] << " is to short for required analysis length of " << analysisLenSeconds << " seconds, and delay of " << startTimeSeconds << " secons! File will not be loaded!!";
                logfile<<"problem: File " << wavFileList[i] << " is to short for required analysis length of " << analysisLenSeconds << " seconds, and delay of " << startTimeSeconds << " secons! File will not be loaded!!";
                fileLengthErrorCount++;
                cout << endl;
            }
            else if (wavFileCreationTimeList[i]==0){
                ofLog()<<"problem: Time data in file name " << wavFileList[i] << " could not be read. check file name! secons! File will not be loaded!!";
                logfile<<"problem: Time data in file name " << wavFileList[i] << " could not be read. check file name! secons! File will not be loaded!!";
                creationTimeErrorCount++;
                cout << endl;
            }
            else  {
         //     newSet.data=audioAna2(audiofile.data(), audiofile.samplerate(), audiofile.length(), audiofile.channels());
         //     data.push_back(newSet);
                  cout << "______file number: " << i << "______file name: " << wavFileList[i] << " ______" << endl;
                rectime t;
                recpos p;
                dataSet dataNow;

                t.length=float(audiofile.length())/float(audiofile.samplerate());
             //   cout << "creation time: " << wavFileCreationTimeList[i] << endl;
                gpsPos.calcValuesEndTime(wavFileCreationTimeList[i],t.length, 2); // the "end-time" function is used, because the redord times point to the end of the recordings. (modification time of file)
                p.lat=gpsPos.getLat();
                p.lon=gpsPos.getLon();
                p.elevation=gpsPos.getElevation();
                t.year=gpsPos.getJear();
                t.month=gpsPos.getMonth();
                t.day=gpsPos.getDay();
                t.hour=gpsPos.getHour();
                t.minute=gpsPos.getMin();
                t.seconds=gpsPos.getSec();
                cout << p.lon << ", " << p.lat << ", " << p.elevation << ", " << t.day << " hour: " << t.hour << ", min: " << t.minute <<", sec: " << t.seconds << " FileTime: " << wavFileCreationTimeList[i] << ", Available Data Sets: " << gpsPos.getNumAviableDataSets() << endl;
                logfile << p.lon << ", " << p.lat << ", " << p.elevation << ", " << t.day << " hour: " << t.hour << ", min: " << t.minute <<", sec: " << t.seconds << " FileTime: " << wavFileCreationTimeList[i] << ", Available Data Sets: " << gpsPos.getNumAviableDataSets() << endl;
                audioAna2 a=audioAna2(audiofile.data(), 2048, 2048/2, audiofile.samplerate(), audiofile.length(), audiofile.channels(),
                                        analysisLengthSamples, startPosition, dbAtMax);

                dataNow.audio=a;
                dataNow.time=t;
                dataNow.pos=p;

                if (gpsPos.getNumAviableDataSets()>0){
                    data.push_back(dataNow);
                    names.push_back(wavFileList[i]);
                    if (globalSpecMax<a.maxSpecoValue){
                        globalSpecMax=a.maxSpecoValue;
                    }
                    if (globalSpecLMax<a.maxSpecValue){
                        globalSpecLMax=a.maxSpecValue;
                    }
                    if (globalSpecfaMax<a.maxSpecfa){
                        globalSpecfaMax=a.maxSpecfa;
                    }
                    if (globalSpecfalMax<a.maxSpecfal){
                        globalSpecfalMax=a.maxSpecfal;
                    }
                    if (globalSpecfalMaxMax<a.maxSpecfalm){
                        globalSpecfalMaxMax=a.maxSpecfalm;
                    }
                    if (globalSpectaMax<a.maxSpecta){
                        globalSpectaMax=a.maxSpecta;
                    }
                    if (globalSpectalMax<a.maxSpectal){
                        globalSpectalMax=a.maxSpectal;
                    }
                    cout << "__________________" << endl;
                    dataSetCounter++;
                }
                else{
                    noGPSDataSetCount++;
                }

            }
            audiofile.free();

        }
        else{
            ofLogError()<<"input file does not exists. path:";
            cout << file << endl;
        }

      }

      if (dataSetCounter>0){
          //set the global audio variables
          globAudio.maxFreq=data[0].audio.getMaxFrequency();
          globAudio.minFreq=data[0].audio.getMinFrequency();
          globAudio.freqStep=data[0].audio.getStepFrequency();
          //global maximum values for normalizing
          globAudio.maxSpecVal=globalSpecMax;
          globAudio.maxSpecLVal=globalSpecLMax;
          globAudio.maxSpecta=globalSpectaMax;
          globAudio.maxSpectal=globalSpectalMax;
          globAudio.maxSpecfalm=globalSpecfalMaxMax;
          globAudio.maxSpecfa=globalSpecfaMax;
          globAudio.maxSpecfal=globalSpecfalMax;
          globAudio.maxDB=dbAtMax;
      }
      if (dataSetCounter==wavFileList.size()){
          cout << "All " << dataSetCounter << " wav-Files in Folder could be loaded for analysis and map creation!" << endl;
      }
      else {
          if (loadErrorCount>0){
              ofLog()<< loadErrorCount << " file/s could not be loaded for analysis. check file paths and validity of files!";
              logfile<< loadErrorCount << " file/s could not be loaded for analysis. check file paths and validity of files!";
          }
          if (creationTimeErrorCount>0){
              ofLog()<< creationTimeErrorCount << " file/s could not be loaded because they contain no valid time data in filename. Check filename!";
              logfile<< creationTimeErrorCount << " file/s could not be loaded because they contain no valid time data in filename. Check filename!";
          }
          if (fileLengthErrorCount>0){
              ofLog()<< fileLengthErrorCount << " file/s could not be loaded because they are to short for required analysis length! Adjust analysis-lenght or live with it!";
              logfile<< fileLengthErrorCount << " file/s could not be loaded because they are to short for required analysis length! Adjust analysis-lenght or live with it!";
          }
          if (noGPSDataSetCount>0){
              ofLog()<< noGPSDataSetCount << " file/s could not be loaded because no GPS-Data-Set exists for that time period!";
              logfile<< noGPSDataSetCount << " file/s could not be loaded because no GPS-Data-Set exists for that time period!";
          }
      }
      cout << dataSetCounter << " wav-file/s out of "<< wavFileList.size() << " wav-files in folder <<" << wavFileFolder << ">> loaded for analysis!" << endl;
      logfile << dataSetCounter << " wav-file/s out of "<< wavFileList.size() << " wav-files in folder <<" << wavFileFolder << ">> loaded for analysis!" << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
