////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2012 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_SSL__OPENSSL_CONNECTION_HPP_
#define MEOW_LIBEV_SSL__OPENSSL_CONNECTION_HPP_

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <meow/str_ref.hpp>
#include <meow/std_unique_ptr.hpp>
#include <meow/tmp_buffer.hpp>
#include <meow/format/format.hpp>
#include <meow/format/inserter/hex_string.hpp>
#include <meow/libev/ssl/base_types.hpp>
#include <meow/libev/detail/generic_connection.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef SSL_CTX openssl_ctx_t;

	struct SSL_CTX_deleter_t {
		void operator()(openssl_ctx_t *ssl_ctx) { SSL_CTX_free(ssl_ctx); }
	};
	typedef std::unique_ptr<openssl_ctx_t, SSL_CTX_deleter_t> openssl_ctx_move_ptr;

	inline openssl_ctx_move_ptr openssl_ctx_create(SSL_METHOD const *method)
	{
		return openssl_ctx_move_ptr(SSL_CTX_new(method));
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef SSL_SESSION openssl_session_t;

	struct SSL_SESSION_deleter_t {
		void operator()(openssl_session_t *sess) { SSL_SESSION_free(sess); }
	};
	typedef std::unique_ptr<openssl_session_t, SSL_SESSION_deleter_t> openssl_session_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef SSL openssl_t;

	struct SSL_deleter_t {
		void operator()(openssl_t *ssl) { SSL_free(ssl); }
	};
	typedef std::unique_ptr<openssl_t, SSL_deleter_t> openssl_move_ptr;

	inline openssl_move_ptr openssl_create(openssl_ctx_t *ctx)
	{
		return openssl_move_ptr(SSL_new(ctx));
	}

	inline int openssl_last_error_code()
	{
		return ERR_get_error();
	}

	typedef meow::tmp_buffer<256> openssl_err_buf_t;
	inline str_ref openssl_get_error_string(int err_code, openssl_err_buf_t const& buf = openssl_err_buf_t())
	{
		format::char_buffer_sink_t sink(buf.get(), buf.size());
		format::write(sink, ERR_func_error_string(err_code), ": ", ERR_reason_error_string(err_code));
		return sink.used_part();
	}

	inline str_ref openssl_get_last_error_string(openssl_err_buf_t const& buf = openssl_err_buf_t())
	{
		return openssl_get_error_string(openssl_last_error_code(), buf);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	struct openssl_connection_t : public generic_connection_t
	{
		virtual ~openssl_connection_t() {}

		virtual void       ssl_init(openssl_ctx_t*) = 0;
		virtual bool       ssl_is_initialized() const = 0;
		virtual openssl_t* ssl_get() const = 0;

	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#include "../detail/generic_connection_impl.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct ssl_info_traits__default
	{
		// should return (only makes sense for successful handshakes)
		//  true  - continue looping and reading data
		//  false - break the loop, outer code has other plans for this connection
		template<class ContextT>
		static bool handshake_finished(ContextT *ctx, int err_code, std::string&& err_string)
		{
			return true; // i.e. rd_consume_status::more;
		}
	};

	template<class Traits>
	struct openssl_connection_repack_traits : public Traits
	{
		typedef openssl_ctx_t     ssl_ctx_t;
		typedef openssl_t         ssl_t;
		typedef openssl_move_ptr  ssl_move_ptr;

		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, ssl_log_writer, ssl_log_writer_traits__default);
		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, ssl_info, ssl_info_traits__default);

		// this shit needs to be defined here
		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, base, generic_connection_traits__base<Traits>);

		typedef typename Traits::read tr_read;

	public: // io context

		struct rw_context_t : public tr_read::context_t
		{
			buffer_move_ptr  ssl_rbuf;
			buffer_chain_t   ssl_wchain;
			ssl_move_ptr     rw_ssl;
			bool             ssl_handshake_done = false;
		};

	public: // helpers

		template<class ContextT>
		static ssl_t* ssl_from_context(ContextT *ctx)
		{
			return ctx->rw_ssl.get();
		}

	public: // connection-related static traits stuff

		struct virtuals
		{
			template<class ContextT>
			static bool has_buffers_to_send(ContextT *ctx)
			{
				return !(ctx->wchain_.empty() && ctx->ssl_wchain.empty());
			}
		};

		struct read
		{
			typedef rw_context_t context_t; // needed by generic_connection_impl_t
			static size_t const network_buffer_size = 16 * 1024;

			template<class ContextT>
			static buffer_ref get_buffer(ContextT *ctx)
			{
				if (NULL == ssl_from_context(ctx))
					return tr_read::get_buffer(ctx);

				buffer_move_ptr& b = ctx->ssl_rbuf;
				if (!b)
					b = create_buffer(network_buffer_size);

				return b->free_part();
			}

			template<class ContextT>
			static rd_consume_status_t consume_buffer(ContextT *ctx, buffer_ref read_part, read_status_t r_status)
			{
				ssl_t *ssl = ssl_from_context(ctx);

				if (NULL == ssl)
					return tr_read::consume_buffer(ctx, read_part, r_status);

				if (read_status::error == r_status)
					return tr_read::consume_buffer(ctx, buffer_ref(), r_status);

				if (read_status::again == r_status && read_part.empty())
					return rd_consume_status::loop_break;

				buffer_move_ptr& b = ctx->ssl_rbuf;
				b->advance_last(read_part.size());

				SSL_LOG_WRITE(ctx, line_mode::single, "{0}; {1} - {{ {2}, {3} }"
						, __func__, read_status::enum_as_str_ref(r_status)
						, read_part.size(), meow::format::as_hex_string(read_part));

				while (true)
				{
					buffer_ref data_buf = tr_read::get_buffer(ctx);

					if (!data_buf)
						return rd_consume_status::loop_break;

					int r = 0;
					if (ctx->ssl_handshake_done)
					{
						r = SSL_read(ssl, data_buf.data(), data_buf.size());
						SSL_LOG_WRITE(ctx, line_mode::prefix, "SSL_read({0}, {1}, {2}) = {3}", ctx, (void*)data_buf.data(), data_buf.size(), r);
					}
					else
					{
						r = SSL_do_handshake(ssl);
						SSL_LOG_WRITE(ctx, line_mode::prefix, "SSL_do_handshake({0}) = {1}", ctx, r);
					}

					if (r <= 0)
					{
						int const err_code = ERR_get_error();
						std::string err_string;
						if (err_code)
						{
							err_string = openssl_get_error_string(err_code).str();
							SSL_LOG_WRITE(ctx, line_mode::suffix, ", ssl_code: {0} - {1}", err_code, err_string);
						}
						else {
							SSL_LOG_WRITE(ctx, line_mode::suffix, "");
						}

						int ssl_code = SSL_get_error(ssl, r);
						switch (ssl_code)
						{
							case SSL_ERROR_ZERO_RETURN:
								SSL_shutdown(ssl);
								ssl_info::handshake_finished(ctx, err_code, move(err_string));
								return tr_read::consume_buffer(ctx, buffer_ref(), read_status::closed);

							case SSL_ERROR_WANT_READ:
							case SSL_ERROR_WANT_WRITE:
								if (read_status::closed == r_status)
								{
									SSL_shutdown(ssl);
									ssl_info::handshake_finished(ctx, err_code, move(err_string));
									return tr_read::consume_buffer(ctx, buffer_ref(), r_status);
								}
								else
									return rd_consume_status::loop_break;

							default:
								// no shutdown, session will be removed from cache
								ssl_info::handshake_finished(ctx, err_code, move(err_string));
								return tr_read::consume_buffer(ctx, buffer_ref(), read_status::error);
						}
					}
					else
					{
						if (ctx->ssl_handshake_done)
						{
							SSL_LOG_WRITE(ctx, line_mode::suffix, ""); // just a newline

							size_t const read_sz = r;
							read_status_t const rst = (read_sz == data_buf.size()) ? read_status::full : read_status::again;

							rd_consume_status_t rdc_status = tr_read::consume_buffer(ctx, buffer_ref(data_buf.data(), read_sz), rst);
							switch (rdc_status) {
								case rd_consume_status::more:
									continue;

								case rd_consume_status::loop_break:
								case rd_consume_status::closed:
									return rdc_status;
							}
						}
						else
						{
							if (r != 1) // openssl promises to return 1 when handshake has been completed, but fuck knows
							{
								SSL_LOG_WRITE(ctx, line_mode::middle, " WHOA (r != 1) as we expect from openssl!");
							}

							SSL_LOG_WRITE(ctx, line_mode::suffix, ""); // just a newline
							ctx->ssl_handshake_done = 1;

							bool const do_continue = ssl_info::handshake_finished(ctx, 0, std::string{});
							if (!do_continue)
								return rd_consume_status::loop_break;

							// continue the read loop, handshake done, might have some data to read from BIO still
							// i.e. might have the case with no data from the network (i.e. rd_consume_status::more would do nothing)
							// but there might still be data already read, but not yet processed from BIO,
							//  (since we're switching from handshake to read mode)
						}
					}
				}

				assert(!"can't be reached");
			}
		};

		struct write : public generic_connection_traits__write<base>
		{
			enum write_result_t { wr_closed, wr_error, wr_okay };

			template<class ContextT>
			static write_result_t write_buffer_to_ssl(ContextT *ctx, buffer_t *b)
			{
				ssl_t *ssl = ssl_from_context(ctx);

				assert(ctx->ssl_handshake_done && "must be impossible to write to ssl connection before handshake is complete");

				while (!b->empty())
				{
					str_ref const buf_data = b->used_part();

					SSL_LOG_WRITE(ctx, line_mode::single, "{0}: {{ {1}, {2} }"
							, __func__, buf_data.size(), meow::format::as_hex_string(buf_data));

					int r = SSL_write(ssl, buf_data.data(), buf_data.size());

					if (r <= 0)
					{
						int err_code = ERR_get_error();
						if (err_code)
							SSL_LOG_WRITE(ctx, line_mode::single, "SSL_write(): {0} - {1}", err_code, openssl_get_error_string(err_code));
						else
							SSL_LOG_WRITE(ctx, line_mode::suffix, "");

						int ssl_code = SSL_get_error(ssl, r);
						switch (ssl_code)
						{
							// can recieve ssl shutdown message while in the middle of a handshake
							case SSL_ERROR_ZERO_RETURN:
								SSL_shutdown(ssl);
								return wr_closed;

							// it's fine, the writer might want to read, in case we're in the middle of a handshake
							case SSL_ERROR_WANT_READ:
								return wr_okay;

							// should never happen, our write callback always accepts everything passed to it
							case SSL_ERROR_WANT_WRITE:
								return wr_okay;

							default:
								return wr_error;
						}
					}
					else
					{
						b->advance_first(r);
					}
				}

				return wr_okay;
			}

			template<class ContextT>
			static write_result_t move_wchain_buffers_from_to(ContextT *ctx, buffer_chain_t& from, buffer_chain_t *to)
			{
				// ssl not enabled
				if (NULL == ssl_from_context(ctx))
				{
					to->append_chain(from);
					return wr_okay;
				}

				// write as much as possible to ssl
				while (!from.empty())
				{
					buffer_t *b = from.front();
					write_result_t wr = write_buffer_to_ssl(ctx, b);

					if (wr_okay != wr)
						return wr;

					if (b->empty())
						from.pop_front();
					else
						break;
				}

				return wr_okay;
			}

			template<class ContextT>
			static wr_complete_status_t writev_bufs(ContextT *ctx)
			{
				write_result_t wr = move_wchain_buffers_from_to(ctx, ctx->wchain_, &ctx->ssl_wchain);

				switch (wr)
				{
					case wr_error:
						ctx->cb_write_closed(io_close_report(io_close_reason::custom_close));
						return wr_complete_status::closed;

					case wr_closed:
						ctx->cb_write_closed(io_close_report(io_close_reason::peer_close));
						return wr_complete_status::closed;

					case wr_okay:
					default:
						return write::writev_from_wchain(ctx, ctx->ssl_wchain);
				}

				assert(!"can't be reached");
			}
		};
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ConnectionT>
	struct openssl_connection_bio
	{
		using connection_traits = typename ConnectionT::connection_traits;
		using ssl_log_writer    = typename connection_traits::ssl_log_writer;

		static int bwrite(BIO *bio, char const *buf, int buf_sz)
		{
			auto *ctx = (ConnectionT*)bio->ptr;

			SSL_LOG_WRITE(ctx, line_mode::single, "{0}; buf: {{ {1}, {2} }", __func__, buf_sz, meow::format::as_hex_string(str_ref(buf, buf_sz)));

			if (buf_sz <= 0)
				return 0;

			buffer_move_ptr b = buffer_create_with_data(buf, buf_sz);
			ctx->ssl_wchain.push_back(move(b));

			return buf_sz;
		}

		static int bread(BIO *bio, char *buf, int buf_sz)
		{
			auto *ctx = (ConnectionT*)bio->ptr;

			SSL_LOG_WRITE(ctx, line_mode::prefix, "{0}; to buf_sz: {1} ", __func__, buf_sz);

			assert(buf_sz > 0);
			buffer_move_ptr& b = ctx->ssl_rbuf;

			if (!b)
			{
				SSL_LOG_WRITE(ctx, line_mode::suffix, "<-- WANT_READ [no buf]");
				BIO_set_retry_read(bio);
				return -1;
			}

			if (b->empty())
			{
				b->clear();
				SSL_LOG_WRITE(ctx, line_mode::suffix, "<-- WANT_READ [empty]");
				BIO_set_retry_read(bio);
				return -1;
			}

			BIO_clear_retry_flags(bio);

			size_t const sz = std::min((size_t)buf_sz, b->used_size());
			std::memcpy(buf, b->first, sz);
			b->advance_first(sz);

			SSL_LOG_WRITE(ctx, line_mode::suffix, "<-- {{ {0}, {1} }", sz, meow::format::as_hex_string(str_ref(buf, sz)));
			return sz;
		}

		static int bputs(BIO *bio, char const *str) { return -1; }
		static int bgets(BIO *bio, char *buf, int buf_sz) { return -1; }

		static int create(BIO *bio)
		{
			bio->init = 1;
			bio->shutdown = 1;
			bio->num = 0;
			bio->ptr = NULL;
			bio->flags = 0;
			return 1;
		}

		static int destroy(BIO *bio)
		{
			bio->init = 0;
			bio->ptr = NULL;
			return 1;
		}

		static long ctrl(BIO *bio, int cmd, long num, void *ptr)
		{
			switch (cmd) {
				case BIO_CTRL_FLUSH: return 1;
				default:             return 0;
			}
		}

	//	static long callback_ctrl(BIO *, int, bio_info_cb *) { return 0; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// extra functions to use with openssl connection final object

	template<class ConnectionT>
	openssl_t* openssl_connection_get_ssl(ConnectionT *conn)
	{
		return conn->rw_ssl.get();
	}

	template<class ConnectionT>
	bool openssl_connection_is_initialized(ConnectionT *conn)
	{
		return !!conn->rw_ssl;
	}

	template<class ConnectionT>
	void openssl_connection_init_acquire_ssl(ConnectionT *conn, openssl_move_ptr ssl)
	{
		using bio_t = openssl_connection_bio<ConnectionT>;

		// TODO: move this to openssl_connection_bio::construct() or something
		static BIO_METHOD openssl_connection_bio_method =
		{
			BIO_TYPE_SOURCE_SINK,
			"openssl_connection_bio",
			bio_t::bwrite,
			bio_t::bread,
			bio_t::bputs,
			bio_t::bgets,
			bio_t::ctrl,
			bio_t::create,
			bio_t::destroy,
			NULL
		};

		BIO *bio = BIO_new(&openssl_connection_bio_method);
		bio->ptr = conn;
		SSL_set_bio(ssl.get(), bio, bio);

		SSL_set_accept_state(ssl.get()); // TODO: maybe move this out as well

		conn->rw_ssl = move(ssl);
	}

	template<class ConnectionT>
	void openssl_connection_init_with_ctx(ConnectionT *conn, openssl_ctx_t *ssl_ctx)
	{
		// TODO: rebuild with meow::error_t
		openssl_move_ptr ssl = openssl_create(ssl_ctx);
		if (!ssl)
			throw std::logic_error("ssl_create() failed: make sure you've got certificate/private-key and memory");

		openssl_connection_init_acquire_ssl(conn, move(ssl));
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class Traits
		, class Interface = generic_connection_t
		>
	struct openssl_connection_impl_t
		: public generic_connection_impl_t<Interface, openssl_connection_repack_traits<Traits> >
	{
		typedef openssl_connection_impl_t                 connection_t;
		typedef openssl_connection_repack_traits<Traits>  connection_traits;

		// typedef openssl_ctx_t     ssl_ctx_t;
		// typedef openssl_move_ptr  ssl_move_ptr;

		typedef generic_connection_impl_t<Interface, connection_traits> impl_t;
		typedef typename impl_t::events_t                   events_t;
		// typedef typename connection_traits::ssl_log_writer  ssl_log_writer;

	public:

		openssl_connection_impl_t(evloop_t *loop, int fd, events_t *ev = NULL)
			: impl_t(loop, fd, ev)
		{
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_SSL__OPENSSL_CONNECTION_HPP_

