/* Copyright 2012 Adam Green (http://mbed.org/users/AdamGreen/)

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

extern "C"
{
#include "try_catch.h"
}

// Include C++ headers for test harness.
#include "CppUTest/TestHarness.h"

TEST_GROUP(TryCatch)
{
    int m_exceptionThrown;
    
    void setup()
    {
        m_exceptionThrown = 0;
    }

    void teardown()
    {
    }
    
    void flagExceptionHit()
    {
        m_exceptionThrown = 1;
    }
    
    void throwNoException()
    {
    }
    
    void throwBufferAndArgumentExceptions()
    {
        throwBufferOverrunException();
        throwInvalidArgumentException();
    }
    
    void throwArgumentAndBufferExceptions()
    {
        throwInvalidArgumentException();
        throwBufferOverrunException();
    }
    
    void throwBufferOverrunException()
    {
        __throw(bufferOverrunException);
    }
    
    void throwInvalidArgumentException()
    {
        __throw(invalidArgumentException);
    }
    
    int throwBufferOverrunExceptionAndReturnNegative1()
    {
        __throw_and_return(bufferOverrunException, -1);
    }
    
    void rethrowBufferOverrunException()
    {
        __try
            throwBufferOverrunException();
        __catch
            __rethrow;
        __throw(invalidArgumentException);
    }
    
    int rethrowBufferOverrunExceptionAndReturnNegative1()
    {
        __try
            throwBufferOverrunException();
        __catch
            __rethrow_and_return(-1);
        __throw_and_return(invalidArgumentException, 0);
    }

    void validateException(int expectedExceptionCode)
    {
        if (expectedExceptionCode == noException)
        {
            CHECK_FALSE(m_exceptionThrown);
        }
        else
        {
            CHECK_TRUE(m_exceptionThrown);
        }
        LONGS_EQUAL(expectedExceptionCode, getExceptionCode());
    }
};

TEST(TryCatch, NoException)
{
    __try
        throwNoException();
    __catch
        flagExceptionHit();
    validateException(noException);
}

TEST(TryCatch, bufferOverrunException)
{
    __try
        throwBufferOverrunException();
    __catch
        flagExceptionHit();
    validateException(bufferOverrunException);
}

TEST(TryCatch, EscalatingExceptions)
{
    __try
        throwBufferAndArgumentExceptions();
    __catch
        flagExceptionHit();

    validateException(invalidArgumentException);
}

TEST(TryCatch, NonEscalatingExceptions)
{
    __try
        throwArgumentAndBufferExceptions();
    __catch
        flagExceptionHit();

    validateException(invalidArgumentException);
}

TEST(TryCatch, CatchFirstThrow)
{
    __try
    {
        __throwing_func( throwBufferOverrunException() );
        __throwing_func( throwInvalidArgumentException() );
    }
    __catch
    {
        flagExceptionHit();
    }
    
    validateException(bufferOverrunException);
}

TEST(TryCatch, CatchSecondThrow)
{
    __try
    {
        __throwing_func( throwNoException() );
        __throwing_func( throwInvalidArgumentException() );
    }
    __catch
    {
        flagExceptionHit();
    }
    
    validateException(invalidArgumentException);
}

TEST(TryCatch, ThrowAndReturn)
{
    int value;
    
    __try
        value = throwBufferOverrunExceptionAndReturnNegative1();
    __catch
        flagExceptionHit();
        
    LONGS_EQUAL( -1, value );
    validateException(bufferOverrunException);
}

TEST(TryCatch, RethrowBufferOverrunException)
{
    __try
        rethrowBufferOverrunException();
    __catch
        flagExceptionHit();
    validateException(bufferOverrunException);
}

TEST(TryCatch, RethrowBufferOverrunExceptionAndReturnNegative1)
{
    int value;
    
    __try
        value = rethrowBufferOverrunExceptionAndReturnNegative1();
    __catch
        flagExceptionHit();

    LONGS_EQUAL( -1, value );
    validateException(bufferOverrunException);
}
