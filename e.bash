SANITIZER_FLAGS="-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr"
INCLUDES="-IMyLibs/ -Iincludes/"
LINUX_FLAGS="-O3 -DDEBUG"
# g++ src/MyLangMain.cpp $INCLUDES $LINUX_FLAGS -o bin/Wrapper
g++ src/MyLangDump.cpp src/MyLangHelpers.cpp src/MylangBack.cpp src/MyLangTreeRead.cpp Smart_Stack/stack.cpp MyLibs/helper_funcs.cpp $INCLUDES $LINUX_FLAGS $SANITIZER_FLAGS -o bin/ProgramToAsm
g++ src/MyLangDump.cpp src/MyLangHelpers.cpp src/MylangLex.cpp  src/MyLangSyntax.cpp   Smart_Stack/stack.cpp MyLibs/helper_funcs.cpp $INCLUDES $LINUX_FLAGS $SANITIZER_FLAGS -o bin/Lang