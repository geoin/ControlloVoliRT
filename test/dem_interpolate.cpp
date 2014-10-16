#include <gtest/gtest.h>

#include <dem_interpolate/dsm.h>
#include <dem_interpolate/pslg.h>

#include <iostream>
#include <fstream>

const std::string path = "../data/";

TEST (DemTest, Triangulation) {

    DSM_Factory f;
    ASSERT_TRUE(f.Open(path + "toscana2.nod", false));
    DSM* d = f.GetDsm();

    ASSERT_EQ(39835, d->Npt());
    ASSERT_EQ(79140, d->Ntriangle());

    std::string npOut = path + "tria.np";
    std::fstream out(npOut.c_str(), std::fstream::out);
    out << std::fixed;

    for (unsigned int i = 0; i < d->Ntriangle(); i++) {
        const TRIANGLE t = d->Triangle(i);
        out << "N: [" << t.n[0] << ", " << t.n[1] << ", " << t.n[2] << "] - ";
        out << "P: [" << t.p[0] << ", " << t.p[1] << ", " << t.p[2] << "]";
        out << std::endl;
    }

    out << std::endl << "Nodes:" << std::endl;

    for (unsigned int i = 0; i < d->Npt(); i++) {
        const DPOINT p = d->Node(i);
        out << p.x << " " << p.y << " " << p.z << std::endl;
    }

    out << std::endl << "Quotas:" << std::endl;

    double xmax = d->Xmax();
    double ymax = d->Ymax();
    double xmin = d->Xmin();
    double ymin  = d->Ymin();

    double xdiff = xmax - xmin;
    double ydiff = ymax - ymin;

    int r = 10;
    for (int i = 0; i < r; i++) {
        double x = xmin + i*xdiff/r;
        double y = ymin + i*ydiff/r;
        out << x << " " << y << " " << d->GetQuota(x, y) << " -> ";

        int idx = d->FindTriangle(x, y);
        if (idx == -1) {
            continue;
        }

        TRIANGLE tr = d->Triangle(idx);

        DPOINT p1 = d->Node(tr.p[0]);
        DPOINT p2 = d->Node(tr.p[1]);
        DPOINT p3 = d->Node(tr.p[2]);

        out << idx << "- p1 (" << p1.x << ", " << p1.y << ", " << p1.z
            << ") p2 (" << p2.x << ", " << p2.y << ", " << p2.z
            <<  ") p3 (" << p3.x << ", " << p3.y << ", " << p3.z <<  ")" << std::endl;

    }

    f.Close();
}

TEST (DemTest, Las) {
    DSM_Factory f;
    ASSERT_TRUE(f.Open(path + "S1C1_strip32.las", false, false));
    PSLG* d = static_cast<PSLG*>(f.GetDsm());

	const DPOINT& lb = d->getLeftMostBottomPoint();
	const DPOINT& lt = d->getLeftMostUpperPoint();

	EXPECT_DOUBLE_EQ(lb.x, lt.x);
	
	const DPOINT& bl = d->getBottomMostLeftPoint();
	const DPOINT& br = d->getBottomMostRightPoint();

	EXPECT_DOUBLE_EQ(bl.y, br.y);
	
	const DPOINT& tl = d->getTopMostLeftPoint();
	const DPOINT& tr = d->getTopMostRightPoint();

	EXPECT_DOUBLE_EQ(tr.y, tr.y);
	
	const DPOINT& rb = d->getRightMostBottomPoint();
	const DPOINT& rt = d->getRightMostUpperPoint();

	EXPECT_DOUBLE_EQ(rb.x, rt.x);

	//ASSERT_TRUE(d->TriCalc());

	f.Close();
}

