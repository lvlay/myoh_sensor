#include "v1_0/maniu_callback_service.h"
#include <hdf_base.h>
#include <hdf_log.h>
#define HDF_LOG_TAG    maniu_callback_service

namespace OHOS {
namespace HDI {
namespace Maniu {
namespace V1_0 {
int32_t ManiuCallbackService::OnDataEvent(const OHOS::HDI::Maniu::V1_0::ManiuEvents& event)
{
    HDF_LOGI("%{public}s: Enter ", __func__);
    return HDF_SUCCESS;
}

} // V1_0
} // Maniu
} // HDI
} // OHOS
