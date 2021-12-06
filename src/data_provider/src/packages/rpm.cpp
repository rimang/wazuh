#include "rpm.h"

#include <exception>
#include <map>
#include <stdexcept>
#include <vector>

// For O_RDONLY
#include <fcntl.h>


RPM::RPM()
{
    static bool initialized = false;
    if (!initialized) {
        rpmInitCrypto();
        rpmReadConfigFiles(nullptr, nullptr);
        initialized = true;
    }
}

std::string RPM::Iterator::getAttribute(rpmTag tag)
{
    if (headerGet(m_header, tag, m_dataContainer, HEADERGET_DEFAULT) == 0)
    {
        return "";
    }
    auto cstr = rpmtdGetString(m_dataContainer);
    if (!cstr)
    {
        return "";
    }
    return cstr;
}

uint64_t RPM::Iterator::getAttributeNumber(rpmTag tag)
{
    if (headerGet(m_header, tag, m_dataContainer, HEADERGET_DEFAULT) == 0)
    {
        return 0;
    }
    return rpmtdGetNumber(m_dataContainer);
}

const RPM::Iterator RPM::END_ITERATOR{true};

RPM::Iterator::Iterator(bool end)
: m_end{end}
{
    if (end) {
        return;
    }
    m_transactionSet = rpmtsCreate();
    if (nullptr == m_transactionSet)
    {
        throw std::runtime_error("rpmtsCreate failed");
    }
    if (rpmtsOpenDB(m_transactionSet, O_RDONLY))
    {
        throw std::runtime_error("rptsOpenDB failed");
    }
    if (rpmtsRun(m_transactionSet, NULL, 0))
    {
        throw std::runtime_error("rpmtsRun failed");
    }
    m_dataContainer = rpmtdNew();
    if (nullptr == m_dataContainer)
    {
        throw std::runtime_error("rpmtdNew failed");
    }
    m_matches = rpmtsInitIterator(m_transactionSet, RPMTAG_NAME, nullptr, 0);
    ++(*this);
}

RPM::Iterator::~Iterator()
{
    rpmtsFree(m_transactionSet);
    if (m_dataContainer) {
        rpmtdFree(m_dataContainer);
    }
    if (m_matches) {
        rpmdbFreeIterator(m_matches);
    }
}

void RPM::Iterator::operator++()
{
    m_header = rpmdbNextIterator(m_matches);
    if (nullptr == m_header)
    {
        m_end = true;
    }
}

RPM::Package RPM::Iterator::operator*()
{
    Package p;
    p.name = getAttribute(RPMTAG_NAME);
    p.version = getAttribute(RPMTAG_VERSION);
    p.release = getAttribute(RPMTAG_RELEASE);
    p.epoch = getAttribute(RPMTAG_EPOCH);
    p.summary = getAttribute(RPMTAG_SUMMARY);
    p.installTime = getAttribute(RPMTAG_INSTALLTIME);
    p.size = getAttributeNumber(RPMTAG_SIZE);
    p.vendor = getAttribute(RPMTAG_VENDOR);
    p.group = getAttribute(RPMTAG_GROUP);
    p.source = getAttribute(RPMTAG_SOURCE);
    p.architecture = getAttribute(RPMTAG_ARCH);
    p.description = getAttribute(RPMTAG_DESCRIPTION);
    return p;
}
