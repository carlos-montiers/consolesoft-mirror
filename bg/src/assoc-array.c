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

#include "assoc-array.h"

void assocarray_init(struct AssocArray *assocarray) {
    assocarray->head = NULL;
    assocarray->tail = NULL;
}

void *assocarray_get(struct AssocArray *assocarray, WCHAR * key) {
    struct AssocArrayElement *el;
    el = assocarray->head;
    while (el) {
        if (!wcsncmp(el->key, key, MAX_KEY_SIZE)) {
            return el->obj;
        }
        el = el->next;
    }
    return NULL;
}

int assocarray_add(struct AssocArray *assocarray, WCHAR * key, void *obj) {
    struct AssocArrayElement *el;
    el = assocarray->head;

    while (el) {
        if (!wcsncmp(el->key, key, MAX_KEY_SIZE)) {
            el->obj = obj;
            return -1; //replaced obj in key
        }
        el = el->next;
    }
    //add new
    el = (struct AssocArrayElement *)
            malloc(sizeof (struct AssocArrayElement));
    if (!el) {
        return 0;
    }
    el->obj = obj;
    wcsncpy(el->key, key, MAX_KEY_SIZE);
    el->key[MAX_KEY_SIZE] = L'\0';
    if (!assocarray->head) {
        assocarray->head = el;
    } else {
        assocarray->tail->next = el;
    }
    (assocarray->tail = el)->next = NULL;
    return 1;
}

/*
void assocarray_clear(struct AssocArray *assocarray)
{
    struct AssocArrayElement *el;
    struct AssocArrayElement *p;

    el = assocarray->head;
    while (el) {
        p = el;
        el = el->next;
        free(p);
    }
    assocarray->head = NULL;
    assocarray->tail = NULL;
}
 */
