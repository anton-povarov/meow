////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__PTR_LIST_HPP_
#define MEOW__PTR_LIST_HPP_

#include <boost/noncopyable.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef boost::intrusive::list_base_hook<> ptr_list_hook_t;

	template<class T>
	struct ptr_list_traits_default
	{
		typedef T value_t;
		typedef boost::static_move_ptr<value_t> value_move_ptr;

		typedef ptr_list_hook_t list_hook_t;
		static bool const constant_time_size = true;
	};

	typedef boost::intrusive::list_base_hook<
						  boost::intrusive::link_mode<boost::intrusive::auto_unlink>
						>
						ptr_list_auto_hook_t;

	template<class T>
	struct ptr_list_traits_auto_unlink
	{
		typedef T value_t;
		typedef boost::static_move_ptr<value_t> value_move_ptr;

		typedef ptr_list_auto_hook_t list_hook_t;
		static bool const constant_time_size = false;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class T
		, class Traits = ptr_list_traits_default<T>
	>
	struct ptr_list_t : private boost::noncopyable
	{
		typedef ptr_list_t  self_t;

		typedef typename Traits::value_t         value_t;
		typedef typename Traits::value_move_ptr  value_move_ptr;
		typedef typename Traits::list_hook_t     list_hook_t;

		typedef boost::intrusive::list<
					  value_t
					, boost::intrusive::base_hook<list_hook_t>
					, boost::intrusive::constant_time_size<Traits::constant_time_size>
					>
					list_t;
	private:
		list_t l_;

	public: // iterators

		template<class I>
		struct my_iterator : public boost::iterator_facade<
							 						  my_iterator<I>
													, value_t
													, typename boost::iterator_traversal<I>::type
													, value_t*
													>
		{
			typedef I list_iterator_t;
			list_iterator_t i_;

			my_iterator() : i_() {}
			my_iterator(list_iterator_t i) : i_(i) {}

		private:

			friend class boost::iterator_core_access;

			void increment() { ++i_; }

			value_t* dereference() const { return &*i_; }

			template<class J>
			bool equal(J const& other) const { return (i_ == other.i_); }
		};

		typedef my_iterator<typename list_t::iterator> 			iterator;
		typedef my_iterator<typename list_t::const_iterator> 	const_iterator;

		iterator       begin()       { return iterator(l_.begin()); }
		const_iterator begin() const { return const_iterator(l_.begin()); }

		iterator       end()       { return iterator(l_.end()); }
		const_iterator end() const { return const_iterator(l_.end()); }

	public:

		~ptr_list_t()
		{
			this->clear();
		}

	public:
	
		bool empty() const { return l_.empty(); }
		size_t size() const { return l_.size(); }

		value_t*       front()       { BOOST_ASSERT(!this->empty()); return &l_.front(); }
		value_t const* front() const { BOOST_ASSERT(!this->empty()); return &l_.front(); }

		value_t*       back()       { BOOST_ASSERT(!this->empty()); return &l_.back(); }
		value_t const* back() const { BOOST_ASSERT(!this->empty()); return &l_.back(); }

		value_move_ptr grab_front()
		{
			value_t *v = this->front();
			l_.pop_front();

			return value_move_ptr(v);
		}

		value_move_ptr grab_by_ptr(value_t *v)
		{
			l_.erase(l_.iterator_to(*v));
			return value_move_ptr(v);
		}

		void pop_front()
		{
			this->grab_front();
		}

		value_t* push_back(value_move_ptr v)
		{
			l_.push_back(*v);
			return v.release();
		}

		void append_chain(self_t& other)
		{
			l_.splice_after(l_.previous(l_.end()), other.l_);
		}

		void clear()
		{
			while (!this->empty())
				this->pop_front();
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__PTR_LIST_HPP_

