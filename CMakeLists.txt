cmake_minimum_required(VERSION 3.22)

# Allow for other subdirectories
set(EXTRA_COMPONENT_DIRS "components" "src")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(jammer)
