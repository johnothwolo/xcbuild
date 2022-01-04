/*
 * Copyright (c) 2005-2010 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_BSD_LICENSE_HEADER_START@
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @APPLE_BSD_LICENSE_HEADER_END@
 */


#include "Plugin.h"
#include "DBDatabase.h"
#include <sys/syslimits.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <fts.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <string>
#include <sstream>
#include <CommonCrypto/CommonDigest.h>

// If the path points to a Mach-O file, records all dylib
// link commands as library dependencies in the database.
// XXX
// Ideally library dependencies are tracked per-architecture.
// For now, we're assuming all architectures contain identical images.
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/swap.h>

extern char** environ;

namespace plugin {

class RegisterFiles : BasicPlugin<DBArray<DBString>> {
public:
  RegisterFiles() : BasicPlugin("registerFiles") {}
  ~RegisterFiles() = default;

  int run(DBDatabase &db, const std::string &build,
          const std::vector<std::string> &argv) override {
    int res = 0;
    int i = 0;

    if (argv.size() < 2 || argv.size() > 3)
      return EINVAL;

    std::string project = argv.at(i++);
    std::string dstRoot = argv.at(i++);
    res = this->register_files(db, build, project, dstRoot);
    return res;
  }
  std::string usage() override { return (std::string) "<project> <dstroot>"; }

  static int compare(const FTSENT **a, const FTSENT **b) {
    return strcmp((*a)->fts_name, (*b)->fts_name);
  }

  static size_t ent_filename(FTSENT *ent, char *filename, size_t bufsiz) {
    if (ent == nullptr)
      return 0;
    if (ent->fts_level > 1) {
      bufsiz = ent_filename(ent->fts_parent, filename, bufsiz);
    }
    strncat(filename, "/", bufsiz);
    bufsiz -= 1;
    if (ent->fts_namelen != 0) {
      strncat(filename, ent->fts_name, bufsiz);
      bufsiz -= strlen(ent->fts_name);
    }
    return bufsiz;
  }

  std::string calculate_digest(int fd) {
    unsigned char md[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1_CTX c;
    CC_SHA1_Init(&c);

    memset(md, 0, CC_SHA1_DIGEST_LENGTH);

    ssize_t len;
    const unsigned int blocklen = 8192;
    static unsigned char *block = nullptr;
    if (block == nullptr) {
      block = static_cast<unsigned char *>(malloc(blocklen));
    }
    while (true) {
      len = read(fd, block, blocklen);
      if (len == 0) {
        close(fd);
        break;
      }
      if ((len < 0) && (errno == EINTR))
        continue;
      if (len < 0) {
        close(fd);
        return nullptr;
      }
      CC_SHA1_Update(&c, block, (CC_LONG)len);
    }

    CC_SHA1_Final(md, &c);
    return this->format("%02x%02x%02x%02x"
                        "%02x%02x%02x%02x"
                        "%02x%02x%02x%02x"
                        "%02x%02x%02x%02x"
                        "%02x%02x%02x%02x",
                        md[0], md[1], md[2], md[3], md[4], md[5], md[6], md[7], md[8], md[9], md[10],
                        md[11], md[12], md[13], md[14], md[15], md[16], md[17], md[18], md[19]);
  }

  std::string calculate_digest(const std::vector<char> &data) {
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
    return this->format("%02x%02x%02x%02x"
                  "%02x%02x%02x%02x"
                  "%02x%02x%02x%02x"
                  "%02x%02x%02x%02x"
                  "%02x%02x%02x%02x",
                  md[0], md[1], md[2], md[3], md[4], md[5], md[6], md[7], md[8], md[9], md[10],
                  md[11], md[12], md[13], md[14], md[15], md[16], md[17], md[18], md[19]);
  }

  int register_mach_header(DBDatabase &database,
                                  const std::string &build,
                                  const std::string &project,
                                  const std::string &path,
                                  struct fat_arch *fa,
                                  int fd, int *isMachO) {
    ssize_t res;
    uint32_t magic;
    int swap = 0;

    struct mach_header *mh = nullptr;
    struct mach_header_64 *mh64 = nullptr;

    if (isMachO)
      *isMachO = 0;

    res = read(fd, &magic, sizeof(uint32_t));
    if (res < sizeof(uint32_t)) {
      return 0;
    }

    //
    // 32-bit, read the rest of the header
    //
    if (magic == MH_MAGIC || magic == MH_CIGAM) {
      if (isMachO)
        *isMachO = 1;
      mh = static_cast<mach_header *>(malloc(sizeof(struct mach_header)));
      if (mh == nullptr)
        return -1;
      memset(mh, 0, sizeof(struct mach_header));
      mh->magic = magic;
      res =
          read(fd, &mh->cputype, sizeof(struct mach_header) - sizeof(uint32_t));
      if (res < sizeof(struct mach_header) - sizeof(uint32_t)) {
        return 0;
      }
      if (magic == MH_CIGAM) {
        swap = 1;
        swap_mach_header(mh, NXHostByteOrder());
      }
      //
      // 64-bit, read the rest of the header
      //
    } else if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64) {
      if (isMachO)
        *isMachO = 1;
      mh64 =
          static_cast<mach_header_64 *>(malloc(sizeof(struct mach_header_64)));
      if (mh64 == nullptr)
        return -1;
      memset(mh64, 0, sizeof(struct mach_header_64));
      mh64->magic = magic;
      res = read(fd, &mh64->cputype,
                 sizeof(struct mach_header_64) - sizeof(uint32_t));
      if (res < sizeof(struct mach_header_64) - sizeof(uint32_t)) {
        return 0;
      }
      if (magic == MH_CIGAM_64) {
        swap = 1;
        swap_mach_header_64(mh64, NXHostByteOrder());
      }
      //
      // Not a Mach-O
      //
    } else {
      return 0;
    }

    switch (mh64 ? mh64->filetype : mh->filetype) {
    case MH_EXECUTE:
    case MH_DYLIB:
    case MH_BUNDLE:
      break;
    case MH_OBJECT:
    default:
      return 0;
    }

    std::string query =
        "INSERT INTO "
        "mach_o_objects (magic, type, cputype, cpusubtype, flags, build, project, path) "
        "VALUES (" +
        std::to_string(mh64 ? mh64->magic : mh->magic) +
        std::to_string(mh64 ? mh64->filetype : mh->filetype) +
        std::to_string(mh64 ? mh64->cputype : mh->cputype) +
        std::to_string(mh64 ? mh64->cpusubtype : mh->cpusubtype) +
        std::to_string(mh64 ? mh64->flags : mh->flags) + build + project +
        path + ")";

    database.query(query);
    uint64_t serial = database.lastRowId();

    //
    // Information needed to parse the symbol table
    //
    int count_nsect = 0;
    unsigned char text_nsect = NO_SECT;
    unsigned char data_nsect = NO_SECT;
    unsigned char bss_nsect = NO_SECT;

    uint32_t nsyms = 0;
    uint8_t *symbols = nullptr;

    uint32_t strsize = 0;
    uint8_t *strings = nullptr;

    int i;
    uint32_t ncmds = mh64 ? mh64->ncmds : mh->ncmds;
    for (i = 0; i < ncmds; ++i) {
      //
      // Read a generic load command into memory.
      // At first, we only know it has a type and size.
      //
      struct load_command lctmp {};

      res = read(fd, &lctmp, sizeof(struct load_command));
      if (res < sizeof(struct load_command)) {
        return 0;
      }

      uint32_t cmd = swap ? OSSwapInt32(lctmp.cmd) : lctmp.cmd;
      uint32_t cmdsize = swap ? OSSwapInt32(lctmp.cmdsize) : lctmp.cmdsize;
      if (cmdsize == 0)
        continue;

      auto *lc = static_cast<load_command *>(malloc(cmdsize));
      if (lc == nullptr) {
        return 0;
      }
      memset(lc, 0, cmdsize);
      memcpy(lc, &lctmp, sizeof(lctmp));

      // Read the remainder of the load command.
      res = read(fd, (uint8_t *)lc + sizeof(struct load_command),
                 cmdsize - sizeof(struct load_command));
      if (res < (cmdsize - sizeof(struct load_command))) {
        free(lc);
        return 0;
      }

      //
      // LC_LOAD_DYLIB and LC_LOAD_WEAK_DYLIB
      // Add dylibs as unresolved "lib" dependencies.
      //
      if (cmd == LC_LOAD_DYLIB || cmd == LC_LOAD_WEAK_DYLIB) {
        auto *dylib = (struct dylib_command *)lc;
        if (swap)
          swap_dylib_command(dylib, NXHostByteOrder());

        // sections immediately follow the dylib_command structure, and are
        // reflected in the cmdsize.

        //			int _strsize = dylib->cmdsize - sizeof(struct dylib_command); 			char* str = malloc(_strsize+1); 			strncpy(str, (char*)((uint8_t*)dylib + dylib->dylib.name.offset), _strsize);
        //			str[_strsize] = 0; // NUL-terminate

        std::stringstream stream{};
        stream << "INSERT INTO unresolved_dependencies (build,project,type,dependency) ";
        stream << "VALUES (" << build << "," << project << ",lib,";
        stream << (char *)((uint8_t *)dylib + dylib->dylib.name.offset) << ")";
        database.query(stream.str());

        //			free(str);

        //
        // LC_LOAD_DYLINKER
        // Add the dynamic linker (usually dyld) as an unresolved "lib" dependency.
        //
      } else if (cmd == LC_LOAD_DYLINKER) {
        auto *dylinker = (struct dylinker_command *)lc;
        if (swap)
          swap_dylinker_command(dylinker, NXHostByteOrder());

        // sections immediately follow the dylib_command structure, and are
        // reflected in the cmdsize.

        //			strsize = dylinker->cmdsize - sizeof(struct dylinker_command); 			char* str = static_cast<char *>(malloc(strsize + 1)); 			strncpy(str, (char*)((uint8_t*)dylinker + dylinker->name.offset), strsize); 			str[strsize] = 0; // NUL-terminate
        //
        //			gDatabse->query("INSERT INTO unresolved_dependencies (build,project,type,dependency) VALUES (%Q,%Q,%Q,%Q)", 			build, project, "lib", str);

        //			free(str);
        std::stringstream stream{};
        stream << "INSERT INTO unresolved_dependencies (build,project,type,dependency) ";
        stream << "VALUES (" << build << "," << project << ",lib,";
        stream << (char *)((uint8_t *)dylinker + dylinker->name.offset) << ")";
        database.query(stream.str());

        //
        // LC_SYMTAB
        // Read the symbol table into memory, we'll process it after we're
        // done with the load commands.
        //
      } else if (cmd == LC_SYMTAB && symbols == nullptr) {
        auto *symtab = (struct symtab_command *)lc;
        if (swap)
          swap_symtab_command(symtab, NXHostByteOrder());

        nsyms = symtab->nsyms;
        uint32_t symsize =
            nsyms * (mh64 ? sizeof(struct nlist_64) : sizeof(struct nlist));
        symbols = static_cast<uint8_t *>(malloc(symsize));

        strsize = symtab->strsize;
        // XXX: check strsize != 0
        strings = static_cast<uint8_t *>(malloc(strsize));

        off_t save = lseek(fd, 0, SEEK_CUR);

        off_t origin = fa ? fa->offset : 0;

        lseek(fd, (off_t)symtab->symoff + origin, SEEK_SET);
        res = read(fd, symbols, symsize);
        if (res < symsize) { /* XXX: leaks */
          return 0;
        }

        lseek(fd, (off_t)symtab->stroff + origin, SEEK_SET);
        res = read(fd, strings, strsize);
        if (res < strsize) { /* XXX: leaks */
          return 0;
        }

        lseek(fd, save, SEEK_SET);

        //
        // LC_SEGMENT
        // We're looking for the section number of the text, data, and bss segments in order to parse symbols.
        //
      } else if (cmd == LC_SEGMENT) {
        auto *seg = (struct segment_command *)lc;
        if (swap)
          swap_segment_command(seg, NXHostByteOrder());

        // sections immediately follow the segment_command structure, and are
        // reflected in the cmdsize.
        int k;
        for (k = 0; k < seg->nsects; ++k) {
          auto *sect = (struct section *)((uint8_t *)seg +
                                          sizeof(struct segment_command) +
                                          k * sizeof(struct section));
          if (swap)
            swap_section(sect, 1, NXHostByteOrder());
          if (strcmp(sect->sectname, SECT_TEXT) == 0 &&
              strcmp(sect->segname, SEG_TEXT) == 0) {
            text_nsect = ++count_nsect;
          } else if (strcmp(sect->sectname, SECT_DATA) == 0 &&
                     strcmp(sect->segname, SEG_DATA) == 0) {
            data_nsect = ++count_nsect;
          } else if (strcmp(sect->sectname, SECT_BSS) == 0 &&
                     strcmp(sect->segname, SEG_DATA) == 0) {
            bss_nsect = ++count_nsect;
          } else {
            ++count_nsect;
          }
        }

        //
        // LC_SEGMENT_64
        // Same as LC_SEGMENT, but for 64-bit binaries.
        //
      } else if (lc->cmd == LC_SEGMENT_64) {
        auto *seg = (struct segment_command_64 *)lc;
        if (swap)
          swap_segment_command_64(seg, NXHostByteOrder());

        // sections immediately follow the segment_command structure, and are
        // reflected in the cmdsize.
        int k;
        for (k = 0; k < seg->nsects; ++k) {
          auto *sect = (struct section_64 *)((uint8_t *)seg +
                                             sizeof(struct segment_command_64) +
                                             k * sizeof(struct section_64));
          if (swap)
            swap_section_64(sect, 1, NXHostByteOrder());
          if (strcmp(sect->sectname, SECT_TEXT) == 0 &&
              strcmp(sect->segname, SEG_TEXT) == 0) {
            text_nsect = ++count_nsect;
          } else if (strcmp(sect->sectname, SECT_DATA) == 0 &&
                     strcmp(sect->segname, SEG_DATA) == 0) {
            data_nsect = ++count_nsect;
          } else if (strcmp(sect->sectname, SECT_BSS) == 0 &&
                     strcmp(sect->segname, SEG_DATA) == 0) {
            bss_nsect = ++count_nsect;
          } else {
            ++count_nsect;
          }
        }
      }

      free(lc);
    }

    //
    // Finished processing the load commands, now insert symbols into the database.
    //
    int j;
    for (j = 0; j < nsyms; ++j) {
      struct nlist_64 symbol {};
      if (mh64) {
        memcpy(&symbol, (symbols + j * sizeof(struct nlist_64)),
               sizeof(struct nlist_64));
        if (swap)
          swap_nlist_64(&symbol, 1, NXHostByteOrder());
      } else {
        symbol.n_value = 0;
        memcpy(&symbol, (symbols + j * sizeof(struct nlist)),
               sizeof(struct nlist));
        if (swap)
          swap_nlist_64(&symbol, 1, NXHostByteOrder());
        // we copied a 32-bit nlist into a 64-bit one, adjust the value accordingly all other fields are identical sizes
        symbol.n_value >>= 32;
      }
      char type = '?';
      switch (symbol.n_type & N_TYPE) {
      case N_UNDF:
      case N_PBUD:
        type = 'u';
        if (symbol.n_value != 0) {
          type = 'c';
        }
        break;
      case N_ABS:
        type = 'a';
        break;
      case N_SECT:
        if (symbol.n_sect == text_nsect) {
          type = 't';
        } else if (symbol.n_sect == data_nsect) {
          type = 'd';
        } else if (symbol.n_sect == bss_nsect) {
          type = 'b';
        } else {
          type = 's';
        }
        break;
      case N_INDR:
        type = 'i';
        break;
      }

      // uppercase indicates an externally visible symbol
      if ((symbol.n_type & N_EXT) && type != '?') {
        type = (char)toupper(type);
      }

      if (type != '?' && type != 'u' && type != 'c') {
        const auto *name = (const uint8_t *)"";
        if (symbol.n_un.n_strx != 0) {
          name = (uint8_t *)(strings + symbol.n_un.n_strx);
        }

        std::stringstream stream{};
        stream << "INSERT INTO mach_o_symbols "; // VALUES (%lld, \'%c\', %lld, %Q)";
        stream << "VALUES (" << serial << ",\'" << type << "\',";
        stream << symbol.n_value << "," << name << ")";
        database.query(stream.str());

        //			res = SQL("INSERT INTO mach_o_symbols VALUES (%lld, \'%c\', %lld, %Q)", 				serial, 				type, 				symbol.n_value, 				name);
      }
    }

    return 0;
  }

  int register_libraries(DBDatabase &database,
                                int fd, const std::string &build,
                                const std::string &project,
                                const std::string &filename, int *isMachO) {
    ssize_t res;

    uint32_t magic;

    res = read(fd, &magic, sizeof(uint32_t));
    if (res < sizeof(uint32_t)) {
      goto error_out;
    }

    if (magic == FAT_MAGIC || magic == FAT_CIGAM) {
      struct fat_header fh {};
      int swap = 0;

      res =
          read(fd, &fh.nfat_arch, sizeof(struct fat_header) - sizeof(uint32_t));
      if (res < sizeof(uint32_t)) {
        goto error_out;
      }

      if (magic == FAT_CIGAM) {
        swap = 1;
        swap_fat_header(&fh, NXHostByteOrder());
      }

      int i;
      for (i = 0; i < fh.nfat_arch; ++i) {
        struct fat_arch fa {};
        res = read(fd, &fa, sizeof(fa));
        if (res < sizeof(fa)) {
          goto error_out;
        }

        if (swap)
          swap_fat_arch(&fa, 1, NXHostByteOrder());

        off_t save = lseek(fd, 0, SEEK_CUR);
        lseek(fd, (off_t)fa.offset, SEEK_SET);

        register_mach_header(database, build, project, filename, &fa, fd, isMachO);

        lseek(fd, save, SEEK_SET);
      }
    } else {
      lseek(fd, 0, SEEK_SET);
      register_mach_header(database, build, project, filename, nullptr, fd, isMachO);
    }
  error_out:
    return 0;
  }

  static int create_tables(DBDatabase &database) {
    std::string files_table =
        "CREATE TABLE files (build text, project text, path text)";
    std::string files_index =
        "CREATE INDEX files_index ON files (build, project, path)";
    std::string unresolved_dependencies_table =
        "CREATE TABLE unresolved_dependencies (build text, project text, type text, dependency)";
    std::string mach_o_objects_table =
        "CREATE TABLE mach_o_objects (serial INTEGER PRIMARY KEY AUTOINCREMENT, magic INTEGER, type INTEGER, cputype INTEGER, cpusubtype INTEGER, flags INTEGER, build TEXT, project TEXT, path TEXT)";
    std::string mach_o_symbols_table =
        "CREATE TABLE mach_o_symbols (mach_o_object INTEGER, type INTEGER, value INTEGER, name TEXT)";

    try {
      database.query(files_table);
      database.query(files_index);
      database.query(unresolved_dependencies_table);
      database.query(mach_o_objects_table);
      database.query(mach_o_symbols_table);
    } catch (std::exception &err) {
      //          std::cout << "exists..............\n";
    }
    return 0;
  }

  int prune_old_entries(DBDatabase &database,
                               const std::string &build,
                               const std::string &project) {
    std::stringstream stream{};
    stream << R"(DELETE FROM files WHERE "build"=")" << build
           << R"(" AND "project"=")" << project << "\"";
    try {
      database.query(stream.str());
    } catch (std::exception &e) {
    }
    stream.str(""); // clear string

    stream << R"(DELETE FROM unresolved_dependencies WHERE "build"=")" << build
           << R"(" AND "project"=")" << project << "\"";
    try {
      database.query(stream.str());
    } catch (std::exception &e) {
    }
    stream.str(""); // clear string

    stream << R"(DELETE FROM mach_o_objects WHERE "build"=")" << build
           << R"(" AND "project"=")" << project << "\"";
    try {
      database.query(stream.str());
      database.query(R"(DELETE FROM mach_o_symbols WHERE "mach_o_object" NOT IN (SELECT serial FROM mach_o_objects))");
    } catch (std::exception &e) {
    }

    return 0;
  }

  std::string format(const std::string format, ...){
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

  int register_files(DBDatabase &database,
                     const std::string &build,
                     const std::string &project,
                     const std::string &path) {
    ssize_t res = 0;
    int loaded = 0;
    std::stringstream stream;
    create_tables(database);
    database.query("BEGIN");
    prune_old_entries(database, build, project);

    //
    // Enumerate the files in the path (DSTROOT) and associate them
    // with the project name and version in the sqlitemodern database.
    // Uses ent->fts_number to mark which files we have and have
    // not seen before.
    //
    // Skip the first result, since that is . of the DSTROOT itself.
    char *path_argv[] = {(char *)path.c_str(), nullptr};
    FTS *fts = fts_open(path_argv, FTS_PHYSICAL | FTS_COMFOLLOW | FTS_XDEV, compare);
    FTSENT *ent = fts_read(fts); // throw away the entry for the DSTROOT itself
    while ((ent = fts_read(fts)) != nullptr) {
      char filename[MAXPATHLEN + 1];
      char symlink[MAXPATHLEN + 1];
      ssize_t len;

      // Filename
      filename[0] = 0;
      ent_filename(ent, filename, MAXPATHLEN);

      // Symlinks
      symlink[0] = 0;
      if (ent->fts_info == FTS_SL || ent->fts_info == FTS_SLNONE) {
        len = readlink(ent->fts_accpath, symlink, MAXPATHLEN);
        if (len >= 0)
          symlink[len] = 0;
      }

      // Default to empty SHA-1 checksum
      std::string checksum = "                                        ";

      // Checksum regular files
      if (ent->fts_info == FTS_F) {
        int fd = open(ent->fts_accpath, O_RDONLY);
        if (fd == -1) {
          perror(filename);
          return -1;
        }
        int isMachO;
        res = register_libraries(database, fd, build, project, filename, &isMachO);
        lseek(fd, (off_t)0, SEEK_SET);
        checksum = calculate_digest(fd);
        close(fd);
      }

      // register regular files and symlinks in the DB
      if (ent->fts_info == FTS_F || ent->fts_info == FTS_SL ||
          ent->fts_info == FTS_SLNONE) {
        std::stringstream local_stream{};
        local_stream << "INSERT INTO files(build, project, path) VALUES(\""
                     << build << "\",\""
                     << project << "\",\""
                     << filename << "\")";
        try {
          database.query(local_stream.str());
        } catch (std::exception &e) {}
        ++loaded;
      }

      // add all regular files, directories, and symlinks to the manifest
      if (ent->fts_info == FTS_F || ent->fts_info == FTS_D ||
          ent->fts_info == FTS_SL || ent->fts_info == FTS_SLNONE) {
//        stream << this->format("%s %o %d %d %lld .%s%s%s",
//                               checksum.c_str(),
//                               ent->fts_statp->st_mode, ent->fts_statp->st_uid,
//                               ent->fts_statp->st_gid,
//                               (ent->fts_info != FTS_D) ? ent->fts_statp->st_size : (off_t)0,
//                               filename, symlink[0] ? " -> " : "", symlink[0] ? symlink : "");
        stream << checksum << " ";
        stream << std::oct << ent->fts_statp->st_mode << " ";
        stream << ent->fts_statp->st_uid << " ";
        stream << ent->fts_statp->st_gid << " ";
        stream << ((ent->fts_info != FTS_D) ? ent->fts_statp->st_size : (off_t)0) << " ";
        stream <<  filename << (symlink[0] ? " -> " : "") << (symlink[0] ? symlink : "");
        stream << std::endl;
      }
    }
    fts_close(fts);

    database.query("COMMIT");

    std::cout << stream.str() << std::endl;
    res = setReceiptData(stream.str());
    std::cerr << project << " - " << loaded << " files registered.\n";
    return (int)res;

  }

  int setReceiptData(const std::string &receiptData){
    std::vector<char> data(receiptData.begin(), receiptData.end());
    auto shaFilename = calculate_digest(data);
    this->m_data.emplace_back("", ""); // so that m_data is never empty
    this->m_data[0] = shaFilename;
    this->m_data[1] = receiptData;
    return 0;
  }
};

DECLARE_PLUGIN(RegisterFiles, registerFiles);

} // namespace plugin