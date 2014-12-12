SET(TARGET_NAME ${exif_target})
SET(DESTINATION_NAME ${exif_dest})
SET(TARGET_IMPL_NAME ${exif_impl})

PKG_CHECK_MODULES(platform_pkgs_exif REQUIRED libexif)

ADD_DEFINITIONS("-fvisibility=hidden")

IF(ENABLE_TV)
INCLUDE_DIRECTORIES(
    ${INCLUDE_COMMON}
    ${tizen_include}
    ${timeutil_include}
    ${filesystem.old_include}
    ${platform_pkgs_exif_INCLUDE_DIRS}
)
ELSE()
INCLUDE_DIRECTORIES(
    ${INCLUDE_COMMON}
    ${tizen_include}
    ${timeutil_include}
    ${filesystem_include}
    ${platform_pkgs_exif_INCLUDE_DIRS}
)
ENDIF(ENABLE_TV)

IF(ENABLE_TV)
SET(CMAKE_INSTALL_RPATH
    ${CMAKE_INSTALL_RPATH}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${timeutil_dest}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${filesystem.old_dest}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${DESTINATION_NAME}
)
ELSE()
SET(CMAKE_INSTALL_RPATH
    ${CMAKE_INSTALL_RPATH}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${timeutil_dest}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${filesystem_dest}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${DESTINATION_NAME}
)
ENDIF(ENABLE_TV)

SET(SRCS_IMPL
    JSExifManager.cpp
    ExifManager.cpp
    JSExifInformation.cpp
    ExifInformation.cpp
    ExifManagerCallbacks.cpp
    ExifUtil.cpp
    ExifTagSaver.cpp
    ExifGPSLocation.cpp
    ExifGPSTime.cpp
    Rational.cpp
    JpegFile.cpp
)

ADD_LIBRARY(${TARGET_IMPL_NAME} SHARED ${SRCS_IMPL})

TARGET_LINK_LIBRARIES(${TARGET_IMPL_NAME}
    ${LIBS_COMMON}
    ${tizen_impl}
    ${timeutil_impl}
    ${filesystem_impl}
    ${filesystem_config}
    ${platform_pkgs_exif_LIBRARIES}
)

SET(SRCS
    plugin_config.cpp
    plugin_initializer.cpp
)

ADD_LIBRARY(${TARGET_NAME} SHARED ${SRCS})

TARGET_LINK_LIBRARIES(${TARGET_NAME}
    ${TARGET_IMPL_NAME}
)

INSTALL(TARGETS ${TARGET_NAME} ${TARGET_IMPL_NAME} LIBRARY DESTINATION ${DESTINATION_LIB_PREFIX}/${DESTINATION_NAME})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.xml DESTINATION ${DESTINATION_LIB_PREFIX}/${DESTINATION_NAME})

INSTALL(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${DESTINATION_HEADER_PREFIX}/exif
    FILES_MATCHING PATTERN "*.h" PATTERN "CMakeFiles" EXCLUDE
)
