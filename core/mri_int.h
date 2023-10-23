/* Copyright 2023 Adam Green (https://github.com/adamgreen/)

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
/* Integer types used by MRI for debuggee registers, pointer addresses, etc. */
#ifndef MRI_INT_H_
#define MRI_INT_H_

#include <stdint.h>

/* The MRI_UINT_TYPE and MRI_INT_TYP macros should be set when calling the compiler to indicate what type should be
   used for integers on your platform. This allows support for 16/32/64-bit integers. */
#ifndef MRI_UINT_TYPE
    #define MRI_UINT_TYPE uintptr_t
#endif
#ifndef MRI_INT_TYPE
    #define MRI_INT_TYPE intptr_t
#endif
typedef MRI_UINT_TYPE uintmri_t;
typedef MRI_INT_TYPE intmri_t;


#endif /* MRI_INT_H_ */
