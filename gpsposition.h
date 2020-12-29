#pragma once

#include "ofMain.h"
#include "ofxCsv.h"

#ifndef GPSPOSITION_H
#define GPSPOSITION_H

class GpsPosition{
    ofxCsv csv;
    double lat;
    double lon;
    double elevation;
    int jear;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    unsigned index;
    unsigned numberOfAvailableDataSets;
    std::vector<unsigned long> times;
    std::vector<double> lats;
    std::vector<double> lons;
    std::vector<double> elevations;

public:
    GpsPosition(std::string path);
    void extractFile();
    void calcValuesStartTime(unsigned long rt, unsigned long len, int correctionHours);
    void calcValuesEndTime(unsigned long rt, unsigned long len, int correctionHours);
    double getLat(){
        return lat;
    }
    double getLon(){
        return lon;
    }
    double getElevation(){
        return  elevation;
    }
    int getJear(){
        if (index>0){
            return atoi(csv[index][0].substr(0,4).c_str());
        }
        else{
            return 0;
        }
    }
    int getMonth(){
        if (index>0){
            return atoi(csv[index][0].substr(5,2).c_str());
        }
        else{
            return 0;
        }
    }
    int getDay(){
        if (index>0){
            return atoi(csv[index][0].substr(8,2).c_str());
        }
        else{
            return 0;
        }
    }
    int getHour(){
        if (index>0){
            return atoi(csv[index][0].substr(11,2).c_str());
        }
        else{
            return 0;
        }
    }
    int getMin(){
        if (index>0){
            return atoi(csv[index][0].substr(14,2).c_str());
        }
        else{
            return 0;
        }
    }
    int getSec(){
        if (index>0){
            return atoi(csv[index][0].substr(17,2).c_str());
        }
        else{
            return 0;
        }
    }
    unsigned long getNumAviableDataSets(){
        if (index>0){
            return numberOfAvailableDataSets;
        }
        else{
            return 0;
        }
    }
};

#endif // GPSPOSITION_H
