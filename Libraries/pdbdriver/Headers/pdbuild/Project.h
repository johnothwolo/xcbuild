//
// Created by John Othwolo on 3/20/22.
//

#ifndef XCBUILD_PROJECT_H
#define XCBUILD_PROJECT_H

#include <unordered_map>
#include <string>
#include <vector>

namespace plist   { class Dictionary; }
namespace plist   { class Array;      }
namespace process { class PDBContext; }
namespace pdbuild { class Options;    }

namespace pdbuild {

class Project {
public:
  Project(const process::PDBContext *context, pdbuild::Options const &options);

  [[nodiscard]] bool hasDependencies() const;
  [[nodiscard]] bool isAlias() const;

  std::string getProjectName();
  [[nodiscard]] std::string getOriginalProjectName() const;
  std::string getProjectNameAndVersion();
  std::string getProjectTarget();
  std::string getProjectVersion();
  plist::Array *getHeaderDependencies();
  plist::Array *getBuildDependencies();
  std::unordered_map<std::string, std::string> getEnvironmentVariables();
  std::unordered_map<std::string, std::string> getXcodeBuildSettings();
  plist::Dictionary *getProjectSourceSite();
  std::string getXcodeProductBundleType();

private:
  bool m_hasDependencies;
  bool m_isAlias;
  struct {
    plist::Dictionary *dictionary; // is not nullptr in alias
    std::string name;
  } m_projectOriginal;
  plist::Dictionary *m_projectDictionary;
  plist::Array *m_headerDependencies;
  plist::Array *m_buildDependencies;
  std::string m_projectName;
  std::string m_projectTarget;
  std::string m_projectVersion;
  std::unordered_map<std::string, std::string> m_environment;
};

} // namespace pdbuild
#endif // XCBUILD_PROJECT_H
