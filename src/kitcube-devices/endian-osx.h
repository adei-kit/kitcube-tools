/*-
 * Copyright (c) 2002 Thomas Moestl <tmm@FreeBSD.org>
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
 *
 */

#ifndef __ENDIAN_H
#define __ENDIAN_H

#if defined(__FreeBSD__)
#include <sys/endian.h>
#else /* __FreeBSD__ */

// Small hack to make sure we really get to load endian macros on OS X
// Will look into this more closely later on but this works for now..
#if !defined(BYTE_ORDER)
    #include <machine/endian.h>
    #if !defined(BYTE_ORDER)
        #error Oops!  Unable load endian macros..
    #endif
#endif


#include <libkern/OSByteOrder.h> 
#define bswap16(x) OSSwapInt16(x) 
#define bswap32(x) OSSwapInt32(x) 
#define bswap64(x) OSSwapInt64(x) 

//#define bswap16(x) _bswap16(x)
//#define bswap32(x) _bswap32(x)
//#define bswap64(x) _bswap64(x)



#if BYTE_ORDER == LITTLE_ENDIAN
#define htobe16(x)      bswap16((x))
#define htobe32(x)      bswap32((x))
#define htobe64(x)      bswap64((x))
#define htole16(x)      ((uint16_t)(x))
#define htole32(x)      ((uint32_t)(x))
#define htole64(x)      ((uint64_t)(x))

#define be16toh(x)      bswap16((x))
#define be32toh(x)      bswap32((x))
#define be64toh(x)      bswap64((x))
#define le16toh(x)      ((uint16_t)(x))
#define le32toh(x)      ((uint32_t)(x))
#define le64toh(x)      ((uint64_t)(x))
#else /* BYTE_ORDER != LITTLE_ENDIAN */
#define htobe16(x)      ((uint16_t)(x))
#define htobe32(x)      ((uint32_t)(x))
#define htobe64(x)      ((uint64_t)(x))
#define htole16(x)      bswap16((x))
#define htole32(x)      bswap32((x))
#define htole64(x)      bswap64((x))

#define be16toh(x)      ((uint16_t)(x))
#define be32toh(x)      ((uint32_t)(x))
#define be64toh(x)      ((uint64_t)(x))
#define le16toh(x)      bswap16((x))
#define le32toh(x)      bswap32((x))
#define le64toh(x)      bswap64((x))
#endif /* BYTE_ORDER == LITTLE_ENDIAN */
#endif /* __FreeBSD__ */

//extern uint16_t _bswap16(uint16_t a_int);
//extern uint32_t _bswap32(uint32_t a_int);
//extern uint64_t _bswap64(uint64_t a_int);

#endif /* __ENDIAN_H */
