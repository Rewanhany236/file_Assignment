#include <iostream>
#include <fstream>
using namespace std;

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
}