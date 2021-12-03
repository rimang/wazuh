#ifndef LIB_RPM_WRAPPER_H
#define LIB_RPM_WRAPPER_H

#include <string>
#include <vector>
#include <memory>

#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

// Abstracts librpm functions.
class RPM final
{
public:
    RPM();
    struct Package
    {
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

    struct Iterator {
        Iterator(bool end = false);
        virtual ~Iterator();
        void operator++();
        Package operator*();
        bool operator!=(const Iterator &other)
        {
            return m_end != other.m_end;
        };
    private:
        std::string getAttribute(rpmTag tag);
        bool m_end = false;
        rpmts m_transactionSet = nullptr;
        rpmdbMatchIterator m_matches = nullptr;
        rpmtd m_dataContainer = nullptr;
        Header m_header = nullptr;
        friend class RPM;
    };
    static const Iterator END_ITERATOR;
    Iterator begin()
    {
        return Iterator{};
    }
    const Iterator &end()
    {
        return END_ITERATOR;
    }
};

#endif // LIB_RPM_WRAPPER_H
