/*
 * This source file is part of Junkyard repository.
 * Licensed under the terms of the WTFPL.
 * Copyright (C) Biswapriyo Nath
 *
 * PokeIoInfo.c: Poke into the hidden pioinfo variable in ucrtbase.dll.
 * Replace or define IOINFO_RVA for your system's ucrtbase.dll.
 * IOINFO_RVA is the difference between base address and pioinfo variable.
 *
 * Compile: cc -DIOINFO_RVA=0x1234 PokeIoInfo.c
 * Related links: https://github.com/ruby/ruby/blob/master/win32/win32.c
 */

#include <io.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

typedef char lowio_text_mode;
typedef char lowio_pipe_lookahead[3];

typedef struct
{
    CRITICAL_SECTION lock;
    intptr_t osfhnd;
    __int64 startpos;
    unsigned char osfile;
    lowio_text_mode textmode;
    lowio_pipe_lookahead _pipe_lookahead;

    uint8_t unicode : 1;
    uint8_t utf8translations : 1;
    uint8_t dbcsBufferUsed : 1;
    char dbcsBuffer[MB_LEN_MAX];
} ioinfo;

static ioinfo **__pioinfo = NULL;

#define IOINFO_L2E 6
#define IOINFO_ARRAY_ELTS (1 << IOINFO_L2E)

#define _pioinfo(i) (__pioinfo[(i) >> IOINFO_L2E] + ((i) & (IOINFO_ARRAY_ELTS - 1)))
#define _osfhnd(i) (_pioinfo(i)->osfhnd)
#define _osfile(i) (_pioinfo(i)->osfile)

int main(void)
{
    HANDLE h = GetModuleHandle("ucrtbase.dll");
    __pioinfo = (ioinfo **)((char *)h + IOINFO_RVA);

    for (int i = 0; i < 64; ++i)
    {
        if (_osfhnd(i) == _get_osfhandle(i))
            printf("matched (%d): %lld %lld\n", i, _osfhnd(i), _get_osfhandle(i));
        else
            printf("unmatched (%d): %lld %lld\n", i, _osfhnd(i), _get_osfhandle(i));
    }
}
