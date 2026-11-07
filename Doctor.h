#ifndef DOCTOR_H
#define DOCTOR_H
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

const char DELETE_FLAG = '*';
const char ACTIVE_FLAG = ' ';
const int FILE_HEADER_SIZE = sizeof(int);

class Doctor
{
    char id[15];        // Fixed Field
    char name[30];      // Fixed Field
    char specialty[20]; // Fixed Field
    char address[30];
public:
    void formatField(char field[], string val, int size);
    bool isDoctorExists(fstream &file, string doctorID);
    void addDoctor(fstream &file);
    void deleteDoctor(fstream &file);
    void updateDoctorName(fstream &file);
    void printDoctorInfo(fstream &file);
    void seedInitialDoctors(fstream &file);

};

#endif