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
#ifndef SENSOR_PARCEL_H
#define SENSOR_PARCEL_H

#include "sensor_errors.h"

namespace OHOS {
namespace Sensors {

#define WRITEBOOL(parcel, data, ...) \
    do { \
        if (!(parcel).WriteBool(data)) { \
            SEN_HILOGE("Parcel writeBool "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEINT32(parcel, data, ...) \
    do { \
        if (!(parcel).WriteInt32(data)) { \
            SEN_HILOGE("Parcel writeInt32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEINT64(parcel, data, ...) \
    do { \
        if (!(parcel).WriteInt64(data)) { \
            SEN_HILOGE("Parcel writeInt64 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEUINT32(parcel, data, ...) \
    do { \
        if (!(parcel).WriteUint32(data)) { \
            SEN_HILOGE("Parcel writeUint32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEDOUBLE(parcel, data, ...) \
    do { \
        if (!(parcel).WriteDouble(data)) { \
            SEN_HILOGE("Parcel writeDouble "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITESTRING(parcel, data, ...) \
    do { \
        if (!(parcel).WriteString(data)) { \
            SEN_HILOGE("Parcel writeString "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITESTRING16(parcel, data, ...) \
    do { \
        if (!(parcel).WriteString16(data)) { \
            SEN_HILOGE("Parcel writeString16 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEREMOTEOBJECT(parcel, data, ...) \
    do { \
        if (!(parcel).WriteRemoteObject(data)) { \
            SEN_HILOGE("Parcel writeRemoteObject "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEUINT8VECTOR(parcel, data, ...) \
    do { \
        if (!(parcel).WriteUInt8Vector(data)) { \
            SEN_HILOGE("Parcel writeUInt8Vector "#data" failed"); \
            return __VA_ARGS__; \
            } \
    } while (0)

#define READBOOL(parcel, data, ...) \
    do { \
        if (!(parcel).ReadBool(data)) { \
            SEN_HILOGE("Parcel readBool "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READINT32(parcel, data, ...) \
    do { \
        if (!(parcel).ReadInt32(data)) { \
            SEN_HILOGE("Parcel readInt32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READINT64(parcel, data, ...) \
    do { \
        if (!(parcel).ReadInt64(data)) { \
            SEN_HILOGE("Parcel readInt64 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READUINT32(parcel, data, ...) \
    do { \
        if (!(parcel).ReadUint32(data)) { \
            SEN_HILOGE("Parcel readUint32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READDOUBLE(parcel, data, ...) \
    do { \
        if (!(parcel).ReadDouble(data)) { \
            SEN_HILOGE("Parcel readDouble "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READSTRING(parcel, data, ...) \
    do { \
        if (!(parcel).ReadString(data)) { \
            SEN_HILOGE("Parcel readString "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READUINT8VECTOR(parcel, data, ...) \
    do { \
        if (!(parcel).ReadUInt8Vector(&data)) { \
            SEN_HILOGE("Parcel readUInt8Vector "#data" failed"); \
            return __VA_ARGS__; \
            } \
    } while (0)

#define READSTRING16(parcel, data, ...) \
    do { \
        if (!(parcel).ReadString16(data)) { \
            SEN_HILOGE("Parcel readString16 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_PARCEL_H