////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_IMPL_HPP_

#include <stdexcept>

#include <meow/unix/fcntl.hpp>
#include <meow/utility/nested_name_alias.hpp>

#include <meow/libev/libev.hpp>
#include <meow/libev/io_machine.hpp>
#include <meow/libev/detail/generic_connection.hpp>
#include <meow/libev/detail/generic_connection_traits.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class Interface
		, class Traits
		, class EventsT = typename Interface::events_t
		>
	struct generic_connection_impl_t
		: public Interface
		, public Traits::read::context_t
	{
		typedef generic_connection_impl_t 		self_t;
		typedef generic_connection_impl_t 		base_t; // macro at the bottom uses it
		typedef EventsT							events_t;

		typedef generic_connection_traits<self_t, Traits> traits_t;
		typedef libev::io_machine_t<self_t, traits_t> iomachine_t;

	public:
		struct option_automatic_startup_io_default { enum { value = true }; };
		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, option_automatic_startup_io, option_automatic_startup_io_default);

	public:
		struct option_automatic_set_nonblocking_default { enum { value = true }; };
		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, option_automatic_set_nonblocking, option_automatic_set_nonblocking_default);

	public:
		// traits need access to this stuff
		//  not all of it, but to avoid padding between members, we just have them in one place

		evloop_t 		*loop_;
		events_t 		*ev_;
		io_context_t 	io_ctx_;
		buffer_chain_t 	wchain_;

	public: // callbacks, for the traits

		void cb_read_closed(io_close_report_t const& r)	{ ev_->on_closed(this, r); }
		void cb_write_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }
		void cb_custom_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }
		void cb_sync_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }

	public:

		generic_connection_impl_t(evloop_t *loop, int fd, events_t *ev)
			: loop_(loop)
			, ev_(ev)
			, io_ctx_(fd)
		{
			if (option_automatic_startup_io::value)
				this->io_startup();
		}

		~generic_connection_impl_t()
		{
			this->io_shutdown();
		}

	public:

		virtual int fd() const { return io_ctx_.fd(); }
		virtual evloop_t* loop() const { return loop_; }

		virtual evio_t*       io_event() { return io_context()->event(); }
		virtual io_context_t* io_context() { return &io_ctx_; }

		virtual events_t* ev() const { return ev_; }
		virtual void set_ev(events_t *ev) { ev_ = ev; }

	public:

		virtual void io_startup()
		{
			if (this->flags->io_started)
				return;

			if (option_automatic_set_nonblocking::value)
				os_unix::nonblocking(fd());

			iomachine_t::prepare_context(this);
			this->flags->io_started = true;
		}

		virtual void io_shutdown()
		{
			if (!this->flags->io_started)
				return;

			iomachine_t::release_context(this);
			this->flags->io_started = false;
		}

		virtual void io_reset()
		{
			io_shutdown();
			io_ctx_.reset_fd();
		}

		virtual void run_loop(int revents)
		{
			BOOST_ASSERT(this->flags->io_started && "call io_startup() first");
			iomachine_t::run_loop(this, revents);
		}

		virtual void activate(int revents)
		{
			BOOST_ASSERT(this->flags->io_started && "call io_startup() first");
			iomachine_t::activate_context(this, revents);
		}

	public:

		virtual void queue_buf(buffer_move_ptr buf)
		{
			if (!buf || buf->empty())
				return;

			wchain_.push_back(move(buf));
		}

		virtual void queue_chain(buffer_chain_t& chain)
		{
			wchain_.append_chain(chain);
		}

		virtual void send(buffer_move_ptr buf)
		{
			if (!buf || buf->empty())
				return;

			this->queue_buf(move(buf));
			this->w_activate();
		}

		virtual void send_chain(buffer_chain_t& chain)
		{
			this->queue_chain(chain);
			this->w_activate();
		}

	public:

		virtual buffer_chain_t& wchain_ref() { return wchain_; }

	public: // closing

		virtual void close_after_write()
		{
			this->flags->is_closing = true;
			this->flags->write_before_close = true;
			this->activate(EV_WRITE /* don't add EV_CUSTOM here, it will cause a close immediately */);
		}

		virtual void close_immediately()
		{
			this->flags->is_closing = true;
			this->flags->write_before_close = false;
			this->activate(EV_CUSTOM);
		}

		virtual void close_syncronously()
		{
			this->cb_sync_closed(io_close_report(io_close_reason::sync_close));
		}
	};

//
// defines a connection wrapper like mmc_connection_t
// example: 
//  MEOW_LIBEV_DEFINE_CONNECTION_WRAPPER(mmc_connection_impl_t, mmc_connection_repack_traits, mmc_connection_t);
//
#define MEOW_LIBEV_DEFINE_CONNECTION_WRAPPER(name, traits, base_interface) 	\
	template<class Traits, class InterfaceT = base_interface >				\
	struct name 															\
		: public generic_connection_impl_t<									\
					  InterfaceT											\
					, traits<Traits>										\
					, typename base_interface::events_t						\
					>														\
	{																		\
		name( 																\
			  evloop_t *loop 												\
			, int fd 														\
			, typename base_interface::events_t *ev = NULL 					\
			)																\
			: name::base_t(loop, fd, ev)									\
		{																	\
		}																	\
	};																		\
/**/

// used from inside connection traits to call callback through connection->events
#define MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, cb_name, args...) \
	ctx->ev()->cb_name(ctx, args)

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_IMPL_HPP_

