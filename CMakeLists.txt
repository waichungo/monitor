cmake_minimum_required(VERSION 3.0.0)
project(monitor VERSION 0.1.0 LANGUAGES C CXX)
SET(CMAKE_CXX_STANDARD 17)
include(CTest)
enable_testing()

add_subdirectory(libs/curl)
SET(CURL_INCLUDE_DIR "libs/curl/include")
add_subdirectory(libs/zip)

include_directories("libs/zip/src")
include_directories("libs/ADVobfuscator/Lib")
include_directories("libs/json-develop/single_include")
SET(CURL_LIBRARY "libcurl")
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS )
else()
    add_link_options(-static-libstdc++ -static-libgcc)
endif()

link_libraries(	Pdh libcurl Ws2_32.lib zip)
add_executable(monitor main.cpp Request.cpp AES.cpp Encrypter.cpp Locker.cpp Utils.cpp SHA256.cpp ZipExt.cpp App.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
