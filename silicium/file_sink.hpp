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

	typedef fast_variant<flush, memory_range> file_sink_element;

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
	};
}

#endif
