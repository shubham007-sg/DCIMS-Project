
#ifndef CASE_H
#define CASE_H

#include <string>
using namespace std;

class Case {
private:
    string caseId;
    string crimeType;
    string location;
    string status;
    string priority;
    string description;

public:
    Case() {
        status = "Pending";
        priority = "Medium";
    }

    Case(string id, string type, string loc,
         string pr, string desc) {
        caseId = id;
        crimeType = type;
        location = loc;
        priority = pr;
        description = desc;
        status = "Pending";
    }

    string getCaseId() const {
        return caseId;
    }

    string getCrimeType() const {
        return crimeType;
    }

    string getLocation() const {
        return location;
    }

    string getStatus() const {
        return status;
    }

    string getPriority() const {
        return priority;
    }

    string getDescription() const {
        return description;
    }

    void setStatus(string newStatus) {
        status = newStatus;
    }

    void setPriority(string newPriority) {
        priority = newPriority;
    }
};

#endif
