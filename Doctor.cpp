#include "Doctor.h"
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"
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

bool Doctor::isDoctorExists(fstream &file, string doctorID, PrimaryIndex &pIndex)
{
    return pIndex.exists(doctorID);
}

void Doctor::addDoctor(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &sIndex)
{
    Doctor d;
    string id, name, spec, addr;

    cout << "======= Add New Doctor ======= ";
    cout << "Enter Doctor ID: ";
    cin >> id;

    if (isDoctorExists(file, id,pIndex)) {
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
    pIndex.insertEntry(id, wrightOffset);
    sIndex.insertEntry(name, id);

    cout << "Doctor added successfully at offset " << wrightOffset << endl;

}

void Doctor::deleteDoctor(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &sIndex)
{
    string id;
    cout << "Enter Doctor ID to delete: ";
    cin >> id;

    int targetOffset = pIndex.getOffset(id);
    if (targetOffset == -1) {
        cout << "Error: Doctor not found.\n";
        return;
    }

    Doctor d;
    file.seekg(targetOffset, ios::beg);
    file.read((char*)&d, sizeof(Doctor));
    string oldName(d.name, 30);

    int firstDeletedOffset = -1;
    file.seekg(0, ios::beg);
    file.read((char*)&firstDeletedOffset, sizeof(int));

    file.seekp(targetOffset, ios::beg);
    file.put(DELETE_FLAG);
    file.write((char*)&firstDeletedOffset, sizeof(int));

    file.seekp(0, ios::beg);
    file.write((char*)&targetOffset, sizeof(int));

    pIndex.deleteEntry(id);
    sIndex.deleteEntry(oldName, id);

    cout << "Doctor deleted successfully at offset " << targetOffset << endl;
}
void Doctor::updateDoctorName(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &sIndex)
{
    string id, name;
    cout << "Enter Doctor ID to update: ";
    cin >> id;

    int offset = pIndex.getOffset(id);
    if (offset == -1)
    {
        cout << "Error: Doctor not found.\n";
        return;
    }
    file.seekg(offset, ios::beg);
    Doctor d;
    file.read((char*)&d, sizeof(Doctor));
    string oldName(d.name, 30);

    cout << "Enter new Doctor Name: ";
    cin.ignore();
    getline(cin, name);
    formatField(d.name, name, 30);
    file.seekp(offset, ios::beg);
    file.write((char*)&d, sizeof(Doctor));

    sIndex.deleteEntry(oldName, id);
    sIndex.insertEntry(name, id);
    cout << "Doctor Name updated successfully at offset " << offset << endl;
}
void Doctor::printDoctorInfo(fstream& file,PrimaryIndex &pIndex)
{
    string id;
    cout << "Enter Doctor ID to print: ";
    cin >> id;

    Doctor d;
    int offset = pIndex.getOffset(id);
    if (offset==-1)
    {
        cout << "Error: Doctor not found.\n";
        return;
    }
    file.seekg(offset, ios::beg);
    file.read((char*)&d, sizeof(Doctor));
    cout << "Doctor ID: " << string(d.id, 15) << endl;
    cout << "Doctor Name: " << string(d.name, 30) << endl;
    cout << "Specialty: " << string(d.specialty, 20) << endl;
    cout << "Address: " << string(d.address, 30) << endl;
}
void Doctor::updateDoctorId(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &sIndex)
{
    string id, newId;
    cout << "Enter Doctor ID to update: ";
    cin >> id;

    int offset = pIndex.getOffset(id);
    if (offset == -1)
    {
        cout << "Error: Doctor not found.\n";
        return;
    }

    cout << "Enter new Doctor ID: ";
    cin >> newId;
    if (isDoctorExists(file, newId, pIndex))
    {
        cout << "Error: Duplicate Doctor ID! This ID already exists.\n";
        return;
    }
    file.seekg(offset, ios::beg);
    Doctor d;
    file.read((char*)&d, sizeof(Doctor));
    string docName(d.name, 30);

    formatField(d.id, newId, 15);
    file.seekp(offset, ios::beg);
    file.write((char*)&d, sizeof(Doctor));

    pIndex.updateKey(id, newId);
    sIndex.deleteEntry(docName, id);
    sIndex.insertEntry(docName, newId);

    cout << "Doctor ID updated successfully at offset " << offset << endl;
}

void Doctor::seedInitialDoctors(fstream& file) // 5 doctors
{

}









