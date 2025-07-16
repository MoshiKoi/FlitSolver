add_library(BotBase)
target_sources(BotBase PUBLIC FILE_SET CXX_MODULES FILES botbase.cpp)
target_link_libraries(BotBase PRIVATE Game)

add_library(RandomBot)
target_sources(RandomBot PUBLIC FILE_SET CXX_MODULES FILES randombot.cpp)
target_link_libraries(RandomBot PRIVATE BotBase)

add_library(Evaluator)
target_sources(Evaluator PUBLIC FILE_SET CXX_MODULES FILES evaluator.cpp)
target_link_libraries(Evaluator PRIVATE Game libassert::assert)

add_library(AlphaBetaBot)
target_sources(AlphaBetaBot PUBLIC FILE_SET CXX_MODULES FILES alphabetabot.cpp)
target_link_libraries(AlphaBetaBot PRIVATE BotBase Evaluator)

add_library(Bots)
target_sources(Bots PUBLIC FILE_SET CXX_MODULES FILES bots.cpp)
target_link_libraries(Bots PRIVATE RandomBot AlphaBetaBot)

if (FLITSOLVER_BUILD_TESTS)
    add_executable(Evaluator.Tests evaluator.tests.cpp)
    target_link_libraries(Evaluator.Tests PRIVATE Evaluator libassert::assert Catch2::Catch2WithMain)
    catch_discover_tests(Evaluator.Tests)
endif()
