#ifndef DIPLOMA_GARBAGE_COLLECTOR_HPP
#define DIPLOMA_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <libprecisegc/details/collectors/index_tree.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_manager.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc { namespace details {

class gc_facade : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_facade();

    void init(std::unique_ptr<gc_strategy> strategy);

    gc_strategy* get_strategy() const;
    std::unique_ptr<gc_strategy> set_strategy(std::unique_ptr<gc_strategy> strategy);

    allocators::gc_alloc_response allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta);

    void commit(gc_cell& cell);
    void commit(gc_cell& cell, const gc_type_meta* type_meta);

    byte* init_ptr(byte* ptr, bool root_flag);
    bool is_root(const gc_handle& handle) const;

    byte* rbarrier(const gc_handle& handle);
    void  wbarrier(gc_handle& dst, const gc_handle& src);

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset);

    byte* pin(const gc_handle& handle);
    void  unpin(byte* ptr);

    byte* push_pin(const gc_handle& handle);
    void  pop_pin(byte* ptr);

    bool compare(const gc_handle& a, const gc_handle& b);

    void initiation_point(initiation_point_type ipt, const gc_options& opt);

    inline void add_to_index(const byte* mem, size_t size, gc_memory_descriptor* entry)
    {
        m_index.add_to_index(mem, size, entry);
    }

    inline void remove_from_index(const byte* mem, size_t size)
    {
        m_index.remove_from_index(mem, size);
    }

    inline gc_memory_descriptor* index_memory(const byte* mem) const
    {
        return m_index.index(mem);
    }

    inline gc_cell index_object(byte* obj_start) const
    {
        return gc_cell::from_obj_start(obj_start, index_memory(obj_start));
    }

    bool is_printer_enabled() const;
    void set_printer_enabled(bool enabled);

    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);

    gc_info  info() const;
    gc_stat  stats() const;
private:
    typedef collectors::index_tree<gc_memory_descriptor> index_tree_t;

    allocators::gc_alloc_response try_allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta);

    bool is_interior_pointer(const gc_handle& handle, byte* iptr);
    bool is_interior_offset(const gc_handle& handle, ptrdiff_t shift);

    std::unique_ptr<gc_strategy> m_strategy;
    gc_manager m_manager;
    std::mutex m_gc_mutex;
    index_tree_t m_index;
};

}}

#endif //DIPLOMA_GARBAGE_COLLECTOR_HPP
