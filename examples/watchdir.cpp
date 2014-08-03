#include <reactive/enumerate.hpp>
#include <reactive/for_each.hpp>
#include <reactive/ref.hpp>
#ifdef _WIN32
#	include <reactive/win32/directory_changes.hpp>
#else
#	include <reactive/linux/inotify.hpp>
#endif
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/numeric.hpp>
#include <boost/detail/scoped_enum_emulation.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/optional.hpp>
#include <iostream>

namespace rx
{
	BOOST_SCOPED_ENUM_DECLARE_BEGIN(file_notification_type)
	{
		add,
		remove,
		change
	}
	BOOST_SCOPED_ENUM_DECLARE_END(file_notification_type)

	struct file_notification
	{
		file_notification_type type = file_notification_type::change;
		boost::filesystem::path name;

		file_notification()
		{
		}

		file_notification(file_notification_type type, boost::filesystem::path name)
			: type(type)
			, name(std::move(name))
		{
		}
	};

#ifdef _WIN32
	namespace win32
	{
		boost::optional<file_notification_type> to_portable_file_notification_type(DWORD action)
		{
			switch (action)
			{
			case FILE_ACTION_ADDED: return file_notification_type::add;
			case FILE_ACTION_REMOVED: return file_notification_type::remove;
			case FILE_ACTION_MODIFIED: return file_notification_type::change;
			default:
				return boost::none; //TODO
			}
		}

		boost::optional<rx::file_notification> to_portable_file_notification(win32::file_notification &&original)
		{
			auto const type = to_portable_file_notification_type(original.action);
			if (!type)
			{
				return boost::none;
			}
			return rx::file_notification(*type, std::move(original.name));
		}
	}

	template <class Element, class Input, class Transformation>
	struct conditional_transformer
		: public observable<Element>
		, private observer<typename Input::element_type>
	{
		typedef Element element_type;

		conditional_transformer()
		{
		}

		explicit conditional_transformer(Input original, Transformation transform)
			: original(std::move(original))
			, transform(std::move(transform))
		{
		}

#ifdef _MSC_VER
		conditional_transformer(conditional_transformer &&other)
			: original(std::move(other.original))
			, transform(std::move(other.transform))
			, receiver_(std::move(other.receiver_))
		{
		}

		conditional_transformer &operator = (conditional_transformer &&other)
		{
			original = std:move(other.original);
			transform = std:move(other.transform);
			receiver_ = std:move(other.receiver_);
			return *this;
		}
#endif

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			fetch();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("to do");
		}

	private:

		Input original;
		Transformation transform;
		observer<element_type> *receiver_ = nullptr;

		BOOST_DELETED_FUNCTION(conditional_transformer(conditional_transformer const &));
		BOOST_DELETED_FUNCTION(conditional_transformer &operator = (conditional_transformer const &));

		void fetch()
		{
			original.async_get_one(*this);
		}

		virtual void got_element(typename Input::element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			auto converted = transform(std::move(value));
			if (converted)
			{
				rx::exchange(receiver_, nullptr)->got_element(std::move(*converted));
			}
			else
			{
				fetch();
			}
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			SILICIUM_UNREACHABLE();
		}
	};

	template <class Element, class Input, class Transformation>
	auto transform_if_initialized(Input &&input, Transformation &&transform) -> conditional_transformer<Element, typename std::decay<Input>::type, typename std::decay<Transformation>::type>
	{
		return conditional_transformer<Element, typename std::decay<Input>::type, typename std::decay<Transformation>::type>(
			std::forward<Input>(input),
			std::forward<Transformation>(transform));
	}
#endif
}

namespace
{
#ifndef _WIN32
	std::string inotify_mask_to_string(boost::uint32_t mask)
	{
		using flag_entry = std::pair<boost::uint32_t, std::string>;
		static std::array<flag_entry, 12> const flag_names
		{{
			{IN_ACCESS, "access"},
			{IN_MODIFY, "modify"},
			{IN_ATTRIB, "attrib"},
			{IN_CLOSE_WRITE, "close_write"},
			{IN_CLOSE_NOWRITE, "close_nowrite"},
			{IN_OPEN, "open"},
			{IN_MOVED_FROM, "moved_from"},
			{IN_MOVED_TO, "moved_to"},
			{IN_CREATE, "create"},
			{IN_DELETE, "delete"},
			{IN_DELETE_SELF, "delete_self"},
			{IN_MOVE_SELF, "move_self"}
		}};
		return boost::accumulate(flag_names | boost::adaptors::filtered([mask](flag_entry const &entry)
		{
			return (entry.first & mask) == entry.first;
		}), std::string(), [](std::string left, flag_entry const &flag)
		{
			if (!left.empty())
			{
				left += "|";
			}
			left += flag.second;
			return left;
		});
	}
#endif
}

int main()
{
	boost::asio::io_service io;

	auto const watched_dir = boost::filesystem::current_path();
	std::cerr << "Watching " << watched_dir << '\n';

#ifdef _WIN32
	auto notifier = rx::transform_if_initialized<rx::file_notification>(rx::enumerate(rx::win32::directory_changes(watched_dir, true)), rx::win32::to_portable_file_notification);
	auto all = rx::for_each(rx::ref(notifier), [](rx::file_notification const &event)
	{
		std::cerr << boost::underlying_cast<int>(event.type) << " " << event.name << '\n';
	});
	boost::asio::io_service::work keep_running(io);
#else
	rx::linux::inotify_observable notifier(io);
	auto w = notifier.watch(watched_dir, IN_ALL_EVENTS);
	auto all = rx::for_each(rx::enumerate(rx::ref(notifier)), [](rx::linux::file_notification const &event)
	{
		std::cerr << inotify_mask_to_string(event.mask) << " " << event.name << '\n';
	});
#endif
	all.start();

	io.run();
}
