Import("env")

optimze_flags = [s for s in env.GetProjectOption("system_flags", "").splitlines() if s]

linker_flags = []

common_flags = [
  "-Wdouble-promotion",
  "-Wunsafe-loop-optimizations",
  "-fsingle-precision-constant",
  "-fno-exceptions",
  "-fno-strict-aliasing",
	"-fstack-usage",
	"-fno-stack-protector",
  "-fomit-frame-pointer",
	"-fno-unwind-tables",
  "-fno-asynchronous-unwind-tables",
	"-fno-math-errno",
  "-fmerge-all-constants",
  "-funsafe-loop-optimizations"
]

if env.GetBuildType() == "release":
  common_flags.insert(0, "-flto")
  common_flags.insert(0, "-O3")
  common_flags.insert(0, "-s")
else:
  common_flags.insert(0, "-O1")

env.Append(
  BUILD_FLAGS=["-std=gnu11"],
  BUILD_UNFLAGS=["-Og", "-Os"],
  ASFLAGS=optimze_flags + common_flags,
  CCFLAGS=linker_flags + optimze_flags + common_flags,
  LINKFLAGS=linker_flags + optimze_flags + common_flags
)