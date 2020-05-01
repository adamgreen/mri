/* Copyright 2020 Adam Green (https://github.com/adamgreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <core/platforms.h>


__attribute__((weak)) uint32_t Platform_RtosGetHaltedThreadId(void)
{
    return 0;
}

__attribute__((weak)) uint32_t Platform_RtosGetFirstThreadId(void)
{
    return 0;
}

__attribute__((weak)) uint32_t Platform_RtosGetNextThreadId(void)
{
    return 0;
}

__attribute__((weak)) const char* Platform_RtosGetExtraThreadInfo(uint32_t threadId)
{
    return NULL;
}

__attribute__((weak)) MriContext* Platform_RtosGetThreadContext(uint32_t threadId)
{
    return NULL;
}

__attribute__((weak)) int Platform_RtosIsThreadActive(uint32_t threadId)
{
    return 0;
}
