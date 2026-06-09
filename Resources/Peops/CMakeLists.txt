project(peops VERSION 1.0 LANGUAGES C ASM)

add_prx_module(peops exports.exp)

target_sources(peops PRIVATE 
   main.c
   src/psp.c
   src/audio.c
   spu/src/decode_xa.c
   spu/src/spu.c
   spu/src/registers.c
   spu/src/xa.c
)

target_compile_options(peops PRIVATE -O3 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division)
target_include_directories(peops PRIVATE include spu/include ${CFWSDK}/include)
target_link_directories(peops PRIVATE ${CFWSDK}/libs)
target_link_libraries(peops PRIVATE
   pspsystemctrl_user
   pspaudio
   pspsysc_user
)