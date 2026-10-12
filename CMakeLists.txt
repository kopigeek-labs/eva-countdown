cmake_minimum_required(VERSION 3.16)
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION" PROJECT_VER LIMIT_COUNT 1)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(eva_countdown)
