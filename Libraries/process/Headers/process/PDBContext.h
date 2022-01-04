//
// Created by John Othwolo on 12/24/21.
//

#ifndef __process_PDBContext_h
#define __process_PDBContext_h

#include <process/DefaultContext.h>

namespace libutil { class Filesystem; }
namespace plist { class Object; }

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
  std::string getBuild(libutil::Filesystem const *filesystem) const;
  std::string getPlistPreference(libutil::Filesystem const *filesystem) const;

public:
  std::shared_ptr<plist::Object>
  openPlist(libutil::Filesystem const *filesystem, const std::string& path = "-") const;

  std::pair<bool, std::vector<uint8_t>>
  Read(libutil::Filesystem const *filesystem, std::string const &path) const;
  bool
  Write(libutil::Filesystem *filesystem,
        std::vector<uint8_t> const &contents,
        std::string const &path = "-") const;

public:
  [[nodiscard]] std::string sha1Checksum(const std::vector<unsigned char> &data) const;
  void abort(const std::string &message) const;

public:
  std::string PDBRoot;

private:
  std::shared_ptr<plist::Object> m_chachedPlistRoot = nullptr;
};

}


#endif // __process_PDBContext_h
