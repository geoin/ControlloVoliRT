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

TEST (DemTest, Ellipse) {
	double xmin = 645851, xmax = 651620, ymin = 4879637, ymax = 4881067;
   InertialEllipse el((xmin + xmax)/2.0, (ymin + ymax)/2.0);
   el.push(xmin, ymin);
   el.push(xmin, ymax);
   el.push(xmax, ymax);
   el.push(xmax, ymin);

   el.compute();

   DPOINT x, y;
   el.getMajorAxis(x, y);
}


TEST (DemTest, Las) {
    DSM_Factory f;
    ASSERT_TRUE(f.Open(path + "S1C1_strip32.las", false, false));
    DSM* d = f.GetDsm();

	DPOINT p1, p2;
	d->getMajorAxis(p1, p2);

	//ASSERT_TRUE(d->TriCalc());

	f.Close();
}

