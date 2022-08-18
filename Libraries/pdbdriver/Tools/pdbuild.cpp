//
//  main.cpp
//  pdbuild
//
//  Created by John Othwolo on 10/8/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//

#include <libutil/DefaultFilesystem.h>
#include <pdbuild/Driver.h>
#include <process/DefaultLauncher.h>
#include <process/DefaultUser.h>
#include <process/PDBContext.h>

using libutil::DefaultFilesystem;

int main(int argc, const char * argv[]) {
  DefaultFilesystem filesystem = DefaultFilesystem();
  auto processContext = process::PDBContext();
  auto processLauncher = process::DefaultLauncher();
  auto user = process::DefaultUser();
  return pdbuild::Driver::Run(&user, &processContext, &processLauncher, &filesystem);
}

