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
/* 'Class' used to parse and tokenize a string based on provided list of separators. */
#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <stddef.h>
#include "try_catch.h"

/* Maximum number of tokens that a string can be separated into. */
#define TOKEN_MAX_TOKENS 10

/* Maximum size of string that can be split into tokens. */
#define TOKEN_MAX_STRING 64

typedef struct
{
    const char* tokenPointers[TOKEN_MAX_TOKENS];
    const char* pTokenSeparators;
    size_t      tokenCount;
    char        copyOfString[TOKEN_MAX_STRING + 1];
} Token;


/* Real name of functions are in __mri namespace. */
         void        __mriToken_Init(Token* pToken);
         void        __mriToken_InitWith(Token* pToken, const char* pTheseTokenSeparators);
__throws void        __mriToken_SplitString(Token* pToken, const char* pStringToSplit);
         size_t      __mriToken_GetTokenCount(Token* pToken);
__throws const char* __mriToken_GetToken(Token* pToken, size_t tokenIndex);
         const char* __mriToken_MatchingString(Token* pToken, const char* pTokenToSearchFor);
         const char* __mriToken_MatchingStringPrefix(Token* pToken, const char* pTokenPrefixToSearchFor);
         void        __mriToken_Copy(Token* pTokenCopy, Token* pTokenOriginal);

/* Macroes which allow code to drop the __mri namespace prefix. */
#define Token_Init                  __mriToken_Init
#define Token_InitWith              __mriToken_InitWith
#define Token_SplitString           __mriToken_SplitString
#define Token_GetTokenCount         __mriToken_GetTokenCount
#define Token_GetToken              __mriToken_GetToken
#define Token_MatchingString        __mriToken_MatchingString
#define Token_MatchingStringPrefix  __mriToken_MatchingStringPrefix
#define Token_Copy                  __mriToken_Copy

#endif /* _TOKEN_H_ */
