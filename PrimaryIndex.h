#ifndef PRIMARY_INDEX_H
#define PRIMARY_INDEX_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


struct PrimaryEntry {
    string key;   
    int         offset;
};


class PrimaryIndex {
private:
    vector<PrimaryEntry> data;   

    
    int binarySearch(const string& key) const {
        int low = 0, high = (int)data.size() - 1;
        while (low <= high) {
            int mid = (low + high) / 2;
            if (data[mid].key == key) return mid;
            if (data[mid].key < key)  low = mid + 1;
            else                      high = mid - 1;
        }
        return -1;
    }

    
    int insertPosition(const string& key) const {
        int low = 0, high = (int)data.size();
        while (low < high) {
            int mid = (low + high) / 2;
            if (data[mid].key < key) low = mid + 1;
            else                     high = mid;
        }
        return low;
    }

    static string trim(const char* buf, int len) {
        string s(buf, len);
        while (!s.empty() && (s.back() == ' ' || s.back() == '\0')) s.pop_back();
        return s;
    }

public:
    int size() const { return (int)data.size(); }

    bool exists(const string& key) const { return binarySearch(key) != -1; }

    
    int getOffset(const string& key) const {
        int pos = binarySearch(key);
        return pos == -1 ? -1 : data[pos].offset;
    }

    
    
    bool insertEntry(const string& key, int offset) {
        if (exists(key)) return false;
        int pos = insertPosition(key);
        data.push_back(PrimaryEntry());                 
        for (int i = (int)data.size() - 1; i > pos; i--) 
            data[i] = data[i - 1];
        data[pos].key = key;
        data[pos].offset = offset;
        return true;
    }

    
    
    bool deleteEntry(const string& key) {
        int pos = binarySearch(key);
        if (pos == -1) return false;
        for (int i = pos; i < (int)data.size() - 1; i++) 
            data[i] = data[i + 1];
        data.pop_back();
        return true;
    }

    
    int updateKey(const string& oldKey, const string& newKey) {
        int pos = binarySearch(oldKey);
        if (pos == -1) return 1;
        if (oldKey != newKey && exists(newKey)) return 2;
        int off = data[pos].offset;
        deleteEntry(oldKey);          
        insertEntry(newKey, off);     
        return 0;
    }

    
    bool rebuild(const string& fileName, int recordSize, int keySize) {
        data.clear();
        ifstream in(fileName.c_str(), ios::binary);
        if (!in) return false;                 

        const int HEADER = (int)sizeof(int);
        in.seekg(0, ios::end);
        long fileSize = (long)in.tellg();

        vector<char> rec(recordSize);
        for (long off = HEADER; off + recordSize <= fileSize; off += recordSize) {
            in.seekg(off, ios::beg);
            in.read(rec.data(), recordSize);
            if (rec[0] == '*') continue;       
            PrimaryEntry e;
            e.key = trim(rec.data(), keySize);
            e.offset = (int)off;
            if (!e.key.empty()) data.push_back(e);
        }
        sort(data.begin(), data.end(),
                  [](const PrimaryEntry& a, const PrimaryEntry& b) { return a.key < b.key; });
        return true;
    }

    void print() const {
        for (size_t i = 0; i < data.size(); i++)
            cout << "[" << i << "] " << data[i].key << " -> " << data[i].offset << "\n";
    }
};




#endif
