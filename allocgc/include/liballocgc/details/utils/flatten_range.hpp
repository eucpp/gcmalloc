#ifndef ALLOCGC_JOIN_RANGE_H
#define ALLOCGC_JOIN_RANGE_H

#include <iterator>
#include <utility>
#include <cassert>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/utils/make_reverse_iterator.hpp>

namespace allocgc { namespace details { namespace utils {

namespace internals {

template<typename OuterIterator>
class flattening_iterator : public boost::iterator_facade<
          flattening_iterator<OuterIterator>
        , decltype(*std::declval<typename OuterIterator::value_type::iterator>())
        , boost::bidirectional_traversal_tag
        , typename std::iterator_traits<typename OuterIterator::value_type::iterator>::reference
        , typename std::iterator_traits<typename OuterIterator::value_type::iterator>::difference_type
    >
{
    typedef boost::iterator_facade<
              flattening_iterator<OuterIterator>
            , decltype(*std::declval<typename OuterIterator::value_type::iterator>())
            , boost::bidirectional_traversal_tag
            , typename std::iterator_traits<typename OuterIterator::value_type::iterator>::reference
            , typename std::iterator_traits<typename OuterIterator::value_type::iterator>::difference_type
    > base;

    typedef OuterIterator outer_iterator;
    typedef typename OuterIterator::value_type::iterator inner_iterator;
public:
    typedef typename base::value_type       value_type;
    typedef typename base::reference        reference;
    typedef typename base::pointer          pointer;
    typedef typename base::difference_type  difference_type;

    flattening_iterator() = default;

    // for end of range construction
    explicit flattening_iterator(const OuterIterator& end)
        : flattening_iterator(end, end)
    {}

    flattening_iterator(const OuterIterator& first, const OuterIterator& last)
        : m_outer_begin(skip_empty_ranges(first, last))
        , m_outer_end(last)
    {
        m_outer_it = m_outer_begin;
        if (m_outer_begin != m_outer_end) {
            m_inner_it = m_outer_it->begin();
        }
    }

    flattening_iterator(const flattening_iterator&) = default;
    flattening_iterator(flattening_iterator&&) = default;

    flattening_iterator& operator=(const flattening_iterator&) = default;
    flattening_iterator& operator=(flattening_iterator&&) = default;
private:
    friend class boost::iterator_core_access;

    auto dereference() const
        -> decltype(*std::declval<inner_iterator>())
    {
        return *m_inner_it;
    }

    void increment()
    {
        assert(m_outer_it != m_outer_end);
        ++m_inner_it;
        if (m_inner_it == m_outer_it->end()) {
            m_outer_it = skip_empty_ranges(++m_outer_it, m_outer_end);
            if (m_outer_it != m_outer_end) {
                m_inner_it = m_outer_it->begin();
            }
        }
    }

    void decrement()
    {
        assert(m_outer_it == m_outer_end || !(m_outer_it == m_outer_begin && m_inner_it == m_outer_it->begin()));
        if (m_outer_it == m_outer_end || m_inner_it == m_outer_it->begin()) {
            m_outer_it = skip_empty_ranges(utils::make_reverse_iterator(m_outer_it),
                                           utils::make_reverse_iterator(m_outer_begin)).base();
            --m_outer_it;
            assert(!is_empty_range(m_outer_it));
            m_inner_it = m_outer_it->end();
        }
        --m_inner_it;
    }

    bool equal(const flattening_iterator& other) const
    {
        return (m_outer_it == m_outer_end && other.m_outer_it == m_outer_end) || (m_inner_it == other.m_inner_it);
    }

    template <typename Iterator>
    static Iterator skip_empty_ranges(Iterator it, const Iterator& end)
    {
        while (it != end && is_empty_range(it)) {
            ++it;
        }
        return it;
    }

    template <typename Iterator>
    static bool is_empty_range(const Iterator& it)
    {
        return it->begin() == it->end();
    }

    inner_iterator m_inner_it;
    outer_iterator m_outer_it;
    outer_iterator m_outer_begin;
    outer_iterator m_outer_end;
};

template<typename OuterIterator>
flattening_iterator<OuterIterator> make_flattening_iterator(const OuterIterator& end)
{
    return flattening_iterator<OuterIterator>(end);
}

template<typename OuterIterator>
flattening_iterator<OuterIterator> make_flattening_iterator(const OuterIterator& first, const OuterIterator& last)
{
    return flattening_iterator<OuterIterator>(first, last);
}

}

template <typename OuterIterator>
using flattened_range = boost::iterator_range<
        internals::flattening_iterator<OuterIterator>
    >;

template <typename OuterIterator>
flattened_range<OuterIterator> flatten_range(const OuterIterator& first, const OuterIterator& last)
{
    return boost::make_iterator_range(internals::make_flattening_iterator(first, last),
                                      internals::make_flattening_iterator(last));
}

template <typename OuterRange>
flattened_range<decltype(std::declval<OuterRange>().begin())> flatten_range(OuterRange& rng)
{
    return boost::make_iterator_range(internals::make_flattening_iterator(rng.begin(), rng.end()),
                                      internals::make_flattening_iterator(rng.end()));
}

}}}

#endif //ALLOCGC_JOIN_RANGE_H
