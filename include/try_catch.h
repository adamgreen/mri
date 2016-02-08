/* Copyright 2014 Adam Green (http://mbed.org/users/AdamGreen/)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.   
*/
/* Very rough exception handling like macros for C. */
#ifndef _MRI_TRY_CATCH_H_
#define _MRI_TRY_CATCH_H_

#define noException                         0
#define bufferOverrunException              1
#define invalidHexDigitException            2
#define invalidValueException               3
#define invalidArgumentException            4
#define timeoutException                    5
#define invalidIndexException               6
#define notFoundException                   7
#define exceededHardwareResourcesException  8
#define invalidDecDigitException            9
#define memFaultException                   10
#define mriMaxException                     15

extern int __mriExceptionCode;


/* Allow an application including MRI to extend with their own exception codes and replace the below declarations. */
#ifndef MRI_SKIP_TRY_CATCH_MACRO_DEFINES

/* On Linux, it is possible that __try and __catch are already defined. */
#undef __try
#undef __catch

#define __throws

#define __try \
        do \
        { \
            clearExceptionCode();

#define __throwing_func(X) \
            X; \
            if (__mriExceptionCode) \
                break;

#define __catch \
        } while (0); \
        if (__mriExceptionCode)

#define __throw(EXCEPTION) return ((void)setExceptionCode(EXCEPTION))

#define __throw_and_return(EXCEPTION, RETURN) return (setExceptionCode(EXCEPTION), (RETURN))
        
#define __rethrow return

#define __rethrow_and_return(RETURN) return RETURN

static inline int getExceptionCode(void)
{
    return __mriExceptionCode;
}

static inline void setExceptionCode(int exceptionCode)
{
    __mriExceptionCode = exceptionCode > __mriExceptionCode ? exceptionCode : __mriExceptionCode;
}

static inline void clearExceptionCode(void)
{
    __mriExceptionCode = noException;
}

#endif /* MRI_SKIP_TRY_CATCH_MACRO_DEFINES */
#endif /* _MRI_TRY_CATCH_H_ */
