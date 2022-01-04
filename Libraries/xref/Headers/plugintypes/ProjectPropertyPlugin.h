//
//  ProjectPropertyPlugin.h
//  pdbdriver
//
//  Created by John Othwolo on 6/22/21.
//  Copyright © 2021 The PureDarwin Project. All rights reserved.
//

#ifndef ProjectPropertyPlugin_h
#define ProjectPropertyPlugin_h

namespace plugin {

template<typename DataType> class ProjectPropertyPlugin : public Plugin<DataType> {
public:
    ProjectPropertyPlugin(std::string name){
        this->m_pluginType = PluginType::ProjectPropertyType;
        this->m_name = name;
    }
   ~ProjectPropertyPlugin(){}
    
    std::string usage() override { return "<project>"; }
};

} // namespace plugin

#endif /* ProjectPropertyPlugin_h */
