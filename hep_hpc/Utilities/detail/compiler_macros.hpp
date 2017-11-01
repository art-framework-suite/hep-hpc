#ifndef cetlib_compiler_macros_h
#define cetlib_compiler_macros_h

// ----------------------------------------------------------------------
// Define gcc version test:
// ----------------------------------------------------------------------

#if defined __GNUC_PATCHLEVEL__
  # define GCC_VERSION (100000 * __GNUC__ \
                       + 1000 * __GNUC_MINOR__ \
                       +        __GNUC_PATCHLEVEL__)

#elif defined __GNUC_MINOR__
  # define GCC_VERSION (10000 * __GNUC__ \
                       + 1000 * __GNUC_MINOR__)

#elif defined __GNUC__
  # define GCC_VERSION (10000 * __GNUC__)

#else
  # define GCC_VERSION 0

#endif

#if defined __clang_patchlevel__
  # define CLANG_VERSION (100000 * __clang_major__ \
                       + 1000 * __clang_minor__ \
                       +        __clang_patchlevel__)

#elif defined __clang_minor__
  # define CLANG_VERSION (10000 * __clang_major__ \
                       + 1000 * __clang_minor__)

#elif defined __clang_major__
  # define CLANG_VERSION (10000 * __clang_major__)

#else
  # define CLANG_VERSION 0

#endif


# define GCC_IS_AT_LEAST(major, minor, patch) \
  (GCC_VERSION >= ((100000*(major)) + (1000*(minor)) + (patch)))

# define CLANG_IS_AT_LEAST(major, minor, patch) \
  (CLANG_VERSION >= ((100000*(major)) + (1000*(minor)) + (patch)))

#endif /* cetlib_compiler_macros_h */

// Local Variables:
// mode: c++
// End:
