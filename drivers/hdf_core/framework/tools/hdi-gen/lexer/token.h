/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_TOKEN_H
#define OHOS_HDI_TOKEN_H

#include <string>

namespace OHOS {
namespace HDI {
enum class TokenType {
    UNKNOWN = 0,
    // types
    VOID,
    BOOLEAN, // "boolean"
    BYTE,    // "byte"
    SHORT,   // "short"
    INT,     // "int"
    LONG,    // "long"
    STRING,  // "std::string"
    FLOAT,   // "float"
    DOUBLE,  // "double"
    FD,      // "FileDescriptor"
    ASHMEM,  // "Ashmem"
    NATIVE_BUFFER, // "NativeBuffer"
    POINTER, // "Pointer"
    LIST,    // "List"
    MAP,     // "Map"
    SMQ,     // "SharedMemQueue"
    CHAR,    // "char"
    // qualifier
    UNSIGNED, // "unsigned"
    // custom types
    ENUM,   // "enum"
    STRUCT, // "struct"
    UNION,  // "union"
    // keywords
    PACKAGE,              // "package"
    SEQ,                  // "sequenceable"
    IMPORT,               // "import"
    INTERFACE,            // "interface"
    EXTENDS,              // "extends"
    ONEWAY,               // "oneway"
    CALLBACK,             // "callback"
    FULL,                 // "full"
    LITE,                 // "lite"
    MINI,                 // "mini"
    IN,                   // "in"
    OUT,                  // "out"
    DOT,                  // "."
    COMMA,                // ","
    COLON,                // ":"
    ASSIGN,               // "="
    SEMICOLON,            // ";"
    BRACES_LEFT,          // "{"
    BRACES_RIGHT,         // "}"
    BRACKETS_LEFT,        // "["
    BRACKETS_RIGHT,       // "]"
    PARENTHESES_LEFT,     // "("
    PARENTHESES_RIGHT,    // ")"
    ANGLE_BRACKETS_LEFT,  // "<"
    ANGLE_BRACKETS_RIGHT, // ">"
    ADD,                  // "+"
    SUB,                  // "-"
    STAR,                 // "*"
    SLASH,                // "/"
    PERCENT_SIGN,         // "%""
    LEFT_SHIFT,           // "<<"
    RIGHT_SHIFT,          // ">>"
    AND,                  // "&"
    XOR,                  // "^"
    OR,                   // "|"
    TILDE,                // "~"
    PPLUS,                // "++"
    MMINUS,               // "--"

    // others
    ID,
    NUM,
    COMMENT_BLOCK,
    COMMENT_LINE,
    END_OF_FILE,
};

struct Location {
    std::string filePath;
    size_t row;
    size_t col;
};

struct Token {
    TokenType kind;
    Location location;
    std::string value;

    std::string Dump();
};

struct TokenTypeCompare {
    bool operator()(const Token &lhs, const Token &rhs) const
    {
        return lhs.kind > rhs.kind;
    }
};

std::string LocInfo(const Token &token);
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_TOKEN_H