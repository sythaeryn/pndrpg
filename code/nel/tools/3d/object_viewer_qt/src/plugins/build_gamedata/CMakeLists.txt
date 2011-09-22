INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${LIBXML2_INCLUDE_DIR}
			${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_BUILD_GAMEDATA_PLUGIN_HDR
			build_gamedata_view.h
			build_gamedata_plugin.h
			build_gamedata_settings_page.h)

SET(OVQT_BUILD_GAMEDATA_PLUGIN_UIS
				build_gamedata_view.ui
				build_gamedata_settings_page.ui)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT4_WRAP_CPP(OVQT_BUILD_GAMEDATA_PLUGIN_MOC_SRC ${OVQT_BUILD_GAMEDATA_PLUGIN_HDR})
QT4_WRAP_UI(OVQT_BUILD_GAMEDATA_PLUGIN_UI_HDRS ${OVQT_BUILD_GAMEDATA_PLUGIN_UIS})

SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_BUILD_GAMEDATA_PLUGIN_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OVQT_BUILD_GAMEDATA_PLUGIN_MOC_SRC})
SOURCE_GROUP("Build Gamedata Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(ovqt_plugin_build_gamedata MODULE ${SRC} ${OVQT_BUILD_GAMEDATA_PLUGIN_MOC_SRC} ${OVQT_EXT_SYS_SRC} ${OVQT_BUILD_GAMEDATA_PLUGIN_UI_HDRS})

TARGET_LINK_LIBRARIES(ovqt_plugin_build_gamedata ovqt_plugin_core nelmisc nel3d ${QT_LIBRARIES})

IF(WITH_STLPORT)
  TARGET_LINK_LIBRARIES(ovqt_plugin_build_gamedata ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WITH_STLPORT)

NL_DEFAULT_PROPS(ovqt_plugin_build_gamedata "NeL, Tools, 3D: Object Viewer Qt Plugin: Build Gamedata")
NL_ADD_RUNTIME_FLAGS(ovqt_plugin_build_gamedata)
NL_ADD_LIB_SUFFIX(ovqt_plugin_build_gamedata)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

INSTALL(TARGETS ovqt_plugin_build_gamedata LIBRARY DESTINATION lib RUNTIME DESTINATION bin ARCHIVE DESTINATION lib COMPONENT tools3d)