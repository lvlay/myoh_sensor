/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PKG_DATABASE_H
#define PKG_DATABASE_H

#include <pthread.h>

#include "data_ability_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"
#include "value_object.h"

namespace OHOS {
namespace ExternalDeviceManager {
static std::string PKG_DB_PATH = "/data/service/el1/public/pkg_service/";

constexpr const char *PKG_DB_NAME = "pkg.db";
constexpr const char *PKG_TABLE_NAME = "pkgInfoTable";
constexpr int32_t DATABASE_OPEN_VERSION = 1;
constexpr int32_t DATABASE_NEW_VERSION = 2;

constexpr const char *CREATE_PKG_TABLE = "CREATE TABLE IF NOT EXISTS [pkgInfoTable]("
                                               "[id] INTEGER PRIMARY KEY AUTOINCREMENT, "
                                               "[driverUid] TEXT,"
                                               "[userId] INTEGER,"
                                               "[appIndex] INTEGER,"
                                               "[bundleAbility] TEXT,"
                                               "[bundleName] TEXT,"
                                               "[driverName] TEXT,"
                                               "[driverInfo] TEXT );";

class PkgDataBase {
public:
    static std::shared_ptr<PkgDataBase> GetInstance();
    int64_t Insert(const OHOS::NativeRdb::ValuesBucket &insertValues);
    int32_t Update(int32_t &changedRows, const OHOS::NativeRdb::ValuesBucket &values,
        const OHOS::NativeRdb::RdbPredicates &predicates);
    int32_t Update(int32_t &changedRows, const OHOS::NativeRdb::ValuesBucket &values, const std::string &whereClause,
        const std::vector<std::string> &whereArgs);
    int32_t Delete(const OHOS::NativeRdb::RdbPredicates &rdbPredicates);
    int32_t Delete(int32_t &changedRows, const std::string &whereClause, const std::vector<std::string> &whereArgs);
    std::shared_ptr<OHOS::NativeRdb::ResultSet> Query(
        const OHOS::NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns);
    int32_t BeginTransaction();
    int32_t Commit();
    int32_t RollBack();
    bool InitDB();

private:
    PkgDataBase();
    DISALLOW_COPY_AND_MOVE(PkgDataBase);
    static std::shared_ptr<PkgDataBase> instance_;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
};

class PkgDataBaseCallBack : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;
    int32_t OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int32_t currentVersion, int32_t targetVersion) override;
};
} // namespace USB
} // namespace OHOS

#endif
