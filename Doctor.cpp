#include "Doctor.h"
#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;


void Doctor::formatField(char field[], string val, int size)
{
    memset(field, ' ', size);
    int len = val.length();
    if (len > size)
        len = size;
    memcpy(field, val.c_str(), len);
}

bool Doctor::isDoctorExists(fstream &file, string doctorID)
{
    char formatId[15];
    formatField(formatId, doctorID, 15);

    file.seekg(FILE_HEADER_SIZE, ios::beg);
    Doctor doctor;
    while (file.read((char *) &doctor, sizeof(Doctor)))
    {
        if (doctor.id[0]!=DELETE_FLAG)
        {
           if ( memcmp(formatId, doctor.id, 15)==0)
               return true;
        }
    }
    return false;
}

void Doctor::addDoctor(fstream& file)
{
    Doctor d;
    string id, name, spec, addr;

    cout << "======= Add New Doctor ======= ";
    cout << "Enter Doctor ID: ";
    cin >> id;

    if (isDoctorExists(file, id)) {
        cout << "Error: Duplicate Doctor ID! This ID already exists.\n";
        return;
    }

    cout << "Enter Doctor Name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Enter Specialty: ";
    getline(cin, spec);
    cout << "Enter Address: ";
    getline(cin, addr);

    d.formatField(d.id, id, 15);
    d.formatField(d.name, name, 30);
    d.formatField(d.specialty, spec, 20);
    d.formatField(d.address, addr, 30);

    int firstDeletedOffset =-1; // -1 means no deleted doctor
    file.seekg(0, ios::beg);
    file.read((char*)&firstDeletedOffset, sizeof(int));

    int wrightOffset ;
    if (firstDeletedOffset == -1)
    {
        file.seekp(0, ios::end);
        wrightOffset = file.tellp();

    }
    else
    {
        wrightOffset = firstDeletedOffset;
        int nextDeleted = -1;
        file.seekg(wrightOffset+1, ios::beg); // +1 for the *
        file.read((char*)&nextDeleted, sizeof(int));

        file.seekp(0, ios::beg);
        file.write((char*)&nextDeleted, sizeof(int));

        file.seekp(wrightOffset, ios::beg);
    }

    file.write((char*)&d, sizeof(Doctor));

    cout << "Doctor added successfully at offset " << wrightOffset << endl;

}


void Doctor::deleteDoctor(fstream& file)
{
    string id;
    cout << "Enter Doctor ID to delete: ";
    cin >> id;

    char searchId[15];
    formatField(searchId, id, 15);

    file.seekg(FILE_HEADER_SIZE, ios::beg);
    Doctor d;
    int targetOffset = -1;

    while (file.read((char*)&d, sizeof(Doctor))) {
        if (d.id[0] != DELETE_FLAG && memcmp(d.id, searchId, 15) == 0) {
            targetOffset = (int)file.tellg() - sizeof(Doctor);
            break;
        }
    }
    if (targetOffset == -1) {
        cout << "Error: Doctor not found.\n";
        return;
    }

    int firstDeletedOffset = -1;
    file.seekg(0, ios::beg);
    file.read((char*)&firstDeletedOffset, sizeof(int));

    file.seekp(targetOffset, ios::beg);
    file.put(DELETE_FLAG);
    file.write((char*)&firstDeletedOffset, sizeof(int));

    file.seekp(0, ios::beg);
    file.write((char*)&targetOffset, sizeof(int));

    cout << "Doctor deleted successfully at offset " << targetOffset << endl;
}


void Doctor::updateDoctorName(fstream& file)
{
    string id, name;
    cout << "Enter Doctor ID to update: ";
    cin >> id;

    int offset = -10; // = pIndex.search
    if (offset==-1)
    {
        cout << "Error: Doctor not found.\n";
        return;
    }
    file.seekg(offset, ios::beg);
    Doctor d;
    file.read((char*)&d, sizeof(Doctor));

    cout << "Enter new Doctor Name: ";
    getline(cin, name);
    formatField(d.name, name, 30);
    file.seekp(offset, ios::beg);
    file.write((char*)&d, sizeof(Doctor));

    cout << "Doctor Name updated successfully at offset " << offset << endl;
}








