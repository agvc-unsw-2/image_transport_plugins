cmake_minimum_required(VERSION 2.8.3)
project(compressed_image_transport)

find_package(OpenCV REQUIRED)
find_package(catkin REQUIRED COMPONENTS cv_bridge dynamic_reconfigure image_transport)

# generate the dynamic_reconfigure config file
generate_dynamic_reconfigure_options(cfg/CompressedPublisher.cfg cfg/CompressedSubscriber.cfg)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS cv_bridge dynamic_reconfigure image_transport
  DEPENDS OpenCV
)

# Zstd library source files
set(ZSTD_LIB_SRCFILES
    src/zstd/bitstream.h
    src/zstd/compiler.h
    src/zstd/cover.c
    src/zstd/cpu.h
    src/zstd/divsufsort.c
    src/zstd/divsufsort.h
    src/zstd/entropy_common.c
    src/zstd/error_private.c
    src/zstd/error_private.h
    src/zstd/fse.h
    src/zstd/fse_compress.c
    src/zstd/fse_decompress.c
    src/zstd/huf.h
    src/zstd/huf_compress.c
    src/zstd/huf_decompress.c
    src/zstd/mem.h
    src/zstd/pool.c
    src/zstd/pool.h
    src/zstd/threading.c
    src/zstd/threading.h
    src/zstd/xxhash.c
    src/zstd/xxhash.h
    src/zstd/zdict.c
    src/zstd/zdict.h
    src/zstd/zstd.h
    src/zstd/zstdmt_compress.c
    src/zstd/zstdmt_compress.h
    src/zstd/zstd_common.c
    src/zstd/zstd_compress.c
    src/zstd/zstd_compress_internal.h
    src/zstd/zstd_decompress.c
    src/zstd/zstd_double_fast.c
    src/zstd/zstd_double_fast.h
    src/zstd/zstd_errors.h
    src/zstd/zstd_fast.c
    src/zstd/zstd_fast.h
    src/zstd/zstd_internal.h
    src/zstd/zstd_lazy.c
    src/zstd/zstd_lazy.h
    src/zstd/zstd_ldm.c
    src/zstd/zstd_ldm.h
    src/zstd/zstd_opt.c
    src/zstd/zstd_opt.h
)

# Zpng library source files
set(ZPNG_LIB_SRCFILES
    src/zpng.cpp
    include/compressed_image_transport/zpng.h
)

include_directories(include ${catkin_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

add_library(${PROJECT_NAME} src/compressed_publisher.cpp src/compressed_subscriber.cpp src/manifest.cpp ${ZPNG_LIB_SRCFILES} ${ZSTD_LIB_SRCFILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_gencfg)
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} libturbojpeg.so)

class_loader_hide_library_symbols(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)

# add xml file
install(FILES compressed_plugins.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
