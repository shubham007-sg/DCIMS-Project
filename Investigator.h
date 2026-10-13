
#ifndef INVESTIGATOR_H
#define INVESTIGATOR_H

#include <string>
using namespace std;

class Investigator {
private:
    string investigatorId;
    string name;
    string rank;
    string department;
    string contact;

public:
    Investigator() {}

    Investigator(string id, string n, string r,
                 string dept, string phone) {
        investigatorId = id;
        name = n;
        rank = r;
        department = dept;
        contact = phone;
    }

    string getInvestigatorId() const {
        return investigatorId;
    }

    string getName() const {
        return name;
    }

    string getRank() const {
        return rank;
    }

    string getDepartment() const {
        return department;
    }

    string getContact() const {
        return contact;
    }

    void setRank(string newRank) {
        rank = newRank;
    }

    void setDepartment(string newDepartment) {
        department = newDepartment;
    }
};

#endif
