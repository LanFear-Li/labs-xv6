// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
// defined by kernel.ld.

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    struct run *freelist;
} kmem;

// page reference count for cow page
uint64 refer_count[(PHYSTOP - KERNBASE) / PGSIZE];

void
kinit() {
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *) PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end) {
    char *p;
    p = (char *) PGROUNDUP((uint64) pa_start);
    for (; p + PGSIZE <= (char *) pa_end; p += PGSIZE)
        kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa) {
    struct run *r;

    if (((uint64) pa % PGSIZE) != 0 || (char *) pa < end || (uint64) pa >= PHYSTOP)
        panic("kfree");

    // decrease cow page count, check necessity
    refer_operator((uint64) pa, 2);
    if (refer_operator((uint64) pa, 4) > 0) {
        return;
    }

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run *) pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void) {
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r)
        kmem.freelist = r->next;
    release(&kmem.lock);

    if (r) {
        memset((char *) r, 5, PGSIZE); // fill with junk
        refer_operator((uint64) r, 3);
    }

    return (void *) r;
}

int refer_operator(uint64 pa, int type) {
    if (pa >= (uint64) end && pa <= PHYSTOP) {
        int index = (pa - (uint64) end) / PGSIZE;
        switch (type) {
            case 1:
                refer_count[index]++;
                break;
            case 2:
                if (refer_count[index] > 0) {
                    refer_count[index]--;
                }
                break;
            case 3:
                refer_count[index] = 1;
                break;
            case 4:
                return refer_count[index];
            default:
                return -1;
        }
    }
    return 0;
}