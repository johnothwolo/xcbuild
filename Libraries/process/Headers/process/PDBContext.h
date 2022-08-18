//
// Created by John Othwolo on 12/24/21.
//

#ifndef __process_PDBContext_h
#define __process_PDBContext_h

#include <process/DefaultContext.h>

#pragma mark "WARNING: This is meant for the 'pdbdriver' library!!"
// FIXME: could this file be better placed?

namespace libutil { class Filesystem; }
namespace plist { class Object; }
namespace plist { class Dictionary; }
namespace plist { class String; }
namespace plist { class Array; }

namespace process {

/*
 * A process context for the current process. Generally, this should only
 * be created in `main()` and passed down to anywhere else that needs it.
 */
class PDBContext : public DefaultContext {
public:
  explicit PDBContext();
  virtual ~PDBContext();

public:
  void setPdBuildRoot(process::Context *context);
  std::string getOsBuild(libutil::Filesystem const *filesystem) const;
  std::string getPlistPreference(libutil::Filesystem const *filesystem) const;

  // plist functions
public:
  void openPlist(libutil::Filesystem const *filesystem, const std::string& path = "-") const;

  [[nodiscard]] plist::Array *getSourceSites() const;
  [[nodiscard]] plist::Dictionary *getProjectDictionary(const std::string& project) const;
  [[nodiscard]] plist::Dictionary *getAllProjects() const;
  [[nodiscard]] plist::Object *getPlistRootObject() const;
  [[nodiscard]] plist::Dictionary *getPlistRootDictionary() const;

  // paths in the pdbuild directory
  // TODO: complete them
public:
  [[nodiscard]] std::string getPdBuildRoot() const;
  [[nodiscard]] std::string getPdBuildRootHiddenDirPath() const;
  [[nodiscard]] std::string getSourceCachePath() const;

  // paths in .../DistributionImage/*
public:
  [[nodiscard]] std::string getDestinationImagePath() const;
  [[nodiscard]] std::string getDestinationRootPath() const;
  [[nodiscard]] std::string getDestinationSourcesPath() const;
  [[nodiscard]] std::string getDestinationTmpPath() const;
  [[nodiscard]] std::string getDestinationVarTmpPath() const;
  [[nodiscard]] std::string getDestinationReceiptsPath() const;

  std::pair<bool, std::vector<uint8_t>>
  Read(libutil::Filesystem const *filesystem, std::string const &path) const;

  bool
  Write(libutil::Filesystem *filesystem, std::vector<uint8_t> const &contents, std::string const &path = "-") const;

public:
  [[nodiscard]] std::string sha1Checksum(const std::vector<unsigned char> &data) const;
  void abort(const std::string &message) const;

private:
  std::string m_pdbRoot;
  std::shared_ptr<plist::Object> m_chachedPlistRoot = nullptr;
  plist::Dictionary *m_chachedPlistRootDictionary = nullptr;
};

}


#endif // __process_PDBContext_h
