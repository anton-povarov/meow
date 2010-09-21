////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_

#include <meow/utility/offsetof.hpp> 	// for MEOW_SELF_FROM_MEMBER

#include <meow/buffer.hpp>
#include <meow/buffer_chain.hpp>

#include <meow/format/format.hpp> 		// FMT_TEMPLATE_PARAMS, etc.
#include <meow/format/format_tmp.hpp>

#include <meow/libev/libev.hpp>
#include <meow/libev/io_context.hpp>
#include <meow/libev/io_close_report.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT>
	struct generic_connection_traits_base
	{
		typedef ContextT context_t;

		static evloop_t* ev_loop(context_t *ctx)
		{
			return ctx->loop_;
		}

		static io_context_t* io_context_ptr(context_t *ctx)
		{
			return &ctx->io_;
		}

		static context_t* context_from_io(libev::io_context_t *io_ctx)
		{
			return MEOW_SELF_FROM_MEMBER(context_t, io_, io_ctx);
		}

		static int io_allowed_ops(context_t *ctx)
		{
			return EV_READ | EV_WRITE | EV_CUSTOM;
		}

		static bool log_is_allowed(context_t *ctx)
		{
			return ctx->cb_log_is_allowed();
		}

		#define DEFINE_CONNECTION_TRAITS_FMT_FUNCTION(z, n, d) 							\
			template<class F FMT_TEMPLATE_PARAMS(n)> 									\
			static void log_message( 													\
					  context_t *ctx 													\
					, line_mode_t lmode 												\
					, F const& fmt 														\
					  FMT_DEF_PARAMS(n)) 												\
			{ 																			\
				ctx->cb_log_debug(lmode, format::fmt_tmp<512>(fmt FMT_CALL_SITE_ARGS(n))); 	\
			} 																			\
		/**/

		BOOST_PP_REPEAT(32, DEFINE_CONNECTION_TRAITS_FMT_FUNCTION, _);
	};

	template<class BaseTraits>
	struct generic_connection_traits_write
	{

		template<class ContextT>
		static buffer_ref write_get_buffer(ContextT *ctx)
		{
			buffer_chain_t& wchain = ctx->wchain_;

			if (wchain.empty())
				return buffer_ref();

			return wchain.front()->used_part();
		}

		template<class ContextT>
		static wr_complete_status_t write_complete(ContextT *ctx, buffer_ref written_br, write_status_t w_status)
		{
			if (write_status::error == w_status)
			{
				ctx->cb_write_closed(io_close_report(io_close_reason::io_error, errno));
				return wr_complete_status::closed;
			}
			if (write_status::closed == w_status)
			{
				ctx->cb_write_closed(io_close_report(io_close_reason::peer_close));
				return wr_complete_status::closed;
			}

			buffer_chain_t& wchain = ctx->wchain_;

			// check if we did actualy write something
			if (!written_br.empty())
			{
				buffer_t *b = wchain.front();
				b->advance_first(written_br.size());

				if (0 == b->used_size())
					wchain.pop_front();
			}
			else
			{
				BOOST_ASSERT(write_status::again == w_status);
			}

			if (wchain.empty())
			{
				if (ctx->cb_is_closing_after_write())
				{
					ctx->cb_write_closed(io_close_report(io_close_reason::write_close));
					return wr_complete_status::closed;
				}
				return wr_complete_status::finished;
			}

			return wr_complete_status::more;
		}
	};

	struct generic_connection_traits_custom_op
	{
		template<class ContextT>
		static bool requires_custom_op(ContextT *ctx)
		{
			return ctx->cb_is_closing_immediately();
		}

		template<class ContextT>
		static custom_op_status_t custom_operation(ContextT *ctx)
		{
			BOOST_ASSERT(ctx->cb_is_closing_immediately());
			ctx->cb_custom_closed(io_close_report(io_close_reason::custom_close));
			ctx->close_->immediately = false;

			return custom_op_status::closed;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_

