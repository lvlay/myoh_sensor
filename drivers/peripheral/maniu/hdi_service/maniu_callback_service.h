#ifndef OHOS_HDI_MANIU_V1_0_MANIUCALLBACKSERVICE_H
#define OHOS_HDI_MANIU_V1_0_MANIUCALLBACKSERVICE_H

#include "v1_0/imaniu_callback.h"

namespace OHOS {
namespace HDI {
namespace Maniu {
namespace V1_0 {
class ManiuCallbackService : public OHOS::HDI::Maniu::V1_0::IManiuCallback {
public:
    ManiuCallbackService() = default;
    virtual ~ManiuCallbackService() = default;

    int32_t OnDataEvent(const OHOS::HDI::Maniu::V1_0::ManiuEvents& event) override;

};
} // V1_0
} // Maniu
} // HDI
} // OHOS

#endif // OHOS_HDI_MANIU_V1_0_MANIUCALLBACKSERVICE_H