//
// Created by John Othwolo on 3/20/22.
//

#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Array.h>

#include <pdbuild/Project.h>
#include <pdbuild/Options.h>

#include <process/PDBContext.h>

using namespace pdbuild;

// we get the project from the context
Project::Project(const process::PDBContext *context, pdbuild::Options const &options) {
  auto projectNameOption = options.project();
  auto projectVersionOption = options.projectVersion();
  // get project dictionary
  m_projectDictionary = context->getProjectDictionary(*projectNameOption);
  if(!m_projectDictionary)
    context->abort("Cannot find project \"" + *projectNameOption + "\"");

  // check if the proj is an alias
  if(std::find(m_projectDictionary->begin(),
                m_projectDictionary->end(),
                "original")
      != m_projectDictionary->end()){
    m_isAlias = true;
    m_projectOriginal.name = m_projectDictionary->value<plist::String>("original")->value();
    m_projectOriginal.dictionary = context->getProjectDictionary(m_projectOriginal.name);
  }

  // make sure we have a project version
  auto projectVersionPlString = m_projectDictionary->value<plist::String>("version");
  // if project version doesn't exist and user didn't specify, abort.
  if (projectVersionPlString == nullptr && projectVersionOption->empty())
    context->abort("Cannot find version for project, \"" + *projectNameOption + "\"");

  m_projectName = *projectNameOption;
  m_projectVersion = projectVersionOption.value_or(projectVersionPlString->value());


  // if project has target, specify, otherwise just leave the target as an empty string.
  // this is normal for single target makefiles/xcodeproj/cmake projects.
  if(std::find(m_projectDictionary->begin(),
                m_projectDictionary->end(), "target") != m_projectDictionary->end())
    m_projectTarget = m_projectDictionary->value<plist::String>("target")->value();

  // check if there are dependencies
  if(std::find(m_projectDictionary->begin(),
                m_projectDictionary->end(), "dependencies") != m_projectDictionary->end())
  {
    m_hasDependencies = true;
    auto deps = m_projectDictionary->value<plist::Dictionary>("dependencies");
    m_headerDependencies = deps->value<plist::Array>("headers");
    m_buildDependencies = deps->value<plist::Array>("build");
  }

  // check if there are any environment variables and add them
  if(std::find(m_projectDictionary->begin(),
                m_projectDictionary->end(), "environment") != m_projectDictionary->end())
  {
    auto env = m_projectDictionary->value<plist::Dictionary>("environment");
    for (int i = 0; i < env->count(); ++i) {
      auto key = env->key(i);
      auto val = env->value<plist::String>(key)->value();
      m_environment.insert({key, val});
    }
  }
}

plist::Dictionary *Project::getProjectSourceSite() {
  return m_projectDictionary->value<plist::Dictionary>("source_site");
}

bool
Project::hasDependencies() const {
  return m_hasDependencies;
}

bool
Project::isAlias() const {
  return m_isAlias;
}

std::string
Project::getProjectName() {
  return m_projectName;
}

std::string
Project::getOriginalProjectName() const {
  return m_projectOriginal.name;
}

std::string
Project::getProjectNameAndVersion() {
  // if we're an alias "project-X.X.X" will point to original project
  if(m_isAlias)
    return m_projectOriginal.name + "-" + m_projectVersion;
  else
    return m_projectName + "-" + m_projectVersion;
}

std::string
Project::getProjectTarget() {
  return m_projectTarget;
}

std::string
Project::getProjectVersion() {
  return m_projectVersion;
}

plist::Array*
Project::getHeaderDependencies() {
  return m_headerDependencies;
}

plist::Array*
Project::getBuildDependencies() {
  return m_buildDependencies;
}

std::unordered_map<std::string, std::string>
Project::getEnvironmentVariables() {
  return m_environment;
}

std::unordered_map<std::string, std::string> 
Project::getXcodeBuildSettings(){
  std::unordered_map<std::string, std::string> ret;
  if(std::find(m_projectDictionary->begin(),
                m_projectDictionary->end(), "xcodeBuildSettings") != m_projectDictionary->end())
  {
    auto settings = m_projectDictionary->value<plist::Dictionary>("xcodeBuildSettings");
    for (int i = 0; i < settings->count(); ++i) {
      auto key = settings->key(i);
      auto val = settings->value<plist::String>(key)->value();
      ret.insert({key, val});
    }
  }
  return ret;
}

std::string
Project::getXcodeProductBundleType(){
  std::string ret;
  if(std::find(m_projectDictionary->begin(),
                m_projectDictionary->end(), "productBundleType") != m_projectDictionary->end())
  {
    ret = m_projectDictionary->value<plist::String>("productBundleType")->value();
  }
  return ret;
}