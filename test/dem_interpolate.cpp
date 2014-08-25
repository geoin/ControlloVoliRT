#include <gtest/gtest.h>

#include <dem_interpolate/dsm.h>

#include <iostream>

const std::string path = "/home/umberto/projects/rt/ControlloVoliRT/test/data/";

TEST (DemTest, Triangulation) {

    DSM_Factory f;
    ASSERT_TRUE(f.Open(path + "toscana2.nod", false));
    DSM* d = f.GetDsm();

    ASSERT_EQ(39835, d->Npt());
    ASSERT_EQ(79140, d->Ntriangle());

    for (unsigned int i = 0; i < d->Ntriangle(); i++) {
        const TRIANGLE t = d->Triangle(i);
    }

    f.Close();
}
