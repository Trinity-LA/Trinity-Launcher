add_executable(test_color_extractor test_color_extractor.cpp)

target_link_libraries(test_color_extractor PRIVATE TrinityCore Qt6::Core Qt6::Gui)
