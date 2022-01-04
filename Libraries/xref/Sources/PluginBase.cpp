//
//  PluginBase.cpp
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/12/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//


#include <cstdio>
#include <stdexcept>
#include <iostream>

#include <csignal>
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
#include <mach-o/loader.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>

#include "Plugin.h"
#include "Xref.h"

#if 0
namespace plugin {

extern uintptr_t start_plugins __asm("section$start$__DATA$__plugins") __hidden;
extern uintptr_t end_plugins  __asm("section$end$__DATA$__plugins") __hidden;
const u_long section_sz = end_plugins - start_plugins;

uintptr_t GetPluginWithName(const std::string& name){
    std::cout << start_plugins << std::endl;
    std::cout << end_plugins << std::endl;
    std::cout << section_sz << std::endl;
    for (int i = 0; i < section_sz; i++) {
        auto plugin = reinterpret_cast<Plugin<std::any>*>((&start_plugins)[i]);
        std::cout << plugin->getName() << std::endl;
        if (plugin->getName() == name) {
            return (uintptr_t) plugin;
        }
    }
    std::cout << name << " is not a command" << std::endl;
    return (uintptr_t) nullptr;
}

void PrintPluginList(){
    for (int i = 0; i < section_sz; i++) {
        auto plugin = reinterpret_cast<Plugin<std::any>*>((&start_plugins)[i]);
        std::cout << "Found Plugin: " << plugin->getName() << std::endl;
    }
}

uintptr_t GetPluginStart(){
    return start_plugins;
}

uintptr_t GetPluginStop(){
    return end_plugins;
}

} // namespace plugin
#endif

namespace plugin {


static __inline __attribute__((__const__)) void **
_plugin_begin(const struct mach_header_64 *_header, const std::string& _set);
static __inline __attribute__((__const__)) void **
_plugin_end(const struct mach_header_64 *_header, const std::string& _set);

static __inline void **
_plugin_begin(const struct mach_header_64 *_header, const std::string& _set)
{
  void *_set_begin;
  unsigned long _size;

  _set_begin = getsectdatafromheader_64(_header,
                                     "__DATA",
                                     _set.c_str(),
                                     reinterpret_cast<uint64_t *>(&_size));
//  std::cout << (uintptr_t)_set_begin << std::endl;

  return (void **) _set_begin;
}

static __inline void **
_plugin_end(const struct mach_header_64 *_header, const std::string& _set)
{
  void *_set_begin;
  unsigned long _size;

  _set_begin = getsectdatafromheader_64(_header,
                                     "__DATA",
                                     _set.c_str(),
                                     reinterpret_cast<uint64_t *>(&_size));

  return (void **) ((uintptr_t) _set_begin + _size);
}

uintptr_t GetPluginWithName(const std::string& name){
  void** start = _plugin_begin(&_mh_dylib_header, "__plugins");
  void** end = _plugin_end(&_mh_dylib_header, "__plugins");
  u_long section_sz = end - start;

//  std::cout << "Plugin start: " << start << std::endl;
//  std::cout << "Plugin end: " << end << std::endl;
//  std::cout << "section size: " << section_sz << std::endl;
//  std::cout << "---------------------------------------"  << std::endl;

  for (int i = 0; i < section_sz; i++) {
    auto ptr = (void**) ((uintptr_t)&_mh_dylib_header + (uintptr_t)start);
//    std::cout << "Ptr = " << std::hex << ptr << std::endl;

    auto plugin = reinterpret_cast<Plugin<std::any>*>(ptr[i]);
    if (plugin->getName() == name) {
//      std::cout << "Found plugin: " << plugin->getName() << std::endl;
      return (uintptr_t) plugin;
    }
//    std::cout << plugin->getName() << " doesn't match "  << name << std::endl;
  }
//  std::cout << name << " is not a command" << std::endl;
  return (uintptr_t) nullptr;
}

void PrintPluginList(){
  auto start = _plugin_begin(&_mh_dylib_header, "__plugins");
  auto end = _plugin_end(&_mh_dylib_header, "__plugins");
  u_long section_sz = end - start;

//  std::cout << section_sz << std::endl;

  for (int i = 0; i < section_sz; i++) {
    auto ptr = (void**) ((uintptr_t)&_mh_dylib_header + (uintptr_t)start);
//    std::cout << "Ptr = " << std::hex << ptr << std::endl;
    auto plugin = reinterpret_cast<Plugin<std::any>*>(ptr[i]);
    std::cerr << "         " << plugin->getName()
              << " " << plugin->usage() << std::endl;
  }
}

} // namespace plugin