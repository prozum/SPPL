if(CCPP)
    add_subdirectory(c)
    add_subdirectory(cpar)
endif()
if(CHASKELL)
    add_subdirectory(haskell)
endif()
if(CGNUASM)
    add_subdirectory(gnuasm)
endif()
if(CLLVM OR JIT OR INTERPRETER)
    add_subdirectory(llvm)
endif()

add_subdirectory(pprinter)
