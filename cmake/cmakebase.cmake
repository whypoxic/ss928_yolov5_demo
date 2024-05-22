# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm ) 

SET(CMAKE_SYSTEM_VERSION 1)

SET(SYSTEM_LINK_LIB
    -lpthread
    -lrt
    -ldl
    -lm
)
SET(DO_FLAG -DO2)
SET(O_FLAG -O2)
SET(CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} ${O_FLAG} -std=c++11 -Wno-deprecated-declarations -ffunction-sections -fdata-sections -Werror -Wno-psabi -Wno-pointer-arith -Wno-int-to-pointer-cast"
)
# SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -lstdc++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lstdc++ -mcpu=cortex-a53 -fno-aggressive-loop-optimizations -ldl -ffunction-sections -fdata-sections -O2 -fstack-protector-strong -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE -pie -s -Wall -fsigned-char")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

