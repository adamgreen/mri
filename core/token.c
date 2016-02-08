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
/* 'Class' used to parse and tokenize a string based on provided list of separators. */
#include <string.h>
#include "token.h"

#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))


void Token_Init(Token* pToken)
{
    Token_InitWith(pToken, " \t");
}


static void clearTokenObject(Token* pToken);
void Token_InitWith(Token* pToken, const char* pTheseTokenSeparators)
{
    clearTokenObject(pToken);
    pToken->pTokenSeparators = pTheseTokenSeparators;
}

static void clearTokenObject(Token* pToken)
{
    memset(pToken->tokenPointers, 0, sizeof(pToken->tokenPointers));
    pToken->tokenCount = 0;
    pToken->copyOfString[0] = '\0';
}


static void copyStringIntoToken(Token* pToken, const char* pStringToCopy);
static void splitStringCopyIntoTokens(Token* pToken);
static char* findFirstNonSeparator(Token* pToken, char* p);
static char* findFirstSeparator(Token* pToken, char* p);
static int charIsSeparator(Token* pToken, char c);
static void addToken(Token* pToken, const char* p);
void Token_SplitString(Token* pToken, const char* pStringToSplit)
{
    __try
    {
        clearTokenObject(pToken);
        __throwing_func( copyStringIntoToken(pToken, pStringToSplit) );
        __throwing_func( splitStringCopyIntoTokens(pToken) );
    }
    __catch
    {
        __rethrow;
    }
}

static void copyStringIntoToken(Token* pToken, const char* pStringToCopy)
{
    size_t      bytesLeft = sizeof(pToken->copyOfString);
    const char* pSource = pStringToCopy;
    char*       pDest = pToken->copyOfString;
    
    while (bytesLeft > 1 && *pSource)
    {
        *pDest++ = *pSource++;
        bytesLeft--;
    }
    *pDest = '\0';
    
    if (*pSource)
        __throw(bufferOverrunException);
}

static void splitStringCopyIntoTokens(Token* pToken)
{
    char* p = pToken->copyOfString;
    while (*p)
    {
        p = findFirstNonSeparator(pToken, p);
        __try
            addToken(pToken, p);
        __catch
            __rethrow;
        p = findFirstSeparator(pToken, p);
        if (*p)
            *p++ = '\0';
    }
}

static char* findFirstNonSeparator(Token* pToken, char* p)
{
    while (*p && charIsSeparator(pToken, *p))
        p++;
    
    return p;
}

static char* findFirstSeparator(Token* pToken, char* p)
{
    while (*p && !charIsSeparator(pToken, *p))
        p++;
    
    return p;
}

static int charIsSeparator(Token* pToken, char c)
{
    const char* pSeparator = pToken->pTokenSeparators;
    
    while (*pSeparator)
    {
        if (c == *pSeparator)
            return 1;
        pSeparator++;
    }
    
    return 0;
}

static void addToken(Token* pToken, const char* p)
{
    if ('\0' == *p)
        return;
        
    if (pToken->tokenCount >= ARRAY_SIZE(pToken->tokenPointers))
        __throw(bufferOverrunException);
        
    pToken->tokenPointers[pToken->tokenCount++] = p;
}


size_t Token_GetTokenCount(Token* pToken)
{
    return pToken->tokenCount;
}


const char* Token_GetToken(Token* pToken, size_t tokenIndex)
{
    if (tokenIndex >= pToken->tokenCount)
        __throw_and_return(invalidIndexException, NULL);

    return pToken->tokenPointers[tokenIndex];
}


const char* Token_MatchingString(Token* pToken, const char* pTokenToSearchFor)
{
    size_t i;
    
    for (i = 0 ; i < pToken->tokenCount ; i++)
    {
        if (0 == strcmp(pToken->tokenPointers[i], pTokenToSearchFor))
            return pToken->tokenPointers[i];
    }
    return NULL;
}


const char* Token_MatchingStringPrefix(Token* pToken, const char* pTokenPrefixToSearchFor)
{
    size_t i;
    
    for (i = 0 ; i < pToken->tokenCount ; i++)
    {
        if (pToken->tokenPointers[i] == strstr(pToken->tokenPointers[i], pTokenPrefixToSearchFor))
            return pToken->tokenPointers[i];
    }
    return NULL;
}


static void adjustTokenPointers(Token* pToken, const char* pOriginalStringCopyBaseAddress);
void Token_Copy(Token* pTokenCopy, Token* pTokenOriginal)
{
    *pTokenCopy = *pTokenOriginal;
    
    adjustTokenPointers(pTokenCopy, pTokenOriginal->copyOfString);
}

static void adjustTokenPointers(Token* pToken, const char* pOriginalStringCopyBaseAddress)
{
    size_t i;
    
    for (i = 0 ; i < pToken->tokenCount ; i++)
    {
        int tokenOffset = pToken->tokenPointers[i] - pOriginalStringCopyBaseAddress;
        
        pToken->tokenPointers[i] = pToken->copyOfString + tokenOffset;
    }
}
