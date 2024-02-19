# include <stdio.h>
# include <stdlib.h>
# include <assert.h>
# include <limits.h>
# include <stdbool.h>

int n; // num_knight
int m; // num_round

typedef struct _knight {
    int idx;
    long long hp;
    int ap;
    int num_att;
    int size;
    bool done;
    struct _knight *ptr;
} Knight;

typedef struct _heap {
    int total_ap;
    int std_hp;
    int std_num_att;
    int heapsize;
    Knight *heap;
} Heap;

void make_set(Knight *x)
{
    x->ptr = x;
    x->size = 1;
}

void link(Knight *x, Knight *y)
{
    if (x->size > y->size) {
        y->ptr = x;
        x->size += y->size;
    } else {
        x->ptr = y;
        y->size += x->size;
    }
}

Knight *find_set(Knight *x)
{
    if (x != x->ptr)
        x->ptr = find_set(x->ptr);
    return x->ptr;
}

void union_set(Knight *x, Knight *y) // by size
{
    link(find_set(x), find_set(y));
}

int parent(int i)
{
    return ((i - 1) / 2);
}

int left(int i)
{
    return (2 * i + 1);
}

int right(int i)
{
    return (2 * i + 2);
}

void exchange(Knight *a, Knight *b)
{
    Knight tmp = *a;
    *a = *b;
    *b = tmp;
}

void min_heapify(Knight *A, int heapsize, int idx) // top-down
{
    int l = left(idx);
    int r = right(idx);
    int min = idx;

    if (l < heapsize && A[l].hp < A[idx].hp)
        min = l;
    if (r < heapsize && A[r].hp < A[min].hp)
        min = r;
    if (min != idx) {
        exchange(&A[idx], &A[min]);
        min_heapify(A, heapsize, min);
    }
}

void insert(Heap *group, Knight *key)
{
    Knight *tmp = realloc(group->heap, sizeof(Knight) * (group->heapsize + 1));
    group->heap = tmp;
    group->heapsize += 1;

    key->hp -= (key->hp == INT_MAX)? 0 : group->std_hp;
    key->num_att -= group->std_num_att;

    int i = group->heapsize - 1;
    group->heap[i] = *key;
    group->total_ap += (key->done)? 0 : key->ap;

    while (i != 0 && group->heap[i].hp < group->heap[parent(i)].hp) {
        exchange(&group->heap[i], &group->heap[parent(i)]);
        i = parent(i);
    }
}

Knight *extract_min(Heap *group)
{
    group->heap[0].hp = INT_MAX;
    exchange(&group->heap[0], &group->heap[group->heapsize - 1]);
    Knight *min = &group->heap[group->heapsize - 1];
    min_heapify(group->heap, group->heapsize, 0);
    return min;
}

void settle(Heap *tar, Knight *min) {
    min->hp += tar->std_hp;
    min->num_att += tar->std_num_att;
    min->done = true;
}

void union_heap(Heap *tar, Heap *att) // by heapsize
{
    if (att->heapsize < tar->heapsize) {
        for (int i = 0; i < att->heapsize; i++) {
            att->heap[i].num_att += att->std_num_att;
            att->heap[i].hp += (att->heap[i].hp == INT_MAX)? 0 : att->std_hp;
            insert(tar, &att->heap[i]);
        }
        att->heapsize = 0;
        free(att->heap);
        att->heap = NULL;
    } else {
        for (int i = 0; i < tar->heapsize; i++) {
            tar->heap[i].num_att += tar->std_num_att;
            tar->heap[i].hp += (tar->heap[i].hp == INT_MAX)? 0 : tar->std_hp;
            insert(att, &tar->heap[i]);
        }
        tar->heapsize = 0;
        free(tar->heap);
        tar->heap = NULL;
    }
}

int main()
{
    // init data
    scanf("%d%d", &n, &m);
    Knight *knight = calloc(n, sizeof(Knight));
    Heap *group = calloc(n, sizeof(Heap));
    for (int i = 0; i < n; i++)
        knight[i].idx = i + 1;
    for (int i = 0; i < n; i++)
        scanf("%llu", &knight[i].hp);
    for (int i = 0; i < n; i++)
        scanf("%d", &knight[i].ap);
    
    // make set and build min heap
    for (int i = 0; i < n; i++) {
        make_set(&knight[i]);
        insert(&group[i], &knight[i]);
    }

    // main
    int att = 0, tar = 0;
    int att_root_idx = 0;
    int tar_root_idx = 0;
    Knight *att_root;
    Knight *tar_root;
    Knight *min;
    int index;
    for (int step = 0; step < m; step++) {
        scanf("%d%d", &att, &tar);

        // continue condition
        if (knight[att - 1].done || knight[tar - 1].done)
            continue;
        att_root = find_set(&knight[att - 1]);
        att_root_idx = att_root->idx - 1;
        tar_root = find_set(&knight[tar - 1]);
        tar_root_idx = tar_root->idx - 1;
        if (att_root == tar_root)
            continue;
        group[att_root_idx].std_num_att += 1;
        group[tar_root_idx].std_hp -= group[att_root_idx].total_ap;
        
        // death happen
        while (group[tar_root_idx].heap[0].hp + group[tar_root_idx].std_hp <= 0 && group[tar_root_idx].heap[0].done == false) {
            min = extract_min(&group[tar_root_idx]);
            group[tar_root_idx].total_ap -= min->ap;
            settle(&group[tar_root_idx], min);
            index = min->idx - 1;
            knight[index].done = true;
            knight[index].num_att = min->num_att;
        }
        
        // union set and heap
        union_set(&knight[tar_root_idx], &knight[att_root_idx]);
        union_heap(&group[tar_root_idx], &group[att_root_idx]);
    }

    // map heap back to knight
    for (int i = 0; i < n; i++) {
        if (group[i].heapsize <= 0 || group[i].heap == NULL) {
            continue;
        } else {
            for (int j = 0; j < group[i].heapsize; j++) {
                if (group[i].heap[j].done == false) {
                    settle(&group[i], &group[i].heap[j]);
                    index = group[i].heap[j].idx - 1;
                    knight[index] = group[i].heap[j];
                }
            }
            free(group[i].heap);
            group[i].heap = NULL;
        }
    }

    // output
    for (int i = 0; i < n; i++)
        printf((i == n - 1)? "%d\n" : "%d ", knight[i].num_att);
    free(knight);
    free(group);
    return 0;
}
