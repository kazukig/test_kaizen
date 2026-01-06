#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

/*
  example.c
  - many typedefs and macros
  - multiple places where unsigned/signed mismatches occur (marked "unsigned不一致")
  - ~200 lines to exercise the fixer/analyzer
*/

/* typedefs (many aliases) */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

/* more aliases */
typedef u8  byte_t;
typedef u8  small_t;
typedef i8  sbyte_t;
typedef u16 word_t;
typedef i16 sword_t;

/* macros to complicate parsing */
#define MAKE_U8(x) ((u8)(x))
#define MAKE_I8(x) ((i8)(x))
#define TO_U32(x)  ((u32)(x))
#define TO_I32(x)  ((i32)(x))
#define MUL(a,b)   ((a)*(b))
#define ADD(a,b)   ((a)+(b))
#define SUB(a,b)   ((a)-(b))

/* nested macro that expands types */
#define TYPDEF_ALIAS(name, base) typedef base name
TYPDEF_ALIAS(my_u8_t, uint8_t);
TYPDEF_ALIAS(my_i8_t, int8_t);

/* macro functions */
#define CLAMP_U8(x) ((u8)(((x) > 255) ? 255 : (((x) < 0) ? 0 : (x))))
#define SQR(x) ((x)*(x))

/* complex macro using other macros */
#define COMPLEX_OP(a,b,c) (ADD(MUL((a),(b)), (c)))

/* static helper functions to create contexts */
static u8 make_mod(u8 a, u8 b) {
    /* unsigned不一致 */
    return (u8)((a + b + 4) % 256);
}

static i8 make_signed_op(i8 a, i8 b) {
    return (i8)(a - b);
}

/* function that mixes signed and unsigned intentionally */
u8 mix_signed_unsigned(u8 ua, i8 ib) {
    /* unsigned不一致 */
    i32 tmp = (i32)ua + (i32)ib; /* ib promoted, negative values influence result */
    return (u8)tmp;
}

u8 compare_and_select(u8 a, i8 b) {
    /* unsigned不一致 */
    if (a < b) { /* comparing unsigned to signed */
        return MAKE_U8(0);
    } else {
        return MAKE_U8(1);
    }
}

/* more convoluted typedef structure (simulated) */
typedef struct {
    my_u8_t  a;
    my_i8_t  b;
    u16      c;
    i16      d;
} node_t;

typedef node_t * node_ptr;

/* macros to access fields */
#define NODE_SET_A(n, val) ((n).a = MAKE_U8(val))
#define NODE_SET_B(n, val) ((n).b = MAKE_I8(val))

/* inline-like macros */
#define APPLY_CAST_TO_U8(x) ((u8)(x))
#define APPLY_CAST_TO_I8(x) ((i8)(x))

/* Many test cases in main to trigger mismatches */
int main(void) {
    /* variables */
    u8 ua = 200;
    i8 ib = -60;
    u8 uc = 100;
    i8 id = 50;
    u16 uw = 40000;
    i16 sw = -3000;
    my_u8_t m1 = 255;
    my_i8_t m2 = -1;
    node_t nodes[10];
    for (int i = 0; i < 10; ++i) {
        nodes[i].a = (u8)(i * 10);
        nodes[i].b = (i8)(i - 5);
        nodes[i].c = (u16)(i * 1000);
        nodes[i].d = (i16)(i * -200);
    }

    /* 1: arithmetic mixes */
    u8 r1 = mix_signed_unsigned(ua, ib); /* unsigned不一致 */
    printf("r1: %u\n", r1);

    u8 r2 = compare_and_select(uc, id); /* unsigned不一致 */
    printf("r2: %u\n", r2);

    /* 2: direct operations with casts */
    u8 r3 = (u8)(ua + (u8)id); /* id cast, still possible mismatch */ /* unsigned不一致 */
    printf("r3: %u\n", r3);

    /* 3: using macros with signed values */
    u8 r4 = CLAMP_U8((int)ua + (int)ib); /* unsigned不一致 */
    printf("r4: %u\n", r4);

    /* 4: mixing in comparisons */
    if ((u8)uw > (u8)sw) { /* unsigned不一致 */
        printf("uw > sw (as u8)\n");
    }

    /* 5: using node structure fields */
    nodes[2].a = mix_signed_unsigned(nodes[2].a, nodes[2].b); /* unsigned不一致 */
    printf("nodes[2].a: %u\n", nodes[2].a);

    /* 6: more macros and nested evaluation */
    u8 r5 = MAKE_U8(COMPLEX_OP(ua, uc, ib)); /* unsigned不一致 */
    printf("r5: %u\n", r5);

    /* 7: loops that perform signed/unsigned mixing */
    for (int i = -5; i < 10; ++i) {
        u8 v = MAKE_U8(i + ua); /* signed i added to unsigned ua */ /* unsigned不一致 */
        if (v < ua) { /* compare unsigned */ /* unsigned不一致 */
            printf("v<ua: %d -> %u\n", i, v);
        }
    }

    /* 8: shifts and bit ops */
    u8 r6 = (u8)(ua >> 1); /* fine */
    i8 r7 = (i8)(ib << 2); /* signed shift */
    /* create mismatch by ORing signed and unsigned after promotion */
    u8 r8 = (u8)((u32)ua | (u32)(i32)ib); /* unsigned不一致 */
    printf("r6:%u r7:%d r8:%u\n", r6, r7, r8);

    /* 9: conditionals mixing signedness */
    if ((i32)ua + (i32)ib < 0) { /* unsigned不一致 */
        printf("sum negative\n");
    } else {
        printf("sum non-negative\n");
    }

    /* 10: function pointers and casts */
    u8 (*fp1)(u8, i8) = mix_signed_unsigned;
    u8 r9 = fp1(120, -200); /* unsigned不一致 */
    printf("r9: %u\n", r9);

    /* 11: using typedef chains */
    my_u8_t chain = MAKE_U8(10);
    my_i8_t chain2 = MAKE_I8(-20);
    chain = mix_signed_unsigned(chain, chain2); /* unsigned不一致 */
    printf("chain: %u\n", chain);

    /* 12: arithmetic that overflows signed when mixed */
    for (int i = 0; i < 20; ++i) {
        i8 s = (i8)(i - 10);
        u8 t = (u8)(i * 15);
        u8 z = (u8)(s + t); /* unsigned不一致 */
        (void)z;
    }

    /* 13: pointer arithmetic with signed/unsigned */
    u8 buffer[32];
    memset(buffer, 0xFF, sizeof(buffer));
    i32 offset = -1;
    u8 *p = buffer + (size_t)offset; /* possible mismatch, casting negative to size_t */ /* unsigned不一致 */
    (void)p;

    /* 14: many small expressions to increase count */
    u8 acc = 0;
    acc = MAKE_U8(acc + ua + (u8)ib); /* unsigned不一致 */
    acc = MAKE_U8(acc - (u8)id); /* unsigned不一致 */
    acc = MAKE_U8(acc + (u8)nodes[5].b); /* unsigned不一致 */

    /* 15: macros again */
    u8 a1 = SQR((u8)20); /* fine */
    u8 a2 = CLAMP_U8((int)a1 + (int)nodes[1].b); /* unsigned不一致 */
    printf("a1:%u a2:%u\n", a1, a2);

    /* 16: weird typedef usage */
    small_t st = 128;
    sbyte_t sb = -128;
    st = MAKE_U8(st + (u8)sb); /* unsigned不一致 */
    printf("st:%u sb:%d\n", st, sb);

    /* 17: mixing via functions */
    u8 tmp1 = make_mod(250, 10); /* unsigned不一致 (wrap-around) */
    i8 tmp2 = make_signed_op(-5, 3);
    printf("tmp1:%u tmp2:%d\n", tmp1, tmp2);

    /* 18: more structural uses */
    for (int i = 0; i < 10; ++i) {
        NODE_SET_A(nodes[i], i * 25);
        NODE_SET_B(nodes[i], i - 7);
        nodes[i].a = mix_signed_unsigned(nodes[i].a, nodes[i].b); /* unsigned不一致 */
    }

    /* 19: comparisons involving uint32_t and int32_t */
    u32 bigu = 3000000000u;
    i32 bigs = -1000000000;
    if (bigu > (u32)bigs) { /* unsigned不一致 */
        printf("bigu > bigs as u32\n");
    }

    /* 20: nested macros to obfuscate */
    u8 nm = MAKE_U8(CLAMP_U8(COMPLEX_OP(nodes[0].a, nodes[1].a, nodes[2].b))); /* unsigned不一致 */
    printf("nm:%u\n", nm);

    /* 21: final summary prints */
    printf("Final values ua:%u ib:%d uc:%u id:%d\n", ua, ib, uc, id);

    return 0;
}
```// filepath: /Users/kazukitakasaki/Desktop/python/kaizen/test_kaizen/example.c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

/*
  example.c
  - many typedefs and macros
  - multiple places where unsigned/signed mismatches occur (marked "unsigned不一致")
  - ~200 lines to exercise the fixer/analyzer
*/

/* typedefs (many aliases) */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

/* more aliases */
typedef u8  byte_t;
typedef u8  small_t;
typedef i8  sbyte_t;
typedef u16 word_t;
typedef i16 sword_t;

/* macros to complicate parsing */
#define MAKE_U8(x) ((u8)(x))
#define MAKE_I8(x) ((i8)(x))
#define TO_U32(x)  ((u32)(x))
#define TO_I32(x)  ((i32)(x))
#define MUL(a,b)   ((a)*(b))
#define ADD(a,b)   ((a)+(b))
#define SUB(a,b)   ((a)-(b))

/* nested macro that expands types */
#define TYPDEF_ALIAS(name, base) typedef base name
TYPDEF_ALIAS(my_u8_t, uint8_t);
TYPDEF_ALIAS(my_i8_t, int8_t);

/* macro functions */
#define CLAMP_U8(x) ((u8)(((x) > 255) ? 255 : (((x) < 0) ? 0 : (x))))
#define SQR(x) ((x)*(x))

/* complex macro using other macros */
#define COMPLEX_OP(a,b,c) (ADD(MUL((a),(b)), (c)))

/* static helper functions to create contexts */
static u8 make_mod(u8 a, u8 b) {
    /* unsigned不一致 */
    return (u8)((a + b) % 256);
}

static i8 make_signed_op(i8 a, i8 b) {
    return (i8)(a - b);
}

/* function that mixes signed and unsigned intentionally */
u8 mix_signed_unsigned(u8 ua, i8 ib) {
    /* unsigned不一致 */
    i32 tmp = (i32)ua + (i32)ib; /* ib promoted, negative values influence result */
    return (u8)tmp;
}

u8 compare_and_select(u8 a, i8 b) {
    /* unsigned不一致 */
    if (a < b) { /* comparing unsigned to signed */
        return MAKE_U8(0);
    } else {
        return MAKE_U8(1);
    }
}

/* more convoluted typedef structure (simulated) */
typedef struct {
    my_u8_t  a;
    my_i8_t  b;
    u16      c;
    i16      d;
} node_t;

typedef node_t * node_ptr;

/* macros to access fields */
#define NODE_SET_A(n, val) ((n).a = MAKE_U8(val))
#define NODE_SET_B(n, val) ((n).b = MAKE_I8(val))

/* inline-like macros */
#define APPLY_CAST_TO_U8(x) ((u8)(x))
#define APPLY_CAST_TO_I8(x) ((i8)(x))

/* Many test cases in main to trigger mismatches */
int main(void) {
    /* variables */
    u8 ua = 200;
    i8 ib = -60;
    u8 uc = 100;
    i8 id = 50;
    u16 uw = 40000;
    i16 sw = -3000;
    my_u8_t m1 = 255;
    my_i8_t m2 = -1;
    node_t nodes[10];
    for (int i = 0; i < 10; ++i) {
        nodes[i].a = (u8)(i * 10);
        nodes[i].b = (i8)(i - 5);
        nodes[i].c = (u16)(i * 1000);
        nodes[i].d = (i16)(i * -200);
    }

    /* 1: arithmetic mixes */
    u8 r1 = mix_signed_unsigned(ua, ib); /* unsigned不一致 */
    printf("r1: %u\n", r1);

    u8 r2 = compare_and_select(uc, id); /* unsigned不一致 */
    printf("r2: %u\n", r2);

    /* 2: direct operations with casts */
    u8 r3 = (u8)(ua + (u8)id); /* id cast, still possible mismatch */ /* unsigned不一致 */
    printf("r3: %u\n", r3);

    /* 3: using macros with signed values */
    u8 r4 = CLAMP_U8((int)ua + (int)ib); /* unsigned不一致 */
    printf("r4: %u\n", r4);

    /* 4: mixing in comparisons */
    if ((u8)uw > (u8)sw) { /* unsigned不一致 */
        printf("uw > sw (as u8)\n");
    }

    /* 5: using node structure fields */
    nodes[2].a = mix_signed_unsigned(nodes[2].a, nodes[2].b); /* unsigned不一致 */
    printf("nodes[2].a: %u\n", nodes[2].a);

    /* 6: more macros and nested evaluation */
    u8 r5 = MAKE_U8(COMPLEX_OP(ua, uc, ib)); /* unsigned不一致 */
    printf("r5: %u\n", r5);

    /* 7: loops that perform signed/unsigned mixing */
    for (int i = -5; i < 10; ++i) {
        u8 v = MAKE_U8(i + ua); /* signed i added to unsigned ua */ /* unsigned不一致 */
        if (v < ua) { /* compare unsigned */ /* unsigned不一致 */
            printf("v<ua: %d -> %u\n", i, v);
        }
    }

    /* 8: shifts and bit ops */
    u8 r6 = (u8)(ua >> 1); /* fine */
    i8 r7 = (i8)(ib << 2); /* signed shift */
    /* create mismatch by ORing signed and unsigned after promotion */
    u8 r8 = (u8)((u32)ua | (u32)(i32)ib); /* unsigned不一致 */
    printf("r6:%u r7:%d r8:%u\n", r6, r7, r8);

    /* 9: conditionals mixing signedness */
    if ((i32)ua + (i32)ib < 0) { /* unsigned不一致 */
        printf("sum negative\n");
    } else {
        printf("sum non-negative\n");
    }

    /* 10: function pointers and casts */
    u8 (*fp1)(u8, i8) = mix_signed_unsigned;
    u8 r9 = fp1(120, -200); /* unsigned不一致 */
    printf("r9: %u\n", r9);

    /* 11: using typedef chains */
    my_u8_t chain = MAKE_U8(10);
    my_i8_t chain2 = MAKE_I8(-20);
    chain = mix_signed_unsigned(chain, chain2); /* unsigned不一致 */
    printf("chain: %u\n", chain);

    /* 12: arithmetic that overflows signed when mixed */
    for (int i = 0; i < 20; ++i) {
        i8 s = (i8)(i - 10);
        u8 t = (u8)(i * 15);
        u8 z = (u8)(s + t); /* unsigned不一致 */
        (void)z;
    }

    /* 13: pointer arithmetic with signed/unsigned */
    u8 buffer[32];
    memset(buffer, 0xFF, sizeof(buffer));
    i32 offset = -1;
    u8 *p = buffer + (size_t)offset; /* possible mismatch, casting negative to size_t */ /* unsigned不一致 */
    (void)p;

    /* 14: many small expressions to increase count */
    u8 acc = 0;
    acc = MAKE_U8(acc + ua + (u8)ib); /* unsigned不一致 */
    acc = MAKE_U8(acc - (u8)id); /* unsigned不一致 */
    acc = MAKE_U8(acc + (u8)nodes[5].b); /* unsigned不一致 */

    /* 15: macros again */
    u8 a1 = SQR((u8)20); /* fine */
    u8 a2 = CLAMP_U8((int)a1 + (int)nodes[1].b); /* unsigned不一致 */
    printf("a1:%u a2:%u\n", a1, a2);

    /* 16: weird typedef usage */
    small_t st = 128;
    sbyte_t sb = -128;
    st = MAKE_U8(st + (u8)sb); /* unsigned不一致 */
    printf("st:%u sb:%d\n", st, sb);

    /* 17: mixing via functions */
    u8 tmp1 = make_mod(250, 10); /* unsigned不一致 (wrap-around) */
    i8 tmp2 = make_signed_op(-5, 3);
    printf("tmp1:%u tmp2:%d\n", tmp1, tmp2);

    /* 18: more structural uses */
    for (int i = 0; i < 10; ++i) {
        NODE_SET_A(nodes[i], i * 25);
        NODE_SET_B(nodes[i], i - 7);
        nodes[i].a = mix_signed_unsigned(nodes[i].a, nodes[i].b); /* unsigned不一致 */
    }

    /* 19: comparisons involving uint32_t and int32_t */
    u32 bigu = 3000000000u;
    i32 bigs = -1000000000;
    if (bigu > (u32)bigs) { /* unsigned不一致 */
        printf("bigu > bigs as u32\n");
    }

    /* 20: nested macros to obfuscate */
    u8 nm = MAKE_U8(CLAMP_U8(COMPLEX_OP(nodes[0].a, nodes[1].a, nodes[2].b))); /* unsigned不一致 */
    printf("nm:%u\n", nm);

    /* 21: final summary prints */
    printf("Final values ua:%u ib:%d uc:%u id:%d\n", ua, ib, uc, id);

    return 0;
}