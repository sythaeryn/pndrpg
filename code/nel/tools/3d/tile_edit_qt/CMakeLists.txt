INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/pic)
INCLUDE( ${QT_USE_FILE} )

FIND_PACKAGE(JPEG)
INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})

FILE(GLOB TILE_EDIT_SRC *.cpp *.h)
FILE(GLOB PIC_SRC pic/*.c pic/*.cpp)
SET(TILE_EDIT_HDR items_edit_dlg.h tile_browser_dlg.h tile_edit_dlg.h tile_rotation_dlg.h tiles_model.h tile_widget.h)
SET(TILE_EDIT_UIS tile_widget_qt.ui tile_rotation_qt.ui tile_edit_qt.ui tile_browser_qt.ui items_edit_qt.ui)
SET(TILE_EDIT_RCS tile_edit_qt.qrc)

QT4_ADD_RESOURCES( TILE_EDIT_RC_SRCS ${TILE_EDIT_RCS} )
QT4_WRAP_UI( TILE_EDIT_UI_HDRS ${TILE_EDIT_UIS} )
QT4_WRAP_CPP( TILE_EDIT_MOC_SRCS ${TILE_EDIT_HDR})

SOURCE_GROUP(QtResources FILES ${TILE_EDIT_UIS} ${TILE_EDIT_RCS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${TILE_EDIT_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${TILE_EDIT_MOC_SRCS})
SOURCE_GROUP(PICLibrary FILES ${PIC_SRC})

ADD_EXECUTABLE(tile_edit_qt ${TILE_EDIT_SRC} ${PIC_SRC} ${TILE_EDIT_MOC_SRCS} ${TILE_EDIT_RC_SRCS} ${TILE_EDIT_UI_HDRS})

TARGET_LINK_LIBRARIES(tile_edit_qt ${QT_LIBRARIES} ${JPEG_LIBRARY} nelmisc nel3d)
NL_DEFAULT_PROPS(tile_edit_qt "NeL, Tools, 3D: Qt Tile Editor")
NL_ADD_RUNTIME_FLAGS(tile_edit_qt)

ADD_DEFINITIONS(${QT_DEFINITIONS})

INSTALL(TARGETS tile_edit_qt RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)

