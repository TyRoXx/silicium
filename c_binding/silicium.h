#ifndef SILICIUM_C_BINDING_H
#define SILICIUM_C_BINDING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct silicium_memory_range
{
	char const *begin;
	char const *end;
} silicium_memory_range;

typedef struct silicium_yield_context silicium_yield_context;

typedef struct silicium_observable silicium_observable;

typedef void *(*silicium_coroutine_function)(void *user_data, silicium_yield_context *yield);
silicium_observable *silicium_make_coroutine(silicium_coroutine_function action, void *user_data);

typedef void (*silicium_observer_function)(void *user_data, void *element);
void silicium_async_get_one(silicium_observable *observable, silicium_observer_function callback, void *user_data);

void silicium_free_observable(silicium_observable *observable);

void *silicium_yield_get_one(silicium_yield_context *yield, silicium_observable *from);

#ifdef __cplusplus
}
#endif

#endif
