Import("env")

#
# Add -m32 to the linker (since PlatformIo is not capable directly!)
# (and since we are at it, make everyting statically linked ;-)
#

env.Append(
  LINKFLAGS=[
      "-m32",
      "-static-libgcc",
      "-static-libstdc++"
  ]
)
