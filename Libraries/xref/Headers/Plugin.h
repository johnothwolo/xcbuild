//
//  Plugin.h
//  pdbdriver
//
//  Created by John Othwolo on 6/22/21.
//  Copyright © 2021 The PureDarwin Project. All rights reserved.
//

#ifndef Plugin_h
#define Plugin_h

#include <typeindex>
#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <any>

#include "DBDatabase.h"

#define __plugin_section    __attribute__ ((used, section ("__DATA,__plugins")))
#define __hidden            __attribute__ ((visibility("hidden")))
#define DECLARE_PLUGIN(type,varnom)                                 \
                __hidden type varnom;                               \
                __plugin_section void *__ptr_##varnom = &varnom

namespace plugin {

template <typename N> using DBOptional   = std::optional<N>;
template <typename N> using DBArray      = std::vector<N>;
template <typename K, typename V> using DBDictionary = std::unordered_map<K, V>;
template <typename T> using DBData       = std::vector<T>;
typedef std::string DBString;
typedef std::any DBType;

enum class PluginType : char {
    NullType =              0,
    BasicType =             1,
    PropertyType =          2,
    ProjectPropertyType =   3,
    BuildPropertyType =     4,
};

template<typename DataType>
class Plugin {

public:

    PluginType              getType(){ return m_pluginType; };
    std::string             getName(){ return m_name; };
    std::type_index         getDataType(){ return typeid(m_data); };
    DataType                getData(){ return m_data; };

    virtual std::string     usage(){
      if (m_pluginType == PluginType::ProjectPropertyType) return (std::string)"<project>";
      if (m_pluginType == PluginType::PropertyType) return (std::string)"[<project>]";
      else return (std::string)""; // m_pluginType == PluginType::BuildPropertyType
    };
    
    
    virtual int run(DBDatabase &db, const std::string& build, const std::vector<std::string>& argv){
        std::string project;
        std::cout << "[INFO]: Defualt Run method running for plugin: " << m_name << std::endl;

        // PluginType::ProjectPropertyType must have project argument,
        // PluginType::BuildPropertyType must not have project argument,
        // PluginType::PropertyType may have project argument.
        if ((m_pluginType == PluginType::ProjectPropertyType && argv.size() != 1) ||
            ((m_pluginType == PluginType::BuildPropertyType ||
              m_pluginType == PluginType::PropertyType) && !argv.empty()))
        {
            std::cout << "<unknown property type>" << std::endl;
            return -1;
        }

        if(typeid(DataType) == typeid(DBString)){
          std::cout << db.getString(build, project, m_name) << std::endl;
        } else if (typeid(DataType) == typeid(DBArray<DBString>)){
            auto values = db.getVectorProp<std::string>(build, project, m_name);
            // kDBPluginPropertyType: if no value in project, look in build.
//            if (values.empty())
//                values = db.getVectorProp<std::string>(build, project, m_name);
            // print values
            for (auto &value : values)
                std::cout << value << std::endl;
        } else {
          std::cerr << "internal error: no default handler for \""
                    << ((std::type_index)typeid(DataType)).name()
                    <<"\" type" << std::endl;
            return -1;
        }
        
        return 0;
    }

protected:
    std::string     m_name;
    DataType        m_data;
    PluginType      m_pluginType;
};

} // namespace plugin

/* Include PluginTypes */

#include "plugintypes/BasicPlugin.h"
#include "plugintypes/PropertyPlugin.h"
#include "plugintypes/ProjectPropertyPlugin.h"
#include "plugintypes/BuildPropertyPlugin.h"


#endif /* Plugin_h */
