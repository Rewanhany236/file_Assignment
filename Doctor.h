#ifndef DOCTOR_H
#define DOCTOR_H
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"
using namespace std;

const char DELETE_FLAG = '*';
const int FILE_HEADER_SIZE = sizeof(int);

class Doctor
{
    char id[15];        // Fixed Field
    char name[30];      // Fixed Field
    char specialty[20]; // Fixed Field
    char address[30];
public:
    void formatField(char field[], string val, int size);
    bool isDoctorExists(fstream &file, string doctorID ,PrimaryIndex &pIndex);
    void addDoctor(fstream &file, PrimaryIndex &pIndex, SecondaryIndex &sIndex,SecondaryIndex &specIndex);
    void deleteDoctor(fstream &file, PrimaryIndex &pIndex, SecondaryIndex &sIndex, SecondaryIndex &specIndex);
    void updateDoctorName(fstream &file, PrimaryIndex &pIndex, SecondaryIndex &sIndex);
    void updateDoctorId(fstream &file, PrimaryIndex &pIndex, SecondaryIndex &sIndex, SecondaryIndex &specIndex);
    void printDoctorInfo(fstream &file,PrimaryIndex &pIndex);
    void seedInitialDoctors(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &nameIndex, SecondaryIndex &specIndex);

};

#endif