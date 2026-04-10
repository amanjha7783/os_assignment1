/*
 * ================================================================
 *   COMPLETE MEMORY MANAGEMENT IN C
 *   Sections:
 *     1. First Fit / Best Fit / Worst Fit / Next Fit
 *     2. Paging  (virtual -> physical address translation)
 *     3. Page Replacement  (FIFO, LRU, Optimal/Belady)
 *     4. Frame Allocation  (equal and proportional)
 *     5. Segmentation      (segment table, bounds checking)
 *     6. TLB simulation    (Translation Lookaside Buffer)
 *     7. Working Set Model (thrashing analysis)
 *
 *   Compile:  gcc -Wall -o memory_mgmt memory_mgmt_final.c -lm
 *   Run:      ./memory_mgmt
 * ================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ================================================================
   SECTION 1 : MEMORY ALLOCATION STRATEGIES
   (First Fit, Best Fit, Worst Fit, Next Fit)
   ================================================================ */

#define MAX_BLOCKS 10
#define MAX_PROCS  10

typedef struct {
    int id;
    int size;
    int allocated_to;   /* process id occupying this block; -1 = free */
} Block;

typedef struct {
    int id;
    int size;
    int block_id;       /* block id assigned; -1 = not allocated yet */
} Process;

/* ----------------------------------------------------------
   reset_state : restore all blocks to free, processes to
                 unallocated, ready for the next algorithm run
   ---------------------------------------------------------- */
static void reset_state(Block   *b,  int nb, const int *bsz,
                         Process *p,  int np, const int *psz)
{
    for (int i = 0; i < nb; i++)
        b[i] = (Block){ i + 1, bsz[i], -1 };
    for (int i = 0; i < np; i++)
        p[i] = (Process){ i + 1, psz[i], -1 };
}

/* ----------------------------------------------------------
   print_alloc : display block and process tables plus stats
   ---------------------------------------------------------- */
static void print_alloc(Block *b, int nb, Process *p, int np,
                         const char *label)
{
    int used = 0, waste = 0, free_kb = 0;

    printf("\n+-- %s ", label);
    for (int i = 0; i < 40 - (int)strlen(label); i++) putchar('-');
    puts("+");

    printf("| %-5s  %-8s  %-15s  %s\n",
           "Block", "Size(KB)", "Allocated To", "Waste(KB)");
    puts("|-----  --------  ---------------  ---------");

    for (int i = 0; i < nb; i++) {
        int w = 0;
        if (b[i].allocated_to != -1) {
            for (int j = 0; j < np; j++)
                if (p[j].id == b[i].allocated_to) {
                    w = b[i].size - p[j].size;
                    break;
                }
            used  += b[i].size - w;
            waste += w;
        } else {
            free_kb += b[i].size;
        }
        char proc_label[8];
        if (b[i].allocated_to == -1) strcpy(proc_label, "FREE");
        else                          snprintf(proc_label, sizeof(proc_label),
                                               "P%d", b[i].allocated_to);
        printf("| B%-4d  %-8d  %-15s  %d\n",
               b[i].id, b[i].size, proc_label, w);
    }

    printf("|\n| Used: %d KB | Fragmentation: %d KB | Free: %d KB\n",
           used, waste, free_kb);
    puts("|");

    printf("| %-9s  %-8s  %-14s  %s\n",
           "Process", "Need(KB)", "Block", "Status");
    puts("|  -------   --------  --------------  -------------");

    for (int i = 0; i < np; i++) {
        char blk_label[8];
        if (p[i].block_id == -1) strcpy(blk_label, "---");
        else                      snprintf(blk_label, sizeof(blk_label),
                                           "B%d", p[i].block_id);
        printf("| P%-8d  %-8d  %-14s  %s\n",
               p[i].id, p[i].size, blk_label,
               p[i].block_id == -1 ? "NOT ALLOCATED" : "OK");
    }
    puts("+--------------------------------------------------+");
}

/* ----------------------------------------------------------
   first_fit : allocate each process to the FIRST block
               that is large enough
   ---------------------------------------------------------- */
void first_fit(Block *b, int nb, Process *p, int np)
{
    for (int i = 0; i < np; i++)
        for (int j = 0; j < nb; j++)
            if (b[j].allocated_to == -1 && b[j].size >= p[i].size) {
                b[j].allocated_to = p[i].id;
                p[i].block_id     = b[j].id;
                break;                      /* stop at first fit */
            }
    print_alloc(b, nb, p, np, "FIRST FIT");
}

/* ----------------------------------------------------------
   best_fit : allocate each process to the SMALLEST block
              that is still large enough
   ---------------------------------------------------------- */
void best_fit(Block *b, int nb, Process *p, int np)
{
    for (int i = 0; i < np; i++) {
        int best = -1;
        for (int j = 0; j < nb; j++)
            if (b[j].allocated_to == -1 && b[j].size >= p[i].size)
                if (best == -1 || b[j].size < b[best].size)
                    best = j;
        if (best != -1) {
            b[best].allocated_to = p[i].id;
            p[i].block_id        = b[best].id;
        }
    }
    print_alloc(b, nb, p, np, "BEST FIT");
}

/* ----------------------------------------------------------
   worst_fit : allocate each process to the LARGEST available
               block (leaves biggest remainder for later)
   ---------------------------------------------------------- */
void worst_fit(Block *b, int nb, Process *p, int np)
{
    for (int i = 0; i < np; i++) {
        int worst = -1;
        for (int j = 0; j < nb; j++)
            if (b[j].allocated_to == -1 && b[j].size >= p[i].size)
                if (worst == -1 || b[j].size > b[worst].size)
                    worst = j;
        if (worst != -1) {
            b[worst].allocated_to = p[i].id;
            p[i].block_id         = b[worst].id;
        }
    }
    print_alloc(b, nb, p, np, "WORST FIT");
}

/* ----------------------------------------------------------
   next_fit : like first fit, but resumes scanning from
              where the last allocation was made
   ---------------------------------------------------------- */
void next_fit(Block *b, int nb, Process *p, int np)
{
    int last = 0;                /* remember last used position */
    for (int i = 0; i < np; i++) {
        int j = last;
        do {
            if (b[j].allocated_to == -1 && b[j].size >= p[i].size) {
                b[j].allocated_to = p[i].id;
                p[i].block_id     = b[j].id;
                last = j;
                break;
            }
            j = (j + 1) % nb;
        } while (j != last);
    }
    print_alloc(b, nb, p, np, "NEXT FIT");
}

void run_allocation(void)
{
    puts("\n+================================================+");
    puts("|   SECTION 1 : ALLOCATION STRATEGIES           |");
    puts("+================================================+");

    const int bsz[] = { 100, 500, 200, 300, 600 };  /* block sizes  (KB) */
    const int psz[] = { 212, 417, 112, 426 };        /* process sizes(KB) */
    int nb = 5, np = 4;

    printf("Blocks   : ");
    for (int i = 0; i < nb; i++) printf("%dKB ", bsz[i]);
    printf("\nProcesses: ");
    for (int i = 0; i < np; i++) printf("P%d=%dKB ", i+1, psz[i]);
    puts("\n");

    Block   blk[MAX_BLOCKS];
    Process prc[MAX_PROCS];

    reset_state(blk, nb, bsz, prc, np, psz);  first_fit (blk, nb, prc, np);
    reset_state(blk, nb, bsz, prc, np, psz);  best_fit  (blk, nb, prc, np);
    reset_state(blk, nb, bsz, prc, np, psz);  worst_fit (blk, nb, prc, np);
    reset_state(blk, nb, bsz, prc, np, psz);  next_fit  (blk, nb, prc, np);
}


/* ================================================================
   SECTION 2 : PAGING  (virtual address -> physical address)
   ================================================================ */

#define PAGE_SIZE   256   /* bytes per page / frame  */
#define NUM_PAGES    16   /* virtual address space   */
#define NUM_FRAMES    8   /* physical memory frames  */

typedef struct {
    int valid;            /* 1 = resident in memory; 0 = page fault */
    int frame;            /* which physical frame holds this page   */
} PageTableEntry;

static void build_page_table(PageTableEntry pt[], int n)
{
    /* frame = -1 means not loaded (will trigger page fault) */
    int frames[] = { 3, -1,  0,  5,  2, -1,  7,  1,
                    -1,  4,  6, -1, -1,  2,  4,  6 };
    for (int i = 0; i < n; i++) {
        pt[i].valid = (frames[i] != -1);
        pt[i].frame =  frames[i];
    }
}

/* ----------------------------------------------------------
   translate_address
     va         : 16-bit virtual address
     PAGE_SIZE  : must be a power of 2
     offset_bits: log2(PAGE_SIZE)   e.g. 8 for 256-byte pages
     page_num   : upper bits of va
     offset     : lower bits of va
     PA         : frame * PAGE_SIZE + offset
   ---------------------------------------------------------- */
static void translate_address(PageTableEntry pt[], unsigned int va)
{
    int offset_bits = (int)log2((double)PAGE_SIZE);
    int page_num    = (int)(va >> offset_bits);
    int offset      = (int)(va &  (PAGE_SIZE - 1));

    printf("  VA 0x%04X  -> page %-3d  offset %-4d  ->  ",
           va, page_num, offset);

    if (page_num >= NUM_PAGES || !pt[page_num].valid) {
        printf("PAGE FAULT (page %d not in memory)\n", page_num);
    } else {
        unsigned int pa = (unsigned int)(pt[page_num].frame * PAGE_SIZE + offset);
        printf("PA 0x%04X  (frame %d)\n", pa, pt[page_num].frame);
    }
}

void run_paging(void)
{
    puts("\n+================================================+");
    puts("|   SECTION 2 : PAGING                          |");
    puts("+================================================+");

    PageTableEntry pt[NUM_PAGES];
    build_page_table(pt, NUM_PAGES);

    int offset_bits = (int)log2((double)PAGE_SIZE);
    printf("\n  Page size   : %d bytes\n", PAGE_SIZE);
    printf("  Offset bits : %d  (lower bits of VA)\n", offset_bits);
    printf("  Page bits   : %d  (upper bits of VA)\n", (int)log2((double)NUM_PAGES));
    printf("  Pages       : %d virtual  |  %d physical frames\n\n",
           NUM_PAGES, NUM_FRAMES);

    puts("  Page Table:");
    puts("  Page   Frame   Valid   Comment");
    puts("  -----  ------  ------  ---------------------");
    for (int i = 0; i < NUM_PAGES; i++) {
        if (pt[i].valid)
            printf("  %-6d %-7d %-7d in RAM\n", i, pt[i].frame, pt[i].valid);
        else
            printf("  %-6d %-7s %-7d not loaded (will fault)\n", i, "---", 0);
    }

    puts("\n  Address Translations:");
    puts("  -------------------------------------------------------");

    unsigned int test_va[] = { 0x0000, 0x00FF, 0x0100, 0x02A5,
                                0x05F0, 0x0812, 0x0E00, 0x0FFF };
    for (int i = 0; i < 8; i++)
        translate_address(pt, test_va[i]);
}


/* ================================================================
   SECTION 3 : PAGE REPLACEMENT ALGORITHMS
   (FIFO, LRU, Optimal)
   ================================================================ */

#define FRAMES_REP  3    /* physical frames available for replacement demo */
#define REF_LEN    12    /* length of reference string                      */

static int frames_rep[FRAMES_REP];

static int in_frame(int page)
{
    for (int i = 0; i < FRAMES_REP; i++)
        if (frames_rep[i] == page) return i;
    return -1;
}

static void print_rep_header(void)
{
    printf("  %-4s  ", "Ref");
    for (int i = 0; i < FRAMES_REP; i++) printf(" F%d ", i+1);
    printf("  Result\n");
    printf("  ----  ");
    for (int i = 0; i < FRAMES_REP; i++) printf("----");
    puts("  --------");
}

static void print_rep_row(int ref, int is_fault)
{
    printf("  %-4d  ", ref);
    for (int i = 0; i < FRAMES_REP; i++)
        frames_rep[i] == -1 ? printf("  - ") : printf(" %2d ", frames_rep[i]);
    printf("  %s\n", is_fault ? "FAULT <--" : "");
}

/* ----------------------------------------------------------
   fifo : evict the page that has been in memory the longest
          (circular queue with a pointer)
   ---------------------------------------------------------- */
void fifo(int refs[], int n)
{
    puts("\n  -- FIFO (First In First Out) --------------------");
    memset(frames_rep, -1, sizeof(frames_rep));
    int ptr = 0, faults = 0;
    print_rep_header();
    for (int i = 0; i < n; i++) {
        int f = 0;
        if (in_frame(refs[i]) == -1) {
            frames_rep[ptr] = refs[i];      /* overwrite oldest slot */
            ptr = (ptr + 1) % FRAMES_REP;  /* advance circular ptr  */
            faults++;  f = 1;
        }
        print_rep_row(refs[i], f);
    }
    printf("  Total page faults (FIFO): %d\n", faults);
}

/* ----------------------------------------------------------
   lru : evict the page that has NOT been used for the
         longest time (track last-access tick per frame)
   ---------------------------------------------------------- */
void lru(int refs[], int n)
{
    puts("\n  -- LRU (Least Recently Used) --------------------");
    memset(frames_rep, -1, sizeof(frames_rep));
    int last_used[FRAMES_REP];
    memset(last_used, 0, sizeof(last_used));
    int faults = 0;
    print_rep_header();
    for (int i = 0; i < n; i++) {
        int idx = in_frame(refs[i]), f = 0;
        if (idx == -1) {
            /* find least recently used (or empty) slot */
            int rep = 0;
            for (int j = 0; j < FRAMES_REP; j++) {
                if (frames_rep[j] == -1) { rep = j; break; }
                if (last_used[j] < last_used[rep]) rep = j;
            }
            frames_rep[rep] = refs[i];
            last_used[rep]  = i + 1;        /* record access time */
            faults++;  f = 1;
        } else {
            last_used[idx] = i + 1;         /* update access time */
        }
        print_rep_row(refs[i], f);
    }
    printf("  Total page faults (LRU): %d\n", faults);
}

/* ----------------------------------------------------------
   optimal : evict the page that will not be needed for
             the longest time in the future (Belady's algo)
             theoretical lower bound -- not implementable
   ---------------------------------------------------------- */
void optimal(int refs[], int n)
{
    puts("\n  -- OPTIMAL (Belady's Algorithm) -----------------");
    memset(frames_rep, -1, sizeof(frames_rep));
    int faults = 0;
    print_rep_header();
    for (int i = 0; i < n; i++) {
        int f = 0;
        if (in_frame(refs[i]) == -1) {
            /* first try to find an empty slot */
            int rep = -1;
            for (int j = 0; j < FRAMES_REP; j++)
                if (frames_rep[j] == -1) { rep = j; break; }

            /* no empty slot -- evict page with farthest next use */
            if (rep == -1) {
                int farthest = -1;
                for (int j = 0; j < FRAMES_REP; j++) {
                    int k = i + 1;
                    while (k < n && refs[k] != frames_rep[j]) k++;
                    if (k > farthest) { farthest = k; rep = j; }
                }
            }
            frames_rep[rep] = refs[i];
            faults++;  f = 1;
        }
        print_rep_row(refs[i], f);
    }
    printf("  Total page faults (Optimal): %d\n", faults);
}

void run_page_replacement(void)
{
    puts("\n+================================================+");
    puts("|   SECTION 3 : PAGE REPLACEMENT                |");
    puts("+================================================+");

    int refs[REF_LEN] = { 7,0,1,2,0,3,0,4,2,3,0,3 };
    printf("\n  Reference string : ");
    for (int i = 0; i < REF_LEN; i++) printf("%d ", refs[i]);
    printf("\n  Frames available : %d\n", FRAMES_REP);

    fifo   (refs, REF_LEN);
    lru    (refs, REF_LEN);
    optimal(refs, REF_LEN);
}


/* ================================================================
   SECTION 4 : FRAME ALLOCATION
   (equal allocation vs proportional allocation)
   ================================================================ */
void run_frame_allocation(void)
{
    puts("\n+================================================+");
    puts("|   SECTION 4 : FRAME ALLOCATION                |");
    puts("+================================================+");

    int total_frames = 12;
    int proc_sizes[] = { 10, 40, 20, 30 };   /* relative sizes */
    int n            = 4;
    int total_size   = 0;

    for (int i = 0; i < n; i++) total_size += proc_sizes[i];

    printf("\n  Total frames: %d  |  Processes: %d  |  Total size: %d\n\n",
           total_frames, n, total_size);
    printf("  %-10s  %-8s  %-14s  %s\n",
           "Process", "Size", "Equal Alloc", "Proportional");
    puts("  ----------  --------  --------------  --------------");

    int used_prop = 0;
    for (int i = 0; i < n; i++) {
        int equal = total_frames / n;
        int prop  = (proc_sizes[i] * total_frames) / total_size;
        used_prop += prop;
        printf("  P%-9d  %-8d  %-14d  %d\n",
               i+1, proc_sizes[i], equal, prop);
    }
    printf("\n  Leftover frames (proportional): %d\n",
           total_frames - used_prop);
    puts("\n  Note: Proportional gives more frames to larger processes,");
    puts("        improving performance for memory-hungry workloads.");
}


/* ================================================================
   SECTION 5 : SEGMENTATION
   ================================================================ */
#define MAX_SEGS 8

typedef struct {
    int         seg_id;
    const char *name;
    int         base;    /* physical base address  */
    int         limit;   /* segment size (max offset is limit-1) */
} Segment;

/* ----------------------------------------------------------
   segmentation_translate
     seg_num : which segment (index into segment table)
     offset  : byte offset within that segment
     If offset >= limit -> hardware TRAP (segfault)
   ---------------------------------------------------------- */
static void segmentation_translate(Segment st[], int ns,
                                    int seg_num, int offset)
{
    printf("  (seg %d '%s'  offset %4d) -> ",
           seg_num,
           seg_num < ns ? st[seg_num].name : "???",
           offset);

    if (seg_num >= ns) {
        printf("TRAP: invalid segment number %d\n", seg_num);
        return;
    }
    if (offset >= st[seg_num].limit) {
        printf("TRAP: offset %d >= limit %d  (segfault)\n",
               offset, st[seg_num].limit);
        return;
    }
    int pa = st[seg_num].base + offset;
    printf("PA = %d + %d = %d\n", st[seg_num].base, offset, pa);
}

void run_segmentation(void)
{
    puts("\n+================================================+");
    puts("|   SECTION 5 : SEGMENTATION                    |");
    puts("+================================================+");

    Segment st[] = {
        { 0, "code ",  1000, 600 },
        { 1, "heap ",  6300, 500 },
        { 2, "stack",  4300, 500 },
        { 3, "data ",  3200, 192 },
    };
    int ns = 4;

    puts("\n  Segment Table:");
    printf("  %-5s  %-7s  %-8s  %-8s  %s\n",
           "Seg", "Name", "Base", "Limit", "End");
    puts("  -----  -------  --------  --------  ------");
    for (int i = 0; i < ns; i++)
        printf("  %-5d  %-7s  %-8d  %-8d  %d\n",
               st[i].seg_id, st[i].name,
               st[i].base, st[i].limit,
               st[i].base + st[i].limit);

    puts("\n  Address Translations (seg_num : offset):");
    puts("  --------------------------------------------------");
    segmentation_translate(st, ns, 0, 430);  /* code:430  -> OK     */
    segmentation_translate(st, ns, 1,  10);  /* heap:10   -> OK     */
    segmentation_translate(st, ns, 2, 400);  /* stack:400 -> OK     */
    segmentation_translate(st, ns, 3, 200);  /* data:200  -> TRAP   */
    segmentation_translate(st, ns, 5,  50);  /* bad seg   -> TRAP   */

    puts("\n  Note: External fragmentation can occur because segments");
    puts("        have variable size. Gaps between segments are wasted");
    puts("        unless the OS performs memory compaction.");
}


/* ================================================================
   SECTION 6 : TLB SIMULATION (Translation Lookaside Buffer)
   ================================================================ */
#define TLB_ENTRIES  4
#define PT_ENTRIES   16

typedef struct {
    int page;
    int frame;
    int valid;
    int lru_tick;   /* last access time, used for LRU eviction */
} TLBEntry;

static TLBEntry tlb[TLB_ENTRIES];

/* Full page table (backing store if TLB misses) */
static const int page_table[PT_ENTRIES] = {
     5, 10,  2,  8, 15,  3,  7, 12,
     0,  6, 11,  4,  9,  1, 13, 14
};

/* ----------------------------------------------------------
   tlb_lookup : search TLB for a page, update LRU tick on hit
   Returns frame number on hit, -1 on miss
   ---------------------------------------------------------- */
static int tlb_lookup(int page, int tick)
{
    for (int i = 0; i < TLB_ENTRIES; i++)
        if (tlb[i].valid && tlb[i].page == page) {
            tlb[i].lru_tick = tick;
            return tlb[i].frame;
        }
    return -1;
}

/* ----------------------------------------------------------
   tlb_insert : bring a new (page, frame) into the TLB.
   Evicts the LRU entry if the TLB is full.
   ---------------------------------------------------------- */
static void tlb_insert(int page, int frame, int tick)
{
    /* first look for an empty slot */
    for (int i = 0; i < TLB_ENTRIES; i++)
        if (!tlb[i].valid) {
            tlb[i] = (TLBEntry){ page, frame, 1, tick };
            return;
        }
    /* TLB full -- evict LRU */
    int lru_idx = 0;
    for (int i = 1; i < TLB_ENTRIES; i++)
        if (tlb[i].lru_tick < tlb[lru_idx].lru_tick)
            lru_idx = i;
    tlb[lru_idx] = (TLBEntry){ page, frame, 1, tick };
}

void run_tlb(void)
{
    puts("\n+================================================+");
    puts("|   SECTION 6 : TLB SIMULATION                  |");
    puts("+================================================+");

    /* clear TLB */
    memset(tlb, 0, sizeof(tlb));

    int accesses[] = { 0, 1, 2, 0, 3, 1, 4, 0, 2, 5, 1, 3 };
    int na = 12, hits = 0, misses = 0;

    printf("\n  TLB size: %d entries  |  Eviction policy: LRU\n\n",
           TLB_ENTRIES);
    printf("  %-5s  %-5s  %-6s  %-8s  TLB State\n",
           "Tick", "Page", "Frame", "Result");
    puts("  -----  -----  ------  --------  --------------------------------");

    for (int i = 0; i < na; i++) {
        int page  = accesses[i];
        int tick  = i + 1;
        int frame = tlb_lookup(page, tick);

        if (frame == -1) {
            /* TLB miss -- consult page table */
            misses++;
            frame = page_table[page];
            tlb_insert(page, frame, tick);
        } else {
            hits++;
        }

        /* print current TLB state */
        char state[128] = "[";
        for (int j = 0; j < TLB_ENTRIES; j++) {
            char tmp[24];
            if (tlb[j].valid)
                snprintf(tmp, sizeof(tmp), "P%d->F%d", tlb[j].page, tlb[j].frame);
            else
                snprintf(tmp, sizeof(tmp), " -- ");
            strcat(state, tmp);
            if (j < TLB_ENTRIES - 1) strcat(state, " | ");
        }
        strcat(state, "]");

        printf("  %-5d  %-5d  %-6d  %-8s  %s\n",
               tick, page, frame,
               frame == page_table[page] && misses > 0 ? "MISS" : "HIT",
               state);
    }

    printf("\n  Hits: %d  |  Misses: %d  |  Hit ratio: %.1f%%\n",
           hits, misses, hits * 100.0 / (hits + misses));
    puts("\n  Note: A TLB hit avoids one memory access for the page table.");
    puts("        Modern CPUs achieve ~99% hit rate with 512-1024 entries.");
}


/* ================================================================
   SECTION 7 : WORKING SET MODEL & THRASHING
   ================================================================ */
#define WS_DELTA    5     /* working-set window size (Δ)  */
#define WS_LEN     12     /* length of reference string   */

void run_working_set(void)
{
    puts("\n+================================================+");
    puts("|   SECTION 7 : WORKING SET & THRASHING         |");
    puts("+================================================+");

    int refs[WS_LEN] = { 2,3,2,1,2,3,4,3,4,3,2,3 };

    printf("\n  Reference string : ");
    for (int i = 0; i < WS_LEN; i++) printf("%d ", refs[i]);
    printf("\n  Window Δ         : %d\n\n", WS_DELTA);

    /* ---- Working Set computation ---- */
    puts("  Working Set W(t, delta):");
    printf("  %-5s  %-25s  %s\n", "t", "W(t, delta)", "|W|");
    puts("  -----  -------------------------  ---");

    for (int t = WS_DELTA - 1; t < WS_LEN; t++) {
        /* collect unique pages in window [t-delta+1 .. t] */
        int ws[16], wsize = 0;
        for (int j = t; j >= t - WS_DELTA + 1; j--) {
            int found = 0;
            for (int k = 0; k < wsize; k++)
                if (ws[k] == refs[j]) { found = 1; break; }
            if (!found) ws[wsize++] = refs[j];
        }
        /* print window contents and working set */
        char wset[64] = "{ ";
        for (int k = 0; k < wsize; k++) {
            char tmp[8];
            snprintf(tmp, sizeof(tmp), "%d ", ws[k]);
            strcat(wset, tmp);
        }
        strcat(wset, "}");
        printf("  %-5d  %-25s  %d\n", t + 1, wset, wsize);
    }

    /* ---- Thrashing table ---- */
    puts("\n  Frames vs fault rate (higher frames = less thrashing):");
    puts("  --------------------------------------------------------");
    printf("  %-8s  %-14s  %-24s  %s\n",
           "Frames", "Faults/sec", "Rate bar", "Status");
    puts("  --------  --------------  ------------------------  --------");

    struct { int f; int faults; const char *status; } tbl[] = {
        { 1, 120, "THRASHING (severe)"   },
        { 2,  85, "THRASHING (high)"     },
        { 3,  45, "THRASHING (moderate)" },
        { 4,  12, "Acceptable"           },
        { 5,   4, "Good"                 },
        { 6,   1, "Excellent"            },
        { 7,   0, "Ideal (WS fits)"      },
    };
    for (int i = 0; i < 7; i++) {
        /* scale bar: 120 faults -> 24 chars */
        char bar[32];
        int blen = tbl[i].faults / 5;
        memset(bar, '#', blen);
        bar[blen] = '\0';
        printf("  %-8d  %-14d  %-24s  %s\n",
               tbl[i].f, tbl[i].faults, bar, tbl[i].status);
    }

    puts("\n  Key insight: if frames < |working set|, every reference");
    puts("  causes a page fault -> CPU spends all time on I/O = thrashing.");
    puts("  Solution: track working set, ensure each process has enough frames.");
}


/* ================================================================
   MAIN
   ================================================================ */
int main(void)
{
    int choice;

    puts("+==================================================+");
    puts("| COMPLETE MEMORY MANAGEMENT -- C IMPLEMENTATION  |");
    puts("+==================================================+");

    while (1)
    {
        printf("\n================= MENU =================\n");
        printf("1. Memory Allocation (First/Best/Worst/Next Fit)\n");
        printf("2. Paging (Virtual -> Physical Address)\n");
        printf("3. Page Replacement (FIFO, LRU, Optimal)\n");
        printf("4. Frame Allocation\n");
        printf("5. Segmentation\n");
        printf("6. TLB Simulation\n");
        printf("7. Working Set & Thrashing\n");
        printf("8. Run ALL Sections\n");
        printf("9. Exit\n");
        printf("Enter your choice: ");

        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                run_allocation();
                break;

            case 2:
                run_paging();
                break;

            case 3:
                run_page_replacement();
                break;

            case 4:
                run_frame_allocation();
                break;

            case 5:
                run_segmentation();
                break;

            case 6:
                run_tlb();
                break;

            case 7:
                run_working_set();
                break;

            case 8:
                run_allocation();
                run_paging();
                run_page_replacement();
                run_frame_allocation();
                run_segmentation();
                run_tlb();
                run_working_set();
                break;

            case 9:
                printf("Exiting program...\n");
                exit(0);

            default:
                printf("Invalid choice! Try again.\n");
        }
    }
    return 0;
}
