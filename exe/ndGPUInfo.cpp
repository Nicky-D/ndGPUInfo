
#include <iostream>
#include <string>

#include "ndGPUInfo.h"

int main( [[maybe_unused]] int i, [[maybe_unused]]  char **a )
{
    auto initVal = ndgpuinfo::init();
    std::cout << "Total memory: " << ndgpuinfo::getTotalMemory(initVal) << std::endl;
    std::cout << "Used memory : " << ndgpuinfo::getUsedMemory(initVal) << std::endl;
    std::cout << "Temperature : " << ndgpuinfo::getTemperature(initVal) << std::endl;

    ndgpuinfo::shutdown(initVal);

    return 0;
}
