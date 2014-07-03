#include <reactive/bridge.hpp>
#include <reactive/ptr_observable.hpp>
#include <reactive/tuple.hpp>
#include <reactive/transform.hpp>
#include <reactive/while.hpp>
#include <reactive/finite_state_machine.hpp>
#include <reactive/filter.hpp>
#include <reactive/generate.hpp>
#include <reactive/total_consumer.hpp>
#include <reactive/delay.hpp>
#include <SDL2/SDL.h>
#include <boost/system/system_error.hpp>
#include <boost/scope_exit.hpp>
#include <boost/variant.hpp>
#include <boost/asio.hpp>

namespace rx
{
	typedef std::shared_ptr<boost::asio::ip::tcp::socket> shared_socket;
	typedef boost::variant<shared_socket, boost::system::error_code> connector_element;

	template <class SharedSocketFactory, class EndpointObservable>
	struct connector
			: public observable<connector_element>
			, private observer<boost::asio::ip::tcp::endpoint>
	{
		typedef connector_element element_type;

		explicit connector(SharedSocketFactory create_socket, EndpointObservable destination)
			: impl_(std::make_shared<impl>())
		{
			impl_->create_socket = std::move(create_socket);
			impl_->destination = std::move(destination);
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!impl_->receiver_);
			impl_->receiver_ = &receiver;
			return impl_->destination.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(impl_->receiver_);
			if (impl_->connecting)
			{
				impl_->connecting->cancel();
			}
			else
			{
				impl_->destination.cancel();
			}
		}

	private:

		struct impl
		{
			SharedSocketFactory create_socket;
			EndpointObservable destination;
			observer<element_type> *receiver_ = nullptr;
			shared_socket connecting;
		};

		std::shared_ptr<impl> impl_;

		virtual void got_element(boost::asio::ip::tcp::endpoint value) SILICIUM_OVERRIDE
		{
			impl_->fetching_destination = false;
			auto socket = impl_->create_socket();
			std::weak_ptr<impl> weak_impl = impl_;
			socket->async_connect(value, [weak_impl, socket](boost::system::error_code error)
			{
				auto const impl_locked = weak_impl.lock();
				if (!impl_locked)
				{
					return;
				}
				connector_element result;
				if (error)
				{
					if (error == boost::asio::error::operation_aborted)
					{
						return;
					}
					result = error;
				}
				else
				{
					result = socket;
				}
				exchange(impl_locked->receiver_, nullptr)->got_element(std::move(result));
			});
			impl_->connecting = socket;
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			impl_->fetching_destination = false;
			exchange(impl_->receiver_, nullptr)->ended();
		}
	};

	struct socket_receiver : observable<char>
	{
		typedef char element_type;

		explicit socket_receiver(shared_socket socket)
			: impl_(std::make_shared<impl>())
		{
			impl_->socket = std::move(socket);
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!impl_->receiver_);
			impl_->receiver_ = &receiver;
			return start();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(impl_->receiver_);
			impl_->socket->cancel();
			impl_->receiver_ = nullptr;
		}

	private:

		struct impl
		{
			shared_socket socket;
			observer<element_type> *receiver_ = nullptr;
			char buffer = 0;
		};
		std::shared_ptr<impl> impl_;

		void start()
		{
			std::weak_ptr<impl> weak_impl = impl_;
			impl_->socket->async_receive(
				boost::asio::buffer(&impl_->buffer, 1),
				[weak_impl](boost::system::error_code error, std::size_t bytes_transferred)
			{
				auto locked_impl = weak_impl.lock();
				if (!locked_impl)
				{
					return;
				}
				if (error)
				{
					if (error == boost::asio::error::operation_aborted)
					{
						//was cancelled
						return;
					}
					exchange(locked_impl->receiver_, nullptr)->ended(); //TODO proper error handling?
				}
				else
				{
					assert(bytes_transferred == 1);
					exchange(locked_impl->receiver_, nullptr)->got_element(locked_impl->buffer);
				}
			});
		}
	};
}

namespace
{
	void throw_error()
	{
		throw std::runtime_error(std::string("SDL error :") + SDL_GetError());
	}

	void check_sdl(int rc)
	{
		if (rc < 0)
		{
			throw_error();
		}
	}

	struct window_destructor
	{
		void operator()(SDL_Window *w) const
		{
			SDL_DestroyWindow(w);
		}
	};

	struct renderer_destructor
	{
		void operator()(SDL_Renderer *r) const
		{
			SDL_DestroyRenderer(r);
		}
	};

	void set_render_draw_color(SDL_Renderer &renderer, SDL_Color color)
	{
		SDL_SetRenderDrawColor(&renderer, color.r, color.g, color.b, color.a);
	}

	SDL_Color make_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	{
		SDL_Color result;
		result.r = red;
		result.g = green;
		result.b = blue;
		result.a = alpha;
		return result;
	}

	SDL_Rect make_rect(int x, int y, int w, int h)
	{
		SDL_Rect result;
		result.x = x;
		result.y = y;
		result.w = w;
		result.h = h;
		return result;
	}

	struct draw_filled_rect
	{
		SDL_Color color;
		SDL_Rect where;
	};

	typedef boost::variant<
		draw_filled_rect
	> draw_operation;

	struct frame
	{
		std::vector<draw_operation> operations;
	};

	struct draw_operation_renderer : boost::static_visitor<>
	{
		explicit draw_operation_renderer(SDL_Renderer &sdl)
			: sdl(&sdl)
		{
		}

		void operator()(draw_filled_rect const &operation) const
		{
			set_render_draw_color(*sdl, operation.color);
			SDL_RenderDrawRect(sdl, &operation.where);
		}

	private:

		SDL_Renderer *sdl;
	};

	void render_frame(SDL_Renderer &sdl, frame const &frame_)
	{
		set_render_draw_color(sdl, make_color(0, 0, 0, 0xff));
		SDL_RenderClear(&sdl);
		for (auto const &operation : frame_.operations)
		{
			boost::apply_visitor(draw_operation_renderer{sdl}, operation);
		}
		SDL_RenderPresent(&sdl);
	}

	struct game_state
	{
		int x = 100;
		int y = 100;
		int count = 1;
	};

	int const max_rect_count = 5;

	frame draw_game_state(game_state const &state)
	{
		std::vector<draw_operation> operations;
		for (int i = 0; i < state.count; ++i)
		{
			int const max_width = 80;
			int const single_offset = (max_width / 2) / max_rect_count;
			int const total_offset = single_offset * i;
			int const width = max_width - total_offset * 2;
			operations.emplace_back(draw_filled_rect{make_color(0xff, 0x00, 0x00, 0xff), make_rect(state.x + total_offset, state.y + total_offset, width, width)});
		}
		return frame
		{
			std::move(operations)
		};
	}

	game_state step_game_state(game_state previous, SDL_Event event_)
	{
		switch (event_.type)
		{
		case SDL_KEYUP:
			{
				switch (event_.key.keysym.sym)
				{
				case SDLK_LEFT:
					previous.x -= 10;
					break;

				case SDLK_RIGHT:
					previous.x += 10;
					break;

				case SDLK_UP:
					previous.y -= 10;
					break;

				case SDLK_DOWN:
					previous.y += 10;
					break;

				case SDLK_PLUS:
					previous.count = std::min(max_rect_count, previous.count + 1);
					break;

				case SDLK_MINUS:
					previous.count = std::max(1, previous.count - 1);
					break;
				}
				break;
			}
		}
		return previous;
	}

	template <class Events>
	auto make_frames(Events &&input)
	{
		game_state initial_state;
		auto is_no_quit = [](SDL_Event const &event_) -> bool
		{
			switch (event_.type)
			{
			case SDL_QUIT:
				return false;

			case SDL_KEYUP:
				return (event_.key.keysym.sym != SDLK_ESCAPE);
			}
			return true;
		};
		auto model = rx::make_finite_state_machine(rx::while_(std::forward<Events>(input), is_no_quit), initial_state, step_game_state);
		return rx::transform(std::move(model), draw_game_state);
	}

	struct nothing
	{
	};
}

int main()
{
	check_sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));
	BOOST_SCOPE_EXIT(void)
	{
		SDL_Quit();
	}
	BOOST_SCOPE_EXIT_END;

	std::unique_ptr<SDL_Window, window_destructor> window(SDL_CreateWindow("Silicium react.cpp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0));
	if (!window)
	{
		throw_error();
		return 1;
	}

	std::unique_ptr<SDL_Renderer, renderer_destructor> renderer(SDL_CreateRenderer(window.get(), -1, 0));

	rx::bridge<SDL_Event> frame_events;
	auto frames = rx::wrap<frame>(make_frames(rx::ref(frame_events)));

	boost::asio::io_service io;

	auto input_polled = rx::generate([&frame_events]
	{
		SDL_Event event;
		while (frame_events.is_waiting() &&
			   SDL_PollEvent(&event))
		{
			frame_events.got_element(event);
		}
		return nothing{};
	});
	auto delayed_input_polled = rx::delay(input_polled, io, std::chrono::milliseconds(16));
	auto rendered = rx::transform(
		rx::make_tuple(
			delayed_input_polled,
			frames),
		[&renderer](std::tuple<nothing, frame> const &ready_frame)
	{
		render_frame(*renderer, std::get<1>(ready_frame));
		return nothing{};
	});

	auto all_rendered = rx::make_total_consumer(rx::ref(rendered));
	all_rendered.start();

	io.run();
}
