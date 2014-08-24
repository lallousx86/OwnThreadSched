#include "stdafx.h"

/* -----------------------------------------------------------------------------
* Copyright (c) Elias Bachaalany <lallousz-x86@yahoo.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
* -----------------------------------------------------------------------------
*/

#define ATOMIC_METHOD_BEGIN(name) \
  static int _counter = 1; \
  int _pci = 1;

#define ATOMIC_STATEMENT(x) \
  if (_pci++ == _counter) \
  { \
    x; \
  }

#define ATOMIC_METHOD_END \
  _counter++; \
  ATOMIC_STATEMENT(_counter = 1; return -1)

#define ATOMIC_DECLARE_FOR(v, s, e) \
  static int v, _##v##_counter_start ; \
  int _##v##_start = s, _##v##_end = e;

#define ATOMIC_FOR_BEGIN(v) if (_pci++ == _counter) \
  { \
    _##v##_counter_start = _counter; \
    v = _##v##_start; \
  } \

#define ATOMIC_FOR_END(v) \
  ATOMIC_STATEMENT( if (v < _##v##_end) { _counter = _##v##_counter_start; v++; } )

#define ATOMIC_FOR_START(v, s, e) \
    ATOMIC_DECLARE_FOR(v, s, e); ATOMIC_FOR_BEGIN(v);

//-------------------------------------------------------------------------
int method1()
{
    ATOMIC_METHOD_BEGIN(method1);

    ATOMIC_STATEMENT(printf("m1:step1\n"));
    ATOMIC_STATEMENT(printf("m1:step2\n"));
    ATOMIC_STATEMENT(printf("m1:step3\n"));
    ATOMIC_STATEMENT(printf("m1:step4\n"));

    ATOMIC_METHOD_END;
    return 0;
}

//-------------------------------------------------------------------------
int method2()
{
    ATOMIC_METHOD_BEGIN(method2);

    ATOMIC_STATEMENT(printf("m2:step1\n"));
    ATOMIC_STATEMENT(printf("m2:step2\n"));
    ATOMIC_STATEMENT(printf("m2:step3\n"));
    ATOMIC_STATEMENT(printf("m2:step4\n"));

    ATOMIC_METHOD_END;
    return 0;
}

//-------------------------------------------------------------------------
int test_method3()
{
    //ATOMIC_METHOD_BEGIN(method3);
    static int _counter = 1;
    int _pci = 1;

    // DECLARE FOR
    static int _i, _i_counter_start;
    int _i_start = 1, _i_end = 4;

    // BEGIN FOR
    if (_pci++ == _counter)
    {
        _i_counter_start = _counter;
        _i = _i_start;
    }

    ATOMIC_STATEMENT(printf("i=%d\n", _i));

    // FOR END
    if (_pci++ == _counter)
    {
        if (_i < _i_end)
        {
            _counter = _i_counter_start;
            _i++;
        }
    }

    //#define ATOMIC_METHOD_END 
    if (_pci++ == _counter)
    {
        _counter = 1;
        return -1;
    }

    _counter++;

    return 0;
}

//-------------------------------------------------------------------------
int method3()
{
    ATOMIC_METHOD_BEGIN(method3);

    ATOMIC_FOR_START(i, 1, 10);
    {
        ATOMIC_FOR_START(j, 1, i);
        {
            ATOMIC_STATEMENT(printf("*"));
        }
        ATOMIC_FOR_END(j);
        ATOMIC_STATEMENT(printf("\n"));
    }
    ATOMIC_FOR_END(i);

    ATOMIC_METHOD_END;

    return 0;
}

//-------------------------------------------------------------------------
int simple_thread_controller(int count, ...)
{
    typedef int (*atomic_method_t)(void);
    va_list args;
    int i;

    atomic_method_t *methods = new atomic_method_t[count];
    bool *done = new bool[count];

    for (i = 0; i < count; i++)
        done[i] = false;

    va_start(args, count);

    for (i = 0; i < count; i++)
        methods[i] = va_arg(args, atomic_method_t);

    va_end(args);

    do
    {
        int exec_something = 0;
        for (i = 0; i < count; i++)
        {
            // skip done statement
            if (done[i])
                continue;

            if (methods[i]() == -1)
                done[i] = true;
            else
                exec_something = 1;
        }
        if (!exec_something)
            break;
    } while (true);

    delete[] methods;
    delete[] done;

    return 0;
}

//-------------------------------------------------------------------------
int main()
{
    simple_thread_controller(
        3, 
        method1, 
        method2, 
        method3);
    return 0;
}