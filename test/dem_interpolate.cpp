#include <gtest/gtest.h>

#include <dem_interpolate/dsm.h>

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
        out << d->GetQuota(xmin + i*xdiff/r, ymin + i*ydiff/r) << std::endl;
    }

    f.Close();
}

TEST (DemTest, Las) {
    DSM_Factory f;
    ASSERT_TRUE(f.Open(path + " S1C1_strip32.las", false));
    DSM* d = f.GetDsm();

   f.Close();
}

