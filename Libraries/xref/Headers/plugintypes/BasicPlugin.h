//
//  BasicPlugin.h
//  pdbdriver
//
//  Created by John Othwolo on 6/22/21.
//  Copyright © 2021 The PureDarwin Project. All rights reserved.
//

#ifndef BasicPlugin_h
#define BasicPlugin_h

namespace plugin {

template<typename DataType> class BasicPlugin : public Plugin<DataType> {
public:
    BasicPlugin(std::string name){
        this->m_pluginType = PluginType::BasicType;
        this->m_name = name;
    }
   ~BasicPlugin(){}
    
    std::string usage() override { return ""; }
};

} // namespace plugin

#endif /* BasicPlugin_h */
