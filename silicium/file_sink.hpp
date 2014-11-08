#ifndef SILICIUM_FILE_SINK_HPP
#define SILICIUM_FILE_SINK_HPP

#include <silicium/sink.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/file_descriptor.hpp>
#include <silicium/memory_range.hpp>

namespace Si
{
	struct flush
	{
	};

	struct seek_set
	{
		boost::uint64_t from_beginning;
	};

	struct seek_add
	{
		boost::int64_t from_current;
	};

	typedef fast_variant<seek_set, seek_add> seek_request;
	typedef fast_variant<flush, memory_range, seek_request> file_sink_element;

	struct file_sink : sink<file_sink_element>
	{
		file_sink()
		{
		}

		explicit file_sink(native_file_handle destination)
			: m_destination(destination)
		{
		}

		virtual error_type append(boost::iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			for (auto &element : data)
			{
				error_type err = append_one(element);
				if (err)
				{
					return err;
				}
			}
			return error_type();
		}

	private:

		native_file_handle m_destination;

		error_type append_one(element_type const &element)
		{
			return visit<error_type>(
				element,
				[this](flush) -> error_type
				{
					if (fdatasync(m_destination) == 0)
					{
						return error_type();
					}
					return error_type(errno, boost::system::system_category());
				},
				[this](memory_range const &content) -> error_type
				{
					return write_piece(content);
				},
				[this](seek_request const &request) -> error_type
				{
					return seek(request);
				}
			);
		}

		error_type write_piece(memory_range const &content)
		{
			size_t written = 0;
			while (written < content.size())
			{
				ssize_t rc = ::write(m_destination, content.begin() + written, content.size() - written);
				if (rc < 0)
				{
					return error_type(errno, boost::system::system_category());
				}
				written += static_cast<size_t>(rc);
			}
			return error_type();
		}

		error_type seek(seek_request const &request)
		{
			return visit<error_type>(
				request,
				[this](seek_set set)
				{
					if (lseek64(m_destination, set.from_beginning, SEEK_SET) == static_cast<off_t>(-1))
					{
						return error_type(errno, boost::system::system_category());
					}
					return error_type();
				},
				[this](seek_add add)
				{
					if (lseek64(m_destination, add.from_current, SEEK_CUR) == static_cast<off_t>(-1))
					{
						return error_type(errno, boost::system::system_category());
					}
					return error_type();
				}
			);
		}
	};
}

#endif
