/*
  Copyright (C) 2017 Carlos Montiers Aguilera

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Carlos Montiers Aguilera
  cmontiers@gmail.com
 */

#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEY_SIZE 32

struct AssocArrayElement {
    WCHAR key[MAX_KEY_SIZE + 1];
    void *obj;
    struct AssocArrayElement *next;
};

struct AssocArray {
    struct AssocArrayElement *head;
    struct AssocArrayElement *tail;
};

void assocarray_init(struct AssocArray *assocarray);

void *assocarray_get(struct AssocArray *assocarray, WCHAR * key);

int assocarray_add(struct AssocArray *assocarray, WCHAR * key, void *obj);

/*void assocarray_clear(struct AssocArray *assocarray);
 */
