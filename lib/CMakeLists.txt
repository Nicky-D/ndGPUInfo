file(GLOB CPP_FILES *.cpp )
file(GLOB HEADER_FILES *.h )

find_path( DRM_PATH drm.h PATHS /usr/include/libdrm )

add_library(ndGPUInfo STATIC ${CPP_FILES} ${HEADER_FILES} )
target_include_directories(ndGPUInfo PRIVATE SYSTEM ${DRM_PATH} )
target_include_directories(ndGPUInfo INTERFACE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries(ndGPUInfo PUBLIC drm )

install( TARGETS ndGPUInfo DESTINATION lib )
install( FILES ndGPUInfo.h DESTINATION include/ )
