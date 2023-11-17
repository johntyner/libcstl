#ifndef CSTL_INTERNAL_CHECK_H
#define CSTL_INTERNAL_CHECK_H

#include <check.h>
#include <setjmp.h>

#define CK_JMP_BUF(NAME)            ck_jmp_buf_##NAME
#define DECLARE_CK_JMP_BUF(NAME)    jmp_buf CK_JMP_BUF(signal)

#define ck_assert_signal(SIGNUM, EXPR)                  \
    do {                                                \
        extern DECLARE_CK_JMP_BUF(signal);              \
        const int res = setjmp(CK_JMP_BUF(signal));     \
        if (res == 0) {                                 \
            EXPR;                                       \
            ck_abort_msg(                               \
                "exepcted signal %d, none received",    \
                SIGNUM);                                \
        } else if (res != SIGNUM) {                     \
            ck_abort_msg(                               \
                "exepcted signal %d, received %d",      \
                SIGNUM, res);                           \
        }                                               \
    } while (0)

#define ck_raise_signal(SIGNUM)                 \
    do {                                        \
        extern DECLARE_CK_JMP_BUF(signal);      \
        longjmp(CK_JMP_BUF(signal), SIGNUM);    \
    } while (0)

#endif
