#include "QueryEngine.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

namespace {

const int HEADER_SIZE = sizeof(int);
const int DOCTOR_RECORD_SIZE = 95;
const int APPOINTMENT_RECORD_SIZE = 60;

enum class AccessPath {
    PrimaryIndex,
    SecondaryIndex,
    FullFileScan
};

string trimText(const string& text) {
    size_t first = 0;
    while (first < text.size() && isspace((unsigned char)text[first])) {
        ++first;
    }

    size_t last = text.size();
    while (last > first && isspace((unsigned char)text[last - 1])) {
        --last;
    }

    return text.substr(first, last - first);
}

string cleanField(const char* buffer, int length) {
    string value(buffer, length);

    while (!value.empty() &&
           (value.back() == ' ' || value.back() == '\0')) {
        value.pop_back();
    }

    return trimText(value);
}

string toLowerCase(string value) {
    for (char& ch : value) {
        ch = (char)tolower((unsigned char)ch);
    }
    return value;
}

bool isWordCharacter(char ch) {
    return isalnum((unsigned char)ch) || ch == '_';
}

size_t findKeyword(const string& text, const string& keyword,
                   size_t startPosition = 0) {
    string lowerText = toLowerCase(text);
    string lowerKeyword = toLowerCase(keyword);

    size_t position = lowerText.find(lowerKeyword, startPosition);

    while (position != string::npos) {
        bool leftBoundary =
            (position == 0 || !isWordCharacter(text[position - 1]));

        size_t afterKeyword = position + keyword.size();
        bool rightBoundary =
            (afterKeyword >= text.size() ||
             !isWordCharacter(text[afterKeyword]));

        if (leftBoundary && rightBoundary) {
            return position;
        }

        position = lowerText.find(lowerKeyword, position + 1);
    }

    return string::npos;
}

string normalizeField(const string& tableName, const string& fieldName) {
    string field = toLowerCase(trimText(fieldName));

    if (tableName == "Doctors") {
        if (field == "id" || field == "doctor id") return "ID";
        if (field == "name" || field == "doctor name") return "Name";
        if (field == "specialty" || field == "doctor specialty") return "Specialty";
        if (field == "address" || field == "doctor address") return "Address";
    }
    else if (tableName == "Appointments") {
        if (field == "appointment id" || field == "appointment_id")
            return "Appointment_ID";
        if (field == "appointment date" || field == "appointment_date")
            return "Appointment_Date";
        if (field == "doctor id" || field == "doctor_id")
            return "Doctor_ID";
    }

    return "";
}

vector<string> splitConditions(const string& text) {
    vector<string> conditions;
    string current;
    char activeQuote = '\0';

    for (size_t i = 0; i < text.size(); ++i) {
        char ch = text[i];

        if (ch == '\'' || ch == '"') {
            if (activeQuote == '\0') {
                activeQuote = ch;
            }
            else if (activeQuote == ch) {
                activeQuote = '\0';
            }
        }

        bool isAnd = false;
        if (activeQuote == '\0' && i + 3 <= text.size() &&
            toLowerCase(text.substr(i, 3)) == "and") {

            bool leftBoundary =
                (i == 0 || isspace((unsigned char)text[i - 1]));
            bool rightBoundary =
                (i + 3 == text.size() ||
                 isspace((unsigned char)text[i + 3]));

            isAnd = leftBoundary && rightBoundary;
        }

        if (isAnd) {
            string condition = trimText(current);
            if (!condition.empty()) conditions.push_back(condition);
            current.clear();
            i += 2;
        }
        else {
            current += ch;
        }
    }

    current = trimText(current);
    if (!current.empty()) conditions.push_back(current);

    return conditions;
}

vector<string> splitSelectedFields(const string& text) {
    vector<string> fields;
    string current;
    stringstream ss(text);

    while (getline(ss, current, ',')) {
        current = trimText(current);
        if (!current.empty()) fields.push_back(current);
    }

    return fields;
}

AccessPath chooseAccessPath(const string& tableName,
                            const string& fieldName) {
    if (tableName == "Doctors") {
        if (fieldName == "ID") return AccessPath::PrimaryIndex;
        if (fieldName == "Name" || fieldName == "Specialty")
            return AccessPath::SecondaryIndex;
        return AccessPath::FullFileScan;
    }

    if (tableName == "Appointments") {
        if (fieldName == "Appointment_ID")
            return AccessPath::PrimaryIndex;
        if (fieldName == "Doctor_ID")
            return AccessPath::SecondaryIndex;
        return AccessPath::FullFileScan;
    }

    return AccessPath::FullFileScan;
}

string accessPathName(AccessPath path) {
    if (path == AccessPath::PrimaryIndex) return "Primary Index";
    if (path == AccessPath::SecondaryIndex) return "Secondary Index";
    return "Full File Scan";
}

fstream& getTableFile(const string& tableName,
                      fstream& doctorFile,
                      fstream& appointmentsFile) {
    if (tableName == "Doctors") return doctorFile;
    return appointmentsFile;
}

PrimaryIndex& getPrimaryIndex(const string& tableName,
                              PrimaryIndex& doctorIdx,
                              PrimaryIndex& appointmentsIdx) {
    if (tableName == "Doctors") return doctorIdx;
    return appointmentsIdx;
}

bool getFieldLayout(const string& tableName, const string& fieldName,
                    int& offset, int& length) {
    if (tableName == "Doctors") {
        if (fieldName == "ID") {
            offset = 0; length = 15; return true;
        }
        if (fieldName == "Name") {
            offset = 15; length = 30; return true;
        }
        if (fieldName == "Specialty") {
            offset = 45; length = 20; return true;
        }
        if (fieldName == "Address") {
            offset = 65; length = 30; return true;
        }
    }
    else if (tableName == "Appointments") {
        if (fieldName == "Appointment_ID") {
            offset = 0; length = 15; return true;
        }
        if (fieldName == "Appointment_Date") {
            offset = 15; length = 30; return true;
        }
        if (fieldName == "Doctor_ID") {
            offset = 45; length = 15; return true;
        }
    }

    return false;
}

int getRecordSize(const string& tableName) {
    return (tableName == "Doctors")
        ? DOCTOR_RECORD_SIZE
        : APPOINTMENT_RECORD_SIZE;
}

vector<string> getAllKeys(const string& tableName,
                          fstream& doctorFile,
                          fstream& appointmentsFile) {
    vector<string> keys;
    fstream& file = getTableFile(tableName, doctorFile, appointmentsFile);
    const int recordSize = getRecordSize(tableName);

    if (!file.is_open()) return keys;

    file.clear();
    file.seekg(0, ios::end);
    streamoff fileSize = file.tellg();

    if (fileSize < HEADER_SIZE) return keys;

    vector<char> record(recordSize);

    for (streamoff offset = HEADER_SIZE;
         offset + recordSize <= fileSize;
         offset += recordSize) {

        file.clear();
        file.seekg(offset, ios::beg);
        file.read(record.data(), recordSize);

        if (!file) break;
        if (record[0] == '*') continue;

        string key = cleanField(record.data(), 15);
        if (!key.empty()) keys.push_back(key);
    }

    return keys;
}

string getValueFromRecord(const vector<char>& record,
                          const string& tableName,
                          const string& fieldName) {
    int offset = 0;
    int length = 0;

    if (!getFieldLayout(tableName, fieldName, offset, length)) {
        return "";
    }

    return cleanField(record.data() + offset, length);
}

vector<string> executeCondition(
    const string& tableName,
    const string& fieldName,
    const string& value,
    fstream& doctorFile,
    fstream& appointmentsFile,
    PrimaryIndex& doctorIdx,
    PrimaryIndex& appointmentsIdx,
    SecondaryIndex& docNameIdx,
    SecondaryIndex& docSpecIdx,
    SecondaryIndex& appDocIdIdx) {

    vector<string> results;
    AccessPath path = chooseAccessPath(tableName, fieldName);

    if (path == AccessPath::PrimaryIndex) {
        PrimaryIndex& index =
            getPrimaryIndex(tableName, doctorIdx, appointmentsIdx);

        if (index.getOffset(value) != -1) {
            results.push_back(value);
        }
    }
    else if (path == AccessPath::SecondaryIndex) {
        if (tableName == "Doctors" && fieldName == "Name") {
            results = docNameIdx.search(value);
        }
        else if (tableName == "Doctors" && fieldName == "Specialty") {
            results = docSpecIdx.search(value);
        }
        else if (tableName == "Appointments" && fieldName == "Doctor_ID") {
            results = appDocIdIdx.search(value);
        }
    }
    else {
        fstream& file =
            getTableFile(tableName, doctorFile, appointmentsFile);

        int fieldOffset = 0;
        int fieldLength = 0;
        getFieldLayout(tableName, fieldName, fieldOffset, fieldLength);

        const int recordSize = getRecordSize(tableName);
        vector<char> record(recordSize);

        if (!file.is_open()) return results;

        file.clear();
        file.seekg(0, ios::end);
        streamoff fileSize = file.tellg();

        for (streamoff offset = HEADER_SIZE;
             offset + recordSize <= fileSize;
             offset += recordSize) {

            file.clear();
            file.seekg(offset, ios::beg);
            file.read(record.data(), recordSize);

            if (!file) break;
            if (record[0] == '*') continue;

            string fieldValue = cleanField(
                record.data() + fieldOffset, fieldLength);

            if (fieldValue == value) {
                string primaryKey = cleanField(record.data(), 15);
                if (!primaryKey.empty()) results.push_back(primaryKey);
            }
        }
    }

    return results;
}

void displayResults(
    const string& tableName,
    const vector<string>& keys,
    const vector<string>& selectedFields,
    bool selectAll,
    fstream& doctorFile,
    fstream& appointmentsFile,
    PrimaryIndex& doctorIdx,
    PrimaryIndex& appointmentsIdx) {

    if (keys.empty()) {
        cout << "No records found.\n";
        return;
    }

    fstream& file = getTableFile(tableName, doctorFile, appointmentsFile);
    PrimaryIndex& primaryIndex =
        getPrimaryIndex(tableName, doctorIdx, appointmentsIdx);

    const int recordSize = getRecordSize(tableName);
    vector<char> record(recordSize);

    cout << "\n========== Query Results ==========\n";

    for (const string& key : keys) {
        int offset = primaryIndex.getOffset(key);

        if (offset == -1) continue;

        file.clear();
        file.seekg(offset, ios::beg);
        file.read(record.data(), recordSize);

        if (!file) continue;
        if (record[0] == '*') continue;

        if (selectAll) {
            if (tableName == "Doctors") {
                cout << "Doctor ID: "
                     << getValueFromRecord(record, tableName, "ID") << '\n';
                cout << "Doctor Name: "
                     << getValueFromRecord(record, tableName, "Name") << '\n';
                cout << "Doctor Specialty: "
                     << getValueFromRecord(record, tableName, "Specialty") << '\n';
                cout << "Doctor Address: "
                     << getValueFromRecord(record, tableName, "Address") << '\n';
            }
            else {
                cout << "Appointment ID: "
                     << getValueFromRecord(record, tableName, "Appointment_ID") << '\n';
                cout << "Appointment Date: "
                     << getValueFromRecord(record, tableName, "Appointment_Date") << '\n';
                cout << "Doctor ID: "
                     << getValueFromRecord(record, tableName, "Doctor_ID") << '\n';
            }
        }
        else {
            for (const string& field : selectedFields) {
                cout << field << ": "
                     << getValueFromRecord(record, tableName, field) << '\n';
            }
        }

        cout << "------------------------------\n";
    }
}

}

QueryEngine::QueryEngine(
    fstream& doctorFile,
    fstream& appointmentsFile,
    PrimaryIndex& doctorIdx,
    PrimaryIndex& appointmentsIdx,
    SecondaryIndex& docNameIdx,
    SecondaryIndex& docSpecIdx,
    SecondaryIndex& appDocIdIdx)
    : doctorFile(doctorFile),
      appointmentsFile(appointmentsFile),
      doctorIdx(doctorIdx),
      appointmentsIdx(appointmentsIdx),
      docNameIdx(docNameIdx),
      docSpecIdx(docSpecIdx),
      appDocIdIdx(appDocIdIdx) {
}

void QueryEngine::executeQuery(string query) {
    query = trimText(query);

    if (!query.empty() && query.back() == ';') {
        query.pop_back();
        query = trimText(query);
    }

    if (query.empty()) {
        cout << "Error: Query is empty.\n";
        return;
    }

    size_t fromPosition = findKeyword(query, "FROM");

    if (fromPosition == string::npos) {
        cout << "Error: Missing FROM keyword.\n";
        return;
    }

    string selectPart = trimText(query.substr(0, fromPosition));

    if (selectPart.size() < 6 ||
        toLowerCase(selectPart.substr(0, 6)) != "select" ||
        (selectPart.size() > 6 &&
         !isspace((unsigned char)selectPart[6]))) {
        cout << "Error: Query must start with SELECT.\n";
        return;
    }

    string projectionText = trimText(selectPart.substr(6));

    if (projectionText.empty()) {
        cout << "Error: Specify * / all or a field after SELECT.\n";
        return;
    }

    string afterFrom = trimText(query.substr(fromPosition + 4));
    stringstream tableStream(afterFrom);
    string tableName;
    tableStream >> tableName;

    string afterTable;
    getline(tableStream, afterTable);
    afterTable = trimText(afterTable);

    if (tableName.empty()) {
        cout << "Error: Missing table name after FROM.\n";
        return;
    }

    string lowerTable = toLowerCase(tableName);
    if (lowerTable == "doctors") {
        tableName = "Doctors";
    }
    else if (lowerTable == "appointments") {
        tableName = "Appointments";
    }
    else {
        cout << "Error: Unknown table '" << tableName << "'.\n";
        return;
    }

    bool selectAll =
        (toLowerCase(projectionText) == "all" || projectionText == "*");

    vector<string> selectedFields;

    if (!selectAll) {
        vector<string> requestedFields = splitSelectedFields(projectionText);

        if (requestedFields.empty()) {
            cout << "Error: No selected fields were provided.\n";
            return;
        }

        for (const string& requestedField : requestedFields) {
            string normalized = normalizeField(tableName, requestedField);

            if (normalized.empty()) {
                cout << "Error: Unknown field '" << requestedField
                     << "' in table '" << tableName << "'.\n";
                return;
            }

            selectedFields.push_back(normalized);
        }
    }

    vector<string> results;

    if (afterTable.empty()) {
        results = getAllKeys(tableName, doctorFile, appointmentsFile);
        displayResults(tableName, results, selectedFields, selectAll,
                       doctorFile, appointmentsFile,
                       doctorIdx, appointmentsIdx);
        return;
    }

    stringstream whereStream(afterTable);
    string whereWord;
    whereStream >> whereWord;

    if (toLowerCase(whereWord) != "where") {
        cout << "Error: Expected WHERE after the table name.\n";
        return;
    }

    string conditionsText;
    getline(whereStream, conditionsText);
    conditionsText = trimText(conditionsText);

    if (conditionsText.empty()) {
        cout << "Error: WHERE must be followed by a condition.\n";
        return;
    }

    vector<string> conditions = splitConditions(conditionsText);

    if (conditions.empty()) {
        cout << "Error: No valid WHERE condition was found.\n";
        return;
    }

    bool firstCondition = true;

    for (const string& condition : conditions) {
        size_t equalPosition = condition.find('=');

        if (equalPosition == string::npos) {
            cout << "Error: Invalid condition '" << condition
                 << "'. Expected Field = Value.\n";
            return;
        }

        string fieldText = trimText(condition.substr(0, equalPosition));
        string value = trimText(condition.substr(equalPosition + 1));

        if (!value.empty() && value.back() == ';') {
            value.pop_back();
            value = trimText(value);
        }

        if (value.size() >= 2 &&
            ((value.front() == '\'' && value.back() == '\'') ||
             (value.front() == '"' && value.back() == '"'))) {
            value = value.substr(1, value.size() - 2);
        }

        value = trimText(value);

        if (fieldText.empty() || value.empty()) {
            cout << "Error: Condition field and value cannot be empty.\n";
            return;
        }

        string fieldName = normalizeField(tableName, fieldText);

        if (fieldName.empty()) {
            cout << "Error: Unknown field '" << fieldText
                 << "' in table '" << tableName << "'.\n";
            return;
        }

        AccessPath path = chooseAccessPath(tableName, fieldName);

        cout << "Condition: " << fieldName << " = " << value << '\n';
        cout << "Access Path: " << accessPathName(path) << '\n';

        vector<string> currentResults = executeCondition(
            tableName,
            fieldName,
            value,
            doctorFile,
            appointmentsFile,
            doctorIdx,
            appointmentsIdx,
            docNameIdx,
            docSpecIdx,
            appDocIdIdx);

        if (firstCondition) {
            results = currentResults;
            firstCondition = false;
        }
        else {

            results = SecondaryIndex::intersect(results, currentResults);
        }

        if (results.empty()) break;
    }

    displayResults(tableName, results, selectedFields, selectAll,
                   doctorFile, appointmentsFile,
                   doctorIdx, appointmentsIdx);
}
