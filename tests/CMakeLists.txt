set (
    TESTS
    lirs-test
    ideal-test
)

foreach(TEST ${TESTS})
    add_subdirectory(${TEST})
endforeach()

set (TESTS "${TESTS}" PARENT_SCOPE)