set(MODULE_NAME "mod-auto-announcer")
set(MODULE_SRCS
    src/mod-auto-announcer.cpp
)

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/mod-auto-announcer.cpp")

# 설정 파일 설치
install(FILES
    conf/mod-auto-announcer.conf.dist
    DESTINATION "${CMAKE_INSTALL_PREFIX}/conf"
)