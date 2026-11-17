qt_add_executable(test_types_json
    test_types_json.cpp
)

target_link_libraries(test_types_json
    PRIVATE
        selectum_abi
        Qt::Test
)

add_test(NAME test_types_json COMMAND test_types_json)

qt_add_executable(test_match_engine
    test_match_engine.cpp
)

target_link_libraries(test_match_engine
    PRIVATE
        selectum_core
        Qt::Test
)

add_test(NAME test_match_engine COMMAND test_match_engine)

qt_add_executable(test_plugin_manifest
    test_plugin_manifest.cpp
)

target_link_libraries(test_plugin_manifest
    PRIVATE
        selectum_core
        Qt::Test
)

target_compile_definitions(test_plugin_manifest
    PRIVATE
        SELECTUM_TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

add_test(NAME test_plugin_manifest COMMAND test_plugin_manifest)

qt_add_executable(test_plugin_host
    test_plugin_host.cpp
)

target_link_libraries(test_plugin_host
    PRIVATE
        selectum_core
        Qt::Test
)

target_compile_definitions(test_plugin_host
    PRIVATE
        SELECTUM_TEST_PLUGINS_DIR="${CMAKE_BINARY_DIR}/plugins"
)

add_test(NAME test_plugin_host COMMAND test_plugin_host)
