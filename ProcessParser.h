#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"

using namespace std;

class ProcessParser {
private:
    std::ifstream stream;
public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static string getVmSize(string pid);
    static string getCpuPercent(string pid);
    static string getSysUpTime();
    static string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getNumberOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static string PrintCpuStats(vector<string> values1, vector<string> values2);
    static bool isPidExisting(string pid);
};

// helper methods
vector<string> splitString(const string& str, char delim) {
    vector<string> v;
    string buf = "";
    for(int i = 0; i < str.size(); ++i) {
        if(str[i] == delim) {
            if(buf.size() > 0) {
                v.push_back(buf);
                buf = "";
            }
        }
        else{
            buf += str[i];
        }
    }
    if(buf.size() > 0) v.push_back(buf);
    return v;
}
vector<string> getAllStatusInfo(string pid) {
    ifstream stream("/proc/" + pid + "/stat");
    string statusinfo;
    vector<string> v;
    if(stream.is_open()) {
        getline(stream, statusinfo);
        v = splitString(statusinfo, ' ');
    }
    return v;
}
int filter(const struct dirent * dir) {
    if(dir->d_name[0] >= '0' && dir->d_name[0] <= '9')
        return 1;
    return 0;
}
int compareDirent(const struct dirent **a, const struct dirent **b) {
    return (*a)->d_name < (*b)->d_name;
}

// information of one specific process
string ProcessParser::getCmd(string pid) {
    string cmdline;
    ifstream stream("/proc/" + pid + "/cmdline");
    if (stream.is_open()) {
        getline(stream, cmdline);
    }
    return cmdline;
}
string ProcessParser::getProcUpTime(string pid) {
    vector<string> status = getAllStatusInfo(pid);
    string startTime = status[21]; // in clock ticks
    ifstream stream("/proc/uptime");
    long int sysUpSeconds = 0;
    if(stream.is_open()) stream >> sysUpSeconds;
    return Util::convertToTime(sysUpSeconds - stol(startTime) / sysconf(_SC_CLK_TCK));
}
string ProcessParser::getProcUser(string pid) {
    ifstream stream1("/proc/" + pid + "/status");
    string uid = "";
    if(stream1.is_open()) {
        string s;
        while (stream1 >> s) {
            if (s.compare("Uid:") == 0) {
                stream1 >> s; // uid
                uid = s;
                break;
            }
        }
    }
    ifstream stream2("/etc/passwd");
    string line, username, id;    
    while (getline(stream2, line)) {
        int i = 0;
        username = "";
        id = "";
        while(i < line.size()) {
            if(line[i] != ':') 
                username += line[i];
            else break;
            ++i;
        }  
        i += 3;
        while(i < line.size()) {
            if(line[i] >= '0' && line[i] <= '9')
                id += line[i];
            else break;
            ++i;
        }
        if(id.compare(uid) == 0) 
            return username;
    }
    return "";
}
string ProcessParser::getVmSize(string pid) {
    vector<string> status = getAllStatusInfo(pid);
    return status[22];
}
string ProcessParser::getCpuPercent(string pid) {
    return "";
}

// information of the whole system
vector<string> ProcessParser::getPidList() {
    struct dirent **namelist;
    int n;
    vector<string> pidList;
    n = scandir("/proc", &namelist, filter, compareDirent);
    if (n < 0)
        perror("scandir");
    else {
        while (n--) {
            pidList.push_back(namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
    return pidList;
}

string ProcessParser::getSysUpTime() {
    ifstream stream("/proc/uptime");
    long int seconds = 0;
    if(stream.is_open()) stream >> seconds;
    return Util::convertToTime(seconds);
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
    vector<string> v;
    return v;
}
float ProcessParser::getSysRamPercent() {
    ifstream stream("/proc/meminfo");
    
    return 0.0;
}
string ProcessParser::getSysKernelVersion() {
    ifstream stream("/proc/sys/kernel/version");
    string s = "";
    getline(stream, s);
    return s;
}
int ProcessParser::getNumberOfCores() {
    ifstream stream("/proc/cpuinfo");
    int cores = 0;
    if(stream.is_open()) {
        string line;
        while(getline(stream, line)) {
            if(line.find("cores") != string::npos) {
                int pos = line.find(":");
                cores = stoi(line.substr(pos + 2));
                break;
            }
        }
    }
    return cores;
}
int ProcessParser::getTotalThreads() {
    struct dirent **namelist;
    int n, totalThreads = 0;
    n = scandir("/proc", &namelist, filter, compareDirent);
    if (n < 0)
        perror("scandir");
    else {
        while (n--) {
            vector<string> status = getAllStatusInfo(namelist[n]->d_name);
            totalThreads += stoi(status[19]);
            free(namelist[n]);
        }
        free(namelist);
    }
    return totalThreads;
}
int ProcessParser::getTotalNumberOfProcesses() {
    ifstream stream("/proc/stat");
    string s;
    while(getline(stream, s)) {
        if(s.find("processes") != string::npos) {
            size_t pos = s.find(" ");
            return stoi(s.substr(pos + 1));
        }
    }
    return 0;
}
int ProcessParser::getNumberOfRunningProcesses() {
    ifstream stream("/proc/stat");
    string s;
    while(getline(stream, s)) {
        if(s.find("procs_running") != string::npos) {
            size_t pos = s.find(" ");
            return stoi(s.substr(pos + 1));
        }
    }
    return 0;
}
string ProcessParser::getOSName() {
    ifstream stream("/etc/lsb-release");
    string s;
    getline(stream, s);
    size_t pos = s.find("=");
    return s.substr(pos + 1);
};

string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2) {
    return "";
}

bool ProcessParser::isPidExisting(string pid) {
    bool flag = false;
    struct dirent **namelist;
    int n = scandir("/proc", &namelist, filter, NULL);

    if (n < 0)
        perror("scandir");
    else {
        while (n--) {
            if(string(namelist[n]->d_name).compare(pid) == 0)
                flag = true;
                free(namelist[n]);
        }
        free(namelist);
    }
    return flag;
}
