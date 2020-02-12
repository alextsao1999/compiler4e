# CMake generated Testfile for 
# Source directory: C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory
# Build directory: C:/Users/Administrator/Desktop/compiler4e/vsbuild/third_party/json/test/cmake_add_subdirectory
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(cmake_add_subdirectory_configure "E:/cmake/bin/cmake.exe" "-G" "Visual Studio 16 2019" "-DCMAKE_CXX_COMPILER=F:/Visual Studio/2019/Community/VC/Tools/MSVC/14.24.28314/bin/Hostx64/x64/cl.exe" "-Dnlohmann_json_source=C:/Users/Administrator/Desktop/compiler4e/third_party/json" "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/project")
  set_tests_properties(cmake_add_subdirectory_configure PROPERTIES  FIXTURES_SETUP "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;1;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(cmake_add_subdirectory_configure "E:/cmake/bin/cmake.exe" "-G" "Visual Studio 16 2019" "-DCMAKE_CXX_COMPILER=F:/Visual Studio/2019/Community/VC/Tools/MSVC/14.24.28314/bin/Hostx64/x64/cl.exe" "-Dnlohmann_json_source=C:/Users/Administrator/Desktop/compiler4e/third_party/json" "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/project")
  set_tests_properties(cmake_add_subdirectory_configure PROPERTIES  FIXTURES_SETUP "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;1;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(cmake_add_subdirectory_configure "E:/cmake/bin/cmake.exe" "-G" "Visual Studio 16 2019" "-DCMAKE_CXX_COMPILER=F:/Visual Studio/2019/Community/VC/Tools/MSVC/14.24.28314/bin/Hostx64/x64/cl.exe" "-Dnlohmann_json_source=C:/Users/Administrator/Desktop/compiler4e/third_party/json" "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/project")
  set_tests_properties(cmake_add_subdirectory_configure PROPERTIES  FIXTURES_SETUP "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;1;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(cmake_add_subdirectory_configure "E:/cmake/bin/cmake.exe" "-G" "Visual Studio 16 2019" "-DCMAKE_CXX_COMPILER=F:/Visual Studio/2019/Community/VC/Tools/MSVC/14.24.28314/bin/Hostx64/x64/cl.exe" "-Dnlohmann_json_source=C:/Users/Administrator/Desktop/compiler4e/third_party/json" "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/project")
  set_tests_properties(cmake_add_subdirectory_configure PROPERTIES  FIXTURES_SETUP "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;1;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
else()
  add_test(cmake_add_subdirectory_configure NOT_AVAILABLE)
endif()
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(cmake_add_subdirectory_build "E:/cmake/bin/cmake.exe" "--build" ".")
  set_tests_properties(cmake_add_subdirectory_build PROPERTIES  FIXTURES_REQUIRED "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;8;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(cmake_add_subdirectory_build "E:/cmake/bin/cmake.exe" "--build" ".")
  set_tests_properties(cmake_add_subdirectory_build PROPERTIES  FIXTURES_REQUIRED "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;8;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(cmake_add_subdirectory_build "E:/cmake/bin/cmake.exe" "--build" ".")
  set_tests_properties(cmake_add_subdirectory_build PROPERTIES  FIXTURES_REQUIRED "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;8;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(cmake_add_subdirectory_build "E:/cmake/bin/cmake.exe" "--build" ".")
  set_tests_properties(cmake_add_subdirectory_build PROPERTIES  FIXTURES_REQUIRED "cmake_add_subdirectory" _BACKTRACE_TRIPLES "C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;8;add_test;C:/Users/Administrator/Desktop/compiler4e/third_party/json/test/cmake_add_subdirectory/CMakeLists.txt;0;")
else()
  add_test(cmake_add_subdirectory_build NOT_AVAILABLE)
endif()
