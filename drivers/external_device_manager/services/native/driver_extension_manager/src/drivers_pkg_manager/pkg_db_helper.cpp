/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pkg_db_helper.h"
#include "bundle_installer_interface.h"
#include "hilog_wrapper.h"
#include "pkg_database.h"

using namespace OHOS::NativeRdb;

namespace OHOS {
namespace ExternalDeviceManager {
std::shared_ptr<PkgDbHelper> PkgDbHelper::instance_;
bool g_dbInitSucc = false;

PkgDbHelper::PkgDbHelper()
{
    rightDatabase_ = PkgDataBase::GetInstance();
    g_dbInitSucc = rightDatabase_->InitDB();
}

std::shared_ptr<PkgDbHelper> PkgDbHelper::GetInstance()
{
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> guard(instanceMutex);
    if (instance_ == nullptr || !g_dbInitSucc) {
        EDM_LOGE(MODULE_PKG_MGR, "PkgDbHelper reset to new instance");
        instance_.reset(new PkgDbHelper());
    }
    return instance_;
}

int32_t PkgDbHelper::DeleteAndNoOtherOperation(
    const std::string &whereClause, const std::vector<std::string> &whereArgs)
{
    int32_t ret = rightDatabase_->BeginTransaction();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction error: %{public}d", ret);
        return ret;
    }
    int32_t changedRows = 0;
    ret = rightDatabase_->Delete(changedRows, whereClause, whereArgs);
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Delete error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
        return ret;
    }
    ret = rightDatabase_->Commit();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
    }
    return ret;
}

int32_t PkgDbHelper::DeleteRightRecord(const std::string &bundleName)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::string whereClause = {"bundleName = ?"};
    std::vector<std::string> whereArgs = {bundleName};
    int32_t ret = DeleteAndNoOtherOperation(whereClause, whereArgs);
    if (ret != PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "failed: detale(uid, dev, app): %{public}d", ret);
    }
    return ret;
}

int32_t PkgDbHelper::AddOrUpdatePkgInfo(const std::vector<PkgInfoTable> &pkgInfos, const std::string &bundleName)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int32_t ret = rightDatabase_->BeginTransaction();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction error: %{public}d", ret);
        return ret;
    }

    int32_t changedRows = 0;
    std::string whereClause = "";
    std::vector<std::string> whereArgs;
    if (!bundleName.empty()) {
        whereClause.append("bundleName = ?");
        whereArgs.emplace_back(bundleName);
    }
    ret = rightDatabase_->Delete(changedRows, whereClause, whereArgs);
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "delete error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
        return ret;
    }

    ValuesBucket values;
    for (const auto &pkgInfo: pkgInfos) {
        values.Clear();
        values.PutString("driverUid", pkgInfo.driverUid);
        values.PutLong("userId", pkgInfo.userId);
        values.PutLong("appIndex", pkgInfo.appIndex);
        values.PutString("bundleAbility", pkgInfo.bundleAbility);
        values.PutString("bundleName", pkgInfo.bundleName);
        values.PutString("driverName", pkgInfo.driverName);
        values.PutString("driverInfo", pkgInfo.driverInfo);
        ret = rightDatabase_->Insert(values);
        if (ret < PKG_OK) {
            EDM_LOGE(MODULE_PKG_MGR, "Insert error: %{public}d", ret);
            (void)rightDatabase_->RollBack();
            return ret;
        }
    }
    ret = rightDatabase_->Commit();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
    }
    return ret;
}

int32_t PkgDbHelper::AddOrUpdateRightRecord(
    const std::string & bundleName, const std::string & bundleAbility, const std::string &driverInfo)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    int32_t ret = rightDatabase_->BeginTransaction();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction error: %{public}d", ret);
        return ret;
    }
    bool isUpdate = false;
    ret = CheckIfNeedUpdateEx(isUpdate, bundleAbility);
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "check if need update error: %{public}d", ret);
        return ret;
    }
    ret = AddOrUpdateRightRecordEx(isUpdate, bundleName, bundleAbility, driverInfo);
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "add or update error: %{public}d", ret);
        return ret;
    }
    ret = rightDatabase_->Commit();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
    }
    return ret;
}

int32_t PkgDbHelper::CheckIfNeedUpdateEx(
    bool &isUpdate, const std::string &bundleAbility)
{
    std::vector<std::string> columns;
    RdbPredicates rdbPredicates(PKG_TABLE_NAME);
    rdbPredicates.BeginWrap()
        ->EqualTo("bundleAbility", bundleAbility)
        ->EndWrap();
    auto resultSet = rightDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Query error");
        (void)rightDatabase_->RollBack();
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    int32_t rowCount = 0;
    if (resultSet->GetRowCount(rowCount) != E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "GetRowCount error");
        (void)rightDatabase_->RollBack();
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    isUpdate = (rowCount > 0 ? true : false);

    return PKG_OK;
}

int32_t PkgDbHelper::AddOrUpdateRightRecordEx(bool isUpdate,
    const std::string & bundleName, const std::string & bundleAbility, const std::string &driverInfo)
{
    int32_t ret = 0;
    ValuesBucket values;
    values.Clear();
    values.PutString("bundleName", bundleName);
    values.PutString("bundleAbility", bundleAbility);
    values.PutString("driverInfo", driverInfo);
    EDM_LOGI(MODULE_PKG_MGR, "bundleName: %{public}s driverInfo: %{public}s",
        bundleName.c_str(), driverInfo.c_str());
    if (isUpdate) {
        int32_t changedRows = 0;
        ret = rightDatabase_->Update(changedRows, values, "bundleAbility = ?",
            std::vector<std::string> {bundleAbility});
    } else {
        ret = rightDatabase_->Insert(values);
    }
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Insert or Update error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
    }
    return ret;
}

int32_t PkgDbHelper::QueryAllDriverInfos(std::vector<std::string> &driverInfos)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::vector<std::string> columns = {"driverInfo"};
    RdbPredicates rdbPredicates(PKG_TABLE_NAME);
    return QueryAndGetResultColumnValues(rdbPredicates, columns, "driverInfo", driverInfos);
}

int32_t PkgDbHelper::QueryAllBundleAbilityNames(const std::string &bundleName,
    std::vector<std::string> &bundleAbilityNames)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::vector<std::string> columns = {"bundleAbility"};
    RdbPredicates rdbPredicates(PKG_TABLE_NAME);
    rdbPredicates.EqualTo("bundleName", bundleName)->Distinct();
    return QueryAndGetResultColumnValues(rdbPredicates, columns, "bundleAbility", bundleAbilityNames);
}

static bool ParseToPkgInfos(const std::shared_ptr<ResultSet> &resultSet, std::vector<PkgInfoTable> &pkgInfos)
{
    if (resultSet == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "resultSet is nullptr");
        return false;
    }
    int32_t rowCount = 0;
    int32_t driverUidIndex = 0;
    int32_t bundleNameIndex = 0;
    int32_t driverNameIndex = 0;
    int32_t driverInfoIndex = 0;
    if (resultSet->GetRowCount(rowCount) != E_OK
        || resultSet->GetColumnIndex("driverUid", driverUidIndex) != E_OK
        || resultSet->GetColumnIndex("bundleName", bundleNameIndex) != E_OK
        || resultSet->GetColumnIndex("driverName", driverNameIndex) != E_OK
        || resultSet->GetColumnIndex("driverInfo", driverInfoIndex) != E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "get table info failed");
        return false;
    }
    EDM_LOGD(MODULE_PKG_MGR, "rowCount=%{public}d", rowCount);
    bool endFlag = false;
    for (int32_t i = 0; i < rowCount && !endFlag; i++, resultSet->IsEnded(endFlag)) {
        if (resultSet->GoToRow(i) != E_OK) {
            EDM_LOGE(MODULE_PKG_MGR, "GoToRow %{public}d", i);
            return false;
        }
        
        PkgInfoTable pkgInfo;
        if (resultSet->GetString(driverUidIndex, pkgInfo.driverUid) != E_OK
            || resultSet->GetString(bundleNameIndex, pkgInfo.bundleName) != E_OK
            || resultSet->GetString(driverNameIndex, pkgInfo.driverName) != E_OK
            || resultSet->GetString(driverInfoIndex, pkgInfo.driverInfo) != E_OK) {
            EDM_LOGE(MODULE_PKG_MGR, "GetString failed");
            return false;
        }
        pkgInfos.push_back(pkgInfo);
    }
    return true;
}

int32_t PkgDbHelper::QueryPkgInfos(std::vector<PkgInfoTable> &pkgInfos,
    bool isByDriverUid, const std::string &driverUid)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::vector<std::string> columns = { "driverUid", "bundleName", "driverName", "driverInfo" };
    RdbPredicates rdbPredicates(PKG_TABLE_NAME);
    if (isByDriverUid) {
        rdbPredicates.EqualTo("driverUid", driverUid);
    }
    int32_t ret = rightDatabase_->BeginTransaction();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction error: %{public}d", ret);
        return ret;
    }
    auto resultSet = rightDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Query error");
        (void)rightDatabase_->RollBack();
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    ret = rightDatabase_->Commit();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
        return ret;
    }
    if (!ParseToPkgInfos(resultSet, pkgInfos)) {
        EDM_LOGE(MODULE_PKG_MGR, "ParseToPkgInfos failed");
        return PKG_FAILURE;
    }

    return static_cast<int32_t>(pkgInfos.size());
}

int32_t PkgDbHelper::QueryAllSize(std::vector<std::string> &allBundleAbility)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::vector<std::string> columns = {"bundleAbility"};
    RdbPredicates rdbPredicates(PKG_TABLE_NAME);
    return QueryAndGetResultColumnValues(rdbPredicates, columns, "bundleAbility", allBundleAbility);
}

int32_t PkgDbHelper::QueryAndGetResultColumnValues(const RdbPredicates &rdbPredicates,
    const std::vector<std::string> &columns, const std::string &columnName, std::vector<std::string> &columnValues)
{
    int32_t ret = rightDatabase_->BeginTransaction();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction error: %{public}d", ret);
        return ret;
    }
    auto resultSet = rightDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Query error");
        (void)rightDatabase_->RollBack();
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    ret = rightDatabase_->Commit();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
        return ret;
    }
    int32_t rowCount = 0;
    int32_t columnIndex = 0;
    if (resultSet->GetRowCount(rowCount) != E_OK || resultSet->GetColumnIndex(columnName, columnIndex) != E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "get table info failed");
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    bool endFlag = false;
    for (int32_t i = 0; (i < rowCount) && !endFlag; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            EDM_LOGE(MODULE_PKG_MGR, "GoToRow %{public}d", i);
            return PKG_RDB_EXECUTE_FAILTURE;
        }
        std::string tempStr;
        if (resultSet->GetString(columnIndex, tempStr) == E_OK) {
            columnValues.push_back(tempStr);
        }
        resultSet->IsEnded(endFlag);
    }
    int32_t position = 0;
    resultSet->GetRowIndex(position);
    resultSet->IsEnded(endFlag);
    EDM_LOGI(MODULE_PKG_MGR, "idx=%{public}d rows=%{public}d pos=%{public}d ret=%{public}zu end=%{public}s",
        columnIndex, rowCount, position, columnValues.size(), (endFlag ? "yes" : "no"));
    return columnValues.size();
}

std::string PkgDbHelper::QueryBundleInfoNames(const std::string &driverInfo)
{
    std::lock_guard<std::mutex> guard(databaseMutex_);
    std::vector<std::string> columns = {"bundleAbility"};
    RdbPredicates rdbPredicates(PKG_TABLE_NAME);
    rdbPredicates.EqualTo("driverInfo", driverInfo)->Distinct();
    int32_t ret = rightDatabase_->BeginTransaction();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction error: %{public}d", ret);
        return "";
    }
    auto resultSet = rightDatabase_->Query(rdbPredicates, columns);
    if (resultSet == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Query error");
        (void)rightDatabase_->RollBack();
        return "";
    }
    ret = rightDatabase_->Commit();
    if (ret < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit error: %{public}d", ret);
        (void)rightDatabase_->RollBack();
        return "";
    }
    int32_t rowCount = 0;
    ret = resultSet->GetRowCount(rowCount);
    if (ret != E_OK || rowCount == 0) {
        EDM_LOGE(MODULE_PKG_MGR, "Query data error: %{public}d, count: %{public}d", ret, rowCount);
        return "";
    }
    ret = resultSet->GoToRow(0);
    if (ret != E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "GoToRow 0 error: %{public}d", ret);
        return "";
    }
    std::string s;
    ret = resultSet->GetString(0, s);
    if (ret != E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "get value error: %{public}d", ret);
        return "";
    }
    return s;
}
} // namespace USB
} // namespace OHOS
