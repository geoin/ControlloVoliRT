#ifndef FASTPARSER_H
#define FASTPARSER_H

#include <vector>
#include <string>

class FastParser {
public:
	FastParser();
	FastParser(const std::string& mes, const char* sep = NULL);
	int Parse(const std::string& mes);
	const std::string& GetString(int i) const;
	int GetInt(int i) const;
	double GetFloat(int i ) const;
private:
	inline bool _is_valid(int index) const {
		return index >= 0 && index < (int) _vtok.size();
	}
	void _init(const char* sep = NULL);
	void _tokenize(const char* mes);
	double _atof(const char* p) const;
	mutable std::string _dum;
	std::string _sep;
	std::vector<std::string> _vtok; // vettore con gli elementi parserizzati
};
#endif
