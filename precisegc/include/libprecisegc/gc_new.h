/*************************************************************************************//**
		* File: gc_new.h
		* Description: This file consists memory allocation primitive gc_new and
			object meta classes realisations.
*****************************************************************************************/

#pragma once

#include <cstdio>
#include <pthread.h>
#include <vector>
#include <assert.h>

#include "gc_ptr.h"
#include "debug_print.h"
#include "thread.h"
#include "tlvars.h"
#include "malloc.h"
#include "details/class_meta.h"
#include "details/gc_pause.h"

/**
* @function gc_new
* @brief library memory allocation primitive
* @detailed allocates memory in program heap for managed object
* @return smart pointer gc_ptr (points to managed memory)
* @param types arguments for non-default constructor call
* @param count is an array size (default = 1 --- single object)
* @param T is an allocating object type
* @param Types --- arguments types of non-default constructor arguments
*/
template <class T, typename ... Types>
gc_ptr<T> gc_new (Types ... types, size_t count = 1) {
    using namespace precisegc::details;

	gc_pause_lock pause_lock;
	lock_guard<gc_pause_lock> pause_guard(pause_lock);

	assert(count >= 0);
    typedef precisegc::details::class_meta_provider<T> class_meta_provider;
    pthread_mutex_lock(&precisegc::gc_mutex);
    tlvars * new_obj_flags = precisegc::get_thread_handler()->tlflags;
    pthread_mutex_unlock(&precisegc::gc_mutex);
	if (new_obj_flags->nesting_level == 0) {
//		safepoint();
	}
	dprintf("in gc_new: \n");
	// get pointer to class meta or NULL if it is no meta for this class
//	size_t * clMeta = get_meta<T>();
//	dprintf("\tclMeta=%p\n", clMeta);

	/* set global active flags */
	bool old_new_active = new_obj_flags->new_active;
	new_obj_flags->new_active = true;  /* set flag that object creates(allocates) in heap */
	bool old_no_active = new_obj_flags->no_active;
	new_obj_flags->no_active = class_meta_provider::is_created();
	/* result */
	void * res = NULL;
	new_obj_flags->nesting_level++;

	if (class_meta_provider::is_created()) {
		dprintf("\tclMeta != NULL\n");
		res = gcmalloc(sizeof(T), class_meta_provider::get_meta_ptr(), count);
		if (count == 1) {
			dprintf("\t\tcount == 1\n");
			new (res) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
		} else {
			dprintf("\t\tcount != 1\n");
			new (res) T[count];
		}
	} else {
		/* in this case we might to create clMeta (class meta information) and save in in ClassMetaList */
		dprintf("\tclMeta == NULL\n");
		res = gcmalloc(sizeof(T), NULL, count);
		/* save old offsets */
		std::vector<size_t> temp;
		temp.swap(new_obj_flags->offsets);
		/* save current pointer to the allocating object; it uses in gc_ptr constructor to evaluate offsets size */
		size_t old_current_pointer_to_object = new_obj_flags->current_pointer_to_object;
		new_obj_flags->current_pointer_to_object = (size_t)res;

		if (count == 1) {
			dprintf("\t\tcount == 1\n");
			new (res) T(types ... );  /* create object in allocated space, call gc_ptr constructor, get new struct offsets */
		} else {
			dprintf("\t\tcount != 1\n");
			// calculate data for clMeta and allocates one object from array
			new (res) T();
			new_obj_flags->no_active = true;
			// allocate all other objects
			new ((char *)res + sizeof(T)) T[count - 1];
		}
		// calculate meta
        class_meta_provider::create_meta(new_obj_flags->offsets);
		set_meta_after_gcmalloc(res, class_meta_provider::get_meta_ptr());
//		assert(set_meta == 0);

		/* restore old global variable values */
		new_obj_flags->offsets.swap(temp);
		new_obj_flags->current_pointer_to_object = old_current_pointer_to_object;
	}
	dprintf("\tgc_new: will restore global values\n");
	/* restore old global variable values */
	new_obj_flags->new_active = old_new_active;
	new_obj_flags->nesting_level--;
	new_obj_flags->no_active = old_no_active;

	dprintf("\tgc_new : before remove\n");
	/* return ptr on allocated space */
	assert(res != NULL);
	return gc_ptr<T>((T*)(res));
}