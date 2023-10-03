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

#pragma once

#include <cstdint>

namespace ndgpuinfo
{
    struct gpu_info;
    gpu_info* init();
    void shutdown( gpu_info* );

    uint64_t getTotalMemory( gpu_info* );
    uint64_t getUsedMemory( gpu_info* );
	float getTemperature( gpu_info* );
	uint64_t getGPULoad( gpu_info* );
}
