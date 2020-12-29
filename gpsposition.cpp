#include "gpsposition.h"
#include "ofApp.h"

GpsPosition::GpsPosition(std::string path){
    if (csv.load(path.c_str())){
        ofLog() << "csv file loaded!";
    }
    else {
        ofLogError() << "error loading csv file!";
    }
    extractFile();
}

void GpsPosition::extractFile(){
    for (unsigned i=1; i<csv.getNumRows(); i++){
        times.push_back(atoi(csv[i][0].substr(11,2).c_str()) * 3600 + //hours
                        atoi(csv[i][0].substr(14,2).c_str()) * 60 + //minutes
                        atoi(csv[i][0].substr(17,2).c_str())); //seconds
        lats.push_back(std::stod(csv[i][1].c_str()));
        lons.push_back(std::stod(csv[i][2].c_str()));
        elevations.push_back(std::stod(csv[i][3].c_str()));
    }
    ofLog()<< "Number of GPS-datasets: " << times.size() - 1 << " " << times[0] << endl;
    ofLog()<<"csv-file extracted!";
}

void GpsPosition::calcValuesStartTime(unsigned long rt, unsigned long len, int corH){
    /* This function taked the recording time from file name in seconds sinc midnight, the length of the recording, and the correctin time between gps-time and local time in hours.
     * Then it calculates the average position in this period [rt, rt+len] using data extracted from the gps-file and stores those averages in the lat, lon and elevation variables.
    */
    lat = 0.;
    lon = 0.;
    elevation = 0.;
    double latav=0.;
    double lonav=0.;
    double eleav=0.;
    bool through=false;
    unsigned indexU=0;
    unsigned indexO=0;
    unsigned i=1;
    unsigned cnt=0;
    unsigned corS=3600*corH;
    unsigned long timecorr;
   // std::cout << times[0]+corS << " " << rt << endl;
   // std::cout << int(rt>timecorr) << endl;
    while (!through && i<times.size() && rt>=times[0]+corS){
        timecorr=times[i]+corS;
        if (timecorr>=rt+len){
            through=true;
            indexO=i;
            break;
        }
        if (timecorr>=rt){
            if (cnt==0){
                indexU=i;
            }
            latav+=lats[i];
            lonav+=lons[i];
            eleav+=elevations[i];
            cnt++;
        }

        i++;
    }

    if (!through) {
        numberOfAvailableDataSets=0;
        index=0;
        ofLog()<<"error extracting position from csv file: no time data in filename?";
    }
    else{
        index=indexU;
        numberOfAvailableDataSets=cnt;
        lat=latav/double(cnt);
        lon=lonav/double(cnt);
        elevation=eleav/double(cnt);
    }

}

void GpsPosition::calcValuesEndTime(unsigned long rt, unsigned long len, int corH){
    /* This function taked the recording time from file name in seconds sinc midnight, the length of the recording, and the correctin time between gps-time and local time in hours.
     * Then it calculates the average position in this period [rt, rt+len] using data extracted from the gps-file and stores those averages in the lat, lon and elevation variables.
    */
    lat = 0.;
    lon = 0.;
    elevation = 0.;
    double latav=0.;
    double lonav=0.;
    double eleav=0.;
    bool through=false;
    unsigned indexU=0;
    unsigned indexO=0;
    unsigned i=1;
    unsigned cnt=0;
    unsigned corS=3600*corH;
    unsigned long timecorr;
   // std::cout << times[0]+corS << " " << rt << endl;
   // std::cout << int(rt>timecorr) << endl;
    while (!through && i<times.size() && rt>=times[0]+corS){
        timecorr=times[i]+corS;
        if (timecorr>=rt){
            through=true;
            indexO=i;
            break;
        }
        if (timecorr>=rt-len){
            if (cnt==0){
                indexU=i;
            }
            latav+=lats[i]; //Sum up position values in recording time period
            lonav+=lons[i];
            eleav+=elevations[i];
            cnt++;
        }

        i++;
    }

    if (!through) { //if no gps data sets are in the given recording interval, there are no datasets avialable, the file was not recorded in that period.
        numberOfAvailableDataSets=0;
        index=0;
        ofLog()<<"error extracting position from csv file: no time data in filename?";
    }
    else{ //if we went through, the running sums get divided by the number of data sets for averaging.
        index=indexU;
        numberOfAvailableDataSets=cnt;
        lat=latav/double(cnt);
        lon=lonav/double(cnt);
        elevation=eleav/double(cnt);
    }
}
