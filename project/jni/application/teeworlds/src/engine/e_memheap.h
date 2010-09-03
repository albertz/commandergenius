/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HEAP_t HEAP;
HEAP *memheap_create();
void memheap_destroy(HEAP *heap);
void *memheap_allocate(HEAP *heap, unsigned int size);

#ifdef __cplusplus
}
#endif
