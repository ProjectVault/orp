/** @file util.c

    Contains various helpers that do not otherwise fit directly inside a module

*/
/*
   Copyright 2015, Google Inc.

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

#include "util.h"

inline int ilog2(uint32_t op)
{
    int pos,last;
    
    /* find highest set bit (shifts in a loop should be faster than looking up value in a table) */
    for(pos=0,last=0;pos<sizeof(op)*8; pos++)
    {
	if(op & (1<<pos))
	    last = pos;
    }
    /* round up */
    return (op != (1<<last)) ? -1 : last;
}
