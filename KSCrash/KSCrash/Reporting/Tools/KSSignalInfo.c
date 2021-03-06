//
//  KSSignalInfo.c
//
//  Created by Karl Stenerud on 12-02-03.
//
//  Copyright (c) 2012 Karl Stenerud. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall remain in place
// in this source code.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


#include "KSSignalInfo.h"

#include <signal.h>


typedef struct
{
    const int code;
    const char* const name;
} KSSignalCodeInfo;

typedef struct
{
    const int sigNum;
    const char* const name;
    const KSSignalCodeInfo* const codes;
    const int numCodes;
} KSSignalInfo;

static const KSSignalCodeInfo g_sigIllCodes[] =
{
    {ILL_NOOP, "ILL_NOOP"},
    {ILL_ILLOPC, "ILL_ILLOPC"},
    {ILL_ILLTRP, "ILL_ILLTRP"},
    {ILL_PRVOPC, "ILL_PRVOPC"},
    {ILL_ILLOPN, "ILL_ILLOPN"},
    {ILL_ILLADR, "ILL_ILLADR"},
    {ILL_PRVREG, "ILL_PRVREG"},
    {ILL_COPROC, "ILL_COPROC"},
    {ILL_BADSTK, "ILL_BADSTK"},
};

static const KSSignalCodeInfo g_sigTrapCodes[] =
{
    {0, "0"},
    {TRAP_BRKPT, "TRAP_BRKPT"},
    {TRAP_TRACE, "TRAP_TRACE"},
};

static const KSSignalCodeInfo g_sigFPECodes[] =
{
    {FPE_NOOP, "FPE_NOOP"},
    {FPE_FLTDIV, "FPE_FLTDIV"},
    {FPE_FLTOVF, "FPE_FLTOVF"},
    {FPE_FLTUND, "FPE_FLTUND"},
    {FPE_FLTRES, "FPE_FLTRES"},
    {FPE_FLTINV, "FPE_FLTINV"},
    {FPE_FLTSUB, "FPE_FLTSUB"},
    {FPE_INTDIV, "FPE_INTDIV"},
    {FPE_INTOVF, "FPE_INTOVF"},
};

static const KSSignalCodeInfo g_sigBusCodes[] =
{
    {BUS_NOOP, "BUS_NOOP"},
    {BUS_ADRALN, "BUS_ADRALN"},
    {BUS_ADRERR, "BUS_ADRERR"},
    {BUS_OBJERR, "BUS_OBJERR"},
};

static const KSSignalCodeInfo g_sigSegVCodes[] =
{
    {SEGV_NOOP, "SEGV_NOOP"},
    {SEGV_MAPERR, "SEGV_MAPERR"},
    {SEGV_ACCERR, "SEGV_ACCERR"},
};


static const KSSignalInfo g_fatalSignalData[] =
{
    {SIGABRT, "SIGABRT", 0, 0},
    {SIGBUS, "SIGBUS", g_sigBusCodes, sizeof(g_sigBusCodes) / sizeof(g_sigBusCodes[0])},
    {SIGFPE, "SIGFPE", g_sigFPECodes, sizeof(g_sigFPECodes) / sizeof(g_sigFPECodes[0])},
    {SIGILL, "SIGILL", g_sigIllCodes, sizeof(g_sigIllCodes) / sizeof(g_sigIllCodes[0])},
    {SIGPIPE, "SIGPIPE", 0, 0},
    {SIGSEGV, "SIGSEGV", g_sigSegVCodes, sizeof(g_sigSegVCodes) / sizeof(g_sigSegVCodes[0])},
    {SIGSYS, "SIGSYS", 0, 0},
    {SIGTRAP, "SIGTRAP", g_sigTrapCodes, sizeof(g_sigTrapCodes) / sizeof(g_sigTrapCodes[0])},
};
static const int g_fatalSignalsCount = sizeof(g_fatalSignalData) / sizeof(*g_fatalSignalData);

// Note: Dereferencing a NULL pointer causes SIGILL, ILL_ILLOPC on i386
//       but causes SIGTRAP, 0 on arm.
static const int g_fatalSignals[] =
{
    SIGABRT,
    SIGBUS,
    SIGFPE,
    SIGILL,
    SIGPIPE,
    SIGSEGV,
    SIGSYS,
    SIGTRAP,
};

const char* kssignal_signalName(const int sigNum)
{
    for(int i = 0; i < g_fatalSignalsCount; i++)
    {
        if(g_fatalSignalData[i].sigNum == sigNum)
        {
            return g_fatalSignalData[i].name;
        }
    }
    return NULL;
}

const char* kssignal_signalCodeName(const int sigNum, const int code)
{
    for(int si = 0; si < g_fatalSignalsCount; si++)
    {
        if(g_fatalSignalData[si].sigNum == sigNum)
        {
            for(int ci = 0; ci < g_fatalSignalData[si].numCodes; ci++)
            {
                if(g_fatalSignalData[si].codes[ci].code == code)
                {
                    return g_fatalSignalData[si].codes[ci].name;
                }
            }
        }
    }
    return NULL;
}

const int* kssignal_fatalSignals(void)
{
    return g_fatalSignals;
}

int kssignal_numFatalSignals(void)
{
    return g_fatalSignalsCount;
}

#define EXC_UNIX_BAD_SYSCALL 0x10000 /* SIGSYS */
#define EXC_UNIX_BAD_PIPE    0x10001 /* SIGPIPE */
#define EXC_UNIX_ABORT       0x10002 /* SIGABRT */

int kssignal_machExceptionForSignal(const int sigNum)
{
    switch(sigNum)
    {
        case SIGFPE:
            return EXC_ARITHMETIC;
        case SIGSEGV:
            return EXC_BAD_ACCESS;
        case SIGBUS:
            return EXC_BAD_ACCESS;
        case SIGILL:
            return EXC_BAD_INSTRUCTION;
        case SIGTRAP:
            return EXC_BREAKPOINT;
        case SIGEMT:
            return EXC_EMULATION;
        case SIGSYS:
            return EXC_UNIX_BAD_SYSCALL;
        case SIGPIPE:
            return EXC_UNIX_BAD_PIPE;
        case SIGABRT:
            // The Apple reporter uses EXC_CRASH instead of EXC_UNIX_ABORT
            return EXC_CRASH;
        case SIGKILL:
            return EXC_SOFT_SIGNAL;
    }
    return 0;
}

int kssignal_signalForMachException(const int exception,
                                    const mach_exception_code_t code)
{
    switch(exception)
    {
        case EXC_ARITHMETIC:
            return SIGFPE;
        case EXC_BAD_ACCESS:
            return code == KERN_INVALID_ADDRESS ? SIGSEGV : SIGBUS;
        case EXC_BAD_INSTRUCTION:
            return SIGILL;
        case EXC_BREAKPOINT:
            return SIGTRAP;
        case EXC_EMULATION:
            return SIGEMT;
        case EXC_SOFTWARE:
        {
            switch (code)
            {
                case EXC_UNIX_BAD_SYSCALL:
                    return SIGSYS;
                case EXC_UNIX_BAD_PIPE:
                    return SIGPIPE;
                case EXC_UNIX_ABORT:
                    return SIGABRT;
                case EXC_SOFT_SIGNAL:
                    return SIGKILL;
            }
            break;
        }
    }
    return 0;
}
