#include <stdio.h>

// #include "stc/types.h"
// forward_list(list_i32, int32_t);
// forward_list(list_pnt, struct Point);

// #define i_TYPE list_i32, int32_t
#define i_type list_i32
#define i_base list
#define i_key  int32_t
#include "stc/arena.h"
// #define i_is_forward
// #include "stc/list.h"

typedef struct Point {
  int x, y;
} Point;
int point_cmp(const Point* a, const Point* b) {
  int c = a->x - b->x;
  return c ? c : a->y - b->y;
}

// #define i_TYPE list_pnt, Point
#define i_type list_pnt
#define i_base list
#define i_key  Point
#define i_cmp  point_cmp
#include "stc/arena.h"
// #define i_is_forward
// #include "stc/list.h"

#define i_key float
#define i_use_cmp  // use < and == operators for comparison
#include "stc/list.h"

typedef struct {
  list_i32_ext intlist;
  list_pnt_ext pntlist;
} MyStruct;

// void MyStruct_drop(MyStruct* s);
#define i_type MyList
#define i_key  MyStruct
#define i_base list
#include "stc/arena.h"
// #define i_keydrop MyStruct_drop  // define drop function
// #define i_no_clone  // must explicitely exclude or define cloning support because of drop.
// #include "stc/list.h"

// void MyStruct_drop(MyStruct* s) {
//   list_i32_drop(&s->intlist.get);
//   list_pnt_drop(&s->pntlist.get);
// }

int main(void) {
  enum { cap = 1 << 12 };
  byte* heap = malloc(cap);
  byte* persist = heap;
  Arena arena = newarena(&persist, cap);

  MyStruct my = {0};
  my.intlist = (list_i32_ext){.arena = &arena};
  my.pntlist = (list_pnt_ext){.arena = &arena};

  list_i32_push_back(&my.intlist.get, 123);
  list_pnt_push_back(&my.pntlist.get, (Point){123, 456});
  list_pnt_push_back(&my.pntlist.get, (Point){42, 14});
  list_pnt_push_back(&my.pntlist.get, (Point){32, 94});
  list_pnt_push_back(&my.pntlist.get, (Point){62, 81});

  list_pnt_sort(&my.pntlist.get);

  c_foreach(i, list_pnt, my.pntlist.get) {
    printf(" (%d %d)", i.ref->x, i.ref->y);
  }
  puts("");

  // MyStruct_drop(&my);

  list_float flist = c_init(list_float, {123.3f, 321.2f, -32.2f, 78.2f});
  list_float_sort(&flist);

  c_foreach(i, list_float, flist) printf(" %g", (double)*i.ref);

  puts("");
  list_float_drop(&flist);

  free(heap);
}
