add_library(Game)
target_sources(Game PUBLIC FILE_SET CXX_MODULES FILES game.cpp)
target_link_libraries(Game PRIVATE xxHash::xxhash libassert::assert)

add_library(Evaluator)
target_sources(Evaluator PUBLIC FILE_SET CXX_MODULES FILES evaluator.cpp)
target_link_libraries(Evaluator PRIVATE Game)

add_library(Repl)
target_sources(Repl PUBLIC FILE_SET CXX_MODULES FILES repl.cpp)
target_link_libraries(Repl PRIVATE Game Evaluator)

add_executable(solver main.cpp)
target_link_libraries(solver PRIVATE Repl)

if (FLITSOLVER_BUILD_TESTS)
    add_executable(Game.Tests game.tests.cpp)
    target_link_libraries(Game.Tests PRIVATE Game libassert::assert Catch2::Catch2WithMain)
    catch_discover_tests(Game.Tests)
endif()
