#ifndef LIB_RPM_WRAPPER_H
#define LIB_RPM_WRAPPER_H

#include <string>
#include <vector>

// Abstracts librpm functions.
class RPM final {
public:
    struct Package {
        std::string name;
        std::string version;
        std::string release;
        std::string epoch;
        std::string summary;
        std::string installTime;
        std::string size;
        std::string vendor;
        std::string group;
        std::string source;
        std::string architecture;
        std::string description;
    };
    RPM();
    virtual ~RPM();
    // Returns a list of all packages installed.
    // Throws std::runtime_error if any of the calls to librpm failed.
    std::vector<RPM::Package> packages();
};

#endif // LIB_RPM_WRAPPER_H