/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/java_client_proxy_code_emitter.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
bool JavaClientProxyCodeEmitter::ResolveDirectory(const std::string &targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE || ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        directory_ = GetFileParentPath(targetDirectory);
    } else {
        return false;
    }

    if (!File::CreateParentDir(directory_)) {
        Logger::E("CppClientInterfaceCodeEmitter", "Create '%s' failed!", directory_.c_str());
        return false;
    }

    return true;
}

void JavaClientProxyCodeEmitter::EmitCode()
{
    if (mode_ == GenMode::IPC) {
        EmitProxyFile();
    }
}

void JavaClientProxyCodeEmitter::EmitProxyFile()
{
    std::string filePath =
        File::AdapterPath(StringHelper::Format("%s/%s.java", directory_.c_str(), FileName(proxyName_).c_str()));
    File file(filePath, File::WRITE);
    StringBuilder sb;

    EmitLicense(sb);
    EmitPackage(sb);
    sb.Append("\n");
    EmitProxyImports(sb);
    sb.Append("\n");
    EmitProxyImpl(sb);

    std::string data = sb.ToString();
    file.WriteData(data.c_str(), data.size());
    file.Flush();
    file.Close();
}

void JavaClientProxyCodeEmitter::EmitProxyImports(StringBuilder &sb) const
{
    EmitProxyCorelibImports(sb);
    EmitProxySelfDefinedTypeImports(sb);
    EmitProxyDBinderImports(sb);
}

void JavaClientProxyCodeEmitter::EmitProxyCorelibImports(StringBuilder &sb) const
{
    bool includeList = false;
    bool includeMap = false;
    const AST::TypeStringMap &types = ast_->GetTypes();
    for (const auto &pair : types) {
        AutoPtr<ASTType> type = pair.second;
        switch (type->GetTypeKind()) {
            case TypeKind::TYPE_LIST: {
                if (!includeList) {
                    sb.Append("import java.util.List;\n");
                    includeList = true;
                }
                break;
            }
            case TypeKind::TYPE_MAP: {
                if (!includeMap) {
                    sb.Append("import java.util.Map;\n");
                    sb.Append("import java.util.HashMap;\n");
                    includeMap = true;
                }
                break;
            }
            default:
                break;
        }
    }
}

void JavaClientProxyCodeEmitter::EmitProxySelfDefinedTypeImports(StringBuilder &sb) const
{
    for (const auto &importPair : ast_->GetImports()) {
        AutoPtr<AST> import = importPair.second;
        sb.AppendFormat("import %s;\n", import->GetFullName().c_str());
    }
}

void JavaClientProxyCodeEmitter::EmitProxyDBinderImports(StringBuilder &sb) const
{
    sb.Append("import ohos.hiviewdfx.HiLog;\n");
    sb.Append("import ohos.hiviewdfx.HiLogLabel;\n");
    sb.Append("import ohos.rpc.IRemoteObject;\n");
    sb.Append("import ohos.rpc.RemoteException;\n");
    sb.Append("import ohos.rpc.MessageParcel;\n");
    sb.Append("import ohos.rpc.MessageOption;\n");
}

void JavaClientProxyCodeEmitter::EmitProxyImpl(StringBuilder &sb)
{
    sb.AppendFormat("public class %s implements %s {\n", proxyName_.c_str(), interfaceName_.c_str());
    EmitProxyConstants(sb, TAB);
    sb.Append("\n");
    sb.Append(TAB).AppendFormat(
        "private static final HiLogLabel TAG = new HiLogLabel(HiLog.LOG_CORE, 0xD002510, \"%s\");\n",
        interfaceFullName_.c_str());
    sb.Append(TAB).Append("private final IRemoteObject remote;\n");
    sb.Append(TAB).Append("private static final int ERR_OK = 0;\n");
    sb.Append("\n");
    EmitProxyConstructor(sb, TAB);
    sb.Append("\n");
    EmitProxyMethodImpls(sb, TAB);
    sb.Append("};");
}

void JavaClientProxyCodeEmitter::EmitProxyConstants(StringBuilder &sb, const std::string &prefix)
{
    sb.Append(prefix).AppendFormat(
        "private static final std::string DESCRIPTOR = \"%s\";\n\n", interfaceFullName_.c_str());
    EmitInterfaceMethodCommands(sb, prefix);
}

void JavaClientProxyCodeEmitter::EmitProxyConstructor(StringBuilder &sb, const std::string &prefix) const
{
    sb.Append(prefix).AppendFormat("public %s(IRemoteObject remote) {\n", proxyName_.c_str());
    sb.Append(prefix + TAB).Append("this.remote = remote;\n");
    sb.Append(prefix).Append("}\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("@Override\n");
    sb.Append(prefix).Append("public IRemoteObject asObject() {\n");
    sb.Append(prefix + TAB).Append("return remote;\n");
    sb.Append(prefix).Append("}\n");
}

void JavaClientProxyCodeEmitter::EmitProxyMethodImpls(StringBuilder &sb, const std::string &prefix) const
{
    for (const auto &method : interface_->GetMethodsBySystem(Options::GetInstance().GetSystemLevel())) {
        EmitProxyMethodImpl(method, sb, prefix);
        sb.Append("\n");
    }
}

void JavaClientProxyCodeEmitter::EmitProxyMethodImpl(
    const AutoPtr<ASTMethod> &method, StringBuilder &sb, const std::string &prefix) const
{
    sb.Append(prefix).Append("@Override\n");
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat(
            "public int %s() throws RemoteException ", MethodName(method->GetName()).c_str());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("public int %s(", MethodName(method->GetName()).c_str());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }
        paramStr.Append(") throws RemoteException");

        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
    EmitProxyMethodBody(method, sb, prefix);
}

void JavaClientProxyCodeEmitter::EmitInterfaceMethodParameter(
    const AutoPtr<ASTParameter> &param, StringBuilder &sb, const std::string &prefix) const
{
    sb.Append(prefix).Append(param->EmitJavaParameter());
}

void JavaClientProxyCodeEmitter::EmitProxyMethodBody(
    const AutoPtr<ASTMethod> &method, StringBuilder &sb, const std::string &prefix) const
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("MessageParcel data = MessageParcel.obtain();\n");
    sb.Append(prefix + TAB).Append("MessageParcel reply = MessageParcel.obtain();\n");
    sb.Append(prefix + TAB).AppendFormat("MessageOption option = new MessageOption(MessageOption.TF_SYNC);\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("    data.writeInterfaceToken(DESCRIPTOR);\n");

    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        param->EmitJavaWriteVar("data", sb, prefix + TAB);
    }
    sb.Append("\n");

    sb.Append(prefix + TAB).Append("try {\n");
    sb.Append(prefix + TAB + TAB).AppendFormat("if (remote.sendRequest(COMMAND_%s, data, reply, option)) {\n",
        ConstantName(method->GetName()).c_str());
    sb.Append(prefix + TAB + TAB + TAB).Append("return 1;\n");
    sb.Append(prefix + TAB + TAB).Append("}\n");
    sb.Append(prefix + TAB).Append("    reply.readException();\n");
    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        param->EmitJavaReadVar("reply", sb, prefix + TAB + TAB);
    }

    sb.Append(prefix + TAB).Append("} finally {\n");
    sb.Append(prefix + TAB + TAB).Append("data.reclaim();\n");
    sb.Append(prefix + TAB + TAB).Append("reply.reclaim();\n");
    sb.Append(prefix + TAB).Append("}\n");
    sb.Append(prefix + TAB).Append("return 0;\n");
    sb.Append(prefix).Append("}\n");
}

void JavaClientProxyCodeEmitter::EmitLocalVariable(
    const AutoPtr<ASTParameter> &param, StringBuilder &sb, const std::string &prefix) const
{
    AutoPtr<ASTType> type = param->GetType();
    if (type->GetTypeKind() == TypeKind::TYPE_SEQUENCEABLE) {
        sb.Append(prefix).AppendFormat("%s %s = new %s();\n", type->EmitJavaType(TypeMode::NO_MODE).c_str(),
            param->GetName().c_str(), type->EmitJavaType(TypeMode::NO_MODE).c_str());
    } else if (type->GetTypeKind() == TypeKind::TYPE_LIST) {
        sb.Append(prefix).AppendFormat("%s %s = new Array%s();\n", type->EmitJavaType(TypeMode::NO_MODE).c_str(),
            param->GetName().c_str(), type->EmitJavaType(TypeMode::NO_MODE).c_str());
    } else if (type->GetTypeKind() == TypeKind::TYPE_MAP) {
        sb.Append(prefix).AppendFormat("%s %s = new Hash%s();\n", type->EmitJavaType(TypeMode::NO_MODE).c_str(),
            param->GetName().c_str(), type->EmitJavaType(TypeMode::NO_MODE).c_str());
    } else {
        sb.Append(prefix).AppendFormat(
            "%s %s;\n", type->EmitJavaType(TypeMode::NO_MODE).c_str(), param->GetName().c_str());
    }
}
} // namespace HDI
} // namespace OHOS