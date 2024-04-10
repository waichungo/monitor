cmake_minimum_required(VERSION 3.5.0)
project(monitor VERSION 0.1.0 LANGUAGES C CXX)
SET(CMAKE_CXX_STANDARD 17)
include(CTest)
enable_testing()
set(OPENSSL_USE_STATIC_LIBS ON)
set(ISDEBUG TRUE)

add_subdirectory(libs/curl)
add_subdirectory(libs/zip)
add_subdirectory(libs/SQLiteCpp-master)
add_subdirectory(libs/libdatachannel)

SET(CURL_INCLUDE_DIR "libs/curl/include")
SET(SQLITECPP_USE_STATIC_RUNTIME TRUE)

	# target_link_libraries(datachannel PRIVATE libssl_static.lib libcrypto_static.lib)

include_directories("libs/zip/src")
include_directories("libs/SQLiteCpp-master/include/SQLiteCpp")
include_directories("libs/ADVobfuscator/Lib")
include_directories("libs/json-develop/single_include")
include_directories("libs/libdatachannel/src")
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

link_libraries(LibDataChannel::LibDataChannelStatic Pdh libssl_static.lib libcrypto_static.lib libcurl Ws2_32.lib  Crypt32.lib zip SQLiteCpp sqlite3)
add_executable(monitor main.cpp MessageProcessor.cpp MessageHandler.cpp Request.cpp AES.cpp Encrypter.cpp Locker.cpp Utils.cpp SHA256.cpp ZipExt.cpp Websocks.cpp App.cpp DB.cpp)

if (MSVC)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        if(ISDEBUG)
				message("Linking openssl x64 debug builds")						
				target_link_directories(monitor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/opensslbuilds/x64/debug")
			else()
				message("Linking openssl x64 release builds")
				target_link_directories(monitor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/opensslbuilds/x64/release")						
		endif()   
	else()
		message("This is 32bit")
		
	endif()
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
