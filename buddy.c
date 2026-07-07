#include "allocator.h"


// ── Buddy system ─────────────────────────────────────────────────────────────
// Separate from your first-fit allocator — uses its own memory pool
// so the two systems don't interfere with each other.



#define BUDDY_POOL_SIZE (1024 * 64)  // 64 KB pool, must be power of two
#define BUDDY_MIN_SIZE  16           // smallest block size (must fit a Block header)

static char   buddy_pool[BUDDY_POOL_SIZE];  // fixed memory pool
static int    buddy_initialized = 0;

typedef struct BuddyBlock {
    size_t size;
    int    is_free;
    struct BuddyBlock *next;  // free list for this size class
} BuddyBlock;

// Free lists — one per power-of-two size
// sizes: 16, 32, 64, 128, ... up to BUDDY_POOL_SIZE
// index 0 = size 16, index 1 = size 32, etc.
#define BUDDY_LEVELS 12  // 16 * 2^12 = 65536 = 64KB
static BuddyBlock *free_lists[BUDDY_LEVELS];

// ── helpers ──────────────────────────────────────────────────────────────────

// which index in free_lists does this size belong to?
static int size_to_level(size_t size) {
    int level = 0;
    size_t s = BUDDY_MIN_SIZE;
    while (s < size) {
        s <<= 1;
        level++;
    }
    return level;
}

// round size up to next power of two >= BUDDY_MIN_SIZE
static size_t buddy_round_up(size_t size) {
    if (size < BUDDY_MIN_SIZE) return BUDDY_MIN_SIZE;
    size_t s = BUDDY_MIN_SIZE;
    while (s < size) s <<= 1;
    return s;
}

// find the buddy of a block
static BuddyBlock *get_buddy(BuddyBlock *block) {
    // offset of this block from the start of the pool
    size_t offset = (char*)block - buddy_pool;
    // XOR with block size flips the bit that separates us from our buddy
    size_t buddy_offset = offset ^ block->size;
    if (buddy_offset >= BUDDY_POOL_SIZE) return NULL;
    return (BuddyBlock*)(buddy_pool + buddy_offset);
}

// add a block to the free list for its size
static void free_list_add(BuddyBlock *block) {
    int level = size_to_level(block->size);
    block->next = free_lists[level];
    free_lists[level] = block;
    block->is_free = 1;
}

// remove a specific block from its free list
static void free_list_remove(BuddyBlock *block) {
    int level = size_to_level(block->size);
    BuddyBlock **cur = &free_lists[level];
    while (*cur) {
        if (*cur == block) {
            *cur = block->next;
            block->next = NULL;
            return;
        }
        cur = &(*cur)->next;
    }
}

// ── init ──────────────────────────────────────────────────────────────────────

static void buddy_init() {
    // start with one giant free block covering the whole pool
    BuddyBlock *initial = (BuddyBlock*)buddy_pool;
    initial->size    = BUDDY_POOL_SIZE;
    initial->is_free = 1;
    initial->next    = NULL;

    // put it in the top-level free list
    int top_level = size_to_level(BUDDY_POOL_SIZE);
    free_lists[top_level] = initial;

    buddy_initialized = 1;
}

// ── buddy_malloc ──────────────────────────────────────────────────────────────

void *buddy_malloc(size_t size) {
    if (!buddy_initialized) buddy_init();
    if (size == 0) return NULL;

    // how big a block do we actually need?
    // size + header, rounded up to power of two
    size_t needed = buddy_round_up(size + sizeof(BuddyBlock));
    int level = size_to_level(needed);

    // find the smallest free list at or above our level that has a block
    int found_level = -1;
    for (int i = level; i < BUDDY_LEVELS; i++) {
        if (free_lists[i] != NULL) {
            found_level = i;
            break;
        }
    }
    if (found_level == -1) return NULL;  // out of memory

    // take a block from that free list
    BuddyBlock *block = free_lists[found_level];
    free_lists[found_level] = block->next;
    block->next    = NULL;
    block->is_free = 0;

    // split the block in half repeatedly until it's the right size
    while (found_level > level) {
        // split: right half becomes a free buddy
        found_level--;
        size_t half = block->size / 2;

        // right buddy sits at block + half
        BuddyBlock *buddy = (BuddyBlock*)((char*)block + half);
        buddy->size    = half;
        buddy->is_free = 1;
        buddy->next    = NULL;
        free_list_add(buddy);

        // left half is now our working block
        block->size = half;
    }

    // return pointer past the header
    return (void*)(block + 1);
}

// ── buddy_free ────────────────────────────────────────────────────────────────

void buddy_free(void *ptr) {
    if (!ptr) return;

    BuddyBlock *block = (BuddyBlock*)ptr - 1;
    block->is_free = 1;

    // keep merging with free buddies as long as possible
    while (1) {
        BuddyBlock *buddy = get_buddy(block);

        // stop if: no buddy, buddy is in use, or buddy is a different size
        // (different size means the buddy was split further and isn't whole)
        if (!buddy || !buddy->is_free || buddy->size != block->size)
            break;

        // remove buddy from its free list — we're merging
        free_list_remove(buddy);

        // the merged block starts at whichever address is lower
        if (buddy < block) block = buddy;
        block->size *= 2;  // merged block is twice the size
    }

    // add the (possibly merged) block back to the free list
    free_list_add(block);
}