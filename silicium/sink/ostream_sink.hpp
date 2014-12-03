#ifndef SILICIUM_OSTREAM_SINK_HPP
#define SILICIUM_OSTREAM_SINK_HPP

#include <silicium/sink/virtualized_sink.hpp>

namespace Si
{
	struct ostream_ref_sink
	{
		ostream_ref_sink()
			: m_file(nullptr)
		{
		}

		explicit ostream_ref_sink(std::ostream &file)
		   : m_file(&file)
		{
		}

		void append(iterator_range<char const *> data)
		{
			assert(m_file);
			m_file->write(data.begin(), data.size());
		}

	private:

		std::ostream *m_file;
	};

	struct ostream_sink
	{
		typedef char element_type;
		typedef boost::system::error_code error_type;

		//unique_ptr to make ostreams movable
		explicit ostream_sink(std::unique_ptr<std::ostream> file)
			: m_file(std::move(file))
		{
			m_file->exceptions(std::ios::failbit | std::ios::badbit);
		}

		boost::system::error_code append(iterator_range<char const *> data)
		{
			m_file->write(data.begin(), data.size());
			return {};
		}

	private:

		std::unique_ptr<std::ostream> m_file;
	};

	inline std::unique_ptr<sink<char, boost::system::error_code>> make_file_sink(boost::filesystem::path const &name)
	{
		std::unique_ptr<std::ostream> file(new std::ofstream(name.string(), std::ios::binary));
		if (!*file)
		{
			throw std::runtime_error("Cannot open file for writing: " + name.string());
		}
		return to_unique(virtualize_sink(ostream_sink(std::move(file))));
	}
}

#endif
