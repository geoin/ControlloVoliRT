// dem_test.cpp : Defines the entry point for the console application.
//

#include <string>
#include <set>
#include <vector>
#include "dem_interpolate/dsm.h"
#include <stdio.h>
#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include "Poco/StringTokenizer.h"
#include "projlib/Proj_helper.h"
#include <vector>
#include <iostream>
#include <windows.h>

#define _gauss_ovest_ "+proj=tmerc +ellps=intl +lat_0=0 +lon_0=9 +x_0=1500000 +y_0=0 +k=0.9996 +towgs84=-104.1,-49.1,-9.9,0.971,-2.917,0.714,-11.68"
#define _utm32_ "+proj=utm +ellps=WGS84 +zone=32 +datum=WGS84 +units=m +no_defs"
#define _igm249_ "+proj=utm +ellps=WGS84 +zone=32 +datum=WGS84 +units=m +no_defs +igmgrids=C:/Google _drive/grigliati/249.gr1,2"

bool test_overground(const std::string& nome)
{
	std::string nm = "overground" + nome;
	Poco::Path f1("I:/2012Lidar/2Consegnato-da-cgr/OVERGROUND", nm);
	f1.setExtension("xyzic");

	long d1 = GetTickCount();
	DSM_Factory df1;
	df1.SetEcho(0); // c'è un solo impulso
	df1.SetMask(File_Mask(5, 1, 2, 3, 0, 0));
	if ( !df1.Open(f1.toString(), false) )
		return false;
	DSM* ds1 = df1.GetDsm();
	unsigned int n1 = ds1->Npt();
	long d2 = GetTickCount();
	printf("Lettura overground : %.3ld\n", (d2 - d1) );

	nm = "dati_grezzi" + nome;
	Poco::Path f2("I:/2012Lidar/2Consegnato-da-cgr/DATI_GREZZI", nm);
	f2.setExtension("all");

	DSM_Factory df2;
	df2.SetEcho(1); // primo impulso
	df2.SetMask(File_Mask(11, 3, 4, 5, 9, 10));
	if ( !df2.Open(f2.toString(), false) )
		return false;
	DSM* ds2 = df2.GetDsm();
	unsigned int n2 = ds2->Npt();
	long d3 = GetTickCount();
	printf("Lettura raw %.3ld\n", (d3 - d2) );

	long count_in = 0;
	long count_out = 0;
	for (int i = 0; i < n1; i++ ) {
		double x = ds1->GetX(i);
		double y = ds1->GetY(i);
		double z = ds1->GetZ(i);
		double dz = ds2->GetQuota(x, y) - z;
		if ( dz > 0.3 )
			count_out++;
		else 
			count_in++;
	}
	printf("raw=%ld punti\n", n2);
	printf("overgrounf=%ld punti\n", n1);
	printf("entro 0.5 %ld\n", count_in);
	printf("oltre 0.5 %ld\n", count_out);
	return true;
}
bool test_mds(const std::string& nome)
{
	std::string nm = "overground" + nome;
	Poco::Path f1("I:/2012Lidar/2Consegnato-da-cgr/OVERGROUND", nm);
	f1.setExtension("xyzic");

	long d1 = GetTickCount();
	DSM_Factory df1;
	df1.SetEcho(0); // c'è un solo impulso
	df1.SetMask(File_Mask(5, 1, 2, 3, 0, 0));
	if ( !df1.Open(f1.toString(), false) )
		return false;
	DSM* ds1 = df1.GetDsm();
	unsigned int n1 = ds1->Npt();
	long d2 = GetTickCount();
	printf("Lettura overground : %.3ld\n", (d2 - d1) );

	nm = "MDS" + nome;
	Poco::Path f2("I:/2012Lidar/2Consegnato-da-cgr/MDS/WGS84_ELLISSOIDICHE", nm);
	f2.setExtension("asc");

	DSM_Factory df2;
	df2.SetEcho(1); // primo impulso
	df2.SetMask(File_Mask(11, 3, 4, 5, 9, 10));
	if ( !df2.Open(f2.toString(), false) )
		return false;
	DSM* ds2 = df2.GetDsm();
	unsigned int n2 = ds2->Npt();
	long d3 = GetTickCount();
	printf("Lettura dsm %.3ld\n", (d3 - d2) );

	long count_in = 0;
	long count_out = 0;
	for (int i = 0; i < n2; i++ ) {
		double x = ds2->GetX(i);
		double y = ds2->GetY(i);
		double z = ds2->GetZ(i);
		double z2 = ds1->GetQuota(x, y);
		if ( !ds1->IsValid(z2) )
			continue;
		double dz = fabs(z2 - z);
		if ( dz > 0.3 )
			count_out++;
		else 
			count_in++;
	}
	printf("triangle test\n");
	printf("raw=%ld punti\n", n2);
	printf("overgrounf=%ld punti\n", n1);
	printf("entro 0.3 %ld\n", count_in);
	printf("oltre 0.3 %ld\n", count_out);

	count_in = 0;
	count_out = 0;
	for (int i = 0; i < n2; i++ ) {
		double x = ds2->GetX(i);
		double y = ds2->GetY(i);
		double z = ds2->GetZ(i);
		double z2 = ds1->GetQm(x, y, 0.5, 0.5);
		//printf("x:%.2lf y:%.2lf i=%d\n", x, y, i); 
		if ( !ds1->IsValid(z2) )
			continue;
		double dz = fabs(z2 - z);
		if ( dz > 0.3 )
			count_out++;
		else 
			count_in++;
	}

	printf("mean test\n");
	printf("raw=%ld punti\n", n2);
	printf("overgrounf=%ld punti\n", n1);
	printf("entro 0.3 %ld\n", count_in);
	printf("oltre 0.3 %ld\n", count_out);
	return true;
}
bool test_orto(const std::string& nome)
{
	std::string nm = "MDS" + nome;
	Poco::Path f1("I:/2012Lidar/2Consegnato-da-cgr/MDS/WGS84_ELLISSOIDICHE", nm);
	f1.setExtension("asc");

	long d1 = GetTickCount();
	DSM_Factory df1;
	df1.SetEcho(0); // c'è un solo impulso
	//df1.SetMask(File_Mask(5, 1, 2, 3, 0, 0));
	if ( !df1.Open(f1.toString(), false) )
		return false;
	DSM* ds1 = df1.GetDsm();
	unsigned int n1 = ds1->Npt();
	long d2 = GetTickCount();
	printf("Lettura ellissoidiche : %.3ld\n", (d2 - d1) );

	std::string nm2 = "MDS" + nome + "_WGS";
	Poco::Path f2("I:/2012Lidar/2Consegnato-da-cgr/MDS/WGS84_ORTOMETRICHE", nm2);
	f2.setExtension("asc");

	DSM_Factory df2;
	df1.SetEcho(0); // c'è un solo impulso
	//df1.SetMask(File_Mask(5, 1, 2, 3, 0, 0));
	if ( !df2.Open(f2.toString(), false) )
		return false;
	DSM* ds2 = df2.GetDsm();
	unsigned int n2 = ds2->Npt();
	long d3 = GetTickCount();
	printf("Lettura overground : %.3ld\n", (d3 - d2) );

	long count_out = 0, count_in = 0;
	ProjTransform pjt(_utm32_, _igm249_);
	for ( int i = 0; i < n1; i++) {
		double x1= ds1->GetX(i);
		double y1= ds1->GetY(i);
		double z1= ds1->GetZ(i);
		double x2= ds2->GetX(i);
		double y2= ds2->GetY(i);
		double z2= ds2->GetZ(i);
		if ( fabs(x1 - x2) < 0.1 && fabs(y1 - y2) < 0.1 ) {
			pjt.transform(&x1, &y1, &z1);
			double dz = fabs(z1 - z2);
			if ( dz > 0.3 )
				count_out++;
			else 
				count_in++;
		}
	}
	printf("mean test\n");
	printf("raw=%ld punti\n", n2);
	printf("overgrounf=%ld punti\n", n1);
	printf("entro 0.3 %ld\n", count_in);
	printf("oltre 0.3 %ld\n", count_out);
	return true;
}
int main(int argc, char* argv[])
{
	DPOINT pt0(0, 0), pt1(0, 1);
	double dx = pt0.x - pt1.x;
	double dy = pt0.y - pt1.y;
	double alfa = atan2(dx, dy);
	double alf1 = atan2(dy, dx);
	double alf2 = pt0.angdir(pt1);
	double alf3 = pt0.angdir2(pt1);


	DSM_Factory* df = new DSM_Factory;
	if ( df->Open("C:/Google_drive/Regione Toscana Tools/Dati_test/prova2/cast-pescaia_50.asc", false) ) {
		std::cout << "start 2\n";
		DSM* ds = df->GetDsm();
		std::cout << "start 3 \n";
		std::cout << "start 4 " << ds->Npt() << "\n";

	}
//test_orto("586000-4883000");
	std::cout << "fatto\n";
	ProjTransform pjt(_utm32_, _igm249_);
	double x = 586075; //679057.013
	double y = 4883013; //4851590.576
	double z = 445.54; //399.850   //99.326
	pjt.transform(&x, &y, &z);
	//test_overground("586000-4883000");
	//test_mds("586000-4883000");

	//long d1 = GetTickCount();
	//DSM_Factory df1;
	//df1.SetEcho(0); // c'è un solo impulso
	//df1.SetMask(File_Mask(5, 1, 2, 3, 0, 0));
	//if ( !df1.Open("I:/2012Lidar/2Consegnato-da-cgr/OVERGROUND/overground586000-4883000.xyzic", false) )
	//	return 0;
	//DSM* ds1 = df1.GetDsm();
	//unsigned int n1 = ds1->Npt();
	//long d2 = GetTickCount();
	//printf("Lettura overground : %.3ld\n", (d2 - d1) );

	//DSM_Factory df2;
	//df2.SetEcho(1); // primo impulso
	//df2.SetMask(File_Mask(11, 3, 4, 5, 9, 10));
	//if ( !df2.Open("I:/2012Lidar/2Consegnato-da-cgr/DATI_GREZZI/dati_grezzi586000-4883000.all", false) )
	//	return 0;
	//DSM* ds2 = df2.GetDsm();
	//unsigned int n2 = ds2->Npt();
	//long d3 = GetTickCount();
	//printf("Lettura raw %.3ld\n", (d3 - d2) );

	//long count_in = 0;
	//long count_out = 0;
	//for (int i = 0; i < n1; i++ ) {
	//	double x = ds1->GetX(i);
	//	double y = ds1->GetY(i);
	//	double z = ds1->GetZ(i);
	//	double dz = ds2->GetQuota(x, y) - z;
	//	if ( dz > 0.5 )
	//		count_out++;
	//	else 
	//		count_in++;
	//}
	//printf("raw=%ld punti\n", n2);
	//printf("overgrounf=%ld punti\n", n1);
	//printf("entro 0.5 %ld\n", count_in);
	//printf("oltre 0.5 %ld\n", count_out);
	//int a = 1;

	return 0;
}

