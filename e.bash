SANITIZER_FLAGS="-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr"
INCLUDES="-I/mnt/c/users/azerty/my_project/my_libs -I../my_libs/ -Iincludes/ -L/mnt/c/Users/Azerty/my_project/my_libs"
LINUX_FLAGS="-O3"
g++ ../my_libs/FlagParser.cpp src/MyLangMain.cpp            $INCLUDES $LINUX_FLAGS -o bin/Wrapper
g++ src/MyLangBackEnd.cpp src/MyLangDump.cpp                $INCLUDES $LINUX_FLAGS -o bin/ProgramToAsm
g++ src/MyLangDump.cpp src/Mylang.cpp src/MyLangSyntax.cpp  $INCLUDES $LINUX_FLAGS -o bin/Lang