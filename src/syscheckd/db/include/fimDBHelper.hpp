/*
 * Wazuh Syscheckd
 * Copyright (C) 2015-2021, Wazuh Inc.
 * September 23, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _FIMDBHELPER_HPP
#define _FIMDBHELPER_HPP
#include "fimDB.hpp"
#include "dbItem.hpp"

namespace FIMDBHelper
{
    template<typename T>
#ifndef WIN32
    /**
    * @brief Init the FIM DB instance.
    *
    * @param sync_interval Interval when the sync is performed-
    * @param file_limit Max number of files.
    * @param sync_callback Synchronization callback.
    * @param logCallback Logging callback.
    */
    void initDB(unsigned int sync_interval, unsigned int file_limit,
                            fim_sync_callback_t sync_callback, logging_callback_t logCallback,
                            std::shared_ptr<DBSync>handler_DBSync, std::shared_ptr<RemoteSync>handler_RSync)
    {
        T::getInstance().init(sync_interval, file_limit, sync_callback, logCallback, handler_DBSync, handler_RSync);
    }
#else
    /**
    * @brief Init the FIM DB instance.
    *
    * @param sync_interval Interval when the sync is performed-
    * @param file_limit Max number of files.
    * @param registry_limit Max number of registries.
    * @param sync_callback Synchronization callback.
    * @param logCallback Logging callback.
    */
    void initDB(unsigned int sync_interval, unsigned int file_limit, unsigned int registry_limit,
                             fim_sync_callback_t sync_callback, logging_callback_t logCallback,
                             std::shared_ptr<DBSync>handler_DBSync, std::shared_ptr<RemoteSync>handler_RSync)
    {
        T::getInstance().init(sync_interval, file_limit, registry_limit, sync_callback, logCallback, handler_DBSync,
                              handler_RSync);
    }
#endif
    /**
    * @brief Convert a DBSync error to a FIM error
    *
    * @param error a int with a FIM error
    *
    * @return 0 on success, another value otherwise.
    */
    template<typename T>
    int queryError(const T & error)
    {
        switch (error)
        {
            case SUCCESS:
                return 0;
                break;
            case MAX_ROWS_ERROR:
                return -2;
                break;
            case DBSYNC_ERROR:
                return -1;
                break;
        }

        return SUCCESS;
    }

    /**
    * @brief Delete a row from a table
    *
    * @param tableName a string with the table name
    * @param query a json with a filter to delete an element to the database
    *
    * @return 0 on success, another value otherwise.
    */
    template<typename T>
    int removeFromDB(const std::string & tableName, const nlohmann::json & filter)
    {
        const auto deleteJsonStatement = R"({
                                                "table": "",
                                                "query": {
                                                    "data":[
                                                    {
                                                    }],
                                                    "where_filter_opt":""
                                                }
        })";
        auto deleteJson = nlohmann::json::parse(deleteJsonStatement);
        deleteJson["table"] = tableName;
        deleteJson["query"]["data"] = {filter};

        return queryError(T::getInstance().removeItem(deleteJson));
    }
    /**
    * @brief Get count of all entries in a table
    *
    * @param tableName a string with the table name
    * @param count a int with count values
    * @param query a json to modify the query
    *
    * @return amount of entries on success, 0 otherwise.
    */
    template<typename T>
    int getCount(const std::string & tableName, int & count, const nlohmann::json & query)
    {
        nlohmann::json countQuery;
        if (!query.empty())
        {
            countQuery = query;
        }
        else
        {
            const auto countQueryStatement = R"({
                                                    "table":"",
                                                    "query":{"column_list":["count(*) AS count"],
                                                    "row_filter":"",
                                                    "distinct_opt":false,
                                                    "order_by_opt":"",
                                                    "count_opt":100}
            })";
            countQuery = nlohmann::json::parse(countQueryStatement);
            countQuery["table"] = tableName;
        }
        auto callback {
            [&count](ReturnTypeCallback type, const nlohmann::json & jsonResult)
            {
                if(type == ReturnTypeCallback::SELECTED)
                {
                   count = jsonResult["query"]["count"];
                }
            }
        };

        return queryError(T::getInstance().executeQuery(countQuery, callback));
    }

    /**
    * @brief Insert a new row from a table.
    *
    * @param tableName a string with the table name
    * @param item a RegistryKey, RegistryValue or File with their parameters
    *
    * @return 0 on success, another value otherwise.
    */
    template<typename T>
    int insertItem(const std::string & tableName, const nlohmann::json & item)
    {
        const auto insertStatement = R"(
                                            {
                                                "table": "",
                                                "data":[
                                                    {
                                                    }
                                                ]
                                            }
        )";
        auto insert =  nlohmann::json::parse(insertStatement);
        insert["table"] = tableName;
        insert["data"] = {item};

        return queryError(T::getInstance().insertItem(insert));
    }

    /**
    * @brief Update a row from a table.
    *
    * @param tableName a string with the table name
    * @param item a RegistryKey, RegistryValue or File with their parameters
    *
    * @return 0 on success, another value otherwise.
    */
    template<typename T>
    int updateItem(const std::string & tableName, const nlohmann::json & item)
    {
        const auto updateStatement = R"(
                                            {
                                                "table": "",
                                                "data":[
                                                    {
                                                    }
                                                ]
                                            }
        )";
        auto update = nlohmann::json::parse(updateStatement);
        update["table"] = tableName;
        update["data"] = {item};
        bool error = false;
        auto callback {
            [&error](ReturnTypeCallback type, const nlohmann::json &)
            {
                if (type == ReturnTypeCallback::DB_ERROR)
                {
                    error = true;
                }
            }
        };
        if(error)
        {
            return static_cast<int>(dbQueryResult::DBSYNC_ERROR);
        }

        return queryError(T::getInstance().updateItem(update, callback));
    }

    /**
    * @brief Get a item from a query
    *
    * @param item a json object where will be saved the query information
    * @param query a json with a query to the database
    *
    * @return 0 on success, another value otherwise.
    */
    template<typename T>
    int getDBItem(nlohmann::json & item, const nlohmann::json & query)
    {
        auto callback {
            [&item](ReturnTypeCallback type, const nlohmann::json & jsonResult)
            {
                if (type == ReturnTypeCallback::SELECTED)
                {
                    item = jsonResult["query"];
                }
            }
        };

        return queryError(T::getInstance().executeQuery(query, callback));
    }

    /**
    * @brief Create a new query to database
    *
    * @param tableName a string with table name
    * @param columnList an array with the column list
    * @param filter a string with a filter to a table
    * @param order a string with the column to order in result
    *
    * @return a nlohmann::json with a database query
    */
    nlohmann::json dbQuery(const std::string & tableName, const nlohmann::json & columnList, const std::string & filter,
                           const std::string & order)
    {
        nlohmann::json query;
        query["table"] = tableName;
        query["query"]["column_list"] = columnList["column_list"];
        query["query"]["row_filter"] = filter;
        query["query"]["distinct_opt"] = false;
        query["query"]["order_by_opt"] = order;
        query["query"]["count_opt"] = 100;

        return query;
    }
}

#endif //_FIMDBHELPER_H
