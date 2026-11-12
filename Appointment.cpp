#include "Appointment.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Appointments Ap;

void Appointments::addAppointment(
    fstream& file,
    PrimaryIndex& doctorIndex,
    PrimaryIndex& appointmentIndex,
    SecondaryIndex& AppointmentDoctorIndex
) {
    Appointments Ap{};

    cout << "Enter Appointment ID: ";
    cin.getline(Ap.Appointment_ID, 15);

    cout << "Enter Appointment Date: ";
    cin.getline(Ap.Appointment_Date, 30);

    cout << "Enter Doctor ID: ";
    cin.getline(Ap.Doctor_ID, 15);

    string doctorID = Ap.Doctor_ID;
    string appointmentID = Ap.Appointment_ID;

    // Check if Appointment ID already exists
    if (appointmentIndex.exists(appointmentID)) {
        cout << "Appointment ID already exists" << endl;
        return;
    }

    // Check if Doctor ID exists
    if (!doctorIndex.exists(doctorID)) {
        cout << "Doctor ID does not exist" << endl;
        return;
    }

    int list_header;

    file.clear();
    file.seekg(0, ios::beg);
    file.read((char*)&list_header, sizeof(int));

    if (!file) {
        cout << "Error reading file header" << endl;
        return;
    }

    int appointment_offset;

    // No deleted records: append at the end
    if (list_header == -1) {

        file.clear();
        file.seekp(0, ios::end);

        appointment_offset = (int)file.tellp();

        file.write((char*)&Ap, sizeof(Appointments));
    }
    else {

        int next_header;

        // Read the next deleted record offset
        file.clear();
        file.seekg(list_header + 1, ios::beg);
        file.read((char*)&next_header, sizeof(int));

        if (!file) {
            cout << "Error reading deleted record" << endl;
            return;
        }

        // Update the file header
        file.clear();
        file.seekp(0, ios::beg);
        file.write((char*)&next_header, sizeof(int));

        // Reuse the deleted record
        appointment_offset = list_header;

        file.seekp(appointment_offset, ios::beg);
        file.write((char*)&Ap, sizeof(Appointments));
    }

    if (!file) {
        cout << "Error writing appointment" << endl;
        return;
    }

    file.flush();

    // Add the new appointment to the Primary Index
    if (!appointmentIndex.insertEntry(appointmentID, appointment_offset)) {
        cout << "Error inserting appointment into index" << endl;
        return;
    }

    // Add the new appointment to the Secondary Index
    AppointmentDoctorIndex.insertEntry(doctorID, appointmentID);

    cout << "Appointment added successfully" << endl;
}


bool Appointments::delete_Appointment(
    fstream& file,
    PrimaryIndex& appointmentIndex,
    SecondaryIndex& AppointmentDoctorIndex
) {
    int list_header = -1;
    char c;

    string appointmentID;

    cout << "Enter Appointment ID: ";
    getline(cin, appointmentID);

    // Search for Appointment ID in the Primary Index
    int appointment_offset = appointmentIndex.getOffset(appointmentID);

    if (appointment_offset == -1) {
        cout << "Appointment not found" << endl;
        return false;
    }

    // Read current AVAIL LIST header
    file.clear();
    file.seekg(0, ios::beg);
    file.read((char*)&list_header, sizeof(int));

    if (!file) {
        cout << "Error reading file header" << endl;
        return false;
    }

    // Check if the record is already deleted
    file.clear();
    file.seekg(appointment_offset, ios::beg);
    file.get(c);

    if (!file) {
        cout << "Error reading appointment" << endl;
        return false;
    }

    if (c == delete_flag) {
        cout << "Record is already deleted" << endl;
        return false;
    }

    // Read appointment before deleting it
    Appointments AP{};

    file.clear();
    file.seekg(appointment_offset, ios::beg);
    file.read((char*)&AP, sizeof(Appointments));

    if (!file) {
        cout << "Error reading appointment" << endl;
        return false;
    }

    string doctorID = AP.Doctor_ID;

    // Mark the record as deleted
    file.clear();
    file.seekp(appointment_offset, ios::beg);
    file.put(delete_flag);

    // Store the previous list_header after '*'
    file.write((char*)&list_header, sizeof(int));

    if (!file) {
        cout << "Error deleting appointment" << endl;
        return false;
    }

    // Make this deleted record the new AVAIL LIST header
    file.clear();
    file.seekp(0, ios::beg);
    file.write((char*)&appointment_offset, sizeof(int));

    if (!file) {
        cout << "Error updating file header" << endl;
        return false;
    }

    file.flush();

    // Remove the deleted appointment from the Primary Index
    appointmentIndex.deleteEntry(appointmentID);

    // Remove the deleted appointment from the Secondary Index
    AppointmentDoctorIndex.deleteEntry(doctorID, appointmentID);

    cout << "Appointment deleted successfully" << endl;

    return true;
}


void Appointments::updateAppointment_date(
    fstream& file,
    PrimaryIndex& appointmentIndex
) {
    Appointments AP{};

    cout << "Enter Appointment ID: ";
    cin.getline(AP.Appointment_ID, 15);

    string appointmentID = AP.Appointment_ID;

    // Search using Appointment Primary Index
    int offset = appointmentIndex.getOffset(appointmentID);

    if (offset == -1) {
        cout << "Appointment not found" << endl;
        return;
    }

    // Check that the record is not deleted
    char c;

    file.clear();
    file.seekg(offset, ios::beg);
    file.get(c);

    if (!file || c == delete_flag) {
        cout << "Appointment not found" << endl;
        return;
    }

    cout << "Enter The Updated Appointment Date: ";
    cin.getline(AP.Appointment_Date, 30);

    // Go to the beginning of Appointment Date
    file.clear();
    file.seekp(offset + sizeof(AP.Appointment_ID), ios::beg);

    // Update the date in place
    file.write(
        AP.Appointment_Date,
        sizeof(AP.Appointment_Date)
    );

    if (!file) {
        cout << "Error updating appointment date" << endl;
        return;
    }

    file.flush();

    cout << "Appointment Date updated successfully" << endl;
}


void Appointments::print_Appointment_info(
    fstream& file,
    PrimaryIndex& appointmentIndex
) {
    string appointmentID;

    cout << "Enter Appointment ID: ";
    getline(cin, appointmentID);

    // Search using Appointment Primary Index
    int offset = appointmentIndex.getOffset(appointmentID);

    if (offset == -1) {
        cout << "Appointment not found" << endl;
        return;
    }

    char c;

    file.clear();
    file.seekg(offset, ios::beg);
    file.get(c);

    if (!file) {
        cout << "Error reading appointment" << endl;
        return;
    }

    if (c == delete_flag) {
        cout << "This appointment is deleted" << endl;
        return;
    }

    Appointments AP{};

    file.clear();
    file.seekg(offset, ios::beg);
    file.read((char*)&AP, sizeof(Appointments));

    if (!file) {
        cout << "Error reading appointment" << endl;
        return;
    }

    cout << "Appointment ID: " << AP.Appointment_ID << endl;
    cout << "Appointment Date: " << AP.Appointment_Date << endl;
    cout << "Doctor ID: " << AP.Doctor_ID << endl;
}
