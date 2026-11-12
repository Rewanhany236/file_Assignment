#ifndef QUERYENGINE_H
#define QUERYENGINE_H

#include "PrimaryIndex.h"
#include "SecondaryIndex.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
class QueryEngine {
private:
    fstream& doctorFile;
    fstream& appointmentsFile;

    PrimaryIndex& doctorIdx;
    PrimaryIndex& appointmentsIdx;

    SecondaryIndex& docNameIdx;
    SecondaryIndex& docSpecIdx;
    SecondaryIndex& appDocIdIdx;

public:
    QueryEngine(
        fstream& doctorFile,
        fstream& appointmentsFile,
        PrimaryIndex& doctorIdx,
        PrimaryIndex& appointmentsIdx,
        SecondaryIndex& docNameIdx,
        SecondaryIndex& docSpecIdx,
        SecondaryIndex& appDocIdIdx
    );

    void executeQuery(string query);
};

#endif


