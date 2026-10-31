# CMake generated Testfile for 
# Source directory: C:/Users/voron/OneDrive/Documents/GUIlibrary
# Build directory: C:/Users/voron/OneDrive/Documents/GUIlibrary/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(glimmer_tests "C:/Users/voron/OneDrive/Documents/GUIlibrary/build/Debug/glimmer_tests.exe")
  set_tests_properties(glimmer_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;47;add_test;C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(glimmer_tests "C:/Users/voron/OneDrive/Documents/GUIlibrary/build/Release/glimmer_tests.exe")
  set_tests_properties(glimmer_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;47;add_test;C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(glimmer_tests "C:/Users/voron/OneDrive/Documents/GUIlibrary/build/MinSizeRel/glimmer_tests.exe")
  set_tests_properties(glimmer_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;47;add_test;C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(glimmer_tests "C:/Users/voron/OneDrive/Documents/GUIlibrary/build/RelWithDebInfo/glimmer_tests.exe")
  set_tests_properties(glimmer_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;47;add_test;C:/Users/voron/OneDrive/Documents/GUIlibrary/CMakeLists.txt;0;")
else()
  add_test(glimmer_tests NOT_AVAILABLE)
endif()
