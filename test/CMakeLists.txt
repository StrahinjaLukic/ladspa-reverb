add_executable(ladspa_descriptor_unittest ladspa_descriptor_unittest.cpp)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(ladspa_descriptor_unittest PRIVATE GTest::gtest_main ${CMAKE_PROJECT_NAME})

add_test(
        NAME ladspa_descriptor_unittest
        COMMAND ladspa_descriptor_unittest
)

set_tests_properties(ladspa_descriptor_unittest PROPERTIES TIMEOUT 60)
