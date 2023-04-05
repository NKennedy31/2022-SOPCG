# Libraries
LIBS=commons shared pthread

# Custom libraries' paths
SHARED_LIBPATHS=../shared
STATIC_LIBPATHS=

# Compiler flags
CDEBUG=-g -w -Wno-unused-function -DDEBUG
CRELEASE=-O3 -Wall -Wno-unused-function -DNDEBUG

# Arguments when executing with start, memcheck or helgrind
ARGS=configsCpu/estabilidad.config 0.0.0.0
#configsCpu/estabilidad.config 0.0.0.0

# Valgrind flags
MEMCHECK_FLAGS=--track-origins=yes --log-file="memcheck.log"
HELGRIND_FLAGS=--log-file="helgrind.log"