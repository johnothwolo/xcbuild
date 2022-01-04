//
//  Receipt.cpp
//  darwibuild
//
//  Created by John Othwolo on 12/8/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//

#include <pdbuild/Receipt.h>
#include <pdbuild/Options.h>

#include <pdbuild/Usage.h>
#include <libutil/FSUtil.h>
#include <process/PDBContext.h>
#include <libutil/Filesystem.h>
#include <Xref.h>
#include <Plugin.h>
#include <fstream>
#include <sstream>

#include <cstdio>
#include <dirent.h>
#include <iostream>
#include <libutil/DefaultFilesystem.h>
#include <unistd.h>
#include <any>

using namespace plugin;
using pdbuild::Receipt;
using ReceiptType = pdbuild::Receipt::ReceiptType;
using pdbuild::Usage;
using libutil::FSUtil;
using std::endl;

/* Create new receipt, or retrieve existing receipt */
Receipt
Receipt::GetReceipt(process::PDBContext *context,
                    libutil::Filesystem *filesystem,
                    Options const &options,
                    ReceiptType type)
{
    return Receipt{context, filesystem, options, type};
}

// private constructor
Receipt::Receipt(process::PDBContext *context,
                 libutil::Filesystem *filesystem,
                 Options const &options,
                 ReceiptType type)
: m_type(type), m_options(options)
{
    std::string receipt = type == HEADERS ? options.project().value_or("") + "hdrs" : options.project().value_or("");
    if (filesystem->exists(context->PDBRoot+RECEIPTDIR+receipt))
        m_onDisk = true;
}

int
Receipt::existsOnDisk(void)
{
    return m_onDisk;
}

int
Receipt::createOnDisk(process::PDBContext *context,
                      libutil::Filesystem *filesystem,
                      Options const &options,
                      const std::string &dstroot)
{
//    DIR *rdir = opendir((context->PDBRoot+RECEIPTDIR).c_str());
//    dirent *dp = readdir(rdir);
//    std::unordered_map<std::string, bool> seen;
//
//    if (rdir == nullptr)
//      return errno;
//
////    while ((dp = readdir(rdir)) != nullptr)
////      seen[dp->d_name] = true;
////
////    rewinddir(rdir);
//
//    while ((dp = readdir(rdir)) != nullptr) {
//        std::string str;
//        auto rcpath = RECEIPTDIR+std::string(dp->d_name);
//        if (filesystem->type(rcpath) == libutil::Filesystem::Type::SymbolicLink) {
//            str = "%-20s -> %s", std::string(dp->d_name), filesystem->readSymbolicLink(rcpath);
//        } else if (!filesystem->exists(seen[(dp->d_name)])) {
//            str = "%-24s%s", "", "\$x";
//        }
//        std::cout << str << std::endl;
//    }
//
//    closedir(rdir);
//
  std::string receiptName = m_type == HEADERS ? m_options.project().value_or("") + "hdrs"
                                          : m_options.project().value_or("");
  // double check receipts dir
  if(!filesystem->exists(context->PDBRoot+RECEIPTDIR))
    filesystem->createDirectory(context->PDBRoot+RECEIPTDIR, true);

  if (filesystem->exists(context->PDBRoot+RECEIPTDIR+receiptName)){
    m_onDisk = true;
    return EEXIST;
  }
  auto buildRoot = context->PDBRoot;
  // get db path
  auto dbpath = buildRoot + "/.pdbuild/xref.db";

  // get build number
  auto build = context->getBuild(filesystem);

  // verify db path
  if(!filesystem->exists(dbpath)) {
    std::cerr << "xref.db not found";
    return EBADF;
  }

  // run plugin
  auto db = plugin::DBDatabase(dbpath);
  auto plugin = (Plugin<std::any> *) plugin::GetPluginWithName("registerFiles");
  plugin->run(db, build, {options.project().value(), dstroot});
  // get plugin data
  auto pluginData = ((plugin::Plugin<std::vector<std::string>>*)plugin)->getData();
  std::string shaFilename = pluginData[0];
  std::ofstream fstream(context->PDBRoot+RECEIPTDIR+shaFilename, std::ios::out);

  if(!fstream.is_open())
    return -errno;

  fstream << pluginData[1]; // write receiptData to file
  fstream.close();

  return 0;
}

