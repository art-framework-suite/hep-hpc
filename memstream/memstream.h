#if defined(__linux__)
# include <features.h>
#endif

#include <stdio.h>

#if _POSIX_C_SOURCE < 200809L

#ifdef __cplusplus
extern "C" {
#endif
FILE *open_memstream(char **ptr, size_t *sizeloc);
#ifdef __cplusplus
}
#endif
#endif /* _POSIX_C_SOURCE < 200809L */
