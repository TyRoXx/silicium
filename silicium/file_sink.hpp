#ifndef SILICIUM_FILE_SINK_HPP
#define SILICIUM_FILE_SINK_HPP

#include <silicium/sink.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/file_descriptor.hpp>
#include <silicium/memory_range.hpp>
#include <sys/uio.h>

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

	typedef fast_variant<flush, memory_range, seek_set, seek_add> file_sink_element;

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
			boost::optional<size_t> write_streak_length;
			size_t i = 0;
			auto flush_writes = [&, this]() -> error_type
			{
				if (!write_streak_length)
				{
					return error_type();
				}
				if (*write_streak_length == 1)
				{
					auto error = write_piece(*try_get_ptr<memory_range>(*(data.begin() + i - *write_streak_length)));
					write_streak_length = boost::none;
					return error;
				}
				else
				{
					auto error = write_vector(data.begin() + i - *write_streak_length, data.begin() + i);
					write_streak_length = boost::none;
					return error;
				}
			};
			for (; i < data.size(); ++i)
			{
				auto &element = data[i];
				int const max_streak_length = 1024;
				if (try_get_ptr<memory_range>(element))
				{
					if (write_streak_length)
					{
						++(*write_streak_length);
						if (*write_streak_length < max_streak_length)
						{
							continue;
						}
					}
					else
					{
						write_streak_length = 1;
						continue;
					}
				}

				error_type err = flush_writes();
				if (err)
				{
					return err;
				}

				err = append_one(element);
				if (err)
				{
					return err;
				}
			}
			return flush_writes();
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
				[this](seek_set const request) -> error_type
				{
					if (lseek64(m_destination, request.from_beginning, SEEK_SET) == static_cast<off_t>(-1))
					{
						return error_type(errno, boost::system::system_category());
					}
					return error_type();
				},
				[this](seek_add const request) -> error_type
				{
					if (lseek64(m_destination, request.from_current, SEEK_CUR) == static_cast<off_t>(-1))
					{
						return error_type(errno, boost::system::system_category());
					}
					return error_type();
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

		error_type write_vector(element_type const *begin, element_type const *end)
		{
			std::vector<iovec> vector(std::distance(begin, end));
			for (size_t i = 0; i < vector.size(); ++i, ++begin)
			{
				memory_range const &piece = *try_get_ptr<memory_range>(*begin);
				vector[i].iov_base = const_cast<void *>(static_cast<void const *>(piece.begin()));
				vector[i].iov_len = piece.size();
			}
			assert(vector.size() <= std::numeric_limits<int>::max());
			ssize_t rc = ::writev(m_destination, vector.data(), static_cast<int>(vector.size()));
			if (rc < 0)
			{
				return error_type(errno, boost::system::system_category());
			}
			//assume that everything has been written
			return error_type();
		}
	};
}

#endif
