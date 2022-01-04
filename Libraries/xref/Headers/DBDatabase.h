//
//  DBDatabase.h
//  libxref
//
//  Created by John Othwolo on 5/18/21.
//  Copyright © 2021 The PureDarwin Project. All rights reserved.
//

#ifndef DBManager_hpp
#define DBManager_hpp

#include <SQLiteModern.h>
#include <any>
#include <sqlite3.h>
#include <string>
#include <unordered_map>

namespace plugin {

typedef int(*sqlite_callback_t)(void*,int,char**,char**);

class DBDatabase {

public:

    DBDatabase(const std::string& db);

  int beginTransaction();
    int rollbackTransaction();
    int commitTransaction();
    
    static DBDatabase& FromSourcePlist();

    template<typename T>
    T query(const std::string& query);
    template<typename ElemtTyp>
    std::vector<ElemtTyp> queryArray(const std::string& query);
    void query(const std::string& query);

    int lastRowId(){ return (int)m_db.last_insert_rowid(); };

    template<typename T>
    T getProperty(const std::string& build, const std::string& project, const std::string& property);
    template <typename ElementType>
    std::vector<ElementType>getVectorProp(const std::string& build, const std::string& project, const std::string& property);
    template <typename K, typename V>
    std::unordered_map<K,V>getDictionaryProp(const std::string& build, const std::string& project, const std::string& property);

    std::string getString(const std::string& build, const std::string& project, const std::string& property);
    // db manager routines

    std::vector<std::string> getProjectNames(const std::string& build, bool inherit);
    
    template <typename T>
    std::vector<T> copyGroupMembers(const std::string& build, const std::string& group);
    int setGroupMembers(const std::string& build, const std::string& group, const std::vector<std::string>& members);
private:
    int db_exec(sqlite_callback_t callback,
                 void *context,
                 va_list args,
                 char *format,
                 char**err);

    
    std::atomic_int __nestedTransactions;
    sqlite::database m_db;
};


// Templates
template<typename T> T
DBDatabase::getProperty(const std::string& build, const std::string& project, const std::string& property){
    T result;
    if ((typeid(T) == typeid(std::string)) || (typeid(T) == typeid(std::vector<char>))) {
        m_db << "SELECT value FROM properties WHERE property=? AND build=? AND project=?"
             << property
             << build
             << (project.empty() ? "NULL" : project)
             >>
          [&](std::optional<T> value){
            result = value.template value_or(result);
          };
    }
    return result;
}

// array
template <typename ElementType> std::vector<ElementType>
DBDatabase::getVectorProp(const std::string& build, const std::string& project, const std::string& property){
    std::vector<ElementType> result;
    std::string query = "SELECT value FROM properties WHERE "
                        "property=? AND "
                        "build=? AND " +
                        (project.empty() ? "project is NULL"
                                         : (std::string) ("project="+project)) +
                        " "
                        "ORDER BY key";
    m_db << query
         << property
         << build
    >> [&](ElementType elem){
            result.push_back(elem);
    };
    return result;
}

// dictionary
template <typename K, typename V> std::unordered_map<K,V>
DBDatabase::getDictionaryProp(const std::string& build, const std::string& project, const std::string& property){
    std::unordered_map<K, V> result;
    m_db << "SELECT DISTINCT key,value FROM properties WHERE property=? "
            "AND build=? AND project=? ORDER BY key"
         << property
         << build
         << (project.empty() ? "NULL" : project)
    >> [&](K key, V val){
        result.insert_or_assign(key, val);
    };
    return result;
}

template <typename T> std::vector<T>
DBDatabase::copyGroupMembers(const std::string& build, const std::string& group) {
  std::vector<T> result;

  m_db << "SELECT DISTINCT member FROM groups WHERE build=? AND name=? ORDER BY member"
       << build
       << group >> [&](T elem){
        result.push_back(elem);
      };

  return result;
}


template<typename Typ> Typ
DBDatabase::query(const std::string& query){
    Typ result;
    m_db << query >> result;
    return result;
}

template<typename ElemtTyp> std::vector<ElemtTyp>
DBDatabase::queryArray(const std::string& query){
    std::vector<ElemtTyp> result;
    m_db << query >> [&](ElemtTyp elem){
        result.push_back(elem);
    };
    return result;
}

} // namespace plugin

#endif /* DBManager_hpp */
