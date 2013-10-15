/*================================================================================
!   Filename:  GNZIP.H
!
!   Contenuto:	Gnzip class declaration
!
!   History:
!			09/01/2009	created
!			09/01/2009	updated
!================================================================================*/
#ifndef MYUNZIP_H
#define MYUNZIP_H

#include"exports.h"
#include <iostream>
#include <fstream>

class TOOLS_EXPORTS MyUnZip {
public:
	MyUnZip(void) {};
	MyUnZip(const std::string& nome);
	~MyUnZip();
	void Close(void);
	bool Extract(const std::string& nome);
	int Extract(void);
	int Count(void);
	bool IsValid(void) const { return _in.is_open(); }
	std::string GetItem(int n);
	std::vector<std::string>& GetDir(void);
	std::string GetOutName(void) const {
		return _onome;
	}
private:
	std::ifstream	_in;
	std::string		_dir;
	std::string		_nome;
	std::vector<std::string> _vdir;
	std::string _onome;
};
#endif
