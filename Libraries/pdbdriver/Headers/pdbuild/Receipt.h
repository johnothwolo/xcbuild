//
//  Receipt.hpp
//  darwibuild
//
//  Created by John Othwolo on 12/8/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//

#ifndef Receipt_hpp
#define Receipt_hpp

#include <string>

namespace libutil { class Filesystem; }
namespace process { class PDBContext; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pdbuild {

class Options;

class Receipt {
public:
    typedef enum { SOURCES, HEADERS } ReceiptType;
private:
    Receipt(process::PDBContext *context,
          libutil::Filesystem *filesystem,
          Options const &options,
          Receipt::ReceiptType type);
public:
    Receipt();
   ~Receipt();
    
public:
    static constexpr char* RECEIPTDIR = (char*)"/usr/local/pdbuild/receipts";
private:
    bool m_onDisk = false;
    ReceiptType m_type;
    Options const &m_options;
    
public:
    static Receipt GetReceipt(process::PDBContext *context,
                            libutil::Filesystem *filesystem,
                            Options const &options, Receipt::ReceiptType type);
public:
    int existsOnDisk();
    int createOnDisk(process::PDBContext *processContext,
                     libutil::Filesystem *filesystem,
                     Options const &options,
                     const std::string &dstroot);
};

}

#endif /* Receipt_hpp */
