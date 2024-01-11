#define i_implement
#include "stc/cstr.h"

void stringdemo1(void) {
  cstr cs = cstr_lit("one-nine-three-seven-five");
  printf("%s.\n", cstr_str(&cs));

  cstr_insert(&cs, 3, "-two");
  printf("%s.\n", cstr_str(&cs));

  cstr_erase(&cs, 7, 5);  // -nine
  printf("%s.\n", cstr_str(&cs));

  cstr_replace(&cs, "seven", "four", 1);
  printf("%s.\n", cstr_str(&cs));

  cstr_take(&cs, cstr_from_fmt("%s *** %s", cstr_str(&cs), cstr_str(&cs)));
  printf("%s.\n", cstr_str(&cs));

  printf("find \"four\": %s\n", cstr_str(&cs) + cstr_find(&cs, "four"));

  // reassign:
  cstr_assign(&cs, "one two three four five six seven");
  cstr_append(&cs, " eight");
  printf("append: %s\n", cstr_str(&cs));

  cstr_drop(&cs);
}

// clang-format off
#define i_type list_int
#define i_base list
#define i_key  int
#define i_use_cmp
#include "stc/arena.h"
// clang-format on

int64_t *listdemo1(Arena arena) {
  list_int_ext nums_ext = {.arena = &arena};
  list_int_ext nums2_ext = {.arena = &arena};
  list_int *nums = &nums_ext.get;
  list_int *nums2 = &nums2_ext.get;
  for (int i = 0; i < 10; ++i) list_int_push_back(nums, i);
  for (int i = 100; i < 110; ++i) list_int_push_back(nums2, i);

  /* splice nums2 to front of nums */
  list_int_splice(nums, list_int_begin(nums), nums2);
  int64_t *sum = New(&arena, int64_t);
  c_foreach(i, list_int, *nums) {
    printf("spliced: %d\n", *i.ref);
    *sum += *i.ref;
  }
  printf("sum=%ld\n", *sum);

  *list_int_find(nums, 104).ref += 50;
  list_int_remove(nums, 103);
  list_int_iter it = list_int_begin(nums);
  list_int_erase_range(nums, list_int_advance(it, 5), list_int_advance(it, 15));
  list_int_pop_front(nums);
  list_int_push_back(nums, -99);
  list_int_sort(nums);

  c_foreach(i, list_int, *nums) {
    printf("sorted: %d\n", *i.ref);
    *sum += *i.ref;
  }

  return sum;
  // c_drop(list_int, nums, nums2);
}

#define i_type vec_ll
#define i_key  long long
#define i_base vec
#include "stc/arena.h"

void vectordemo1(Arena arena) {
  vec_ll_ext ext = {.arena = &arena};
  vec_ll *bignums = &ext.get;
  vec_ll_reserve(bignums, 100);
  for (int i = 10; i <= 100; i += 10) vec_ll_push(bignums, i * i);

  printf("erase - %d: %lld\n", 3, bignums->data[3]);
  vec_ll_erase_n(bignums, 3, 1);  // erase index 3

  vec_ll_pop(bignums);            // erase the last
  vec_ll_erase_n(bignums, 0, 1);  // erase the first

  // vec_ll_shrink_to_fit(bignums);

  for (int i = 0; i < vec_ll_size(bignums); ++i) {
    printf("%d: %lld\n", i, bignums->data[i]);
  }

  // vec_ll_drop(bignums);
}

#define i_type vec_str
#define i_base vec
#define i_key_str
#define i_use_cmp
#include "stc/arena.h"

typedef struct {
  int32_t *data;
  ptrdiff_t len;
  ptrdiff_t cap;
} int32s;

void vectordemo2(Arena arena) {
  LogArena(arena);
  byte *persist = 0;
  Arena slice = subarena(&arena, &persist);
  LogArena(arena);
  LogArena(slice);

  int32s fibs = {0};
  *Push(&fibs, &arena) = -1;
  *Push(&fibs, &arena) = 0;
  *Push(&fibs, &arena) = 1;

  for (int i = 3; i <= 17 + 1; i++) {
    *Push(&fibs, &slice) = fibs.data[i - 2] + fibs.data[i - 1];
  }

  puts("fibs: ");
  for (int i = fibs.len - 1; i > 0; --i) {
    printf("%d ", fibs.data[i]);
  }
  puts("");

  vec_str_ext ext = {.arena = &arena};
  vec_str *names = &ext.get;

  vec_str_emplace_back(names, "Joe");
  vec_str_emplace_back(names, "Chris");
  vec_str_emplace_back(names, "Mary");
  cstr_assign(&names->data[1], "Jane");  // replace Joe
  printf("names[1]: %s\n", cstr_str(&names->data[1]));

  vec_str_sort(names);  // Sort the array

  c_foreach(i, vec_str, *names) {
    printf("sorted: %s\n", cstr_str(i.ref));
  }

  LogArena(arena);

  // vec_str_drop(names);
}

// clang-format off
#define i_type hmap_si
#define i_base hmap
#define i_key_str
#define i_val int
#include "stc/arena.h"
// clang-format on

void mapdemo2(Arena arena) {
  Arena scratch = getscratch(&arena);
  hmap_si_ext nums_ext = {.arena = &scratch};
  hmap_si *nums = &nums_ext.get;
  hmap_si_emplace_or_assign(nums, "Hello", 64);
  hmap_si_emplace_or_assign(nums, "Groovy", 121);
  hmap_si_emplace_or_assign(nums, "Groovy", 200);  // overwrite previous

  // iterate the map:
  for (hmap_si_iter i = hmap_si_begin(nums); i.ref; hmap_si_next(&i))
    printf("long: %s: %d\n", cstr_str(&i.ref->first), i.ref->second);

  // or rather use the short form:
  c_foreach(i, hmap_si, *nums) {
    printf("short: %s: %d\n", cstr_str(&i.ref->first), i.ref->second);
  }

  hmap_si_drop(nums);
}

#define i_type hmap_str
#define i_base hmap
#define i_key_str
#define i_val_str
#include "stc/arena.h"

void mapdemo3(Arena local) {
  hmap_str_ext ext = {.arena = &local};
  hmap_str *table = &ext.get;
  hmap_str_emplace(table, "Map", "test");
  hmap_str_emplace(table, "Make", "my");
  hmap_str_emplace(table, "Sunny", "day");

  hmap_str_iter it = hmap_str_find(table, "Make");
  c_foreach(i, hmap_str, *table)
    printf("entry: %s: %s\n", cstr_str(&i.ref->first), cstr_str(&i.ref->second));
  printf("size %" c_ZI ": remove: Make: %s\n", hmap_str_size(table), cstr_str(&it.ref->second));
  // hmap_str_erase(&table, "Make");
  hmap_str_erase_at(table, it);

  printf("size %" c_ZI "\n", hmap_str_size(table));
  c_foreach(i, hmap_str, *table)
    printf("entry: %s: %s\n", cstr_str(&i.ref->first), cstr_str(&i.ref->second));

  LogArena(local);
  // hmap_str_drop(table);  // frees key and value cstrs, and hash table.
}

int main(void) {
  enum { cap = 1 << 10 };
  byte *buffer = malloc(cap);
  byte *persist = buffer;
  Arena arena = newarena(&persist, cap);

  if (ArenaOOM(&arena)) {
    fputs("!!! OOM realloc !!!\n", stderr);
    LogArena(arena);
    ssize cap = 1 << 20;
    buffer = persist = realloc(buffer, cap);
    arena.persist = &persist;
    arena.scratch = persist + cap;

    if (ArenaOOM(&arena)) {
      fputs("!!! OOM exit !!!\n", stderr);
      exit(1);
    }
  }

  printf("\nSTRING DEMO1\n");
  stringdemo1();
  LogArena(arena);

  {
    Arena tmp = arena;
    LogArena(tmp);
    Arena scratch = getscratch(&tmp);

    printf("\nLIST DEMO1\n");
    printf("sum=%ld", *listdemo1(tmp));
    LogArena(tmp);

    LogArena(scratch);
    printf("\nVEC DEMO1\n");
    vectordemo1(scratch);
    LogArena(tmp);

    printf("\nVEC DEMO2\n");
    vectordemo2(scratch);
    LogArena(tmp);
  }

  {
    Arena tmp = arena;
    Arena scratch = getscratch(&tmp);
    LogArena(tmp);

    printf("\nMAP DEMO2\n");
    mapdemo2(scratch);
    LogArena(tmp);

    printf("\nMAP DEMO3\n");
    mapdemo3(scratch);
    LogArena(tmp);
  }

  free(buffer);
}
