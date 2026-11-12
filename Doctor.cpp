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

void Doctor::addDoctor(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &nameIndex, SecondaryIndex &specIndex)
{
    Doctor d;
    string id, name, spec, addr;

    cout << "======= Add New Doctor =======\n";
    cout << "Enter Doctor ID: ";
    cin >> id;

    if (isDoctorExists(file, id, pIndex)) {
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

    int firstDeletedOffset = -1;
    file.seekg(0, ios::beg);
    file.read((char*)&firstDeletedOffset, sizeof(int));

    int wrightOffset;
    if (firstDeletedOffset == -1)
    {
        file.seekp(0, ios::end);
        wrightOffset = file.tellp();
    }
    else
    {
        wrightOffset = firstDeletedOffset;
        int nextDeleted = -1;
        file.seekg(wrightOffset + 1, ios::beg);
        file.read((char*)&nextDeleted, sizeof(int));

        file.seekp(0, ios::beg);
        file.write((char*)&nextDeleted, sizeof(int));

        file.seekp(wrightOffset, ios::beg);
    }

    file.write((char*)&d, sizeof(Doctor));
    pIndex.insertEntry(id, wrightOffset);
    nameIndex.insertEntry(name, id);
    specIndex.insertEntry(spec, id);

    cout << "Doctor added successfully at offset " << wrightOffset << endl;
}

void Doctor::deleteDoctor(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &nameIndex, SecondaryIndex &specIndex)
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
    string oldSpec(d.specialty, 20);

    int firstDeletedOffset = -1;
    file.seekg(0, ios::beg);
    file.read((char*)&firstDeletedOffset, sizeof(int));

    file.seekp(targetOffset, ios::beg);
    file.put(DELETE_FLAG);
    file.write((char*)&firstDeletedOffset, sizeof(int));

    file.seekp(0, ios::beg);
    file.write((char*)&targetOffset, sizeof(int));

    pIndex.deleteEntry(id);
    nameIndex.deleteEntry(oldName, id);
    specIndex.deleteEntry(oldSpec, id);

    cout << "Doctor deleted successfully at offset " << targetOffset << endl;
}

void Doctor::updateDoctorName(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &nameIndex)
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

    nameIndex.deleteEntry(oldName, id);
    nameIndex.insertEntry(name, id);
    cout << "Doctor Name updated successfully at offset " << offset << endl;
}

void Doctor::printDoctorInfo(fstream& file, PrimaryIndex &pIndex)
{
    string id;
    cout << "Enter Doctor ID to print: ";
    cin >> id;

    Doctor d;
    int offset = pIndex.getOffset(id);
    if (offset == -1)
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

void Doctor::updateDoctorId(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &nameIndex, SecondaryIndex &specIndex)
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
    string docSpec(d.specialty, 20);

    formatField(d.id, newId, 15);
    file.seekp(offset, ios::beg);
    file.write((char*)&d, sizeof(Doctor));

    pIndex.updateKey(id, newId);
    nameIndex.deleteEntry(docName, id);
    nameIndex.insertEntry(docName, newId);
    specIndex.deleteEntry(docSpec, id);
    specIndex.insertEntry(docSpec, newId);

    cout << "Doctor ID updated successfully at offset " << offset << endl;
}

void Doctor::seedInitialDoctors(fstream& file, PrimaryIndex &pIndex, SecondaryIndex &nameIndex, SecondaryIndex &specIndex)
{
    file.seekg(0, ios::end);
    if (file.tellg() > FILE_HEADER_SIZE)
        return;

    string ids[5] = {"DOC001", "DOC002", "DOC003", "DOC004", "DOC005"};
    string names[5] = {"Ahmed Ali", "Sara Hassan", "Omar Khaled", "Mona Adel", "Tarek Samir"};
    string specs[5] = {"Cardiology", "Dermatology", "Pediatrics", "Neurology", "Cardiology"};
    string addrs[5] = {"Cairo", "Giza", "Alexandria", "Nasr City", "Maadi"};

    file.seekp(0, ios::end);
    for (int i = 0; i < 5; i++)
    {
        Doctor d;
        formatField(d.id, ids[i], 15);
        formatField(d.name, names[i], 30);
        formatField(d.specialty, specs[i], 20);
        formatField(d.address, addrs[i], 30);

        int offset = file.tellp();
        file.write((char*)&d, sizeof(Doctor));

        pIndex.insertEntry(ids[i], offset);
        nameIndex.insertEntry(names[i], ids[i]);
        specIndex.insertEntry(specs[i], ids[i]);
    }
}