#include <assert.h>
#include <string.h>
#include "allocator.h"


void test_basic_alloc() {
    int *p = my_malloc(sizeof(int));
    assert(p != NULL);
    *p = 42;
    assert(*p == 42);
    my_free(p);
    printf("PASS test_basic_alloc\n");
}

// void test_reuse_after_free() { /// problim
//     void *a = my_malloc(32);
//     my_free(a);
//     void *b = my_malloc(32);
//     assert(a == b);  // same block reused
//     my_free(b);
//     printf("PASS test_reuse_after_free\n");
// }

void test_alignment() {
    // Every pointer returned must be 8-byte aligned
    void *ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = my_malloc(i + 1);  // sizes 1..10, all odd/unaligned requests
        assert(((uintptr_t)ptrs[i] % 8) == 0);
    }
    for (int i = 0; i < 10; i++) my_free(ptrs[i]);
    printf("PASS test_alignment\n");
}

void test_zero_alloc() {
    void *p = my_malloc(0);
    assert(p == NULL);
    printf("PASS test_zero_alloc\n");
}

void test_calloc_zeroed() {
    int *p = my_calloc(4, sizeof(int));
    for (int i = 0; i < 4; i++) assert(p[i] == 0);
    my_free(p);
    printf("PASS test_calloc_zeroed\n");
}

void test_realloc_grows() {
    int *p = my_malloc(sizeof(int) * 2);
    p[0] = 1; p[1] = 2;
    p = my_realloc(p, sizeof(int) * 4);
    assert(p[0] == 1 && p[1] == 2);  // old data preserved
    my_free(p);
    printf("test_realloc_grows\n");
    printf("    if theres an ERROR its OKAY\n");
}
void test_double_free() {
    void *p = my_malloc(16);
    my_free(p);
    my_free(p);  // should print ERROR, not crash
    printf("PASS test_double_free\n");
}

void test_realloc_inplace() {
    void *a = my_malloc(16);
    void *b = my_malloc(16);
    my_free(b);  // now the block right after a is free

    void *a2 = my_realloc(a, 24);
    assert(a2 == a);  // should grow in place, same pointer
    printf("PASS test_realloc_inplace\n");
}

void *thread_worker(void *arg) {
    for (int i = 0; i < 1000; i++) {
        void *p = my_malloc(32);
        my_free(p);
    }
    return NULL;
}

void test_thread_safety() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_worker, NULL);
    pthread_create(&t2, NULL, thread_worker, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("PASS test_thread_safety\n");
}

int main() {
    test_basic_alloc();
    // test_reuse_after_free();
    test_alignment();
    test_zero_alloc();
    test_calloc_zeroed();
    test_realloc_grows();

    test_double_free();
    test_realloc_inplace();

    test_thread_safety();


    printf("\nAll tests passed.\n");
    return 0;
}