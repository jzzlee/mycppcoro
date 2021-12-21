#ifndef CPPCORO_TESTS_COUNTED_HPP_INCLUDED
#define CPPCORO_TESTS_COUNTED_HPP_INCLUDED

struct counted
{
    static inline int default_construction_count = 0;
    static inline int copy_construction_count = 0;
    static inline int move_construction_count = 0;
    static inline int destruction_count = 0;

    int id;

    static void reset_counts()
    {
        default_construction_count = 0;
        copy_construction_count = 0;
        move_construction_count = 0;
        destruction_count = 0;
    }

    static int construction_count()
    {
        return default_construction_count + copy_construction_count + move_construction_count;
    }

    static int active_count()
    {
        return construction_count() - destruction_count;
    }

    counted() : id(default_construction_count++) {}
    counted(const counted& other) : id(other.id) {++copy_construction_count;}
    counted(counted&& other) : id(other.id) {++move_construction_count;}
    ~counted() { ++destruction_count; }
};

#endif
