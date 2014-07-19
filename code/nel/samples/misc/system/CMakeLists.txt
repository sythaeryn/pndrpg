FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nel_sample_system ${SRC})

TARGET_LINK_LIBRARIES(nel_sample_system nelmisc)
NL_DEFAULT_PROPS(nel_sample_system "NeL, Samples, Misc: System")
NL_ADD_RUNTIME_FLAGS(nel_sample_system)

INSTALL(TARGETS nel_sample_system RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
