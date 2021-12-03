/*
 * Wazuh SYSINFO
 * Copyright (C) 2015-2021, Wazuh Inc.
 * April 16, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "sharedDefs.h"
#include "packageLinuxParserHelper.h"
#include "berkeleyRpmDbHelper.h"
#include "filesystemHelper.h"
#include "rpm.h"


void getRpmInfo(std::function<void(nlohmann::json&)> callback)
{
    if (!Utils::existsRegular(RPM_DATABASE)) {
        // We are probably using RPM >= 1.17 – get the packages from librpm.
        RPM rpm;
        for (const auto &p : rpm)
        {
           auto packageJson = PackageLinuxHelper::parseRpm(p);
            if (!packageJson.empty())
            {
                callback(packageJson);
            }
        }
    } else {
        BerkeleyRpmDBReader db {std::make_shared<BerkeleyDbWrapper>(RPM_DATABASE)};
        auto row = db.getNext();
        // Get the packages from the Berkeley DB.
        while (!row.empty())
        {
            auto package = PackageLinuxHelper::parseRpm(row);
            if (!package.empty())
            {
                callback(package);
            }
            row = db.getNext();
        }
    }
}

void getDpkgInfo(const std::string& fileName, std::function<void(nlohmann::json&)> callback)
{
    std::fstream file{fileName, std::ios_base::in};

    if (file.is_open())
    {
        while (file.good())
        {
            std::string line;
            std::vector<std::string> data;

            do
            {
                std::getline(file, line);

                if (line.front() == ' ') //additional info
                {
                    data.back() = data.back() + line + "\n";
                }
                else
                {
                    data.push_back(line + "\n");
                }
            }
            while (!line.empty()); //end of package item info

            auto packageInfo = PackageLinuxHelper::parseDpkg(data);

            if (!packageInfo.empty())
            {
                callback(packageInfo);
            }
        }
    }
}
