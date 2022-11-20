include <bosl2/std.scad>

$fn = 80;

eps = 0.1;

inner_d = 42;
outer_d = 51;
board_t = 6;
rim_t = 3;
tab = 10;
hole_d = 4;

grid_n = 15;
grid_t = 0.8;

difference() {
    union() {
        zcyl(board_t + rim_t, d = outer_d, anchor = BOTTOM);
        up(board_t) {
            cuboid([outer_d + 2*tab, tab, rim_t], anchor = BOTTOM);
        }
    }
    union() {
        down(eps) zcyl(board_t + rim_t + 2*eps, d = inner_d, anchor = BOTTOM);
        xcopies(l = (outer_d + tab), n = 2) 
        zcyl(board_t + rim_t + eps, d = hole_d, anchor=BOTTOM);
    }
}


intersection() {
    zcyl(board_t + rim_t, d = outer_d, anchor = BOTTOM);
    union() {
        xcopies(l = outer_d, n = grid_n)
        cuboid([grid_t,outer_d,grid_t], anchor = BOTTOM);
        ycopies(l = outer_d, n = grid_n)
        cuboid([outer_d,grid_t,grid_t], anchor = BOTTOM);
    }
}