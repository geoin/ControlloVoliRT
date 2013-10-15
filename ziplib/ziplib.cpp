/*================================================================================
!   Filename:  GNZIP.CPP
!
!   Contenuto:	Gnzip class implementation
!
!   History:
!			09/01/2009	created
!			04/10/2012	updated
!================================================================================*/
#include <string>
#include <fstream>
#include "Poco/Zip/Decompress.h"
#include "Poco/Zip/ZipArchive.h"
#include "Poco/Zip/ZipStream.h"
#include "Poco/StreamCopier.h"
#include "ziplib.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Zip/Decompress.h>

using namespace Poco;
using namespace Poco::Zip;
using namespace std;

extern "C" {
int _lzw(const char* nome, char* onome);
}

MyUnZip::MyUnZip(const std::string& nome)
{
	_nome = nome;
	_dir = Poco::Path(_nome).parent().toString();
	_in.open(_nome.c_str(), std::ios::binary);
}
MyUnZip::~MyUnZip()
{
	Close();
}

int MyUnZip::Count(void)
{
	long count = 0;

	if ( IsValid() ) {
		_in.clear();
		_in.seekg(std::streampos());
		Poco::Zip::ZipArchive arch(_in);

		Poco::Zip::ZipArchive::FileHeaders::const_iterator it;

		for ( it = arch.headerBegin(); it != arch.headerEnd(); it++) {
			count++;
		}
	}
	return count;
}
std::vector<std::string>& MyUnZip::GetDir(void)
{
	_vdir.clear();
	if ( IsValid() ) {
		_in.clear();
		_in.seekg(std::streampos());
		Poco::Zip::ZipArchive arch(_in);

		Poco::Zip::ZipArchive::FileHeaders::const_iterator it;
		for ( it = arch.headerBegin(); it != arch.headerEnd(); it++) {
			_vdir.push_back(it->first);
		}
	}
	return _vdir;
}
bool MyUnZip::Extract(const std::string& nome)
{
	Poco::Path fn1(nome);
	std::string ext = Poco::Path(nome).getExtension();

        //stricmp to strcasecom
        if ( !strcmp(ext.c_str(), "z") || !strcmp(ext.c_str(), "Z") ) {
			char onome[256];
		if ( _lzw(nome.c_str(), onome) ) {
			_onome = onome;
			return true;
		}
		return false;
	}
	if ( !IsValid() )
		return false;
	_in.clear();
	_in.seekg(0, std::ios_base::beg);
	Poco::Zip::ZipArchive arch(_in);

	Poco::Zip::ZipArchive::FileHeaders::const_iterator it = arch.findHeader(nome.c_str());
	if ( it == arch.headerEnd() )
		return false;

	Poco::Zip::ZipInputStream zipin(_in, it->second);
	Poco::Path fn(_dir);
	fn.setFileName(nome);
	std::ofstream out(fn.toString().c_str(), std::ios::binary);
	Poco::StreamCopier::copyStream(zipin, out);
	return true;
}

int MyUnZip::Extract()
{
	if ( _in.is_open() ) {
		_in.clear();
		_in.seekg(0, std::ios_base::beg);
		Poco::Zip::Decompress c(_in, _dir);
		c.decompressAllFiles();
		return Count();
	}
	return 0;
}
void MyUnZip::Close()
{
	_in.close();
}
