#ifndef DATAREAD_H
#define DATAREAD_H

#include <vector>
#include <string>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

class DataRead{
    std::string audioFolderPath;
    std::vector<std::string> files; //store all files in dir
    std::vector<unsigned long> filesCreationTimes; //store all files in dir
    std::vector<std::string> wavFiles; //for all the wav-files in the directory
    std::vector<unsigned long> wavFilesCreationTimes; //for all the wav-files in the directory
public:

    DataRead(std::string path);
    void read_directory(const std::string& name, std::vector<std::string>& v, std::vector<unsigned long>& vt); //get complet list of all files in directory
    //std::string getFileCreationTime(char *path);
    unsigned long getFileCreationTime(std::string n);
    void print_wavFiles();
    void getFileList(); //get all .wav files in directory and return them as vector of strings for use in application
    std::vector<std::string> returnFileList(){
        return wavFiles;
    } //get all .wav files in directory and return them as vector of strings for use in application
    std::vector<unsigned long> returnFileCreationTimeList(){
        return wavFilesCreationTimes;
    }
    void findWavFiles(std::vector<std::string>& v, std::vector<std::string>& wv, std::vector<unsigned long>& vt, std::vector<unsigned long>& wvt); //find all wav-files and copy them into a new list wv
};

#endif // DATAREAD_H
