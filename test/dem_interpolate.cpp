#include <gtest/gtest.h>

#include <dem_interpolate/dsm.h>

#include <iostream>
#include <fstream>

const std::string path = "/home/umberto/projects/rt/ControlloVoliRT/test/data/";

TEST (DemTest, Triangulation) {

    DSM_Factory f;
    ASSERT_TRUE(f.Open(path + "toscana2.nod", false));
    DSM* d = f.GetDsm();

    ASSERT_EQ(39835, d->Npt());
    ASSERT_EQ(79140, d->Ntriangle());

    std::string npOut = path + "tria.np";
    std::fstream out(npOut.c_str(), std::fstream::out);
    for (unsigned int i = 0; i < d->Ntriangle(); i++) {
        const TRIANGLE t = d->Triangle(i);
        out << "N: [" << t.n[0] << ", " << t.n[1] << ", " << t.n[2] << "] - ";
        out << "P: [" << t.p[0] << ", " << t.p[1] << ", " << t.p[2] << "]";
        out << std::endl;
    }

    f.Close();
}
