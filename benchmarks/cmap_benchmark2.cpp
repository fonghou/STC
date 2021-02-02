#include <stc/crandom.h>
#include <stc/cstr.h>
#include <stc/cmap.h>
#include <cmath>
#include <string>
#include <unordered_map>
#include "others/bytell_hash_map.hpp"
#include "others/robin_hood.hpp"
#include "others/hopscotch_map.h"
#include "others/sparsepp/spp.h"

#define PICOBENCH_IMPLEMENT_WITH_MAIN
#include "picobench.hpp"

enum {N1 = 4000000, S1 = 1, MaxLoadFactor100 = 80};
uint64_t seed = time(NULL);

static inline uint32_t hash32(const void* data, size_t len) {
    return *(const uint32_t *)data * 2654435769u;
}
static inline uint32_t hash64(const void* data, size_t len) {
    uint64_t x = *(const uint64_t *)data * 11400714819323198485ull;
    return x ^ (x >> 24);
}
template <class K, class V> using umap = std::unordered_map<K, V>;
template <class K, class V> using bmap = ska::bytell_hash_map<K, V>;
template <class K, class V> using fmap = ska::flat_hash_map<K, V>;
template <class K, class V> using hmap = tsl::hopscotch_map<K, V>;
template <class K, class V> using smap = spp::sparse_hash_map<K, V>;
template <class K, class V> using rmap = robin_hood::unordered_flat_map<K, V, robin_hood::hash<K>,
                                                                        std::equal_to<K>, MaxLoadFactor100>;
#define DEFMAP(map, ...) \
    using u##map = umap __VA_ARGS__; \
    using b##map = bmap __VA_ARGS__; \
    using f##map = fmap __VA_ARGS__; \
    using h##map = hmap __VA_ARGS__; \
    using s##map = smap __VA_ARGS__; \
    using r##map = rmap __VA_ARGS__

DEFMAP(map_i, <int, int>);
DEFMAP(map_x, <uint64_t, uint64_t>);
DEFMAP(map_s, <std::string, std::string>);

using_cmap(i, int, int, c_default_equals, hash32);
using_cmap(x, uint64_t, uint64_t, c_default_equals, hash64);
using_cmap_strkey(s, cstr, cstr_del, cstr_clone);

PICOBENCH_SUITE("Map1");

template <class MapInt>
static void ctor_and_ins_one_i(picobench::state& s)
{
    size_t result = 0;

    picobench::scope scope(s);
    c_forrange (n, s.iterations()) {
        MapInt map;
        map[n];
        result += map.size();
    }
    s.set_result(result);
}

static void ctor_and_ins_one_cmap_i(picobench::state& s)
{
    size_t result = 0;

    picobench::scope scope(s);
    c_forrange (n, s.iterations()) {
        cmap_i map = cmap_i_init();
        cmap_i_emplace(&map, n, 0);
        result += cmap_i_size(map);
        cmap_i_del(&map);
    }
    s.set_result(result);
}

#define P samples(S1).iterations({N1})
PICOBENCH(ctor_and_ins_one_i<umap_i>).P;
PICOBENCH(ctor_and_ins_one_i<bmap_i>).P;
PICOBENCH(ctor_and_ins_one_i<fmap_i>).P;
PICOBENCH(ctor_and_ins_one_i<hmap_i>).P;
PICOBENCH(ctor_and_ins_one_i<smap_i>).P;
PICOBENCH(ctor_and_ins_one_i<rmap_i>).P;
PICOBENCH(ctor_and_ins_one_cmap_i).P;
#undef P

PICOBENCH_SUITE("Map2");

template <class MapInt>
static void ins_and_erase_i(picobench::state& s)
{
    size_t result = 0;
    MapInt map;
    map.max_load_factor(MaxLoadFactor100 / 100.0);
    stc64_srandom(seed);

    picobench::scope scope(s);
    c_forrange (s.iterations())
        map[stc64_random()];
    map.clear();
    stc64_srandom(seed);
    c_forrange (s.iterations())
        map[stc64_random()];
    stc64_srandom(seed);
    c_forrange (s.iterations())
        map.erase(stc64_random());
    s.set_result(map.size());
}

static void ins_and_erase_cmap_i(picobench::state& s)
{
    cmap_i map = cmap_i_init();
    cmap_i_set_load_factors(&map, 0.0, MaxLoadFactor100 / 100.0);
    stc64_srandom(seed);

    picobench::scope scope(s);
    c_forrange (s.iterations())
        cmap_i_emplace(&map, stc64_random(), 0);
    cmap_i_clear(&map);
    stc64_srandom(seed);
    c_forrange (s.iterations())
        cmap_i_emplace(&map, stc64_random(), 0);
    stc64_srandom(seed);
    c_forrange (s.iterations())
        cmap_i_erase(&map, stc64_random());
    s.set_result(cmap_i_size(map));
    cmap_i_del(&map);
}

static void ins_and_erase_cmap_x(picobench::state& s)
{
    cmap_x map = cmap_x_init();
    cmap_x_set_load_factors(&map, 0.0, MaxLoadFactor100 / 100.0);
    stc64_srandom(seed);

    picobench::scope scope(s);
    c_forrange (s.iterations())
        cmap_x_emplace(&map, stc64_random(), 0);
    cmap_x_clear(&map);
    stc64_srandom(seed);
    c_forrange (s.iterations())
        cmap_x_emplace(&map, stc64_random(), 0);
    stc64_srandom(seed);
    c_forrange (s.iterations())
        cmap_x_erase(&map, stc64_random());
    s.set_result(cmap_x_size(map));
    cmap_x_del(&map);
}

#define P samples(S1).iterations({N1/4})
PICOBENCH(ins_and_erase_i<umap_x>).P;
PICOBENCH(ins_and_erase_i<bmap_x>).P;
PICOBENCH(ins_and_erase_i<fmap_x>).P;
PICOBENCH(ins_and_erase_i<hmap_x>).P;
PICOBENCH(ins_and_erase_i<smap_x>).P;
PICOBENCH(ins_and_erase_i<rmap_x>).P;
PICOBENCH(ins_and_erase_cmap_x).P;
#undef P

PICOBENCH_SUITE("Map3");

template <class MapInt>
static void ins_and_access_i(picobench::state& s)
{
    uint64_t mask = (1ull << s.arg()) - 1;
    size_t result = 0;
    MapInt map;
    map.max_load_factor(MaxLoadFactor100 / 100.0);
    stc64_srandom(seed);

    picobench::scope scope(s);
    c_forrange (N1)
        result += ++map[stc64_random() & mask];
    s.set_result(result);
}

static void ins_and_access_cmap_i(picobench::state& s)
{
    uint64_t mask = (1ull << s.arg()) - 1;
    size_t result = 0;
    cmap_i map = cmap_i_init();
    cmap_i_set_load_factors(&map, 0.0, MaxLoadFactor100 / 100.0);
    stc64_srandom(seed);

    picobench::scope scope(s);
    c_forrange (N1)
        result += ++cmap_i_emplace(&map, stc64_random() & mask, 0).first->second;
    s.set_result(result);
    cmap_i_del(&map);
}

#define P samples(S1).iterations({N1, N1, N1, N1}).args({18, 23, 25, 31})
PICOBENCH(ins_and_access_i<umap_i>).P;
PICOBENCH(ins_and_access_i<bmap_i>).P;
PICOBENCH(ins_and_access_i<fmap_i>).P;
PICOBENCH(ins_and_access_i<hmap_i>).P;
PICOBENCH(ins_and_access_i<smap_i>).P;
PICOBENCH(ins_and_access_i<rmap_i>).P;
PICOBENCH(ins_and_access_cmap_i).P;
#undef P

PICOBENCH_SUITE("Map4");

static void randomize(char* str, size_t len) {
    union {uint64_t i; char c[8];} r = {.i = stc64_random()};
    for (int i = len - 7, j = 0; i < len; ++j, ++i)
        str[i] = (r.c[j] & 63) + 48;
}

template <class MapStr>
static void ins_and_access_s(picobench::state& s)
{
    std::string str(s.arg(), 'x');
    size_t result = 0;
    MapStr map;
    map.max_load_factor(MaxLoadFactor100 / 100.0);
    stc64_srandom(seed);

    picobench::scope scope(s);
    c_forrange (s.iterations()) {
        randomize(&str[0], str.size());
        map[str] = str;
        randomize(&str[0], str.size());
        auto it = map.find(str);
        if (it != map.end()) {
            ++result;
            map.erase(it);
        }
    }
    s.set_result(result);
}

static void ins_and_access_cmap_s(picobench::state& s)
{
    cstr str = cstr_with_size(s.arg(), 'x');
    size_t result = 0;
    cmap_s map = cmap_s_init();
    cmap_s_set_load_factors(&map, 0.0, MaxLoadFactor100 / 100.0);
    stc64_srandom(seed);

    picobench::scope scope(s);
    c_forrange (s.iterations()) {
        randomize(str.str, cstr_size(str));
        cmap_s_put(&map, str.str, cstr_clone(str));
        randomize(str.str, cstr_size(str));
        cmap_s_value_t* val = cmap_s_find(&map, str.str);
        if (val) {
            ++result;
            cmap_s_erase_entry(&map, val);
        }
    }
    s.set_result(result);
    cstr_del(&str);
    cmap_s_del(&map);
}

#define P samples(S1).iterations({N1/5, N1/5, N1/5, N1/10, N1/40}).args({13, 7, 8, 100, 1000})
PICOBENCH(ins_and_access_s<umap_s>).P;
PICOBENCH(ins_and_access_s<bmap_s>).P;
PICOBENCH(ins_and_access_s<fmap_s>).P;
PICOBENCH(ins_and_access_s<hmap_s>).P;
PICOBENCH(ins_and_access_s<smap_s>).P;
PICOBENCH(ins_and_access_s<rmap_s>).P;
PICOBENCH(ins_and_access_cmap_s).P;
#undef P

PICOBENCH_SUITE("Map5");

template <class MapX>
static void iterate_x(picobench::state& s)
{
    MapX map;
    map.max_load_factor(MaxLoadFactor100 / 100.0);
    uint64_t K = (1ull << s.arg()) - 1;

    picobench::scope scope(s);
    stc64_srandom(seed);
    size_t result = 0;

    // measure insert then iterate whole map
    c_forrange (n, s.iterations()) {
        map[stc64_random()] = n;
        if (!(n & K)) for (auto const& keyVal : map)
            result += keyVal.second;
    }

    // reset rng back to inital state
    stc64_srandom(seed);

    // measure erase then iterate whole map
    c_forrange (n, s.iterations()) {
        map.erase(stc64_random());
        if (!(n & K)) for (auto const& keyVal : map)
            result += keyVal.second;
    }
    s.set_result(result);
}

static void iterate_cmap_x(picobench::state& s)
{
    cmap_x map = cmap_x_init();
    cmap_x_set_load_factors(&map, 0.3, MaxLoadFactor100 / 100.0);
    uint64_t K = (1ull << s.arg()) - 1;

    picobench::scope scope(s);
    stc64_srandom(seed);
    size_t result = 0;

    // measure insert then iterate whole map
    c_forrange (n, s.iterations()) {
        cmap_x_put(&map, stc64_random(), n);
        if (!(n & K)) c_foreach (i, cmap_x, map)
            result += i.ref->second;
    }

    // reset rng back to inital state
    stc64_srandom(seed);

    // measure erase then iterate whole map
    c_forrange (n, s.iterations()) {
        cmap_x_erase(&map, stc64_random());
        if (!(n & K)) c_foreach (i, cmap_x, map)
            result += i.ref->second;
    }
    s.set_result(result);
    cmap_x_del(&map);
}


#define P samples(S1).iterations({N1/20}).args({12})
PICOBENCH(iterate_x<umap_x>).P;
PICOBENCH(iterate_x<bmap_x>).P;
PICOBENCH(iterate_x<fmap_x>).P;
PICOBENCH(iterate_x<hmap_x>).P;
PICOBENCH(iterate_x<smap_x>).P;
PICOBENCH(iterate_x<rmap_x>).P;
PICOBENCH(iterate_cmap_x).P;
#undef P
