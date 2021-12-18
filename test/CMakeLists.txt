add_executable(reverb_unittest reverb_descriptor_unittest.cpp)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(reverb_unittest PRIVATE GTest::gtest_main ${CMAKE_PROJECT_NAME})

add_test(
        NAME reverb_unittest
        COMMAND reverb_unittest
)

set_tests_properties(reverb_unittest PROPERTIES TIMEOUT 60)
