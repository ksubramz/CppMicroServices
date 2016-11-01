
enable_language(C)

#include_directories(${US_INCLUDE_DIRS})

add_definitions(-DUS_RCC_EXECUTABLE_NAME=\"${US_RCC_EXECUTABLE_NAME}\")
add_definitions(-DUS_ZIPINFO_EXECUTABLE_NAME=\"${US_ZIPINFO_EXECUTABLE_NAME}\")

set(rcc_srcs ResourceCompiler.cpp ../../third_party/miniz.c)
set(zipinfo_srcs ZipInfo.cpp ../../third_party/miniz.c)

add_executable(${US_RCC_EXECUTABLE_NAME} ${rcc_srcs})
add_executable(${US_ZIPINFO_EXECUTABLE_NAME} ${zipinfo_srcs})

if(WIN32)
  target_link_libraries(${US_RCC_EXECUTABLE_NAME} Shlwapi)
  target_link_libraries(${US_ZIPINFO_EXECUTABLE_NAME} Shlwapi)
endif()

set_property(
  TARGET ${US_RCC_EXECUTABLE_NAME} APPEND PROPERTY
  COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_READING_API;MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
  )
set_property(
  TARGET ${US_ZIPINFO_EXECUTABLE_NAME} APPEND PROPERTY
  COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_READING_API;MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
  )

if(NOT US_NO_INSTALL)
  install(TARGETS ${US_RCC_EXECUTABLE_NAME}
          EXPORT ${PROJECT_NAME}Targets
          FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
          RUNTIME DESTINATION bin ${US_SDK_INSTALL_COMPONENT})

  install(TARGETS ${US_ZIPINFO_EXECUTABLE_NAME}
          EXPORT ${PROJECT_NAME}Targets
          FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
          RUNTIME DESTINATION bin ${US_SDK_INSTALL_COMPONENT})
endif()


# Test usResourceCompiler and usZipInfo

# 1. Test --manifest-add
add_test(NAME ResourceCompilerTest1
         WORKING_DIRECTORY C:/Users/krishs/Github/CppMicroServices/framework/test/
         COMMAND ${US_RCC_EXECUTABLE_NAME} 
		 --bundle-name mybundle 
		 --out-file ${CppMicroServices_BINARY_DIR}/Example1.zip
		 --manifest-add manifest.json)
add_test(NAME ZipInfoTest1 COMMAND
         ${US_ZIPINFO_EXECUTABLE_NAME}
         ${CppMicroServices_BINARY_DIR}/Example1.zip)
set(pass1 "^File 79 102 3826964651 mybundle/manifest.json\n"
          "Directory 0 0 0 mybundle/\n")
set_property (TEST ZipInfoTest1
              PROPERTY PASS_REGULAR_EXPRESSION "${pass1}")

# 2. Test --manifest-add and --res-add
add_test(NAME ResourceCompilerTest2
         WORKING_DIRECTORY C:/Users/krishs/Github/CppMicroServices/framework/test/
         COMMAND ${US_RCC_EXECUTABLE_NAME} 
		 --bundle-name mybundle 
		 --out-file ${CppMicroServices_BINARY_DIR}/Example2.zip
		 --manifest-add manifest.json
		 --res-add images/cppmicroservices.png)
add_test(NAME ZipInfoTest2 COMMAND
         ${US_ZIPINFO_EXECUTABLE_NAME}
         ${CppMicroServices_BINARY_DIR}/Example2.zip)
set(pass "File 79 102 3826964650 mybundle/manifest.json\n"
         "Directory 0 0 0 mybundle/\n"
		 "File 14054 14457 3259342928 mybundle/cppmicroservices.png")
set_property (TEST ZipInfoTest2
              PROPERTY PASS_REGULAR_EXPRESSION "${pass}")

# 3. Test --manifest-add and --res-add 2 times
add_test(NAME ResourceCompilerTest3
         WORKING_DIRECTORY C:/Users/krishs/Github/CppMicroServices/framework/test/
         COMMAND ${US_RCC_EXECUTABLE_NAME} 
		 --bundle-name mybundle 
		 --out-file ${CppMicroServices_BINARY_DIR}/Example3.zip
		 --manifest-add manifest.json
		 --res-add images/cppmicroservices.png
		 --res-add TestResource.txt)
add_test(NAME ZipInfoTest3 COMMAND
         ${US_ZIPINFO_EXECUTABLE_NAME}
         ${CppMicroServices_BINARY_DIR}/Example3.zip)
set(pass "File 292 768 3418614610 mybundle/manifest.json\n"
         "Directory 0 0 0 mybundle/\n"
		 "File 14054 14457 3259342928 mybundle/cppmicroservices.png\n"
         "File 48 43 1997026158 mybundle/TestResource.txt")
set_property (TEST ZipInfoTest3
              PROPERTY PASS_REGULAR_EXPRESSION "${pass}")
