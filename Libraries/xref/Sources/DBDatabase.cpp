//
//  DBDatabase.cpp
//  darwintrace
//
//  Created by John Othwolo on 10/12/21.
//  Copyright © 2021 The PureDarwin Project. All rights reserved.
//

#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <DBDatabase.h>

namespace plugin {

DBDatabase::DBDatabase(const std::string& db) : m_db(db)
{
    std::atomic_init(&__nestedTransactions, 0);
}

int DBDatabase::beginTransaction(){
    std::atomic_fetch_add(&__nestedTransactions, 1);
    if (__nestedTransactions == 1) {
        try {
            m_db << "BEGIN";
            return 0;
        } catch (sqlite::errors::constraint& e) {
            std::cerr << e.get_code()        << ": "
                      << e.what()            << " during "
                      << std::quoted(e.get_sql()) << std::endl;
            return e.get_code();
        }
    } else {
        return SQLITE_OK;
    }
}

int DBDatabase::rollbackTransaction(){
    std::atomic_store(&__nestedTransactions, 0);
    try {
        m_db << "ROLLBACK";
        return 0;
    } catch (sqlite::errors::constraint& e) {
        std::cerr << e.get_code()        << ": "
                  << e.what()            << " during "
                  << std::quoted(e.get_sql()) << std::endl;
        return e.get_code();
    }
}

int DBDatabase::commitTransaction(){
    std::atomic_fetch_sub(&__nestedTransactions, 1);
    if (__nestedTransactions == 0) {
        try {
            m_db << "COMMIT";
            return 0;
        } catch (sqlite::errors::constraint& e) {
            std::cerr << e.get_code()        << ": "
                      << e.what()            << " during "
                      << std::quoted(e.get_sql()) << std::endl;
            return e.get_code();
        }
    } else {
        return SQLITE_OK;
    }
}

// just an alias
std::string
DBDatabase::getString(const std::string& build, const std::string& project, const std::string& property){
    return getProperty<std::string>(build, project, property);
}

// db manager routines

std::vector<std::string>
DBDatabase::getProjectNames(const std::string& build, bool inherit){
    std::vector<std::string> projects;
    
    m_db << "SELECT DISTINCT project FROM properties WHERE build=? AND project NOT NULL"
         << build
         >> [&](const std::string& proj){
             projects.push_back(proj);
         };

    if (!projects.empty()) {
        std::sort(projects.begin(), projects.end(), std::less<std::string>());
    }
    
    return projects;
}

int
DBDatabase::setGroupMembers(const std::string& build, const std::string& group, const std::vector<std::string>& members) {
    
    m_db << "DELETE FROM groups WHERE build=? AND name=?"
         << build
         << group;
    
    for (auto &member : members) {
        m_db <<"INSERT INTO groups (build,name,member) VALUES (?, ?, ?)"
             << build
             << group
             << member;
    }
    return 0;
}

void
DBDatabase::query(const std::string& query){
    m_db << query;
}

} // namespace plugin
