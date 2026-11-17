qt_add_executable(test_types_json
    test_types_json.cpp
)

target_link_libraries(test_types_json
    PRIVATE
        selectum_abi
        Qt::Test
)

add_test(NAME test_types_json COMMAND test_types_json)
