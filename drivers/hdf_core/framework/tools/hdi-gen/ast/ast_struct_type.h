/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_ASTSTRUCTTYPE_H
#define OHOS_HDI_ASTSTRUCTTYPE_H

#include <tuple>
#include <vector>

#include "ast/ast_attribute.h"
#include "ast/ast_type.h"
#include "util/autoptr.h"

namespace OHOS {
namespace HDI {
class ASTStructType : public ASTType {
public:
    ASTStructType() : ASTType(TypeKind::TYPE_STRUCT, true), attr_(new ASTAttr()), members_() {}

    inline void SetName(const std::string &name) override
    {
        name_ = name;
    }

    inline std::string GetName() override
    {
        return name_;
    }

    inline void SetAttribute(const AutoPtr<ASTAttr> &attr)
    {
        if (attr != nullptr) {
            attr_ = attr;
        }
    }

    inline bool IsFull()
    {
        return attr_ != nullptr ? attr_->HasValue(ASTAttr::FULL) : false;
    }

    inline bool IsLite()
    {
        return attr_ != nullptr ? attr_->HasValue(ASTAttr::LITE) : false;
    }

    void SetParentType(const AutoPtr<ASTStructType> &parentType);

    void AddMember(const AutoPtr<ASTType> &typeName, std::string name);

    inline std::vector<std::tuple<std::string, AutoPtr<ASTType>>> GetMembers()
    {
        return members_;
    }

    inline size_t GetMemberNumber()
    {
        return members_.size();
    }

    inline std::string GetMemberName(size_t index)
    {
        if (index >= members_.size()) {
            return std::string("");
        }
        return std::get<0>(members_[index]);
    }

    inline AutoPtr<ASTType> GetMemberType(size_t index)
    {
        if (index >= members_.size()) {
            return nullptr;
        }
        return std::get<1>(members_[index]);
    }

    bool IsStructType() override;

    std::string Dump(const std::string &prefix) override;

    TypeKind GetTypeKind() override;

    std::string EmitCType(TypeMode mode = TypeMode::NO_MODE) const override;

    std::string EmitCppType(TypeMode mode = TypeMode::NO_MODE) const override;

    std::string EmitJavaType(TypeMode mode, bool isInnerType = false) const override;

    std::string EmitCTypeDecl() const;

    std::string EmitCppTypeDecl() const;

    std::string EmitJavaTypeDecl() const;

    void EmitCWriteVar(const std::string &parcelName, const std::string &name, const std::string &ecName,
        const std::string &gotoLabel, StringBuilder &sb, const std::string &prefix) const override;

    void EmitCProxyReadVar(const std::string &parcelName, const std::string &name, bool isInnerType,
        const std::string &ecName, const std::string &gotoLabel, StringBuilder &sb,
        const std::string &prefix) const override;

    void EmitCStubReadVar(const std::string &parcelName, const std::string &name, const std::string &ecName,
        const std::string &gotoLabel, StringBuilder &sb, const std::string &prefix) const override;

    void EmitCppWriteVar(const std::string &parcelName, const std::string &name, StringBuilder &sb,
        const std::string &prefix, unsigned int innerLevel = 0) const override;

    void EmitCppReadVar(const std::string &parcelName, const std::string &name, StringBuilder &sb,
        const std::string &prefix, bool initVariable, unsigned int innerLevel = 0) const override;

    void EmitCMarshalling(const std::string &name, StringBuilder &sb, const std::string &prefix) const override;

    void EmitCUnMarshalling(const std::string &name, const std::string &gotoLabel, StringBuilder &sb,
        const std::string &prefix, std::vector<std::string> &freeObjStatements) const override;

    void EmitCppMarshalling(const std::string &parcelName, const std::string &name, StringBuilder &sb,
        const std::string &prefix, unsigned int innerLevel = 0) const override;

    void EmitCppUnMarshalling(const std::string &parcelName, const std::string &name, StringBuilder &sb,
        const std::string &prefix, bool emitType, unsigned int innerLevel = 0) const override;

    void EmitMemoryRecycle(const std::string &name, bool ownership, StringBuilder &sb,
        const std::string &prefix) const override;

private:
    AutoPtr<ASTAttr> attr_;
    std::vector<std::tuple<std::string, AutoPtr<ASTType>>> members_;
    AutoPtr<ASTStructType> parentType_; // used to dump parent type when using struct extension identify in idl
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_ASTSTRUCTTYPE_H