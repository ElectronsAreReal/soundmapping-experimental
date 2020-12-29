#pragma once

#include "ofMain.h"
#include "audioana2.h"
#include "dataread.h"
#include "gpsposition.h"
#include <vector>
#include <string>

using namespace std;

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

        void keyPressed(int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);
        void mouseEntered(int x, int y);
        void mouseExited(int x, int y);
        void windowResized(int w, int h);
        void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        struct rectime{
            int day;
            int month;
            int year;
            int hour;
            int minute;
            float seconds;
            float length;
        };
        struct recpos{
            double lon;
            double lat;
            double elevation;
        };

        struct globalAudio{
            float length;
            float minFreq;
            float maxFreq;
            float freqStep;
            float maxSpecVal;
            float maxSpecLVal;
            float maxSpecfa;
            float maxSpecfal;
            float maxSpecfalm;
            float maxSpecta;
            float maxSpectal;
            float maxDB;
        };
        //struct for the main dataset, containing all audiodata and corresponding positions
        struct dataSet {
            audioAna2 audio;
            rectime time;
            recpos pos;
        };
        //vector containing all the audio and position data
        vector<dataSet> data;
        //global data, data that applies for all recordings
        globalAudio globAudio;
        //vector containing the audio-filenames
        vector<std::string> names;
        //fills the data-vector with data from all audio-files that meet the criteria: has a position (is recorded in the time interval the gps-data covers), is long enough (a global lenght is set in setup())
        // all audio-data is cut to the same length. all audio-data in the data-vector has the same length.
        void combineData(string wavFileFolder, string gpsCsvFilePath, float startTimeSeconds, float analysisLenSeconds, float dbAtMax);
        void saveTexture(string filepath);
        void drawMap(string path);
        void drawPoster(string path);
        void drawPoster1(string path);
        void drawWave(unsigned i, float x, float y, float xs, float ys);
        void drawSymbol(unsigned i, float x, float y, float xs, float ys);

        vector<vector<float>> prepFreqsVecs(vector<vector<float>> dims_real);
        vector<float> prepVector(vector<float> dims_real);
        vector<float> prepDimensions(vector<float> dims_real);
        vector<vector<float>> getFreqBands(float f1, float f2, bool logarithmic);
        vector<vector<float>> getIndices();
        vector<vector<float>> getFreqBandsMel(unsigned N, float melmax, float * freqsApp, bool logarithmic);
        vector<vector<float>> getFreqBandsNorm(float f1, float f2);
        vector<vector<vector<float>>> getFreqBandsTime(float f1, float f2, unsigned num);
        vector<float> getDimensions_measured(bool logarithmic);
        //different maps
        void map1(vector<vector<double>> pos, ofVec2f size);
        void map2(vector<ofVec2f> pos, ofVec2f size);
        void map3(vector<ofVec2f> pos, ofVec2f size);
        void mapFcol(vector<ofVec2f> pos, ofVec2f size);
        void mapDcol(vector<ofVec2f> pos, ofVec2f size);
        void mapLine(vector<ofVec2f> pos, ofVec2f size);
        void mapWaveLine(vector<ofVec2f> pos, ofVec2f size);
        void mapFLine(vector<ofVec2f> pos, ofVec2f size);
        void mapRaster(vector<ofVec2f> pos, ofVec2f size);
        void mapRasterSymbol(vector<ofVec2f> pos, ofVec2f size, int symbol);
        void mapPointsSymbol(vector<ofVec2f> pos, ofVec2f size);
        void diagram(vector<ofVec2f> pos, ofVec2f size, ofVec2f sizeall);
        void paths(vector<ofVec2f> pos, ofVec2f size, ofVec2f sizeall);
        void pathsPoster(vector<ofVec2f> pos, ofVec2f size, ofVec2f sizeall);
        void similarityLines(vector<ofVec2f> pos, ofVec2f size);
        void similarityLines1(vector<ofVec2f> pos, ofVec2f size);
        //symbols and diagrams
        void drawGraph(float* data, unsigned dl, ofVec2f pos, ofVec2f size);
        void drawGraph2(float* data, unsigned dl, ofVec2f pos, ofVec2f size);
        void drawGraph3(float* data, unsigned dl, ofVec2f pos, ofVec2f size, float maxValue);
        void drawGraph4(float* data, unsigned dl, ofVec2f pos, ofVec2f size);
        void drawGraph5(float* data, unsigned dl, ofVec2f pos, ofVec2f size, float sta, float sto);
        float triwave(float x);
        void drawSymbolSimple1(ofVec2f pos, ofVec2f size, vector<float> bands);
        void drawSymbol1(ofVec2f pos, ofVec2f size, vector<float> bands, vector<float> indices);
        void drawSymbol2(ofVec2f pos, ofVec2f size, vector<float> bands, vector<float> indices);
        //utility functions. interpolation
        void inverseDistanceInterpolation(vector<float> vals, vector<ofVec2f> pos, float * field, ofVec2f size, float powerParameter);
        void inverseDistanceInterpolationVector(vector<vector<float>> vals, vector<ofVec2f> pos, float * field, ofVec2f size, float powerParameter);
        //find nearest pont to given point
        vector<unsigned> findNearestPts(vector<ofVec2f> pts, ofVec2f pt, unsigned num);
        vector<ofVec3f> findNearestPtsf(vector<ofVec2f> pts, ofVec2f pt, unsigned num);
        //compare vectors by calculating distance
        float compareVec(vector<float> a, vector<float> b);
        //doesnt work
        bool isBetween(vector<ofVec2f> pos, vector<unsigned> inds, ofVec2f pt);
        void mapData01(float * p, unsigned long size);
        //sets markers for georeferencing
        void setMarkers(double mminx, double mminy, double mmaxx, double mmaxy);
        vector<double> getSmallest();
        vector<double> getBiggest();

        ofImage image;

        ofTrueTypeFont	verdana12;
        ofTrueTypeFont	verdana32;

        fstream logfile;

        ofstream fileData;
};
