//
// Created by John Othwolo on 12/24/21.
//

#include <process/PDBContext.h>

#include <libutil/FSUtil.h>
#include <libutil/Filesystem.h>

#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Data.h>
#include <plist/Date.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/Real.h>
#include <plist/String.h>
#include <plist/Object.h>
#include <plist/Format/Format.h>
#include <plist/Format/Any.h>
#include <plist/Format/XML.h>
#include <plist/Format/JSON.h>

#include <CommonCrypto/CommonCrypto.h>

#include <csignal>
#include <fstream>
#include <iostream>

using process::PDBContext;
using process::DefaultContext;
using libutil::Filesystem;
using libutil::FSUtil;

static std::string str_format(const std::string format, ...){
  va_list args;

  va_start(args, format);
  auto len = std::vsnprintf(nullptr, 0, format.c_str(), args);
  va_end(args);

  char vec[len+1];

  va_start(args, format);
  std::vsnprintf(&vec[0], len+1, format.c_str(), args);
  va_end(args);
  return vec;
}

PDBContext::PDBContext() : DefaultContext(){}
PDBContext::~PDBContext(){}

std::string
PDBContext::getBuild(libutil::Filesystem const *filesystem) const {
  auto path = this->PDBRoot + "/.pdbuild/build";
  std::string buildString;
  if (filesystem->exists(path)) {
    std::ifstream file(path);
    if (file.is_open()) {
      file >> buildString;
      file.close();
    } else this->abort("error opening build file");
  } else {
    this->abort("pdbuild not initialized properly");
  }
  return this->environmentVariable("PDB_BUILD").value_or(buildString);
}

std::string
PDBContext::getPlistPreference(libutil::Filesystem const *filesystem) const {
  auto path = this->PDBRoot+"/.pdbuild/plist";
  std::string plString;
  if (filesystem->exists(path)){
    std::ifstream file(path);
    if (file.is_open()) {
//      std::cout << "PList path: " << path << std::endl;
      file >> plString;
      file.close();
    } else this->abort("error opening plist file");
  } else {
    this->abort("pdbuild not initialized properly");
  }
  return this->environmentVariable("PDB_PLISTFILE").value_or(plString);
}
void PDBContext::abort(const std::string &message) const {
  std::cout << message << std::endl;
  raise(SIGTRAP);
}

std::pair<bool, std::vector<uint8_t>>
PDBContext::Read(Filesystem const *filesystem, std::string const &path) const
{
  std::vector<uint8_t> contents;

  if (path.empty()) {
    // just a fastpath
    return std::make_pair(false, std::vector<uint8_t>());
  } else {
    /* Read from file. */
    if (!filesystem->read(&contents, path)) {
      return std::make_pair(false, std::vector<uint8_t>());
    }
  }

  return std::make_pair(true, std::move(contents));
}

bool
PDBContext::Write(Filesystem *filesystem,
                  std::vector<uint8_t> const &contents,
                  std::string const &path) const
{
  if (path == "-") {
    /* - means write to stdout. */
    std::copy(contents.begin(), contents.end(), std::ostream_iterator<char>(std::cout));
  } else {
    /* Read from file. */
    if (!filesystem->write(contents, path)) {
      return false;
    }
  }

  return true;
}

std::shared_ptr<plist::Object>
PDBContext::openPlist(libutil::Filesystem const *filesystem,
                      const std::string& path) const
{
  if (m_chachedPlistRoot != nullptr && path != "-")
    return m_chachedPlistRoot;

  // init might need to pass a path to this function so use ternary op below.
  auto plistPath = path == "-" ? (this->getPlistPreference(filesystem)) : path;
//  std::cout << "PList path: " << plistPath << std::endl;
  auto result = this->Read(filesystem, plistPath);

  if (!result.first) {
    fprintf(stderr, "error: unable to read %s\n", plistPath.c_str());
    return nullptr;
  }

  /* Deserialize input, storing input format. */
  std::unique_ptr<plist::Object> root;

  if (auto any = plist::Format::Any::Identify(result.second)) {
    auto deserialize = plist::Format::Any::Deserialize(result.second, *any);
    if (deserialize.first != nullptr)
      root = std::move(deserialize.first);
    else {
      fprintf(stderr, "error: %s\n", deserialize.second.c_str());
      return nullptr;
    }
  } else {
    auto json = plist::Format::JSON::Create();
    auto deserialize = plist::Format::JSON::Deserialize(result.second, json);
    if (deserialize.first != nullptr)
      root = std::move(deserialize.first);
    else {
      fprintf(stderr, "error: input %s not a plist or json\n", plistPath.c_str());
      return nullptr;
    }
  }

  // create plist cache
  const_cast<PDBContext*>(this)->m_chachedPlistRoot = std::move(root);
  return m_chachedPlistRoot;
}

std::string
PDBContext::sha1Checksum(const std::vector<u_char> &data) const {
  unsigned char md[CC_SHA1_DIGEST_LENGTH] = {0};
  CC_SHA1_CTX c;
  auto pos = 0;
  CC_SHA1_Init(&c);

  size_t size = data.size();
  const unsigned int blockLen = 8192;

  while (pos < size && (pos + blockLen) < size) {
    CC_SHA1_Update(&c, data.data()+pos, (CC_LONG)blockLen);
    pos += blockLen;
  }
  // last block
  CC_SHA1_Update(&c, data.data()+pos, (CC_LONG)(data.size() - pos));

  CC_SHA1_Final(md, &c);
  return str_format("%02x%02x%02x%02x"
                    "%02x%02x%02x%02x"
                    "%02x%02x%02x%02x"
                    "%02x%02x%02x%02x"
                    "%02x%02x%02x%02x",
                    md[0], md[1], md[2], md[3], md[4], md[5], md[6], md[7], md[8], md[9], md[10],
                    md[11], md[12], md[13], md[14], md[15], md[16], md[17], md[18], md[19]);
}