#pragma once

#ifndef SECONDARY_INDEX_H
#define SECONDARY_INDEX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

using namespace std;

struct LinkedNode {
    char primaryKey[15]; 
    int next;            
};


struct SecondaryEntry {
    string secKey; 
    int head;      
};

class SecondaryIndex {
private:
    vector<SecondaryEntry> indexList;
    string llFileName; 

  
    static string trim(const char* buf, int len) {
        string s(buf, len);
        while (!s.empty() && (s.back() == ' ' || s.back() == '\0')) {
            s.pop_back();
        }
        return s;
    }

    
    int binarySearch(const string& key) const {
        int low = 0, high = (int)indexList.size() - 1;
        while (low <= high) {
            int mid = (low + high) / 2;
            if (indexList[mid].secKey == key) return mid;
            if (indexList[mid].secKey < key) low = mid + 1;
            else high = mid - 1;
        }
        return -1;
    }

    
    int insertPosition(const string& key) const {
        int low = 0, high = (int)indexList.size();
        while (low < high) {
            int mid = (low + high) / 2;
            if (indexList[mid].secKey < key) low = mid + 1;
            else high = mid;
        }
        return low;
    }

    
    int appendNodeToFile(const string& pKey, int nextPtr) {
        fstream out(llFileName, ios::in | ios::out | ios::binary);
        out.seekp(0, ios::end);
        int offset = (int)out.tellp();

        LinkedNode node{};
        memset(node.primaryKey, '\0', 15);
        int len = (int)pKey.length() > 14 ? 14 : (int)pKey.length();
        memcpy(node.primaryKey, pKey.c_str(), len);
        node.next = nextPtr;

        out.write((char*)&node, sizeof(LinkedNode));
        out.close();
        return offset;
    }

public:
   
    SecondaryIndex(string fileName) {
        llFileName = fileName;
        ofstream clearFile(llFileName, ios::binary | ios::trunc);
        clearFile.close();
    }

    
    void insertEntry(const string& secKey, const string& primKey) {
        string cleanSec = trim(secKey.c_str(), secKey.length());
        string cleanPrim = trim(primKey.c_str(), primKey.length());

        int pos = binarySearch(cleanSec);

        if (pos == -1) {
            
            int newOffset = appendNodeToFile(cleanPrim, -1);
            int insPos = insertPosition(cleanSec);
            SecondaryEntry newEntry{ cleanSec, newOffset };
            indexList.insert(indexList.begin() + insPos, newEntry);
        }
        else {
  
            int oldHead = indexList[pos].head;
            int newOffset = appendNodeToFile(cleanPrim, oldHead);
            indexList[pos].head = newOffset;
        }
    }

    
    vector<string> search(const string& secKey) const {
        vector<string> results;
        string cleanSec = trim(secKey.c_str(), secKey.length());

        int pos = binarySearch(cleanSec);
        if (pos == -1) return results; 

        ifstream in(llFileName, ios::binary);
        if (!in) return results;

        int currOffset = indexList[pos].head;
        LinkedNode node;

        
        while (currOffset != -1) {
            in.seekg(currOffset, ios::beg);
            in.read((char*)&node, sizeof(LinkedNode));
            results.push_back(trim(node.primaryKey, 15));
            currOffset = node.next;
        }
        in.close();
        return results;
    }

    void deleteEntry(const string& secKey, const string& primKey) {
        string cleanSec = trim(secKey.c_str(), secKey.length());
        string cleanPrim = trim(primKey.c_str(), primKey.length());

        int pos = binarySearch(cleanSec);
        if (pos == -1) return;

        fstream file(llFileName, ios::in | ios::out | ios::binary);
        if (!file) return;

        int currOffset = indexList[pos].head;
        int prevOffset = -1;
        LinkedNode currNode;

        while (currOffset != -1) {
            file.seekg(currOffset, ios::beg);
            file.read((char*)&currNode, sizeof(LinkedNode));
            string nodeKey = trim(currNode.primaryKey, 15);

            if (nodeKey == cleanPrim) {
                
                if (prevOffset == -1) {
                    
                    indexList[pos].head = currNode.next;
                }
                else {
                    
                    LinkedNode prevNode;
                    file.seekg(prevOffset, ios::beg);
                    file.read((char*)&prevNode, sizeof(LinkedNode));
                    prevNode.next = currNode.next;

                    file.seekp(prevOffset, ios::beg);
                    file.write((char*)&prevNode, sizeof(LinkedNode));
                }

           
                if (indexList[pos].head == -1) {
                    indexList.erase(indexList.begin() + pos);
                }
                break;
            }
            prevOffset = currOffset;
            currOffset = currNode.next;
        }
        file.close();
    }


    static vector<string> intersect(const vector<string>& list1, const vector<string>& list2) {
        vector<string> result;
        for (const string& id1 : list1) {
            for (const string& id2 : list2) {
                if (id1 == id2) {
                    result.push_back(id1);
                    break;
                }
            }
        }
        return result;
    }

   
    void rebuild(const string& dataFileName, int recordSize, int secOffset, int secSize, int primOffset, int primSize) {
        indexList.clear();
        ofstream clearFile(llFileName, ios::binary | ios::trunc);
        clearFile.close();

        ifstream in(dataFileName, ios::binary);
        if (!in) return;

        const int HEADER = sizeof(int);
        in.seekg(0, ios::end);
        long fileSize = (long)in.tellg();

        vector<char> rec(recordSize);
        for (long off = HEADER; off + recordSize <= fileSize; off += recordSize) {
            in.seekg(off, ios::beg);
            in.read(rec.data(), recordSize);

            if (rec[0] == '*') continue; 

            string sKey = trim(&rec[secOffset], secSize);
            string pKey = trim(&rec[primOffset], primSize);

            if (!sKey.empty() && !pKey.empty()) {
                insertEntry(sKey, pKey);
            }
        }
        in.close();
    }


    void printIndex() const {
        cout << "--- Secondary Index (" << llFileName << ") ---\n";
        for (const auto& entry : indexList) {
            cout << entry.secKey << " -> ";
            vector<string> ids = search(entry.secKey);
            for (size_t i = 0; i < ids.size(); i++) {
                cout << ids[i] << (i + 1 < ids.size() ? " -> " : "");
            }
            cout << "\n";
        }
    }
};

#endif