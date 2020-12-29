#include "dataread.h"
#include "ofMain.h"

DataRead::DataRead(string audioFilesLocationPath){
    audioFolderPath=audioFilesLocationPath;
    getFileList();
}

void DataRead::getFileList(){
 read_directory(audioFolderPath, files, filesCreationTimes);
 findWavFiles(files, wavFiles, filesCreationTimes, wavFilesCreationTimes);
 //print_files(wavFiles);
}

void DataRead::read_directory(const std::string& name, std::vector<std::string> & v, std::vector<unsigned long> & vt){
    DIR *dirp;
    dirp = opendir(name.c_str());
    struct dirent * dp;
    std::string pfname(name); //new string containing path
    while ((dp = readdir(dirp)) != NULL) {
        v.push_back(dp->d_name);
        std::string fname(dp->d_name);
        /*
        pfname.insert(pfname.size(),fname); //constructing full paht for creation time extraction
        char * pfnamecs=new char[pfname.size()+fname.size()];
        for (unsigned i=0; i<pfname.size(); i++){
            pfnamecs[i]=pfname[i];
        }
        */
        vt.push_back(getFileCreationTime(fname));
    }
    closedir(dirp);
}
/*
std::string DataRead::getFileCreationTime(char *path){
    struct stat attr;
    stat(path, &attr);
    std::string ct(ctime(&attr.st_mtime));
    return ct;
}*/

unsigned long DataRead::getFileCreationTime(std::string n){
    unsigned long rt=0;
    if (n.size()>9){
        std::string h(n.substr(n.length()-12,2));
        std::string m(n.substr(n.length()-9,2));
        std::string s(n.substr(n.length()-6,2));
        rt= 3600*std::atoi(h.c_str())+60*std::atoi(m.c_str())+std::atoi(s.c_str());
    }
    return rt;
}

void DataRead::findWavFiles(std::vector<std::string> &v, std::vector<std::string> & wv, std::vector<unsigned long> & vt, std::vector<unsigned long> & wvt){
    std::string ending(".wav");
    for (unsigned i=0; i<v.size(); i++){
        unsigned l=v[i].size();
        if (l>5){
            if (0==v[i].substr(l-4, l-0).compare(ending)){
                wv.push_back(v[i]);
                wvt.push_back(vt[i]);
            }
        }
    }
}

void DataRead::print_wavFiles(){
    unsigned long l=wavFiles.size();
    std::cout << "files: " << endl;
    for (unsigned long i=0; i<l; i++){
        std::cout << wavFiles[i] << endl;
    }
}
