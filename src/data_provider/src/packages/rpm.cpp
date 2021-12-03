#include "rpm.h"

#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

// For O_RDONLY
#include <fcntl.h>

RPM::RPM()
{
    static bool initialized = false;
    if (!initialized) {
        //rpmInitCrypto();
        rpmReadConfigFiles(nullptr, nullptr);
        initialized = true;
    }
}

RPM::~RPM()
{

}

struct tdDeleter
{
    void operator()(rpmtd td)
    {
        rpmtdFree(td);
    }
};

struct tsDeleter
{
    void operator()(rpmts ts)
    {
        rpmtsFree(ts);
    }
};
struct iteratorDeleter
{
    void operator()(rpmdbMatchIterator it)
    {
        rpmdbFreeIterator(it);
    }
};

using TagDataContainer = std::unique_ptr<rpmtd_s, tdDeleter>;
using TransactionSet = std::unique_ptr<rpmts_s, tsDeleter>;
using Iterator = std::unique_ptr<rpmdbMatchIterator_s, iteratorDeleter>;

static std::string getAttribute(const Header &header, rpmTag tag, const TagDataContainer &td)
{
    std::string str;
    if (headerGet(header, tag, td.get(), HEADERGET_DEFAULT) == 0)
    {
        return str;
    }

    switch (rpmTagGetClass(tag))
    {
    case RPM_NUMERIC_CLASS:
        str = std::to_string(rpmtdGetNumber(td.get()));
        break;
    case RPM_STRING_CLASS:
    {
        auto cstr = rpmtdGetString(td.get());
        if (cstr)
        {
            str = cstr;
        }
        break;
    }
    default:;
    }
    return str;
}


std::vector<RPM::Package> RPM::packages()
{
    std::vector<RPM::Package> result;
    TransactionSet transactionSet {rpmtsCreate()};
    if (transactionSet.get() == nullptr)
    {
        throw std::runtime_error("rptsCreate failed");
    }
    if (rpmtsOpenDB(transactionSet.get(), O_RDONLY))
    {
        throw std::runtime_error("rptsOpenDB failed");
    }
    if (rpmtsRun(transactionSet.get(), NULL, 0)) {
        throw std::runtime_error("rpmtsRun failed");
    }
    Iterator matches{rpmtsInitIterator(transactionSet.get(), RPMTAG_NAME, nullptr, 0)};
    Header h;
    while ((h = rpmdbNextIterator(matches.get())) != nullptr)
    {
        TagDataContainer td{rpmtdNew()};
        Package p;
        p.name = getAttribute(h, RPMTAG_NAME, td);
        p.version = getAttribute(h, RPMTAG_VERSION, td);
        p.release = getAttribute(h, RPMTAG_RELEASE, td);
        p.epoch = getAttribute(h, RPMTAG_EPOCH, td);
        p.summary = getAttribute(h, RPMTAG_SUMMARY, td);
        p.installTime = getAttribute(h, RPMTAG_INSTALLTIME, td);
        p.size = getAttribute(h, RPMTAG_SIZE, td);
        p.vendor = getAttribute(h, RPMTAG_VENDOR, td);
        p.group = getAttribute(h, RPMTAG_GROUP, td);
        p.source = getAttribute(h, RPMTAG_SOURCE, td);
        p.architecture = getAttribute(h, RPMTAG_ARCH, td);
        p.description = getAttribute(h, RPMTAG_DESCRIPTION, td);
        result.push_back(std::move(p));
    }

    return result;
}