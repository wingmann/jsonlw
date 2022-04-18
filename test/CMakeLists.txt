set(TARGET jsonlw_test)

file(GLOB PROJECT_SOURCES *.cpp)

add_executable(${TARGET} ${PROJECT_SOURCES})
target_link_libraries(${TARGET} PUBLIC lib_biginteger gtest_main)

add_test(NAME ${TARGET} COMMAND ${TARGET})