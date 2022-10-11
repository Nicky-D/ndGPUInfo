// SPDX-License-Identifier: GPL-2.0
/**
 * Copyright (C) 2022 Nicky Dasmijn
 *
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program;
 * if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**/

#include <string>
#include <dlfcn.h>

namespace ndgpuinfo
{
    namespace  nv
    {
        const std::string NV_SO{"libnvidia-ml.so"};

        typedef struct nvmlDevice *nvmlDevice_t;
        typedef uint32_t nvmlReturn_t;
        struct nvmlMemory_t
        {
            uint64_t byte_total {0};
            uint64_t byte_free {0};
            uint64_t byte_used {0};
        };

        struct gpu
        {
            void *mNVSO{nullptr};

            nvmlReturn_t (*nvmlInit)(void) {nullptr};
            nvmlReturn_t (*nvmlShutdown)(void) {nullptr};
            nvmlReturn_t (*nvmlDeviceGetCount)(unsigned int *deviceCount) {nullptr};
            nvmlReturn_t (*nvmlDeviceGetHandleByIndex)(unsigned int index, nvmlDevice_t *device) {nullptr};
            const char *(*nvmlErrorString)(nvmlReturn_t) {nullptr};
            nvmlReturn_t (*nvmlDeviceGetName)(nvmlDevice_t device, char *name, unsigned int length) {nullptr};
            nvmlReturn_t (*nvmlDeviceGetMemoryInfo)(nvmlDevice_t device, nvmlMemory_t *memory) {nullptr};

            nvmlDevice_t mDevice{nullptr};
            std::string mDeviceName;

            bool isValid( ) { return mDevice != nullptr; }

            nvmlMemory_t  getMemoryInfo()
            {
                nvmlMemory_t devMemory;
                if( !mDevice)
                    return devMemory;

                if( 0 != nvmlDeviceGetMemoryInfo( mDevice, &devMemory) )
                    return {};

                return devMemory;
            }

            uint64_t  getTotalMemory()
            {
                return getMemoryInfo().byte_total;
            }

            uint64_t  getUsedMemory()
            {
                return getMemoryInfo().byte_used;
            }

            void shutdown()
            {
                if (!mNVSO)
                    return;

                if (nvmlShutdown)
                    nvmlShutdown();

                dlclose(mNVSO);
                mNVSO = nullptr;
            }

            bool findGPU()
            {
                if (!mNVSO)
                    return false;

                unsigned int count{0};

                if (0 != nvmlDeviceGetCount(&count) || 0 == count)
                    return false;

                for (unsigned  int i = 0; mDevice == nullptr && i < count; ++i) {
                    constexpr uint32_t MAX_NAME = 4096;
                    char devName[MAX_NAME + 1] = {0};
                    nvmlMemory_t devMemory;
                    nvmlDevice_t dev;

                    if (0 != nvmlDeviceGetHandleByIndex(i, &dev))
                        continue;

                    nvmlDeviceGetName(dev, devName, MAX_NAME);
                    devName[MAX_NAME] = 0;
                    mDeviceName = devName;
                    mDevice = dev;
                }

                return mDevice != nullptr;
            }

            bool init()
            {
                if (mNVSO)
                    return true;

                mNVSO = dlopen(NV_SO.c_str(), RTLD_NOW);
                if (!mNVSO)
                    return false;

                if (!grabSyms()) {
                    dlclose(mNVSO);
                    mNVSO = nullptr;
                    return false;
                }

                if (nvmlInit() != 0) {
                    dlclose(mNVSO);
                    mNVSO = nullptr;
                    return false;
                }

                return true;
            }

            bool grabSyms()
            {
                try {
                    getSym(nvmlInit, "nvmlInit");
                    getSym(nvmlShutdown, "nvmlShutdown");
                    getSym(nvmlDeviceGetCount, "nvmlDeviceGetCount");
                    getSym(nvmlDeviceGetHandleByIndex, "nvmlDeviceGetHandleByIndex");
                    getSym(nvmlErrorString, "nvmlErrorString");
                    getSym(nvmlDeviceGetName, "nvmlDeviceGetName");
                    getSym(nvmlDeviceGetMemoryInfo, "nvmlDeviceGetMemoryInfo");
                    return true;
                }
                catch (std::exception &) {
                    return false;
                }
            }

            template<typename T>
            void getSym(T &val, char const *name)
            {
                if (!mNVSO || !name)
                    val = nullptr;
                else
                    val = reinterpret_cast<T>( dlsym(mNVSO, name));

                if (!val)
                    throw std::exception();
            }
        };
    }
}
