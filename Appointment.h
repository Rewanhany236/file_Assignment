
#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <iostream>
#include <fstream>
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"
using namespace std;

const char delete_flag = '*';
const int Header = sizeof(int);

class Appointments {
    char Appointment_ID[15];
    char Appointment_Date[30];
    char Doctor_ID[15];

public:
    void addAppointment(
        fstream& file,
        PrimaryIndex& doctorIndex,
        PrimaryIndex& appointmentIndex,
        SecondaryIndex& AppointmentDoctorIndex
    );

    bool delete_Appointment(
        fstream& file,
        PrimaryIndex& appointmentIndex,
        SecondaryIndex& AppointmentDoctorIndex
    );

    void updateAppointment_date(
        fstream& file,
        PrimaryIndex& appointmentIndex
    );

    void print_Appointment_info(
        fstream& file,
        PrimaryIndex& appointmentIndex
    );
};

#endif
