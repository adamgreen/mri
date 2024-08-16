/* Copyright 2024 Adam Green (https://github.com/adamgreen/)

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
/* Breakpoint constants shared between assembly language stubs and MRI's semihost hooks. */

#ifndef MRI_SEMIHOST_ARM_H_
#define MRI_SEMIHOST_ARM_H_

#define MRI_ARM_SEMIHOST_BKPT_NO        0xab

#define MRI_ARM_SEMIHOST_OPEN           1
#define MRI_ARM_SEMIHOST_CLOSE          2
#define MRI_ARM_SEMIHOST_WRITE          5
#define MRI_ARM_SEMIHOST_READ           6
#define MRI_ARM_SEMIHOST_IS_TTY         9
#define MRI_ARM_SEMIHOST_SEEK           10
#define MRI_ARM_SEMIHOST_FILE_LENGTH    12
#define MRI_ARM_SEMIHOST_REMOVE         14
#define MRI_ARM_SEMIHOST_RENAME         15
#define MRI_ARM_SEMIHOST_ERR_NO         19
#define MRI_ARM_SEMIHOST_UUID           257

#endif /* MRI_SEMIHOST_ARM_H_ */
