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
unsigned int __umodsi3(unsigned int a, unsigned int b)
{
    register unsigned int ret;
    register unsigned int a1 = a;
    register unsigned int b1 = b;

    __asm __volatile(
        "l.addi %0, %1, 0   \n"
        "l.divu %1,%1,%2    \n"
        "l.mulu %1,%1,%2    \n"
        "l.sub  %0, %0, %1  \n"
        :"=r"(ret)
        :"r"(a1), "r"(b1)
    );
    
    return ret;
}

int __modsi3(int a, int b)
{
    register int ret;
    register int a1 = a;
    register int b1 = b;

    __asm __volatile(
        "l.addi %0, %1, 0   \n"
        "l.div  %1,%1,%2    \n"
        "l.mul  %1,%1,%2    \n"
        "l.sub  %0, %0, %1  \n"
        :"=r"(ret)
        :"r"(a1), "r"(b1)
    );
    
    return ret;
}

int __divsi3(int dividend, int divisor)
{
    register int quotient;
    register int a = dividend;
    register int b = divisor;

    __asm __volatile(
        "l.div %0, %1, %2 \n"
        :"=r"(quotient)
        :"r"(a), "r"(b)
    );

    return quotient;
}


/*
long __muldi3(long a, long b)
{
  return 0; // TODO
}

unsigned int __udivsi3(unsigned int a, unsigned int b)
{
  return 0; // TODO
}

unsigned long __udivdi3 (unsigned long a, unsigned long b)
{
  return 0; // TODO
}
*/
