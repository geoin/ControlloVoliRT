#include <time.h>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include "sampler.h"

namespace Geoin {
namespace Util {
			
	size_t Sampler::_rnd_generate(size_t min, size_t max) {
		return  min + (size_t) (((float)std::rand() / RAND_MAX) * ( max - min)) ;
	}

	Sampler::Sampler(): _group_count(0), _total_count(0) {}
			
	Sampler::Sampler(size_t max_count): _group_count(0), _total_count(0) {
		_vs.resize(max_count);
	}

	void Sampler::sample(size_t group_count, size_t total_count ) {
		if (total_count > _vs.size()) {
			throw std::runtime_error("total count exceded max size");
		}
		
		_group_count = group_count;
		_total_count = total_count;
		
		if (group_count > total_count) {
			throw std::runtime_error("group count exceded total count");
		}

		srand((unsigned int) time(NULL));
		for (size_t i = 0; i < _vs.size(); i++) {
			_vs[i] = i;	
		}				
		size_t cur_bound = total_count - 1;
		for (size_t cur_count = 0; cur_count < group_count - 1; cur_count++, cur_bound--) {
			size_t k = _rnd_generate(0, cur_bound);
			std::swap(_vs[cur_bound], _vs[k]);
		}
	}
			
	Sampler::iterator Sampler::begin(){
		return _vs.begin() + (_total_count - _group_count);	
	}

	Sampler::const_iterator Sampler::begin() const{
		return _vs.begin() + (_total_count - _group_count);		
	}

	Sampler::iterator Sampler::end(){
		return _vs.begin() + _total_count;
	}

	Sampler::const_iterator Sampler::end() const{
		return _vs.begin() + _total_count;
	}

}}
