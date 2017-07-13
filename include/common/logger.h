#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <sstream>
#include <iomanip>

class Logger {
public:
	Logger(): _isOpen(false) {}
        Logger(const std::string& nome, bool trunc = true) {
		Init(nome, trunc);
	}
	~Logger(void) {
		Close();
	}
	void Init(const std::string& nome, bool trunc = false) {
		Close();
		std::ios_base::openmode mode = std::ios_base::out;
		if ( !trunc ) {
			mode |= std::ios_base::app;
		}
		_fstream.open(nome.c_str(), mode);
		_isOpen = _fstream.is_open();
	}
	void Close(void) {
		if ( _fstream.is_open() )
			_fstream.close();
	}

        typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

        // this is the function signature of std::endl
        typedef CoutType& (*StandardEndLine)(CoutType&);
        Logger& operator<<(StandardEndLine manip)
        {
            std::ostringstream st;
            st << manip;
            _log(st.str());
            return *this;
        }

	Logger &operator<<(const char* str) {
		std::string sts(str);
		_log(sts);
		return *this;
	}
	Logger &operator<<(const long lv) {
		std::ostringstream st;
		st << lv;
		_log(st.str());
		return *this;
	}
        Logger &operator<<(const size_t lv) {
                std::ostringstream st;
                st << lv;
                _log(st.str());
                return *this;
        }
	Logger &operator<<(const int lv) {
		std::ostringstream st;
		st << lv;
		_log(st.str());
		return *this;
	}
	Logger &operator<<(const unsigned int lv) {
		std::ostringstream st;
		st << lv;
		_log(st.str());
		return *this;
	}
	Logger &operator<<(const double dv) {
		std::ostringstream st;
                st << std::fixed << dv;
		_log(st.str());
		return *this;
	}
        static std::string set(const double d, int nd = 3) {
		std::ostringstream st;
                st << std::fixed << std::setprecision(nd) << d;
		std::string str = st.str();
		return st.str();
	}
	Logger &operator<<(const Logger&) {
		return *this;
	}
	Logger &operator<<(const std::string str) {
		_log(str);
		return *this;
	}
	static std::string lf(void) {
		return std::string("\n");
	}
	static std::string set(const int val, int len) {
		std::ostringstream st;
		st << val;
		std::string str = st.str();
		if ( str.size() < (unsigned int) len ) {
			size_t n = len - str.size();
			str.clear();
			for ( size_t i = 0; i < n; i++)
				str.append("0");
			str.append(st.str());
		}
		return str;
	}
	static std::string now(void) {
//		std::SYSTEMTIME lt;
//		GetLocalTime(&lt);
//		std::ostringstream st;
//		st << set(lt.wDay, 2) << "/" << set(lt.wMonth, 2) << "/" << lt.wYear << "\t"
//			<< set(lt.wHour, 2) << ":" << set(lt.wMinute, 2) << ":" << set(lt.wSecond, 2) << "\t";
//		return st.str();
                return std::string("xxx");
	}
	bool IsOpen(void) { return _isOpen; }
private:
	void _log(const std::string str) {
		if ( _fstream.is_open() ) {
			_fstream << str;
			_fstream.flush();
		}
		std::cout << str;
	}
	std::fstream _fstream;
	bool _isOpen;
};

#endif
