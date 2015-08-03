/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <utility>
#include <memory>
#include <limits>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/iterator_adaptors.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace acqua { namespace container { namespace detail {

template <
    typename T,
    typename Node,
    typename Clock,
    typename ValueHash,
    typename NodeEqual,
    typename Allocator
    >
class lru_cache
    : private Allocator
{
    using value_type = T;
    using clock_type = Clock;
    using duration_type = typename clock_type::duration;
    using time_point_type = typename clock_type::time_point;
    using size_type = std::size_t;
    using allocator_type = Allocator;
    using void_pointer = typename std::allocator_traits<allocator_type>::void_pointer;
    using list_hook = boost::intrusive::list_member_hook<
        boost::intrusive::void_pointer<
            void_pointer>
        >;
    using hash_hook = boost::intrusive::unordered_set_member_hook<
        boost::intrusive::void_pointer<
            void_pointer>
        >;
    struct node : public Node
    {
        list_hook list_;
        hash_hook hash_;
        time_point_type time_;

        node(value_type const & v, time_point_type const & time)
            : Node(v), time_(time) {}

        node(value_type && v, time_point_type const & time)
            : Node(std::move(v)), time_(time) {}

        void assign(value_type const & v, time_point_type const & time)
        {
            Node::operator=(v);
            time_ = time;
        }

        void assign(value_type && v, time_point_type const & time)
        {
            Node::operator=(std::move(v));
            time_ = time;
        }
    };
    using node_allocator = typename allocator_type::template rebind<node>::other;

    using list_type = boost::intrusive::list<
        node,
        boost::intrusive::member_hook<
            node, list_hook, &node::list_>
        >;
    using hash_type = boost::intrusive::unordered_set<
        node,
        boost::intrusive::member_hook<
            node, hash_hook, &node::hash_>
        >;
    using bucket_type = typename hash_type::bucket_type;
    using bucket_traits = typename hash_type::bucket_traits;
    using bucket_ptr = typename hash_type::bucket_ptr;
    using bucket_allocator = typename allocator_type::template rebind<bucket_type>::other;

    void init_buckets(bucket_allocator alloc, size_type size)
    {
        auto ptr = alloc.allocate(size);
        auto i = size;
        while(i--)
            alloc.construct(ptr + i);
        hash_ = boost::in_place(bucket_traits(ptr, size));
    }

    void free_buckets(bucket_allocator alloc)
    {
        auto ptr = hash_->bucket_pointer();
        auto size = hash_->bucket_count(), i = size;
        hash_ = boost::none;
        while(i--)
            alloc.destroy(ptr + i);
        alloc.deallocate(ptr, size);
    }

    node & new_node(value_type const & val, time_point_type const & now)
    {
        node_allocator alloc(get_allocator());
        node * ptr = alloc.allocate(1);
        alloc.construct(ptr, val, now);
        return *ptr;
    }


    node & new_node(value_type && val, time_point_type const & now)
    {
        node_allocator alloc(get_allocator());
        node * ptr = alloc.allocate(1);
        alloc.construct(ptr, std::move(val), now);
        return *ptr;
    }

    void delete_node(node & val)
    {
        node_allocator alloc(get_allocator());
        alloc.destroy(&val);
        alloc.deallocate(&val, 1);
    }

public:
    struct iterator
        : boost::iterator_adaptor<iterator, typename list_type::iterator, value_type>
    {
        iterator(typename list_type::iterator it)
            : boost::iterator_adaptor<iterator, typename list_type::iterator, value_type>(it)
        {
        }

        value_type & operator*() const
        {
            return this->base()->value_;
        }

        value_type * operator->() const
        {
            return &(this->base()->value_);
        }
    };

    struct const_iterator
        : boost::iterator_adaptor<const_iterator, typename list_type::const_iterator, value_type>
    {
        const_iterator(typename list_type::iterator it)
            : boost::iterator_adaptor<const_iterator, typename list_type::const_iterator, value_type>(it)
        {
        }

        const_iterator(iterator it)
            : boost::iterator_adaptor<const_iterator, typename list_type::const_iterator, value_type>(it.base())
        {
        }

        value_type & operator*() const
        {
            return this->base()->value_;
        }

        value_type * operator->() const
        {
            return &(this->base()->value_);
        }
    };

    struct reverse_iterator
        : boost::iterator_adaptor<reverse_iterator, typename list_type::reverse_iterator, value_type>
    {
        reverse_iterator(typename list_type::reverse_iterator it)
            : boost::iterator_adaptor<reverse_iterator, typename list_type::reverse_iterator, value_type>(it)
        {
        }

        value_type & operator*() const
        {
            return this->base()->value_;
        }

        value_type * operator->() const
        {
            return &(this->base()->value_);
        }
    };

    struct const_reverse_iterator
        : boost::iterator_adaptor<const_reverse_iterator, typename list_type::const_reverse_iterator, value_type>
    {
        const_reverse_iterator(typename list_type::const_reverse_iterator it)
            : boost::iterator_adaptor<const_reverse_iterator, typename list_type::const_reverse_iterator, value_type>(it)
        {
        }

        const_reverse_iterator(reverse_iterator it)
            : boost::iterator_adaptor<const_reverse_iterator, typename list_type::const_reverse_iterator, value_type>(it.base())
        {
        }

        value_type & operator*() const
        {
            return this->base()->value_;
        }

        value_type * operator->() const
        {
            return &(this->base()->value_);
        }
    };

    lru_cache(size_type bucket_size, allocator_type alloc)
        : allocator_type(alloc)
        , max_size_(std::numeric_limits<size_type>::max())
        , max_duration_(duration_type::max())
    {
        init_buckets(get_allocator(), bucket_size);
    }

    ~lru_cache()
    {
        clear();
        free_buckets(get_allocator());
    }

    allocator_type get_allocator() const
    {
        return *this;
    }

    void clear()
    {
        while(!list_.empty())
            pop();
    }

    void empty() const { return list_.empty(); }
    size_type size() const { return list_.size(); }
    iterator begin() { return list_.begin(); }
    const_iterator begin() const { return list_.begin(); }
    iterator end() { return list_.end(); }
    const_iterator end() const { return list_.end(); }
    reverse_iterator rbegin() { return list_.rbegin(); }
    const_reverse_iterator rbegin() const { return list_.rbegin(); }
    reverse_iterator rend() { return list_.rend(); }
    const_reverse_iterator rend() const { return list_.rend(); }

    template <typename V, typename H, typename P>
    void push(V v, H h, P p)
    {
        auto now = clock_type::now();
        if (list_.empty()) {
            if (max_size_ == 0)
                return;

            auto & val = new_node(v, now);
            hash_->insert(val);
            list_.push_front(val);
            return;
        }

        auto it = hash_->find(v, h, p);
        if (it != hash_->end()) {
            auto & val = *it;
            list_.erase(list_.iterator_to(val));
            val.assign(v, now);
            list_.push_front(val);
            return;
        }

        if (max_size_ <= list_.size()) {
            auto & val = list_.back();
            list_.erase(list_.iterator_to(val));
            hash_->erase(hash_->iterator_to(val));
            val.assign(v, now);
            hash_->insert(val);
            list_.push_front(val);
            return;
        }

        if ((now - list_.back().time_) > max_duration_) {
            auto & val = list_.back();
            list_.erase(list_.iterator_to(val));
            hash_->erase(hash_->iterator_to(val));
            val.assign(v, now);
            hash_->insert(val);
            list_.push_front(val);
            return;
        }

        auto & val = new_node(v, now);
        hash_->insert(val);
        list_.push_front(val);
        return;
    }

    void pop()
    {

        auto & val = *list_.rbegin();
        list_.erase(list_.iterator_to(val));
        hash_->erase(hash_->iterator_to(val));
        delete_node(val);
    }

    value_type & front()
    {
        return list_.begin()->value_;
    }

    value_type const & front() const
    {
        return list_.begin()->value_;
    }

    value_type & back()
    {
        return list_.rbegin()->value_;
    }

    value_type const & back() const
    {
        return list_.rbegin()->value_;
    }

    template <typename K, typename H, typename P>
    iterator find(K k, H h, P p)
    {
        auto it = hash_->find(k, h, p);
        return (it != hash_->end()) ? list_.iterator_to(*it) : list_.end();
    }

    template <typename K, typename H, typename P>
    const_iterator find(K k, H h, P p) const
    {
        auto it = hash_->find(k, h, p);
        return (it != hash_->end()) ? list_.iterator_to(*it) : list_.end();
    }

    iterator erase(const_iterator it)
    {
        if (it != list_.end()) {
            auto & val = *it;
            hash_->erase(hash_->iterator_to(val));
            it = list_.erase(it);
            delete_node(val);
        }

        return it;
    }

    iterator erase(const_iterator beg, const_iterator end)
    {
        while(beg != end) {
            auto & val = *beg;
            hash_->erase(hash_->iterator_to(val));
            beg = list_.erase(beg);
            delete_node(val);
        }

        return end;
    }

    size_type max_size()
    {
        return max_size_;
    }

    void max_size(size_type size)
    {
        while (list_.size() > size)
            pop();
        max_size_ = size;
    }

    void max_duration(duration_type const & dura)
    {
        max_duration_ = dura;
    }

    duration_type const & max_duration() const
    {
        return max_duration_;
    }

private:
    size_type max_size_;
    duration_type max_duration_;
    list_type list_;
    boost::optional<hash_type> hash_;
};

} } }
