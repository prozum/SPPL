add_library(compiler Compiler.cpp Compiler.h)

set (LIBS pprinter)
if (CCPP)
    set (LIBS ${LIBS} cpar cc)
endif()
if (CHASKELL)
    set (LIBS ${LIBS} chaskell)
endif()
if (CGNUASM)
    set (LIBS ${LIBS} cgnuasm)
endif()
if (CLLVM)
    set (LIBS ${LIBS} cllvm)
endif()
if (OPTIMIZER)
    set (LIBS ${LIBS} optimizer)
endif()

target_link_libraries(compiler common parser semantics ${LIBS})

