#pragma once

// Clang compiler specification

#define SG_FORCE_INLINE __attribute__((always_inline)) inline

#define SG_ALIGN(x)     __attribute__((aligned(bytes)))