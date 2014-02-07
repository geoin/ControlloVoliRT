#include "fastparser.h"
#include <vector>

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

FastParser::FastParser() {
	_init();
}
FastParser::FastParser(const std::string& mes, const char* sep)
{
	_init(sep);
	_tokenize(mes.c_str());
}
int FastParser::Parse(const std::string& mes) 
{
	_vtok.clear();
	_tokenize(mes.c_str());
	return _vtok.size();
}
const std::string& FastParser::GetString(int i) const
{
	if ( _is_valid(i) )
		return _vtok[i];
	return _dum;
}
int FastParser::GetInt(int i) const 
{
	if ( _is_valid(i) )
		return atoi(_vtok[i].c_str());
	return 0;
}
double FastParser::GetFloat(int i ) const 
{
	if ( _is_valid(i) )
		return atof(_vtok[i].c_str());
	return 0.;
}
void FastParser::_init(const char* sep)
{
	if ( sep == NULL )
		_sep = ",;\t |";
	else
		_sep = sep;
	_vtok.clear();
}
void FastParser::_tokenize(const char* mes)
{
	char* ch = new char[strlen(mes) + 1];
	strcpy(ch, mes);
	char* p = ch;
	char* cont;
	const char* d = _sep.c_str();
	while ( (p = strtok_s(p, d, &cont)) ) {
		_vtok.push_back(p);
		p = NULL;
	}
	free(ch);
}
double FastParser::_atof(const char* p) const 
{

	int frac;
	double sign, value, scale;

	// Skip leading white space, if any.

	while (white_space(*p) ) {
		p += 1;
	}

	// Get sign, if any.

	sign = 1.0;
	if (*p == '-') {
		sign = -1.0;
		p += 1;

	} else if (*p == '+') {
		p += 1;
	}

	// Get digits before decimal point or exponent, if any.

	for (value = 0.0; valid_digit(*p); p += 1) {
		value = value * 10.0 + (*p - '0');
	}

	// Get digits after decimal point, if any.

	if (*p == '.') {
		double pow10 = 10.0;
		p += 1;
		while (valid_digit(*p)) {
			value += (*p - '0') / pow10;
			pow10 *= 10.0;
			p += 1;
		}
	}

	// Handle exponent, if any.

	frac = 0;
	scale = 1.0;
	if ((*p == 'e') || (*p == 'E')) {
		unsigned int expon;

		// Get sign of exponent, if any.

		p += 1;
		if (*p == '-') {
			frac = 1;
			p += 1;

		} else if (*p == '+') {
			p += 1;
		}

		// Get digits of exponent, if any.

		for (expon = 0; valid_digit(*p); p += 1) {
			expon = expon * 10 + (*p - '0');
		}
		if (expon > 308) expon = 308;

		// Calculate scaling factor.

		while (expon >= 50) { scale *= 1E50; expon -= 50; }
		while (expon >=  8) { scale *= 1E8;  expon -=  8; }
		while (expon >   0) { scale *= 10.0; expon -=  1; }
	}

	// Return signed and scaled floating point result.

	return sign * (frac ? (value / scale) : (value * scale));
}
