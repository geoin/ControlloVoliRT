#include "lidar_raw_exec.h"

#include "Poco/File.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/AutoPtr.h"

#include <sstream>
#include <numeric>
#include <math.h>
#include <cmath>

#define GEO_DB_NAME "geo.sqlite"

using namespace CV;
using namespace Poco;
using namespace Poco::Util;

#define LIDAR "Lidar"

std::string get_key(const std::string& val) {
	return std::string(LIDAR) + "." + val;
}

void lidar_raw_exec::Error(const std::string& operation, const std::exception& e) {
	std::cout << "Error [" << operation << "] : " << e.what() << std::endl;
}

void lidar_raw_exec::Error(const std::string& operation) {
	std::cout << "Error [" << operation << "]" << std::endl;
}

void lidar_raw_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_raw_exec::openDBConnection() {
	try {
		Poco::Path geodb(_proj_dir, GEO_DB_NAME);
		cnn.open(geodb.toString());
		return true;
	} catch (const std::exception& e) {
		Error("Opening connection..", e);
		return false;
	}
}

bool lidar_raw_exec::readReference() {
	try {
		Path ref_file(_proj_dir, "*");
		ref_file.setFileName("refval.xml");
		AutoPtr<XMLConfiguration> pConf;
		pConf = new XMLConfiguration(ref_file.toString());
		LID_TOL_Z = pConf->getDouble(get_key("LID_TOL_Z"));
		PT_DENSITY = pConf->getDouble(get_key("PT_DENSITY"));
		return true;
	} catch (const std::exception& e) {
		Error("Reading reference values..", e);
		return false;
	}
}

bool lidar_raw_exec::_initBlocks() {
	CV::Util::Spatialite::Statement stm(cnn);

	std::stringstream query;
	query << "select AsBinary(GEOM) FROM BLOCKV";
	stm.prepare(query.str());
	CV::Util::Spatialite::Recordset set = stm.recordset();
	if (set.eof()) {
		ERROR("Blocco non presente");
		return false;
	}
	
	Blob b = set[0].toBlob();
	CV::Util::Geometry::OGRGeomPtr geom = b;
	_block.assign(new CV::Lidar::Block(geom));
	
	std::vector<CV::Util::Geometry::OGRGeomPtr> parts;
	_block->split(parts);

	std::vector<CV::Util::Geometry::OGRGeomPtr>::iterator it = parts.begin();
	std::vector<CV::Util::Geometry::OGRGeomPtr>::iterator end = parts.end();
	
	OGREnvelope env;
	for (; it != end; it++) {
		(*it)->getEnvelope(&env);
	}

	return true;
}

bool lidar_raw_exec::init() {
	if (!GetProjData(cnn, _note, std::string())) {
		return false;
    }
	 
	if (!_initBlocks()) {
		return false;
	}

	if (!_initStripFiles()) {
		return false;
	}

	if (!_initControlPoints()) {
		return false;
	}

	if (!_initStripsLayer()) {
		return false;
	}


	return true;
}

bool lidar_raw_exec::run() {
	try {
		_checkDensity();
		_checkIntersection();
			
		return true;
	} catch (const std::exception& e) {
		Error("Running..", e);
		return false;
	}
}

bool lidar_raw_exec::_initControlPoints() {
	try {
		CV::Util::Spatialite::Statement stm(cnn);

		std::stringstream query;
		query << "select X, Y, Z, NAME FROM CONTROL_POINTS";
		stm.prepare(query.str());

		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (set.eof()) {
			throw std::runtime_error("No data in RAW_CONTROL_CLOUD");
		}
		
		while (!set.eof()) {
			double x = set["X"].toDouble();
			double y = set["Y"].toDouble();
			double z = set["Z"].toDouble();

			Lidar::ControlPoint::Ptr point(new Lidar::ControlPoint(x, y, z));
			point->name(set["NAME"].toString());
			_controlVal.push_back(point);

			set.next();
		}
		return true;
	} catch (const std::exception& e) {
		Error("fetching control points", e);
		return false;
	}
}

bool lidar_raw_exec::_initStripFiles() {
	try {
		std::stringstream query;
		query << "select FOLDER from STRIP_RAW_DATA LIMIT 1"; //only one record should be here
		
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare(query.str());
		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (set.eof()) {
			throw std::runtime_error("No data in STRIP_RAW_DATA");
		}

		std::string folder = set["FOLDER"].toString();
		Poco::Path fPath(folder);
		
		std::vector<std::string> folderContent;
		Poco::File(fPath).list(folderContent);

		std::vector<std::string>::iterator it = folderContent.begin();
		std::vector<std::string>::iterator end = folderContent.end();
		for (; it != end; it++) {
			Poco::Path p = Poco::Path(fPath).append(*it);
			if (Poco::File(p).isDirectory()) {
				_traverseFolder(p);
			} else {
				std::string name = p.getBaseName();
				_cloudStripList.insert(std::pair<std::string, Poco::Path>(name, p));
			}
		}

		stm.reset();
		return true;
	} catch (const std::exception& e) {
		Error("fetching strips", e);
		return false;
	}
}

void lidar_raw_exec::_traverseFolder(const Poco::Path& fPath) {
	std::vector<std::string> folderContent;
	Poco::File(fPath).list(folderContent);

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(fPath).append(*it);
		std::string name = p.getBaseName();
		_cloudStripList.insert(std::pair<std::string, Poco::Path>(name, p));
	}
}

bool lidar_raw_exec::_initStripsLayer() {
	try {
		CV::Util::Spatialite::Statement stm(cnn);

		std::stringstream query;
		query << "select Z_STRIP_CS, Z_STRIP_YAW, Z_MISSION, Z_STRIP_LENGTH, AsBinary(GEOM) as GEOM FROM Z_STRIPV";
		stm.prepare(query.str());

		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (set.eof()) {
			throw std::runtime_error("No data in Z_STRIPV");
		}
		
		while (!set.eof()) {
			Blob b = set["GEOM"].toBlob();
			Lidar::Strip::Ptr strip(new Lidar::Strip(b));
			strip->yaw(set["Z_STRIP_YAW"].toDouble());
			strip->missionName(set["Z_MISSION"].toString());
			strip->name(set["Z_STRIP_CS"].toString());

			Lidar::CloudStrip::Ptr cloud(new Lidar::CloudStrip(strip));
			std::map<std::string, Poco::Path>::iterator el = _cloudStripList.find(cloud->name());
			if (el == _cloudStripList.end()) {
				throw std::runtime_error("Strip " + cloud->name() + " missing");
			}	

			cloud->cloudPath(el->second.toString());
			_strips.push_back(cloud);

			set.next();
		}
		return true;
	} catch (const std::exception& e) {
		Error("fetching strips layer", e);
		return false;
	}
}

bool lidar_raw_exec::_checkDensity() {
	bool ret = _strips.size() == _cloudStripList.size();
	/*if (!ret) { 
		throw std::runtime_error("Layer strips number different from cloud files");
	}*/

	std::vector<CV::Lidar::CloudStrip::Ptr>::iterator it = _strips.begin();
	std::vector<CV::Lidar::CloudStrip::Ptr>::iterator end = _strips.end();

	for (; it != end; it++) {
		Lidar::CloudStrip::Ptr cloudStrip = *it;
		cloudStrip->computeDensity();
	}
	
	return ret;
}

bool lidar_raw_exec::_checkIntersection() {
	std::vector<Lidar::CloudStrip::Ptr>::iterator it = _strips.begin();
	std::vector<Lidar::CloudStrip::Ptr>::iterator end = _strips.end();
	
	for (; it != end; it++) {
		Lidar::CloudStrip::Ptr cloud = *it;
		Lidar::Strip::Ptr source = cloud->strip();
		std::vector<Lidar::CloudStrip::Ptr>::const_iterator next = it;
		for (next++; next != end; next++) {
			Lidar::CloudStrip::Ptr cloudTarget = *next;
			Lidar::Strip::Ptr target = cloudTarget->strip();
			if (source->isParallel(target) && source->intersect(target)) {
				Lidar::Strip::Intersection::Ptr intersection = source->intersection(target);

				std::vector<double> diff; 
				Lidar::DSMHandler srcDsm(cloud);
				Lidar::DSMHandler targetDsm(cloudTarget);
				
				unsigned int count = srcDsm->Npt();
				for (unsigned int i = 0; i < count; ++i) {
					DPOINT pt = srcDsm->Node(i);
					if (intersection->contains(pt)) {
						double zTrg = targetDsm->GetQuota(pt.x, pt.y);

						double dZ = pt.z - zTrg; 
						diff.push_back(dZ);
					}
				}
				if (!diff.size()) {
					continue;
				}
				
				Stats s = { target->name(), 0.0, 0.0 };
				_getStats(diff, s);

				_statList.insert(std::pair<std::string, Stats>(source->name(), s));
			}
		}
	}
	return true;
}

void lidar_raw_exec::_getStats(const std::vector<double>& diff, Stats& s) {
	//TODO
	double size = static_cast<double>(diff.size());
	double mean = std::accumulate(diff.begin(), diff.end(), 0.0) / size;

	std::vector<double>::const_iterator j = diff.begin();
	std::vector<double>::const_iterator end = diff.end();
	double mm = 0.0;
	for (; j != end; j++) {
		double val = *j;
		mm += std::pow(val - mean, 2.0);
	}

	s.mean = mean;
	s.stdDev = std::sqrt(mm / size);
}

// report methods

bool lidar_raw_exec::report() {
	try {

		std::string title ="Collaudo dati grezzi";

		Poco::Path doc_file(_proj_dir, "*");
		doc_file.setFileName("check_lidar_raw.xml");

		init_document(_dbook, doc_file.toString(), title, _note);
		char* dtd_ = getenv("DOCBOOKRT");
		std::string dtd;
		if (dtd_ != NULL) {
			dtd = std::string("file:") + dtd_;
		}

		_dbook.set_dtd(dtd);
		_article = _dbook.get_item("article");

		std::cout << "Produzione del report finale: " << _dbook.name() << std::endl;

		_density_report();
		_strip_overlaps_report();

		_dbook.write();	

		return true;

	} catch (const std::exception& e) {
		Error("Printing report..", e);
		return false;
	}
}

void lidar_raw_exec::_density_report() {
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Striciate");

	
    sec->add_item("para")->append("Densita' strisciate");

	Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Calcolo densita' strisciate");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "2");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Strisciata");
    row->add_item("entry", attr)->append("Densita'");
    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	std::vector<CV::Lidar::CloudStrip::Ptr>::iterator it = _strips.begin();
	std::vector<CV::Lidar::CloudStrip::Ptr>::iterator end = _strips.end();

	for (; it != end; it++) {
		const std::string& name = (*it)->name();
		const double density = (*it)->density();

		row = tbody->add_item("row");
        row->add_item("entry", attr)->append(name);
		print_item(row, attrr, density, abs_less_ty, PT_DENSITY);
	}
}

void lidar_raw_exec::_strip_overlaps_report() {
	if (_statList.size() == 0) {
		return;
	}
	
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Striciate");
	
    sec->add_item("para")->append("Sovrapposizione strisciate adiacenti");

	Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Statistiche sovrapposizione nuvole");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "4");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Strisciata 1");
    row->add_item("entry", attr)->append("Strisciata 2");
    row->add_item("entry", attr)->append("Media");
    row->add_item("entry", attr)->append("Deviazione standard");
    Doc_Item tbody = tab->add_item("tbody");

	
    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	std::multimap<std::string, Stats>::iterator it = _statList.begin();
	std::multimap<std::string, Stats>::iterator end = _statList.end();

	for (; it != end; it++) {
		const std::string& source = it->first;
		const Stats& stat = it->second;

		row = tbody->add_item("row");
        row->add_item("entry", attr)->append(source);
		row->add_item("entry", attr)->append(stat.target);
		row->add_item("entry", attr)->append(stat.mean);
		row->add_item("entry", attr)->append(stat.stdDev);
	}
}

/*
void lidar_raw_exec::_control_points_report() {
	if (_controlVal.size() == 0) {
		return;
	}

	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Punti di controllo");

    sec->add_item("para")->append("Validita' punti di controllo");

    Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Punti di controllo");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "2");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Punto di controllo");
    row->add_item("entry", attr)->append("Z diff");
    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	std::vector<CV::Lidar::ControlPoint::Ptr>::iterator it = _controlVal.begin();
	std::vector<CV::Lidar::ControlPoint::Ptr>::iterator end = _controlVal.end();

	for (; it != end; it++) {
		CV::Lidar::ControlPoint::Ptr point = *it;
		const std::string& name = point->name();

		row = tbody->add_item("row");
        row->add_item("entry", attr)->append(name);
		
		if (point->isValid()) {
			double diff = point->zDiff();
			print_item(row, attrr, diff, abs_less_ty, LID_TOL_Z);
		} else {
			Doc_Item r = row->add_item("entry", attr);
			r->add_instr("dbfo", "bgcolor=\"red\"");

			Lidar::ControlPoint::Status status = point->status();
			if (status == Lidar::ControlPoint::NO_VAL) {
				r->append("NO_VAL");
			} else if (status == Lidar::ControlPoint::OUT_VAL) {
				r->append("OUT_VAL");
			}
		}
    }
}
*/