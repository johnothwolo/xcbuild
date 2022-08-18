//
// Created by John Othwolo on 12/24/21.
//

#define _XOPEN_SOURCE

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
#include <sstream>

#include <execinfo.h>
#include <ucontext.h>
#include <cxxabi.h>
#include <dlfcn.h>

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

void
PDBContext::setPdBuildRoot(process::Context *context) {
  // Environment var PDB_BUILDROOT overrides default buildroot which is CWD/pwd.
  m_pdbRoot = context->environmentVariable("PDB_BUILDROOT").value_or(context->currentDirectory());
}

std::string
PDBContext::getPdBuildRoot() const {
  return m_pdbRoot;
}

std::string
PDBContext::getPdBuildRootHiddenDirPath() const {
  return (m_pdbRoot + "/.pdbuild/");
}

std::string
PDBContext::getSourceCachePath() const {
    return (m_pdbRoot + "/SourceCache/");
}

std::string
PDBContext::getOsBuild(libutil::Filesystem const *filesystem) const {
//  auto path = this->PDBRoot + "/.pdbuild/build";
  std::string buildString;
//  if (filesystem->exists(path)) {
//    std::ifstream file(path);
//    if (file.is_open()) {
//      file >> buildString;
//      file.close();
//    } else this->abort("error opening build file");
//  } else {
//    this->abort("pdbuild not initialized properly");
//  }
  buildString = plist::CastTo<plist::String>(m_chachedPlistRootDictionary->value("build"))->value();
  return this->environmentVariable("PDB_BUILD").value_or(buildString);
}

std::string
PDBContext::getPlistPreference(libutil::Filesystem const *filesystem) const {
  auto path = m_pdbRoot+"/.pdbuild/plist";
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

std::string Backtrace(int skip = 1)
{
  void *callstack[128];
  const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
  char buf[1024];
  int nFrames = backtrace(callstack, nMaxFrames);
  char **symbols = backtrace_symbols(callstack, nFrames);

  std::ostringstream trace_buf;
  std::cout << "Backtrace:\n";
  for (int i = skip; i < nFrames; i++) {
//    printf("%s\n", symbols[i]);
    Dl_info info;
    if (dladdr(callstack[i], &info) && info.dli_sname) {
      char *demangled = nullptr;
      int status = -1;
      if (info.dli_sname[0] == '_')
        demangled = abi::__cxa_demangle(info.dli_sname, nullptr, 0, &status);
      snprintf(buf, sizeof(buf), "  #%-3d %*p %s + %zd\n",
               i, int(2 + sizeof(void*) * 2), callstack[i],
               status == 0 ? demangled :
               info.dli_sname == 0 ? symbols[i] : info.dli_sname,
               (char *)callstack[i] - (char *)info.dli_saddr);
      free(demangled);
    } else {
      snprintf(buf, sizeof(buf), "  #%-3d %*p %s\n",
               i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
    }
    trace_buf << buf;
  }
  free(symbols);
  if (nFrames == nMaxFrames)
    trace_buf << "[truncated]\n";
  return trace_buf.str();
}

//[[maybe_unused]]
//static void
//bt_sighandler(int sig) {
//  ucontext_t uctx;
//  getcontext(&uctx);
//  auto *ctx = &uctx.uc_mcontext->__ss;
//  void *trace[16];
//  char **messages = (char **)nullptr;
//  int i, trace_size = 0;
//
//  if (sig == SIGSEGV)
//    printf("Got signal %d, faulty address is %p, from %p\n",
//           sig, (void*)ctx->__cs, (void*)ctx->__rip);
//  else
//    printf("Got signal %d\n", sig);
//
//  trace_size = backtrace(trace, 16);
//  /* overwrite sigaction with caller's address */
//  trace[1] = (void *)ctx->__rip;
//  messages = backtrace_symbols(trace, trace_size);
//  /* skip first stack frame (points here) */
//  printf("[bt] Execution path:\n");
//  for (i=1; i<trace_size; ++i){
////    std::cout << "[bt] " << i << messages[i] << std::endl;
//    printf(" #%d %s\n", i, messages[i]);
//
//    Dl_info info;
//    if(dladdr(trace[i], &info)){
//      int status;
//      char *demangled = __cxxabiv1::__cxa_demangle(info.dli_sname, (char*)nullptr, (size_t*)0, &status);
//    }
//
////    /* find first occurence of '(' or ' ' in message[i] and assume
////     * everything before that is the file name. (Don't go beyond 0 though
////     * (string terminator)*/
////    size_t p = 0;
////    while(messages[i][p] != '(' && messages[i][p] != ' ' && messages[i][p] != 0)
////      ++p;
////
////    char syscom[256];
////    sprintf(syscom, "atos -o %.*s %p ", (char)p, messages[i], trace[i]);
////    std::cout << syscom << std::endl;
////    //last parameter is the file name of the symbol
////    system(syscom);
//  }

//}

static void
bt_sighandler(int sig) {
  std::cout << Backtrace(1) << std::endl;
  exit(0);
}
#pragma clang diagnostic pop

[[maybe_unused]]
static void register_backtrace_hndl(){
  /* Install our signal handler */
  struct sigaction sa{};

  sa.sa_handler = bt_sighandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

//  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGUSR1, &sa, NULL);
}

void PDBContext::abort(const std::string &message) const {
  std::cerr << message << std::endl;
  void* callstack[128];
  int frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);
//  for (int i = 0; i < frames; ++i)
//    std::cout << strs[i] << std::endl;

  std::cout << " Backtrace: " << std::endl;

  // attempt to demangle
  for (int i = 0; i < frames; ++i){
    std::string trace(strs[i]);
    std::string::size_type begin, end;

    // find the beginning and the end of the useful part of the trace
    begin = 1;
    end = trace.find_last_of('+') - 1;

    // if they were found, we'll go ahead and demangle
    if (begin != std::string::npos && end != std::string::npos) {
      trace = trace.substr(begin, end - begin); // get rid of plus at end

      // get rid of stuff at the beginning
      begin = trace.find_last_of(' ') + 1;
      end = *trace.end();
      trace = trace.substr(begin, end - begin);

      size_t maxName = 1024;
      int demangleStatus;

      char* demangledName = (char*) malloc(maxName);
      if ((demangledName = abi::__cxa_demangle(trace.c_str(), demangledName, &maxName,
                                               &demangleStatus)) && demangleStatus == 0) {
        trace = demangledName; // the demangled name is now in our trace string
      }
      free(demangledName);

      std::cout << "   " << frames-i-1 << " " << trace << std::endl;
    }
  }

  free(strs);
  register_backtrace_hndl();
  raise(SIGTRAP);
  raise(SIGUSR1);
  exit(-1);
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

void
PDBContext::openPlist(libutil::Filesystem const *filesystem,
                      const std::string& path) const
{
  if (m_chachedPlistRoot != nullptr)
    return; // if plist was already opened

  // init might need to pass a path to this function so use ternary op below.
  auto plistPath = path == "-" ? (this->getPlistPreference(filesystem)) : path;
//  std::cout << "PList path: " << plistPath << std::endl;
  auto result = this->Read(filesystem, plistPath);

  if (!result.first)
    this->abort("Unable to read plist file");

  /* Deserialize input, storing input format. */
  std::unique_ptr<plist::Object> root;

  if (auto any = plist::Format::Any::Identify(result.second)) {
    auto deserialize = plist::Format::Any::Deserialize(result.second, *any);
    if (deserialize.first != nullptr)
      root = std::move(deserialize.first);
    else this->abort("error: " + deserialize.second);
  } else {
    auto json = plist::Format::JSON::Create();
    auto deserialize = plist::Format::JSON::Deserialize(result.second, json);
    if (deserialize.first != nullptr)
      root = std::move(deserialize.first);
    else this->abort("error: input %s not a plist or json " + plistPath);
  }

  // create plist cache
  const_cast<PDBContext*>(this)->m_chachedPlistRoot = std::move(root);
  // cache with dictionary type too
  const_cast<PDBContext*>(this)->m_chachedPlistRootDictionary = plist::CastTo<plist::Dictionary>(m_chachedPlistRoot.get());
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

plist::Dictionary*
PDBContext::getAllProjects() const {
    return plist::CastTo<plist::Dictionary>(m_chachedPlistRootDictionary->value("projects"));
}

plist::Dictionary*
PDBContext::getProjectDictionary(const std::string& project) const {
  return plist::CastTo<plist::Dictionary>(getAllProjects()->value(project));
}

plist::Array*
PDBContext::getSourceSites() const {
    return plist::CastTo<plist::Array>(m_chachedPlistRootDictionary->value("source_sites"));
};

plist::Object*
PDBContext::getPlistRootObject() const {
  return m_chachedPlistRoot.get();
}

plist::Dictionary*
PDBContext::getPlistRootDictionary() const {
  return m_chachedPlistRootDictionary;
}

std::string
PDBContext::getDestinationImagePath() const {
  return  (m_pdbRoot + "/.pdbuild/distribution_systemimage.sparsebundle");
}

std::string
PDBContext::getDestinationRootPath() const {
  return  (m_pdbRoot + "/DistributionImage/");
}

std::string
PDBContext::getDestinationSourcesPath() const {
  return  (m_pdbRoot + "/DistributionImage/Sources/");
}

std::string
PDBContext::getDestinationTmpPath() const {
  return  (m_pdbRoot + "/DistributionImage/tmp/");
}

std::string
PDBContext::getDestinationVarTmpPath() const {
  return  (m_pdbRoot + "/DistributionImage/private/var/tmp/");
}

std::string
PDBContext::getDestinationReceiptsPath() const {
  return  (m_pdbRoot + "/DistributionImage/usr/local/receipts/");
}

