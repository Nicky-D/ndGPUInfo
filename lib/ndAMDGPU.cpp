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

#include <vector>
#include <string.h>
#include <unistd.h>

#include "ndAMDGPU.h"

#include "xf86drm.h"
#include <fcntl.h>
#include <radeon_drm.h>
#include <libdrm/amdgpu_drm.h>
#include <libdrm/amdgpu.h>


#pragma weak amdgpu_device_initialize
#pragma weak amdgpu_query_info
namespace ndgpuinfo
{
    namespace amd
    {
        namespace amdgpu
        {
            /* Todo, maybe cache amdgpu_device_initialize */
            uint64_t getUsedMemory(int fd)
            {
                uint32_t drm_major, drm_minor;
                amdgpu_device_handle amdgpu_dev;

                if (amdgpu_device_initialize(fd, &drm_major, &drm_minor, &amdgpu_dev))
                    return 0;

                uint64_t vramused{0};
                amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_VRAM_USAGE,
                                  sizeof(uint64_t), &vramused);
                return vramused;
            }

            uint64_t getTotalMemory(int fd)
            {
                uint32_t drm_major, drm_minor;
                amdgpu_device_handle amdgpu_dev;

                if (amdgpu_device_initialize(fd, &drm_major, &drm_minor, &amdgpu_dev))
                    return 0;

                drm_amdgpu_info_vram_gtt vram_gtt;
                if (amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_VRAM_GTT, sizeof(vram_gtt), &vram_gtt))
                    return 0;

                return vram_gtt.vram_size;
            }
        }

        namespace radeon
        {
            uint64_t getUsedMemory(int fd)
            {
                int64_t vramused{0};

                drm_radeon_info info{};

                info.value = (unsigned long) &vramused;
                info.request = RADEON_INFO_VRAM_USAGE;

                auto res = drmCommandWriteRead(fd, DRM_RADEON_INFO, &info, sizeof(info));
                if (res < 0) {
                    return 0;
                }

                return vramused;
            }

            uint64_t getTotalMemory(int fd)
            {
                drm_radeon_gem_info gem{};

                auto res = drmCommandWriteRead(fd, DRM_RADEON_GEM_INFO, &gem, sizeof(gem));
                if (res < 0) {
                    return 0;
                }

                return gem.vram_size;
            }
        }

        DRMDevice open_drm_path(const char *path)
        {
            DRMDevice dev{-1, EType::undef};
            int fd = open(path, O_RDWR);

            if (fd < 0)
                return dev;
            drmVersionPtr ver = drmGetVersion(fd);

            if (!ver) {
                close(fd);
                return dev;
            }

            // We don't seem to need this?
            /*
            drm_magic_t magic{};

            if (drmGetMagic(fd, &magic) >= 0)
            {
                if (drmAuthMagic(fd, magic) == 0)
                {
                    if (drmDropMaster(fd))
                    {
                        LL_WARNS() << "drmDropMaster failed" << LL_ENDL;
                    }
                }
            }
            */

            if (strcmp(ver->name, "radeon") == 0)
                dev = DRMDevice{fd, EType::radeon};
            else if (strcmp(ver->name, "amdgpu") == 0)
                dev = DRMDevice{fd, EType::amdgpu};
            else
                close(fd);

            drmFreeVersion(ver);
            return dev;
        }

        uint32_t VENDOR_AMD = 0x1002;

        DRMDevice probeDRMForGPU()
        {
            DRMDevice dev{-1, EType::undef};
            auto numdevs = drmGetDevices(NULL, 0);

            if (numdevs <= 0)
                return dev;

            std::vector<drmDevicePtr> vDevs;
            vDevs.resize(numdevs);

            numdevs = drmGetDevices(&vDevs[0], numdevs);

            if (numdevs <= 0)
                return dev;

            for (auto pDev: vDevs) {
                if (pDev->bustype != DRM_BUS_PCI || pDev->deviceinfo.pci->vendor_id != VENDOR_AMD)
                    continue;

                for (int32_t j = DRM_NODE_MAX - 1; dev.fd == -1 && j >= 0; --j) {
                    if (0 == (pDev->available_nodes & (1 << j)))
                        continue;

                    auto dev2 = open_drm_path(pDev->nodes[j]);
                    if (dev2.fd >= 0)
                        dev = dev2;
                }

                if (dev.fd >= 0)
                    break;
            }

            drmFreeDevices(&vDevs[0], numdevs);
            return dev;
        }

        bool gpu::findGPU()
        {
            mDevice = probeDRMForGPU();
            return mDevice.type != EType::undef;
        }

        void gpu::shutdown()
        {
            if( mDevice.fd >= 0 )
                close( mDevice.fd);

            mDevice.fd = -1;
            mDevice.type = EType::undef;
        }

        uint64_t gpu::getTotalMemory()
        {
            switch (mDevice.type) {
                case (EType::radeon):
                    return radeon::getTotalMemory(mDevice.fd);
                case (EType::amdgpu):
                    return amdgpu::getTotalMemory(mDevice.fd);
                case EType::undef:
                    break;
            }
            return 0;
        }

        uint64_t gpu::getUsedMemory()
        {
            switch (mDevice.type) {
                case (EType::radeon):
                    return radeon::getUsedMemory(mDevice.fd);
                case (EType::amdgpu):
                    return amdgpu::getUsedMemory(mDevice.fd);
                case EType::undef:
                    break;
            }
            return 0;
        }

    }
}
