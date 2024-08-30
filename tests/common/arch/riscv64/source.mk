# -mno-relax to disable the compiler from using the gp register as a shorthand
#  for __global_pointer$, which I currently don't do anything with. Should
#  probably fix at some point
ARCH_FLAGS += -march=rv64imac -mabi=lp64 -mno-relax
