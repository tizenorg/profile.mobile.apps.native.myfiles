CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(myfile C)
SET(DEBFILE_NAME org.tizen.myfile)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(SRC-DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(UTILITY-DIR ${SRC-DIR}/common)
SET(FS-DIR ${UTILITY-DIR}/file-system)
SET(FO-DIR ${UTILITY-DIR}/file-operation)
SET(WIDGET-DIR ${SRC-DIR}/widget)
SET(MEDIADB-DIR ${SRC-DIR}/media-db)
SET(MAININCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

SET(PKG_NAME "${CMAKE_INSTALL_PKG_NAME}")
SET(DESKTOP_DIR ${CMAKE_DESKTOP_DIR})

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(SHARE ${CMAKE_APP_SHARE_PACKAGES_DIR})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "${CMAKE_INSTALL_DATA_DIR}")
SET(LOCALEDIR "${RESDIR}/locale")
SET(IMAGEDIR "${RESDIR}/images")
SET(EDJDIR "${RESDIR}/edje")
SET(XMLDIR "${SHARE}")


INCLUDE_DIRECTORIES(${MAININCLUDE})
INCLUDE_DIRECTORIES(${SRC-DIR}/thumbnail)

SET( SRCS
	${SRC-DIR}/mf-main.c
	${UTILITY-DIR}/mf-callback.c
	${UTILITY-DIR}/mf-util.c
	${UTILITY-DIR}/mf-file-util.c
	${UTILITY-DIR}/mf-ta.c
	${UTILITY-DIR}/mf-fm-svc-wrapper.c
	${UTILITY-DIR}/mf-inotify-handle.c
	${UTILITY-DIR}/mf-launch.c
	${UTILITY-DIR}/mf-log.c
	${UTILITY-DIR}/mf-media-content.c
	${UTILITY-DIR}/mf-media-data.c
	${UTILITY-DIR}/mf-error.c
	${UTILITY-DIR}/mf-fs-monitor.c
	${UTILITY-DIR}/mf-download-app.c
	${UTILITY-DIR}/mf-detail.c
	${UTILITY-DIR}/mf-ug-detail-fs.c
	${UTILITY-DIR}/mf-ug-detail-media.c

# END_of START_PUBLIC_REMOVED_STRING
    #-----	File operation related ----#
    ${FO-DIR}/mf-cancel.c
    ${FO-DIR}/mf-copy.c
    ${FO-DIR}/mf-copy-internal.c
    ${FO-DIR}/mf-delete.c
    ${FO-DIR}/mf-delete-internal.c
    ${FO-DIR}/mf-fo-internal.c
    ${FO-DIR}/mf-move.c
    ${FO-DIR}/mf-move-internal.c
    ${FO-DIR}/mf-request.c
    ${FO-DIR}/mf-search.c
    ${FO-DIR}/mf-search-internal.c

    #----	File system related	----#
	${FS-DIR}/mf-file-attr.c
	${FS-DIR}/mf-fs-oper.c

    #----	Widget related	----#
	${WIDGET-DIR}/mf-focus-ui.c
	${WIDGET-DIR}/mf-search-bar.c
	${WIDGET-DIR}/mf-navi-bar.c
	${WIDGET-DIR}/mf-genlist.c
	${WIDGET-DIR}/mf-popup.c
	${WIDGET-DIR}/mf-gengrid.c
	${WIDGET-DIR}/mf-context-popup.c
	${WIDGET-DIR}/mf-tray-item.c
	${WIDGET-DIR}/mf-category-view.c
	${WIDGET-DIR}/mf-normal-view.c
	${WIDGET-DIR}/mf-object.c
	${WIDGET-DIR}/mf-object-item.c
	${WIDGET-DIR}/mf-root-view.c
	${WIDGET-DIR}/mf-search-view.c
	${WIDGET-DIR}/mf-setting-view.c
	${WIDGET-DIR}/mf-edit-view.c
	${WIDGET-DIR}/mf-view.c
	${WIDGET-DIR}/mf-storage-view.c
	${WIDGET-DIR}/mf-recent-view.c
	${WIDGET-DIR}/mf-detail-view.c

    #----   Media DB    ----#
	${MEDIADB-DIR}/mf-media.c
	${MEDIADB-DIR}/mf-media-db.c

    #----   Personal Page   ----#
	${SRC-DIR}/thumbnail/mf-thumb-gen.c
	${SRC-DIR}/common/mf-storage-space.c
)

INCLUDE(FindPkgConfig)
	pkg_check_modules(pkgs REQUIRED
	ecore
	efl-extension
	elementary
	evas
	glib-2.0
	libexif
	notification
	pkgmgr
	pkgmgr-info
	storage
	sqlite3
	capi-appfw-application
	capi-appfw-package-manager
	capi-appfw-preference
	capi-base-utils-i18n
	capi-content-media-content
	capi-content-mime-type
	capi-media-metadata-extractor
	capi-system-device
	capi-system-runtime-info
	capi-system-system-settings
	capi-media-thumbnail-util
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
#SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -finstrument-functions")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -g -fpie")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
ADD_DEFINITIONS("-DDATADIR=\"${DATADIR}\"")
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")

ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-DMYFILE_USE_LOG")
#ADD_DEFINITIONS("-DMYFILE_USE_TA")
#ADD_DEFINITIONS("-DMYFILE_CRITICAL_LOG")
ADD_DEFINITIONS("-DMYFILE_TRAY_FEATURE")
ADD_DEFINITIONS("-DMF_FEATURE_POPSYNC")
ADD_DEFINITIONS("-D_USE_SHARE_PANEL")
ADD_DEFINITIONS("-DMF_SPLIT_WINDOW")
#START_PUBLIC_REMOVED_STRING
ADD_DEFINITIONS("-DMYFILE_DETAILS")
ADD_DEFINITIONS("-DMYFILE_WIFI_DIRECT_FEATURE")
ADD_DEFINITIONS("-DMYFILE_SEARCH_ALL")
ADD_DEFINITIONS("-DMYFILE_SEARCH_STRING")
ADD_DEFINITIONS("-DMYFILE_ENABLE_FOCUS")
ADD_DEFINITIONS("-DMYFILE_CATEGORY_VIEW")
ADD_DEFINITIONS("-DMYFILE_HIDEN_FILES_SHOW")
#END_START_PUBLIC_REMOVED_STRING
#ADD_DEFINITIONS("-DMYFILE_DOWNLOAD_APP_FEATURE")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
#ADD_EXECUTABLE(FSTest ${LIB_SRCS})
#ADD_LIBRARY(${PROJECT_NAME}_fs SHARED ${LIB_SRCS})
#TARGET_LINK_LIBRARIES(FSTest ${pkgs_LDFLAGS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm -lpthread)

INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${BINDIR})
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/tools/dumpfiles.sh DESTINATION bin)
#INSTALL(FILES ${MAININCLUDE}/myfile_filesystem_util.h DESTINATION /usr/include)
INSTALL(DIRECTORY DESTINATION ${DATADIR})

#CONFIGURE_FILE(desktop/${PKG_NAME}.desktop.in desktop/${PKG_NAME}.desktop)
INSTALL(FILES ${CMAKE_BINARY_DIR}/${PKG_NAME}.xml DESTINATION ${XMLDIR})


INSTALL(FILES res/${PKG_NAME}.png DESTINATION ${CMAKE_DESKTOP_ICON_DIR})


ADD_DEPENDENCIES(${PROJECT_NAME} edj_build)

ADD_SUBDIRECTORY(data)

