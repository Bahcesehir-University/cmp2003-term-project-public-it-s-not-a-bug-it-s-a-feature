#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <cctype>
using namespace std;

static unordered_map<string, long long> zoneCounts;
static map<pair<string, int>, long long> slotCounts;

static int extractHour(const string& datetime) {
    if (datetime.empty() || datetime.length() < 16) {
        return -1;
    }
    
    size_t spacePos = datetime.find(' ');
    if (spacePos == string::npos || spacePos + 3 >= datetime.length()) {
        return -1;
    }
    
    if (!isdigit(datetime[spacePos + 1]) || !isdigit(datetime[spacePos + 2])) {
        return -1;
    }
    
    int hour = (datetime[spacePos + 1] - '0') * 10 + (datetime[spacePos + 2] - '0');
    
    if (hour < 0 || hour > 23) {
        return -1;
    }
    
    return hour;
}

void TripAnalyzer::ingestFile(const string& csvPath) {
    zoneCounts.clear();
    slotCounts.clear();
    
    ifstream file(csvPath);
    if (!file.is_open()) {
        return;
    }
    
    string line;
    if (!getline(file, line)) {
        return;
    }
    
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        
        vector<string> fields;
        stringstream ss(line);
        string field;
        
        while (getline(ss, field, ',')) {
            fields.push_back(field);
        }
        
        if (fields.size() < 6) {
            continue;
        }
        
        string zone = fields[1];
        if (zone.empty()) {
            continue;
        }
        
        string datetime = fields[3];
        if (datetime.empty()) {
            continue;
        }
        
        int hour = extractHour(datetime);
        if (hour == -1) {
            continue;
        }
        
        zoneCounts[zone]++;
        slotCounts[{zone, hour}]++;
    }
    
    file.close();
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> result;
    
    for (const auto& pair : zoneCounts) {
        result.push_back({pair.first, pair.second});
    }
    
    sort(result.begin(), result.end(),
        [](const ZoneCount& a, const ZoneCount& b) {
            if (a.count != b.count) {
                return a.count > b.count;
            }
            return a.zone < b.zone;
        });
    
    if (k <= 0) {
        return {};
    }
    if (static_cast<size_t>(k) >= result.size()) {
        return result;
    }
    return vector<ZoneCount>(result.begin(), result.begin() + k);
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> result;
    
    for (const auto& pair : slotCounts) {
        result.push_back({pair.first.first, pair.first.second, pair.second});
    }
    
    sort(result.begin(), result.end(),
        [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) {
                return a.count > b.count;
            }
            if (a.zone != b.zone) {
                return a.zone < b.zone;
            }
            return a.hour < b.hour;
        });
    
    if (k <= 0) {
        return {};
    }
    if (static_cast<size_t>(k) >= result.size()) {
        return result;
    }
    return vector<SlotCount>(result.begin(), result.begin() + k);
}
