#ifndef SAMPLER_H
#define SAMPLER_H

#include <vector>

namespace Geoin {
	namespace Util {
		class Sampler {
		public:
			typedef std::vector<size_t>::iterator iterator;
			typedef std::vector<size_t>::const_iterator const_iterator;

			Sampler();
			Sampler(size_t max_count);
			void sample(size_t group_count, size_t total_count); // group_count #of el to extract, total_count size of original block
			
			iterator begin();
			const_iterator begin() const;
			iterator end();
			const_iterator end() const;

		private:
			static size_t _rnd_generate( size_t min, size_t max );
			std::vector<size_t> _vs;
			size_t _group_count, _total_count;
		};
	}
}
#endif