add_executable(synth
    main.cpp
)
target_link_libraries(synth PRIVATE
    ${SDL2_LIBRARY}
    ${SDL2_MIXER_LIBRARY}
)
