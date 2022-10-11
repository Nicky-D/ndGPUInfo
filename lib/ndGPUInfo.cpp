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

#include <dlfcn.h>
#include <string>
#include <iostream>

#include "ndGPUInfo.h"
#include "ndAMDGPU.h"
#include "ndNVGPU.h"

namespace ndgpuinfo
{
    struct gpu_info
    {
        amd::gpu mAMD;
        nv::gpu mNV;
    };

    gpu_info *init()
    {
        gpu_info *ret = new gpu_info();

        if( ret->mAMD.init() )
        {
            if( !ret->mAMD.findGPU() )
                std::cerr << "No AMD GPU found" << std::endl;
        }
        if( ret->mNV.init() )
        {
            if( !ret->mNV.findGPU())
                std::cerr << "No nvidia GPU found" << std::endl;
        }

        return ret;
    }

    void shutdown( gpu_info *aInfo )
    {
        if( !aInfo )
            return;

        aInfo->mAMD.shutdown();
        aInfo->mNV.shutdown();
        delete aInfo;
    }

    uint64_t  getTotalMemory( gpu_info *aInfo )
    {
        if( !aInfo )
            return 0;
        if( aInfo->mAMD.isValid() )
            return aInfo->mAMD.getTotalMemory();
        if( aInfo->mNV.isValid() )
            return aInfo->mNV.getTotalMemory();

        return 0;
    }

    uint64_t  getUsedMemory( gpu_info *aInfo )
    {
        if( !aInfo )
            return 0;

        if( aInfo->mAMD.isValid() )
            return aInfo->mAMD.getUsedMemory();
        if( aInfo->mNV.isValid() )
            return aInfo->mNV.getUsedMemory();
        return 0;
    }
}
