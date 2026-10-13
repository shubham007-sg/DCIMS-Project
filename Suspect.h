
#ifndef SUSPECT_H
#define SUSPECT_H

#include <string>
using namespace std;

class Suspect {
private:
    string suspectId;
    string name;
    int age;
    string address;
    string caseId;
    string status;

public:
    Suspect() {
        age = 0;
        status = "Under Investigation";
    }

    Suspect(string id, string n, int a,
            string addr, string cId) {
        suspectId = id;
        name = n;
        age = a;
        address = addr;
        caseId = cId;
        status = "Under Investigation";
    }

    string getSuspectId() const {
        return suspectId;
    }

    string getName() const {
        return name;
    }

    int getAge() const {
        return age;
    }

    string getAddress() const {
        return address;
    }

    string getCaseId() const {
        return caseId;
    }

    string getStatus() const {
        return status;
    }

    void setStatus(string newStatus) {
        status = newStatus;
    }
};

#endif
