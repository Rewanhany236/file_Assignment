#include <iostream>
#include <fstream>
#include <limits>
#include "Doctor.h"
#include "PrimaryIndex.h"
#include "Appointment.h"

using namespace std;

PrimaryIndex doctorIdx;
PrimaryIndex appointmentIdx;

int main()
{
    // Open Doctors file
    fstream file("Doctors.bin", ios::in | ios::out | ios::binary);

    if (!file) {
        ofstream createFile("Doctors.bin", ios::binary);
        int initialHeader = -1;
        createFile.write((char*)&initialHeader, sizeof(int));
        createFile.close();

        file.open("Doctors.bin", ios::in | ios::out | ios::binary);
    }

    // Open Appointments file
    fstream appointmentFile("Appointments.dat", ios::in | ios::out | ios::binary);

    if (!appointmentFile) {
        ofstream createFile("Appointments.dat", ios::binary);
        int initialHeader = -1;
        createFile.write((char*)&initialHeader, sizeof(int));
        createFile.close();

        appointmentFile.open("Appointments.dat", ios::in | ios::out | ios::binary);
    }

    // Create Secondary Index
    SecondaryIndex AppointmentDoctorIndex("AppointmentDoctorLL.dat");

    // Build indexes when the program starts
    doctorIdx.rebuild("Doctors.bin", 95, 15);
    appointmentIdx.rebuild("Appointments.dat", 60, 15);

    // Build Secondary Index
    // Doctor_ID starts at offset 45
    // Appointment_ID starts at offset 0
    AppointmentDoctorIndex.rebuild(
        "Appointments.dat",
        60,
        45,
        15,
        0,
        15
    );

    Doctor d;
    Appointments Ap;

    int choice;

    while (true) {

        cout << "\n========== Main Menu ==========\n";
        cout << "6. Add New Doctor\n";
        cout << "7. Add New Appointment\n";
        cout << "8. Update Doctor Name (Doctor ID)\n";
        cout << "9. Update Doctor ID (Key Update Test)\n";
        cout << "10. Update Appointment Date (Appointment ID)\n";
        cout << "11. Delete Appointment (Appointment ID)\n";
        cout << "12. Delete Doctor (Doctor ID)\n";
        cout << "13. Print Doctor Info (Doctor ID)\n";
        cout << "14. Print Appointment Info (Appointment ID)\n";
        cout << "15. Write Query\n";
        cout << "16. Exit\n";

        cout << "Enter your choice: ";
        cin >> choice;

        // Remove the remaining newline before using getline
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {

        case 6:
            d.addDoctor(file, doctorIdx);
            break;

        case 7:
            Ap.addAppointment(
                appointmentFile,
                doctorIdx,
                appointmentIdx,
                AppointmentDoctorIndex
            );
            break;

        case 8:
            d.updateDoctorName(file, doctorIdx);
            break;

        case 9:
            d.updateDoctorId(file, doctorIdx);
            break;

        case 10:
            Ap.updateAppointment_date(
                appointmentFile,
                appointmentIdx
            );
            break;

        case 11:
            Ap.delete_Appointment(
                appointmentFile,
                appointmentIdx,
                AppointmentDoctorIndex
            );
            break;

        case 12:
            d.deleteDoctor(file, doctorIdx);
            break;

        case 13:
            // Add your Doctor print function here
            break;

        case 14:
            Ap.print_Appointment_info(
                appointmentFile,
                appointmentIdx
            );
            break;

        case 15:
            // Write Query
            break;

        case 16:
            cout << "Exiting program...\n";
            file.close();
            appointmentFile.close();
            return 0;

        default:
            cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
