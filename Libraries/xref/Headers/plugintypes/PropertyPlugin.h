//
//  PropertyPlugin.h
//  pdbdriver
//
//  Created by John Othwolo on 6/22/21.
//  Copyright © 2021 The PureDarwin Project. All rights reserved.
//

#ifndef PropertyPlugin_h
#define PropertyPlugin_h

namespace plugin {

template<typename DataType> class PropertyPlugin : public Plugin<DataType> {
public:
    PropertyPlugin(std::string name){
        this->m_pluginType = PluginType::PropertyType;
        this->m_name = name;
    }
   ~PropertyPlugin(){}
    
    std::string usage() override { return "[<project>]"; }
};

} // namespace plugin

#endif /* PropertyPlugin_h */
