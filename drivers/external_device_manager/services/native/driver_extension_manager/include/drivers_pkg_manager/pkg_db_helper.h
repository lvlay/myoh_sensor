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

#ifndef PKG_DB_HELPER_H
#define PKG_DB_HELPER_H

#include "data_ability_predicates.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"
#include "pkg_database.h"
#include "value_object.h"
#include "pkg_tables.h"
#include <unordered_set>

namespace OHOS {
namespace ExternalDeviceManager {
class PkgDbHelper {
public:
    static std::shared_ptr<PkgDbHelper> GetInstance();

    /* update (user, device, app) record */
    int32_t QueryAllDriverInfos(std::vector<std::string> &driverInfos);
    int32_t QueryPkgInfos(std::vector<PkgInfoTable> &pkgInfos,
        bool isByDriverUid = false, const std::string &driverUid = "");
    /* add or update (user, device, app) record */
    int32_t AddOrUpdateRightRecord(
        const std::string &bundleName, const std::string &bundleAbility, const std::string &driverInfo);
    std::string QueryBundleInfoNames(const std::string &driverInfo);
    int32_t QueryAllBundleAbilityNames(const std::string &bundleName, std::vector<std::string> &bundleAbilityNames);
    /* delete (user, device, app) record */
    int32_t DeleteRightRecord(const std::string &bundleName);
    
    int32_t CheckIfNeedUpdateEx(
        bool &isUpdate, const std::string &bundleName);
    int32_t QueryAllSize(std::vector<std::string> &allBundleAbility);
    int32_t AddOrUpdatePkgInfo(const std::vector<PkgInfoTable> &pkgInfos, const std::string &bundleName = "");

private:
    PkgDbHelper();
    DISALLOW_COPY_AND_MOVE(PkgDbHelper);
    int32_t AddOrUpdateRightRecordEx(bool isUpdate, const std::string &bundleName,
        const std::string &bundleAbility, const std::string &driverInfo);
    int32_t QueryAndGetResultColumnValues(const OHOS::NativeRdb::RdbPredicates &rdbPredicates,
        const std::vector<std::string> &columns, const std::string &columnName, std::vector<std::string> &columnValues);
    int32_t DeleteAndNoOtherOperation(const std::string &whereClause, const std::vector<std::string> &whereArgs);

    static std::shared_ptr<PkgDbHelper> instance_;
    std::mutex databaseMutex_;
    std::shared_ptr<PkgDataBase> rightDatabase_;
};
} // namespace USB
} // namespace OHOS

#endif
