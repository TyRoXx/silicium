#ifndef SILICIUM_FILE_SINK_HPP
#define SILICIUM_FILE_SINK_HPP

#include <silicium/native_file_descriptor.hpp>
#include <silicium/write.hpp>

namespace Si
{
	struct file_sink
	{
		typedef char element_type;
		typedef boost::system::error_code error_type;

		explicit file_sink(native_file_descriptor file)
		    : m_file(file)
		{
		}

		error_type append(iterator_range<element_type const *> data)
		{
			return write(m_file, data).error();
		}

	private:
		native_file_descriptor m_file;
	};
}

#endif
