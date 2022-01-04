/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef __plist_Format_BaseXMLParser_h
#define __plist_Format_BaseXMLParser_h

#include <plist/Base.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <libxml/xmlreader.h>

namespace plist {
namespace Format {

class BaseXMLParser {
private:
    ::xmlTextReaderPtr _parser;

private:
    bool               _errored;
    std::string        _error;
    size_t             _line;
    size_t             _column;

public:
    BaseXMLParser();

public:
    std::string const &error() const
    { return _error; }
    size_t line() const
    { return _line; }
    size_t column() const
    { return _column; }

protected:
    bool parse(std::vector<uint8_t> const &contents);

protected:
    virtual void onBeginParse();
    virtual void onEndParse(bool success);

protected:
    virtual void onStartElement(std::string const &name, std::unordered_map<std::string, std::string> const &attrs, size_t depth);
    virtual void onEndElement(std::string const &name, size_t depth);
    virtual void onCharacterData(std::string const &cdata, size_t depth);

protected:
    void error(std::string format, ...);
};

}
}

#endif  // !__plist_Format_BaseXMLParser_h
