//
//  xref.h
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/19/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//

#ifndef Xref_h
#define Xref_h

#include <string>
#include <memory>

namespace plugin {
void      PrintPluginList(void);
uintptr_t GetPluginStart(void);
uintptr_t GetPluginStop(void);
uintptr_t GetPluginWithName(const std::string& name);
} // namespace plugin

#endif /* Xref_h */
