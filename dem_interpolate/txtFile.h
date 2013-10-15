#ifndef TXTFILE_H
#define TXTFILE_H

#include <stdio.h>
#include <stdarg.h>

class TxtFile {
public:
	TxtFile(): _fptr(NULL), _txt(NULL), _write(false), _size(512), _line_feed(true) {}
	TxtFile(const std::string& path): _size(512) {
		Init(path);
	}
	bool Init(const std::string& path) {
		_write = false;
		_txt = (char*) malloc(_size);
		_fptr = fopen(path.c_str(), "r");
		return ( _fptr != NULL );
	}
	bool InitW(const  std::string& path) {
		_txt = (char*) malloc(_size);
		_fptr = fopen(path.c_str(), "w");
		if ( _fptr != NULL )
			_write = true;
		return ( _fptr != NULL );
	}
	bool InitA(const  std::string& path) {
		_txt = (char*) malloc(_size);
		_fptr = fopen(path.c_str(), "a+");
		if ( _fptr != NULL )
			_write = true;
		return ( _fptr != NULL );
	}
	void LineFeed(bool val) {
		_line_feed = val;
	}
	void Close(void) {
		if ( _fptr )
			fclose(_fptr);
		_fptr = NULL;
		if ( _txt ) 
			free(_txt);
		_txt = NULL;
	}
	void Rewind(void) {
		if ( _fptr != NULL )
			fseek(_fptr, 0L, 0);
	}
	__int64 GetPos(void) { 
		fpos_t	pos;
		fgetpos(_fptr, &pos);
		return pos;
	}
	//__int64 GetSize(void) {
	//	int fd = fileno(_fptr);
	//	return _filelengthi64(fd);
	//}

	~TxtFile() {
		Close();
	}
	const char* GetNextRecord() {
		long pos = ftell(_fptr);
		bool repeat;
		do {
			repeat = false;
			if ( fgets(_txt, _size - 1, _fptr) == NULL )
				return NULL;

			size_t len = strlen(_txt);
			if ( len == _size - 2 ) {
				_size *= 2;
				char* bf = (char*) realloc(_txt, _size * sizeof(char));
				if ( bf == NULL )
					return NULL;
				_txt = bf;
				fseek(_fptr, pos, SEEK_SET);
				repeat = true;
			}
		} while ( repeat );

		char* ch = strrchr(_txt, '\n');
		if ( ch != NULL ) *ch = '\0';
		return _txt;
	}
	void SetNextRecord(const std::string& txt) {
		if ( _write && _fptr != NULL ) {
			fputs(txt.c_str(), _fptr);
			if ( _line_feed ) {
				if ( txt.empty() || (!txt.empty() && txt.at(txt.size() - 1) != '\n') )
					fputs("\n", _fptr);
			}
		}
	}
	void PrintNextRecord(const char* fmt, ...) {
		va_list	args;
	
		va_start(args, fmt);

		int len = _vscprintf(fmt, args ) + 1; // for the \0 character
		char* txt = (char*) malloc(len * sizeof(char));

		if ( vsprintf_s(txt, len, fmt, args) > 0 )
			SetNextRecord(txt);
		va_end(args);
		free(txt);
	}
	bool IsOpen(void) const {
		return _fptr != NULL;
	}
private:
	bool	_write;
	char*	_txt;
	FILE*	_fptr;
	long	_size;
	bool	_line_feed;
};

#endif