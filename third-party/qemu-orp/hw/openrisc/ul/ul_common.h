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
void ul_gcd(ul dst, const ul x, const ul y);

/* these routines assume a modular inverse of the dividend exist */
void ul_moddiv2(ul dst, const ul src, const mod n);

/* TODO: implement proper divn */
void ul_moddiv3(ul dst, const ul src, const mod n);

void ul_moddiv5(ul dst, const ul src, const mod n);

void ul_moddiv7(ul dst, const ul src, const mod n);

void ul_moddiv11(ul dst, const ul src, const mod n);

void ul_moddiv13(ul dst, const ul src, const mod n);

int ul_modinv(ul dst, const ul src, const ul n);
