#include "common/util.h"
#include "Poco/DateTime.h"
using namespace CV::Util::Spatialite;

bool print_item(Doc_Item& row, Poco::XML::AttributesImpl& attr, double val, CHECK_TYPE ty, double tol1, double tol2)
{
	bool rv = true;
	switch ( ty ) {
		case less_ty:
			rv = val < tol1;
			break;
		case great_ty:
			rv = val > tol1;
			break;
		case abs_less_ty:
			rv = fabs(val) < tol1;
			break;
		case between_ty:
			rv = val > tol1 && val < tol2;
			break;
	}
	if ( !rv ) {
		Doc_Item r = row->add_item("entry", attr);
		r->add_instr("dbfo", "bgcolor=\"red\"");
		r->append(val);
	} else
		row->add_item("entry", attr)->append(val);
	return rv;
}

void init_document(docbook& dbook, const std::string& nome, const std::string& title, const std::string& note)
{
	//Path doc_file(_proj_dir, "*");
	//doc_file.setFileName(_type == FLY_TYPE ? OUT_DOCV : OUT_DOCP);
	//_dbook.set_name(doc_file.toString());	
	dbook.set_name(nome);	

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "lang", "", "it");
	Doc_Item article = dbook.add_item("article", attr);
	article->add_item("title")->append(title);
	//_article->add_item("title")->append(_type == FLY_TYPE ? "Collaudo ripresa aerofotogrammetrica" : "Collaudo progetto di ripresa aerofotogrammetrica");

	Doc_Item sec = article->add_item("section");
	sec->add_item("title")->append("Intestazione");

	std::string head1;
	for ( int i = 0; i < note.size(); i++) {
		if ( note[i] == 10 ) {
			sec->add_item("para")->append(head1);
			head1.clear();
		} else
			head1.push_back(note[i]);
	}
	if ( !head1.empty() )
		sec->add_item("para")->append(head1);

	Poco::DateTime dt;
	dt.makeLocal(+1);
	std::stringstream ss;
	ss << "Data: " << dt.day() << "/" << dt.month() << "/" << dt.year() << "  " << dt.hour() << ":" << dt.minute();
	sec->add_item("para")->append(ss.str());
}
