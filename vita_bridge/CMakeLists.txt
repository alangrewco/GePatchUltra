cmake_minimum_required(VERSION 3.10)

# Use Vita toolchain before project()
set(CMAKE_TOOLCHAIN_FILE $ENV{VITASDK}/share/vita.toolchain.cmake)

project(vita_bridge C)

# Vita helpers (SELF/VPK macros)
include($ENV{VITASDK}/share/vita.cmake REQUIRED)

set(VITA_APP_NAME  "Renderer Bridge")
set(VITA_TITLEID   "RENDER001")
set(VITA_VERSION   "01.00")

add_executable(${PROJECT_NAME}
  src/main.c
)

target_link_libraries(${PROJECT_NAME}
  SceLibKernel_stub
  SceIofilemgr_stub
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME    ${VITA_APP_NAME}
)
