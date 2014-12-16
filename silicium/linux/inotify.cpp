#include "inotify.hpp"
#include <silicium/exchange.hpp>
#include <boost/ref.hpp>

namespace Si
{
	namespace linux
	{
		inotify_observable::inotify_observable()
			: receiver_(nullptr)
		{
		}

		inotify_observable::inotify_observable(boost::asio::io_service &io)
			: notifier(boost::in_place(boost::ref(io)))
			, receiver_(nullptr)
		{
			int fd = inotify_init();
			if (fd < 0)
			{
				throw boost::system::system_error(errno, boost::system::posix_category);
			}
			try
			{
				notifier->assign(fd);
			}
			catch (...)
			{
				close(fd);
				throw;
			}
		}

		error_or<watch_descriptor> inotify_observable::watch(boost::filesystem::path const &target, boost::uint32_t mask)
		{
			assert(notifier);
			int const wd = inotify_add_watch(notifier->native_handle(), target.string().c_str(), mask);
			if (wd < 0)
			{
				return boost::system::error_code(errno, boost::system::posix_category);
			}
			return watch_descriptor(notifier->native_handle(), wd);
		}

		void inotify_observable::async_get_one(ptr_observer<observer<element_type>> receiver)
		{
			assert(!receiver_);
			std::size_t const min_buffer_size = sizeof(inotify_event) + NAME_MAX + 1;
			std::size_t const additional_buffer = 8192;
			read_buffer.resize(min_buffer_size + additional_buffer);
			assert(notifier);
			notifier->async_read_some(boost::asio::buffer(read_buffer), [this](boost::system::error_code error, std::size_t bytes_read)
			{
				if (error)
				{
					if (error == boost::asio::error::operation_aborted)
					{
						return;
					}
					throw std::logic_error("not implemented");
				}
				else
				{
					std::vector<file_notification> changes;
					for (std::size_t i = 0; i < bytes_read; )
					{
						inotify_event const &event = *reinterpret_cast<inotify_event const *>(read_buffer.data() + i);
						changes.emplace_back(file_notification{event.mask, boost::filesystem::path(event.name + 0, std::find(event.name + 0, event.name + event.len, '\0'))});
						i += sizeof(inotify_event);
						i += event.len;
					}
					Si::exchange(this->receiver_, nullptr)->got_element(std::move(changes));
				}
			});
			receiver_ = receiver.get();
		}
	}
}
