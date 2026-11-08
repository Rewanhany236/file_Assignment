#include <iostream>
#include <fstream>
#include "Doctor.h"          // [Member 3]
using namespace std;

PrimaryIndex doctorIdx;      // [Member 3] Index #1 (Doctor ID)

int main()
{
    fstream file("Doctors.bin", ios::in | ios::out | ios::binary);

    if (!file) {
        ofstream createFile("Doctors.bin", ios::binary);
        int initialHeader = -1;
        createFile.write((char*)&initialHeader, sizeof(int));
        createFile.close();

        file.open("Doctors.bin", ios::in | ios::out | ios::binary);
    }

    // [Member 3] build the index from the file every time the program starts
    doctorIdx.rebuild("Doctors.bin", 95, 15);

    // ---- TEMPORARY test menu (to be replaced by the real main menu) ----
    int choice;
    Doctor d;
    while (true) {
        cout << "\n1 Add  2 Delete  3 Update Name  4 Update ID  5 Show index  0 Exit\n> ";
        cin >> choice;
        if (choice == 0) break;
        if (choice == 1) d.addDoctor(file);
        else if (choice == 2) d.deleteDoctor(file);
        else if (choice == 3) d.updateDoctorName(file);
        else if (choice == 4) d.updateDoctorID(file);
        else if (choice == 5) doctorIdx.print();
    }
    file.close();
    return 0;
}
