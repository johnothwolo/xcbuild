//
//  BuildPropertyPlugin.h
//  pdbdriver
//
//  Created by John Othwolo on 6/22/21.
//  Copyright © 2021 The PureDarwin Project. All rights reserved.
//

#ifndef BuildPropertyPlugin_h
#define BuildPropertyPlugin_h

namespace plugin {

template<typename DataType> class BuildPropertyPlugin : public Plugin<DataType> {
public:
    BuildPropertyPlugin(std::string name){
        this->m_pluginType = PluginType::BuildPropertyType;
        this->m_name = name;
    }
   ~BuildPropertyPlugin(){}
        
    std::string usage() override { return "[<project>]"; }
};

} // namespace plugin

#endif /* BuildPropertyPlugin_h */
