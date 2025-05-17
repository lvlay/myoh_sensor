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

#include "pkg_database.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
std::shared_ptr<PkgDataBase> PkgDataBase::instance_ = nullptr;

PkgDataBase::PkgDataBase()
{
}

bool PkgDataBase::InitDB()
{
    std::string rightDatabaseName = PKG_DB_PATH + "pkg.db";
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(rightDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    PkgDataBaseCallBack sqliteOpenHelperCallback;
    store_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    if (errCode != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "GetRdbStore errCode :%{public}d", errCode);
        return false;
    } else {
        EDM_LOGE(MODULE_PKG_MGR, "GetRdbStore success :%{public}d", errCode);
        return true;
    }
}

std::shared_ptr<PkgDataBase> PkgDataBase::GetInstance()
{
    if (instance_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "PkgDataBase reset to new instance");
        instance_.reset(new PkgDataBase());
        return instance_;
    }
    return instance_;
}

int32_t PkgDataBase::BeginTransaction()
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction store_ is nullptr");
        return PKG_RDB_NO_INIT;
    }
    int32_t ret = store_->BeginTransaction();
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "BeginTransaction fail :%{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    return PKG_OK;
}

int32_t PkgDataBase::Commit()
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit store_ is nullptr");
        return PKG_RDB_NO_INIT;
    }
    int32_t ret = store_->Commit();
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Commit fail :%{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    return PKG_OK;
}

int32_t PkgDataBase::RollBack()
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "RollBack store_ is nullptr");
        return PKG_RDB_NO_INIT;
    }
    int32_t ret = store_->RollBack();
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "RollBack fail :%{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    return PKG_OK;
}

int64_t PkgDataBase::Insert(const OHOS::NativeRdb::ValuesBucket &insertValues)
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Insert store_ is  nullptr");
        return PKG_RDB_NO_INIT;
    }
    int64_t outRowId = 0;
    int32_t ret = store_->Insert(outRowId, PKG_TABLE_NAME, insertValues);
    EDM_LOGI(MODULE_PKG_MGR, "Insert id=%{public}" PRIu64 "", outRowId);
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Insert ret :%{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    return outRowId;
}

int32_t PkgDataBase::Update(
    int32_t &changedRows, const OHOS::NativeRdb::ValuesBucket &values, const OHOS::NativeRdb::RdbPredicates &predicates)
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Update(RdbPredicates) store_ is nullptr");
        return PKG_RDB_NO_INIT;
    }
    int32_t ret = store_->Update(changedRows, values, predicates);
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Update(RdbPredicates) ret :%{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    return PKG_OK;
}

int32_t PkgDataBase::Update(int32_t &changedRows, const OHOS::NativeRdb::ValuesBucket &values,
    const std::string &whereClause, const std::vector<std::string> &whereArgs)
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Update(whereClause) store_ is nullptr");
        return PKG_RDB_NO_INIT;
    }
    int32_t ret = store_->Update(changedRows, PKG_TABLE_NAME, values, whereClause, whereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Update(whereClause) ret :%{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    return PKG_OK;
}

int32_t PkgDataBase::Delete(
    int32_t &changedRows, const std::string &whereClause, const std::vector<std::string> &whereArgs)
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Delete store_ is nullptr");
        return PKG_RDB_NO_INIT;
    }
    int32_t ret = store_->Delete(changedRows, PKG_TABLE_NAME, whereClause, whereArgs);
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "Delete(whereClause) ret :%{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    return PKG_OK;
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> PkgDataBase::Query(
    const OHOS::NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns)
{
    if (store_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Query(AbsRdbPredicates) store_ is nullptr");
        return nullptr;
    }
    return store_->Query(predicates, columns);
}

int32_t PkgDataBaseCallBack::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    std::string sql = CREATE_PKG_TABLE;
    int32_t ret = store.ExecuteSql(sql);
    if (ret != OHOS::NativeRdb::E_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "OnCreate failed: %{public}d", ret);
        return PKG_RDB_EXECUTE_FAILTURE;
    }
    EDM_LOGI(MODULE_PKG_MGR, "DB OnCreate Done: %{public}d", ret);
    return PKG_OK;
}

int32_t PkgDataBaseCallBack::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    EDM_LOGI(MODULE_PKG_MGR, "DB OnUpgrade Enter");
    (void)store;
    (void)oldVersion;
    (void)newVersion;
    return PKG_OK;
}

int32_t PkgDataBaseCallBack::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    EDM_LOGI(MODULE_PKG_MGR, "DB OnDowngrade Enter");
    (void)store;
    (void)oldVersion;
    (void)newVersion;
    return PKG_OK;
}
} // namespace USB
} // namespace OHOS
