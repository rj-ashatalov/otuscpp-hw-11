cmake_minimum_required(VERSION 3.2)

project(async VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})

find_package(Threads REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

configure_file(version.h.in ${PROJECT_SOURCE_DIR}/version.h)

add_executable(test_version tests/test_version.cpp src/Bulkmlt.h)

add_executable(${PROJECT_NAME} "main.cpp")

file(GLOB_RECURSE TARGET_SRC "src/*.cpp" "src/*.h")
add_library(libasync SHARED ${TARGET_SRC})
add_library(patch_version lib.cpp)

set_target_properties(libasync PROPERTIES PUBLIC_HEADER src/async.h)

set_target_properties(${PROJECT_NAME} libasync patch_version test_version PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
)

set_target_properties(test_version PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    libasync
    patch_version
    Threads::Threads
)

target_link_libraries(test_version
    ${Boost_LIBRARIES}
    patch_version
)

target_include_directories(libasync PRIVATE src)

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(test_version PUBLIC src)

install(TARGETS libasync
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION lib)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)


set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT a.shatalov@rj-games.com)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

enable_testing()

add_test(${PROJECT_NAME}_tests test_version)

