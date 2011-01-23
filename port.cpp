// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
// ex: set softtabstop=4 tabstop=4 shiftwidth=4 expandtab:

// GENERIC ERLANG PORT VERSION 0.7
// automatically create Erlang bindings to C++/C that requires an OS process

//////////////////////////////////////////////////////////////////////////////
// BSD LICENSE
// 
// Copyright (c) 2009-2011, Michael Truog <mjtruog at gmail dot com>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in
//       the documentation and/or other materials provided with the
//       distribution.
//     * All advertising materials mentioning features or use of this
//       software must display the following acknowledgment:
//         This product includes software developed by Michael Truog
//     * The name of the author may not be used to endorse or promote
//       products derived from this software without specific prior
//       written permission
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <ei.h>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/control/if.hpp>

#include "port.hpp"
#include "realloc_ptr.hpp"

// erlang:open_port/2 option nouse_stdio
#define PORT_READ_FILE_DESCRIPTOR 3
#define PORT_WRITE_FILE_DESCRIPTOR 4

// code below depends on these prefix types
#define INPUT_PREFIX_TYPE    uint16_t // function identifier
#define OUTPUT_PREFIX_TYPE   uint32_t // maximum length


#if ! defined(PORT_NAME)
#error Define PORT_NAME within the functions header file to specify the \
       executable name
#endif
#if ! defined(PORT_FUNCTIONS)
#if defined(PORT_DRIVER_FUNCTIONS)

#define CREATE_PORT_FUNCTIONS_DEFINITION(S, DATA, ELEMENT) (\
    BOOST_PP_TUPLE_ELEM(5, 0, ELEMENT),\
    BOOST_PP_TUPLE_ELEM(5, 1, ELEMENT),\
    BOOST_PP_TUPLE_ELEM(5, 2, ELEMENT),\
    BOOST_PP_TUPLE_ELEM(5, 3, ELEMENT))\

#define PORT_FUNCTIONS \
    BOOST_PP_SEQ_TRANSFORM(CREATE_PORT_FUNCTIONS_DEFINITION, _, \
                           PORT_DRIVER_FUNCTIONS)
#warning Using PORT_DRIVER_FUNCTIONS to determine PORT_FUNCTIONS
#else
#error Define PORT_FUNCTIONS within the functions header file to specify \
       the functions and their types
#endif
#endif

// define the structure of the PORT_FUNCTIONS macro data
// (sequence of tuples)

// 4 tuple elements in the PORT_FUNCTIONS sequence
#define PORT_FUNCTION_ENTRY_LENGTH   4
// specific tuple elements in the PORT_FUNCTIONS sequence
#define PORT_FUNCTION_ENTRY_NAME     0
#define PORT_FUNCTION_ENTRY_ARGC     1
#define PORT_FUNCTION_ENTRY_ARGV     2
#define PORT_FUNCTION_ENTRY_RETURN   3

// macros to access function data in a PORT_FUNCTIONS tuple entry

#define GET_NAME(FUNCTION) \
    BOOST_PP_TUPLE_ELEM(\
        PORT_FUNCTION_ENTRY_LENGTH, \
        PORT_FUNCTION_ENTRY_NAME, FUNCTION\
    )
#define GET_ARGC(FUNCTION) \
    BOOST_PP_TUPLE_ELEM(\
        PORT_FUNCTION_ENTRY_LENGTH, \
        PORT_FUNCTION_ENTRY_ARGC, FUNCTION\
    )
#define GET_ARGV(FUNCTION) \
    BOOST_PP_TUPLE_ELEM(\
        PORT_FUNCTION_ENTRY_LENGTH, \
        PORT_FUNCTION_ENTRY_ARGV, FUNCTION\
    )
#define GET_RETURN(FUNCTION) \
    BOOST_PP_TUPLE_ELEM(\
        PORT_FUNCTION_ENTRY_LENGTH, \
        PORT_FUNCTION_ENTRY_RETURN, FUNCTION\
    )
#define GET_ASYNC(FUNCTION) 0

// enforce inherent implementation limits

#if BOOST_PP_SEQ_SIZE(PORT_FUNCTIONS) > 32767
#error Limited to 32767 port functions (type uint16_t is used for "cmd")
#endif

#if defined(PORT_C_FUNCTIONS_HEADER_FILE)
extern "C"
{
#include PORT_C_FUNCTIONS_HEADER_FILE
}
#elif defined(PORT_CXX_FUNCTIONS_HEADER_FILE)
#include PORT_CXX_FUNCTIONS_HEADER_FILE
#else
#error Neither PORT_C_FUNCTIONS_HEADER_FILE nor \
       PORT_CXX_FUNCTIONS_HEADER_FILE are defined
#endif

#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_void                          \
    (void)
#define STORE_RETURN_VALUE_TYPE_void                                          \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_atom(buffer.get<char>(), &index, "ok"))                     \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_char(N)                                       \
    sizeof(char)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_char(OFFSET)                          \
    *((char *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_char                          \
    char returnValue = 
#define STORE_RETURN_VALUE_TYPE_char                                          \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_long(buffer.get<char>(), &index, returnValue))              \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_uchar(N)                                      \
    sizeof(unsigned char)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_uchar(OFFSET)                         \
    *((unsigned char *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_uchar                         \
    unsigned char returnValue = 
#define STORE_RETURN_VALUE_TYPE_uchar                                         \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_char(buffer.get<char>(), &index, (char) returnValue))       \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_bool(N)                                       \
    sizeof(uint8_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_bool(OFFSET)                          \
    *((uint8_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_bool                          \
    bool returnValue = 
#define STORE_RETURN_VALUE_TYPE_bool                                          \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_boolean(buffer.get<char>(), &index, (int) returnValue))     \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_int8_t(N)                                     \
    sizeof(int8_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_int8_t(OFFSET)                        \
    *((int8_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_int8_t                        \
    int8_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_int8_t                                        \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_long(buffer.get<char>(), &index, returnValue))              \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_uint8_t(N)                                    \
    sizeof(uint8_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_uint8_t(OFFSET)                       \
    *((uint8_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_uint8_t                       \
    uint8_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_uint8_t                                       \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_ulong(buffer.get<char>(), &index, returnValue))             \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_int16_t(N)                                    \
    sizeof(int16_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_int16_t(OFFSET)                       \
    *((int16_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_int16_t                       \
    int16_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_int16_t                                       \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_long(buffer.get<char>(), &index, returnValue))              \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_uint16_t(N)                                   \
    sizeof(uint16_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_uint16_t(OFFSET)                      \
    *((uint16_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_uint16_t                      \
    uint16_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_uint16_t                                      \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_ulong(buffer.get<char>(), &index, returnValue))             \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_int32_t(N)                                    \
    sizeof(int32_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_int32_t(OFFSET)                       \
    *((int32_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_int32_t                       \
    int32_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_int32_t                                       \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_long(buffer.get<char>(), &index, returnValue))              \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_uint32_t(N)                                   \
    sizeof(uint32_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_uint32_t(OFFSET)                      \
    *((uint32_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_uint32_t                      \
    uint32_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_uint32_t                                      \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_ulong(buffer.get<char>(), &index, returnValue))             \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_int64_t(N)                                    \
    sizeof(int64_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_int64_t(OFFSET)                       \
    *((int64_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_int64_t                       \
    int64_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_int64_t                                       \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_longlong(buffer.get<char>(), &index, returnValue))          \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_uint64_t(N)                                   \
    sizeof(uint64_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_uint64_t(OFFSET)                      \
    *((uint64_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_uint64_t                      \
    uint64_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_uint64_t                                      \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_ulonglong(buffer.get<char>(), &index, returnValue))         \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_time_t(N)                                     \
    sizeof(uint64_t)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_time_t(OFFSET)                        \
    *((uint64_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_time_t                        \
    uint64_t returnValue = 
#define STORE_RETURN_VALUE_TYPE_time_t                                        \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_ulonglong(buffer.get<char>(), &index, returnValue))         \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_float(N)                                      \
    sizeof(double)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_float(OFFSET)                         \
    ((float) *((double *) &(buffer[(OFFSET)])))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_float                         \
    double returnValue =
#define STORE_RETURN_VALUE_TYPE_float                                         \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_double(buffer.get<char>(), &index, returnValue))            \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_double(N)                                     \
    sizeof(double)
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_double(OFFSET)                        \
    *((double *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_double                        \
    double returnValue =
#define STORE_RETURN_VALUE_TYPE_double                                        \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (ei_encode_double(buffer.get<char>(), &index, returnValue))            \
        return InternalExitStatus::ei_encode_error;

#define GET_TYPE_SIZE_FROM_TYPE_pchar_len(N)                                  \
    sizeof(uint32_t) + *((uint32_t *) &(buffer[(                              \
        BOOST_PP_CAT(offset_arg, N)                                           \
    )]))
#define GET_FUNCTION_ARGUMENT_FROM_TYPE_pchar_len(OFFSET)                     \
    ((char *) &(buffer[(OFFSET + sizeof(uint32_t))])),                        \
    *((uint32_t *) &(buffer[(OFFSET)]))
#define CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_pchar                         \
    char const * returnValue =
#define STORE_RETURN_VALUE_TYPE_pchar                                         \
    if (ei_encode_version(buffer.get<char>(), &index))                        \
        return InternalExitStatus::ei_encode_error;                           \
    if (buffer.reserve(index + strlen(returnValue) + 1) == false)             \
        return InternalExitStatus::write_overflow;                            \
    if (ei_encode_string(buffer.get<char>(), &index, returnValue))            \
        return InternalExitStatus::ei_encode_error;
    
namespace
{
    // list of non-fatal errors that can be sent back

    // port will send an error for protocol problems
    namespace Error
    {
        char const * const invalid_function = "Invalid function call";
    }

    // provide a single list of exit_status values
    // (use exit_status as an option to erlang:open_port/2)

    // port will exit for conditions related to reading/writing
    namespace InternalExitStatus
    {
        enum
        {
            success = 0,
            erlang_exit = GEPD::ExitStatus::errors_min,
            read_EAGAIN,
            read_EBADF,
            read_EFAULT,
            read_EINTR,
            read_EINVAL,
            read_EIO,
            read_EISDIR,
            read_null,
            read_overflow,
            read_unknown,                     //  90
            write_EAGAIN,
            write_EBADF,
            write_EFAULT,
            write_EFBIG,
            write_EINTR,
            write_EINVAL,
            write_EIO,
            write_ENOSPC,
            write_EPIPE,
            write_null,                       // 100
            write_overflow,
            write_unknown,
            ei_encode_error,
            poll_EBADF,
            poll_EFAULT,
            poll_EINTR,
            poll_EINVAL,
            poll_ENOMEM,
            poll_ERR,
            poll_HUP,                         // 110
            poll_NVAL,
            poll_unknown,
            pipe_EFAULT,
            pipe_EINVAL,
            pipe_EMFILE,
            pipe_ENFILE,
            pipe_unknown,
            dup_EBADF,
            dup_EBUSY,
            dup_EINTR,                        // 120
            dup_EINVAL,
            dup_EMFILE,
            dup_unknown,
            close_EBADF,
            close_EINTR,
            close_EIO,
            close_unknown                     // 127
        };

        int read_errno()
        {
            switch (errno)
            {
                case EAGAIN:
                    return read_EAGAIN;
                case EBADF:
                    return read_EBADF;
                case EFAULT:
                    return read_EFAULT;
                case EINTR:
                    return read_EINTR;
                case EINVAL:
                    return read_EINVAL;
                case EIO:
                    return read_EIO;
                case EISDIR:
                    return read_EISDIR;
                default:
                    return read_unknown;
            }
        }

        int write_errno()
        {
            switch (errno)
            {
                case EAGAIN:
                    return write_EAGAIN;
                case EBADF:
                    return write_EBADF;
                case EFAULT:
                    return write_EFAULT;
                case EFBIG:
                    return write_EFBIG;
                case EINTR:
                    return write_EINTR;
                case EINVAL:
                    return write_EINVAL;
                case EIO:
                    return write_EIO;
                case ENOSPC:
                    return write_ENOSPC;
                case EPIPE:
                    return write_EPIPE;
                default:
                    return write_unknown;
            }
        }

        int poll_errno()
        {
            switch (errno)
            {
                case EBADF:
                    return poll_EBADF;
                case EFAULT:
                    return poll_EFAULT;
                case EINTR:
                    return poll_EINTR;
                case EINVAL:
                    return poll_EINVAL;
                case ENOMEM:
                    return poll_ENOMEM;
                default:
                    return poll_unknown;
            }
        }

        int pipe_errno()
        {
            switch (errno)
            {
                case EFAULT:
                    return pipe_EFAULT;
                case EINVAL:
                    return pipe_EINVAL;
                case EMFILE:
                    return pipe_EMFILE;
                case ENFILE:
                    return pipe_ENFILE;
                default:
                    return pipe_unknown;
            }
        }

        int dup_errno()
        {
            switch (errno)
            {
                case EBADF:
                    return dup_EBADF;
                case EBUSY:
                    return dup_EBUSY;
                case EINTR:
                    return dup_EINTR;
                case EINVAL:
                    return dup_EINVAL;
                case EMFILE:
                    return dup_EMFILE;
                default:
                    return dup_unknown;
            }
        }

        int close_errno()
        {
            switch (errno)
            {
                case EBADF:
                    return close_EBADF;
                case EINTR:
                    return close_EINTR;
                case EIO:
                    return close_EIO;
                default:
                    return close_unknown;
            }
        }
    }

    int read_exact(unsigned char * const buffer,
                   uint32_t const length)
    {
        uint32_t total = 0;
        while (total < length)
        {
            ssize_t const i = read(PORT_READ_FILE_DESCRIPTOR,
                                   buffer + total, length - total);
            if (i <= 0)
            {
                if (i == -1)
                    return InternalExitStatus::read_errno();
                else
                    return InternalExitStatus::read_null;
            }
            total += i;
        }
        if (total > length)
            return InternalExitStatus::read_overflow;
        return InternalExitStatus::success;
    }
    
    int write_exact(unsigned char const * const buffer,
                    uint32_t const length)
    {
        uint32_t total = 0;
        while (total < length)
        {
            ssize_t const i = write(PORT_WRITE_FILE_DESCRIPTOR,
                                    buffer + total, length - total);
            if (i <= 0)
            {
                if (i == -1)
                    return InternalExitStatus::write_errno();
                else
                    return InternalExitStatus::write_null;
            }
            total += i;
        }
        if (total > length)
            return InternalExitStatus::write_overflow;
        return InternalExitStatus::success;
    }
    
    int read_cmd(realloc_ptr<unsigned char> & buffer)
    {
        unsigned char lengthData[4];
        int const status = read_exact(lengthData, 4);
        if (status)
            return status;
        uint32_t const length = (lengthData[0] << 24) |
                                (lengthData[1] << 16) |
                                (lengthData[2] <<  8) |
                                 lengthData[3];
        buffer.reserve(length);
        return read_exact(buffer.get(), length);
    }
    
    int write_cmd(realloc_ptr<unsigned char> & buffer, uint32_t length)
    {
        buffer[0] = (length & 0xff000000) >> 24;
        buffer[1] = (length & 0x00ff0000) >> 16;
        buffer[2] = (length & 0x0000ff00) >> 8;
        buffer[3] =  length & 0x000000ff;
        return write_exact(buffer.get(), length + 4);
    }
    
    int store_std_fd(int & out, int in)
    {
        int fds[2] = {-1, -1};
        if (pipe(fds) == -1)
            return InternalExitStatus::pipe_errno();
        if (dup2(fds[1], in) == -1)
            return InternalExitStatus::dup_errno();
        if (close(fds[1]) == -1)
            return InternalExitStatus::close_errno();
        out = fds[0];
        return InternalExitStatus::success;
    }
    
    int reply_error_string(realloc_ptr<unsigned char> & buffer,
                           int & index, char const * const str)
    {
        if (ei_encode_version(buffer.get<char>(), &index))
            return InternalExitStatus::ei_encode_error;
        if (ei_encode_tuple_header(buffer.get<char>(), &index, 2))
            return InternalExitStatus::ei_encode_error;
        if (ei_encode_atom(buffer.get<char>(), &index, "error"))
            return InternalExitStatus::ei_encode_error;
        if (buffer.reserve(index + strlen(str) + 1) == false)
            return InternalExitStatus::write_overflow;
        if (ei_encode_string(buffer.get<char>(), &index, str))
            return InternalExitStatus::ei_encode_error;
        return InternalExitStatus::success;
    }

#define STORE_RETURN_VALUE(TYPE) \
    BOOST_PP_CAT(STORE_RETURN_VALUE_TYPE_, TYPE)

#define GET_FUNCTION_ARGUMENT(TYPE, OFFSET) \
    BOOST_PP_CAT(\
        GET_FUNCTION_ARGUMENT_FROM_TYPE_, TYPE \
    )(OFFSET)

#define CREATE_FUNCTION_RETURN_VALUE_STORE(TYPE) \
    BOOST_PP_CAT(\
        CREATE_FUNCTION_RETURN_VALUE_STORE_TYPE_, TYPE\
    )

#define GET_TYPE_SIZE(TYPE, N) \
    BOOST_PP_CAT(GET_TYPE_SIZE_FROM_TYPE_, TYPE)(N)

#define STORE_FUNCTION_ARGUMENT_SIZE(TYPE, N, OFFSET) \
    size_t const BOOST_PP_CAT(offset_arg, N) = OFFSET ;

#define STORE_FUNCTION_ARGUMENTS(Z, N, DATA) \
    STORE_FUNCTION_ARGUMENT_SIZE( \
        BOOST_PP_SEQ_ELEM(N, BOOST_PP_SEQ_ELEM(0, DATA)), \
        BOOST_PP_INC(N), \
        BOOST_PP_CAT(offset_arg, N) \
        + GET_TYPE_SIZE(BOOST_PP_SEQ_ELEM(N, BOOST_PP_SEQ_ELEM(0, DATA)), N) \
    )

#define CREATE_FUNCTION_ARGUMENTS(Z, N, DATA) \
    GET_FUNCTION_ARGUMENT(\
        BOOST_PP_SEQ_ELEM(N, BOOST_PP_SEQ_ELEM(0, DATA)), \
        BOOST_PP_CAT(offset_arg, N) \
    )

#define CREATE_FUNCTION(I, OFFSETS, FUNCTION) \
case BOOST_PP_DEC(I):\
{\
    BOOST_PP_IF(\
        GET_ARGC(FUNCTION),\
        size_t const offset_arg0 = BOOST_PP_SEQ_ELEM(0, OFFSETS);, \
        BOOST_PP_EMPTY() \
    )\
    BOOST_PP_REPEAT_FROM_TO( \
        0, \
        BOOST_PP_DEC(GET_ARGC(FUNCTION)), \
        STORE_FUNCTION_ARGUMENTS, \
        (BOOST_PP_TUPLE_TO_SEQ(GET_ARGC(FUNCTION), GET_ARGV(FUNCTION)))\
        (BOOST_PP_SEQ_ELEM(0, OFFSETS)) \
    ) \
    CREATE_FUNCTION_RETURN_VALUE_STORE(GET_RETURN(FUNCTION)) \
    GET_NAME(FUNCTION) \
    BOOST_PP_LPAREN() \
    BOOST_PP_ENUM( \
        GET_ARGC(FUNCTION), \
        CREATE_FUNCTION_ARGUMENTS, \
        (BOOST_PP_TUPLE_TO_SEQ(GET_ARGC(FUNCTION), GET_ARGV(FUNCTION)))\
        (BOOST_PP_SEQ_ELEM(0, OFFSETS)) \
    ) \
    BOOST_PP_RPAREN() \
    ; \
    int index = BOOST_PP_SEQ_ELEM(1, OFFSETS); \
    STORE_RETURN_VALUE(GET_RETURN(FUNCTION)) \
    if (status = write_cmd(buffer, index - BOOST_PP_SEQ_ELEM(1, OFFSETS))) \
        return status; \
    return InternalExitStatus::success;\
}

    int consume_erlang(short & revents, realloc_ptr<unsigned char> & buffer)
    {
        if (revents & POLLERR)
            return InternalExitStatus::poll_ERR;
        else if (revents & POLLHUP)
            return InternalExitStatus::erlang_exit;
        else if (revents & POLLNVAL)
            return InternalExitStatus::poll_NVAL;
        revents = 0;
        int status;
        if ((status = read_cmd(buffer)))
        {
            return status;
        }
        else
        {
            switch (*((INPUT_PREFIX_TYPE *) buffer.get()))
            {
                BOOST_PP_SEQ_FOR_EACH(CREATE_FUNCTION,
                                      (sizeof(INPUT_PREFIX_TYPE))
                                      (sizeof(OUTPUT_PREFIX_TYPE)),
                                      PORT_FUNCTIONS)
    
                default:
                    int index = sizeof(OUTPUT_PREFIX_TYPE);
                    if ((status = reply_error_string(buffer, index,
                                                     Error::invalid_function)))
                        return status;
                    if ((status = write_cmd(buffer, index -
                                            sizeof(OUTPUT_PREFIX_TYPE))))
                        return status;
                    return InternalExitStatus::success;
            }
        }
    }

    int consume_stream(int fd, short & revents, char const * const name,
                       realloc_ptr<unsigned char> & buffer,
                       realloc_ptr<unsigned char> & stream, size_t & i)
    {
        if (revents & POLLERR)
            return InternalExitStatus::poll_ERR;
        else if (revents & POLLHUP)
            return InternalExitStatus::poll_HUP;
        else if (revents & POLLNVAL)
            return InternalExitStatus::poll_NVAL;
        revents = 0;

        ssize_t left = stream.size() - i;
        ssize_t readBytes;
        while ((readBytes = read(fd, &stream[i], left)) == left &&
               stream.grow())
        {
            i += left;
            left = stream.size() - i;
        }
        if (readBytes == 0 && i == 0)
            return InternalExitStatus::success;
        else if (readBytes == -1)
            return InternalExitStatus::read_errno();
        i += readBytes; // i is the next index to read at, always

        // only send stderr output before the last newline character
        bool foundNewline = false;
        size_t iNewline = 0;
        for (ssize_t j = i - 1; ! foundNewline && j >= 0; --j)
        {
            if (stream[j] == '\n')
            {
                foundNewline = true;
                iNewline = j;
            }
        }

        if (foundNewline)
        {
            int index = sizeof(OUTPUT_PREFIX_TYPE);
            if (ei_encode_version(buffer.get<char>(), &index))
                return InternalExitStatus::ei_encode_error;
            if (ei_encode_tuple_header(buffer.get<char>(), &index, 2))
                return InternalExitStatus::ei_encode_error;
            if (ei_encode_atom(buffer.get<char>(), &index, name))
                return InternalExitStatus::ei_encode_error;
            if (buffer.reserve(index + (iNewline + 1) + 1) == false)
                return InternalExitStatus::write_overflow;
            if (ei_encode_string_len(buffer.get<char>(), &index,
                                     stream.get<char>(), iNewline + 1))
                return InternalExitStatus::ei_encode_error;
            int status;
            if ((status = write_cmd(buffer, index -
                                    sizeof(OUTPUT_PREFIX_TYPE))))
                return status;
            // keep any stderr data not yet sent (waiting for a newline)
            if (iNewline == i - 1)
            {
                i = 0;
            }
            else
            {
                size_t const remainingBytes = i - iNewline - 1;
                stream.move(iNewline + 1, remainingBytes, 0);
                i = remainingBytes;
            }
        }
        return InternalExitStatus::success;
    }

    enum
    {
        INDEX_STDOUT = 0,
        INDEX_STDERR,
        INDEX_ERLANG
    };
}

// main loop for handling inherently synchronous function calls
// (a linked-in Erlang port driver that makes synchronous calls with
//  driver level locking should be similar to an Erlang port, except that
//  the port driver is a VM process and the port is an OS process)

int GEPD::default_main()
{
    struct pollfd fds[3];
    int const nfds = 3;
    int const timeout = -1; // milliseconds
    // use the option {packet, 4} for open_port/2
    // (limited by 4MB buffer size below)
    realloc_ptr<unsigned char> buffer(32768, 4194304);
    realloc_ptr<unsigned char> stream1(1, 16384);
    realloc_ptr<unsigned char> stream2(1, 16384);
    int status;
    if ((status = GEPD::init(fds, nfds)))
        return status;
    return GEPD::wait(fds, nfds, timeout, buffer, stream1, stream2);
}

int GEPD::init(struct pollfd * const fds, nfds_t const nfds)
{
    if (nfds < 3)
        return InternalExitStatus::poll_unknown;

    int status;
    if ((status = store_std_fd(fds[INDEX_STDOUT].fd, 1)))
        return status;
    fds[INDEX_STDOUT].events = POLLIN | POLLPRI;
    fds[INDEX_STDOUT].revents = 0;
    if ((status = store_std_fd(fds[INDEX_STDERR].fd, 2)))
        return status;
    fds[INDEX_STDERR].events = POLLIN | POLLPRI;
    fds[INDEX_STDERR].revents = 0;
    fds[INDEX_ERLANG].fd = PORT_READ_FILE_DESCRIPTOR;
    fds[INDEX_ERLANG].events = POLLIN | POLLPRI;
    fds[INDEX_ERLANG].revents = 0;
    return InternalExitStatus::success;
}

int GEPD::wait(struct pollfd * const fds, nfds_t const nfds,
               int const timeout,
               realloc_ptr<unsigned char> & buffer,
               realloc_ptr<unsigned char> & stream1,
               realloc_ptr<unsigned char> & stream2)
{
    static size_t index_stream1 = 0;
    static size_t index_stream2 = 0;
    int count;
    while ((count = poll(fds, nfds, timeout)) > 0)
    {
        int status;
        if (count > 0 && fds[INDEX_ERLANG].revents != 0)
        {
            if ((status = consume_erlang(fds[INDEX_ERLANG].revents, buffer)))
                return status;
            --count;
        }
        fflush(stdout);
        fflush(stderr);
        if (count > 0 && fds[INDEX_STDERR].revents != 0)
        {
            if ((status = consume_stream(fds[INDEX_STDERR].fd, 
                                         fds[INDEX_STDERR].revents,
                                         "stderr", buffer,
                                         stream2, index_stream2)))
                return status;
            --count;
        }
        if (count > 0 && fds[INDEX_STDOUT].revents != 0)
        {
            if ((status = consume_stream(fds[INDEX_STDOUT].fd, 
                                         fds[INDEX_STDOUT].revents,
                                         "stdout", buffer,
                                         stream1, index_stream1)))
                return status;
            --count;
        }
        if (count > 0)
        {
            return GEPD::ExitStatus::ready;
        }
    }
    if (count == 0)
        return GEPD::ExitStatus::timeout;
    else
        return InternalExitStatus::poll_errno();
}

