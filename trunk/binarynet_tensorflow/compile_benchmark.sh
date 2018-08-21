clang-6.0 -target aarch64-linux-gnu -I /usr/aarch64-linux-gnu/include -Wno-builtin-requires-header -mcpu=cortex-a53 ./SRC/benchmark.c benchmark/*.c -ISRC/ -Ibenchmark/ -flto -Ofast -pthread -DPRINT_TIME

