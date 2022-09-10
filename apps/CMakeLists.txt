set(
    APPS
    cacheRun
    idealCache
    compareWIdeal
)

foreach(APP ${APPS})
    add_subdirectory(${APP})
endforeach()

set(APPS "${APPS}" PARENT_SCOPE) 
