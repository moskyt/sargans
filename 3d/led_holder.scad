include <bosl2/std.scad>

$fn = 80;

eps = 0.1;

plate_t = 2;
hole_t = 3.5;
hole_d = 5.5;
plate_d = 8;
screw_d = 3;
tab_w = 6;

difference() {
    union() {
        zcyl(plate_t/2 + hole_t, d = plate_d, anchor = BOTTOM);
        right(plate_d/4)
        cuboid([plate_d/2,plate_d,plate_t/2+hole_t],anchor=BOTTOM);
        cuboid([plate_d/2+tab_w,plate_d,plate_t],anchor=BOTTOM+LEFT);
    }
    up(plate_t/2)
    zcyl(hole_t + eps, d = hole_d, anchor = BOTTOM);
    right(plate_d/2 + tab_w/2)
    down(eps)
    zcyl(plate_t + 2*eps, d = screw_d, anchor = BOTTOM);
}