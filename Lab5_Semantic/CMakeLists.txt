cmake_minimum_required(VERSION 3.8)
project(Lab5_Semantic)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        absyn.c
        absyn.h
        env.c
        env.h
        errormsg.c
        errormsg.h
        lex.yy.c
        parse.c
        parse.h
        parsetest.c
        prabsyn.c
        prabsyn.h
        semant.c
        semant.h
        symbol.c
        symbol.h
        table.c
        table.h
        types.c
        types.h
        util.c
        util.h
        y.tab.c
        y.tab.h)

add_executable(Lab5_Semantic ${SOURCE_FILES})