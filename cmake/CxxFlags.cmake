set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
string(JOIN " " CXX_FLAGS_LIST
    -O2
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wconversion
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-c99-extensions
    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wnull-dereference
    -Wno-double-promotion
    -Wformat=2
    -pedantic
    -Werror
)
set(CMAKE_CXX_FLAGS "${CXX_FLAGS_LIST}")

