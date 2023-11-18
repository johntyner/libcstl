#ifndef CSTL_INTERNAL_CHECK_H
#define CSTL_INTERNAL_CHECK_H

#include <check.h>
#include <signal.h>
#include <setjmp.h>

extern struct sigaction ck_old_sigaction;
void ck_handle_signal(int, siginfo_t *, void *);

#define CK_JMP_BUF(NAME)            ck_jmp_buf_##NAME
#define DECLARE_CK_JMP_BUF(NAME)    jmp_buf CK_JMP_BUF(signal)

#define ck_assert_signal(SIGNUM, EXPR)                                  \
    do {                                                                \
        extern DECLARE_CK_JMP_BUF(signal);                              \
        struct sigaction sigact;                                        \
        int res;                                                        \
        sigact.sa_sigaction = ck_handle_signal;                         \
        sigemptyset(&sigact.sa_mask);                                   \
        sigact.sa_flags = SA_SIGINFO;                                   \
        (void)sigaction(SIGNUM, &sigact, &ck_old_sigaction);            \
        res = setjmp(CK_JMP_BUF(signal));                               \
        if (res == 0) {                                                 \
            EXPR;                                                       \
            (void)sigaction(SIGNUM, &ck_old_sigaction, NULL);           \
            ck_abort_msg("exepcted signal %d, none received", SIGNUM);  \
        }                                                               \
        (void)sigaction(SIGNUM, &ck_old_sigaction, NULL);               \
        if (res != SIGNUM) {                                            \
            ck_abort_msg(                                               \
                "exepcted signal %d, received %d", SIGNUM, res);        \
        }                                                               \
    } while (0)

#endif
