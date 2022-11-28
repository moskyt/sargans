include <bosl2/std.scad>

$fn = 80;

eps = 0.1;

t = 3;
rim = 5;
inner_w = 198;
inner_h = 33;


difference() {
    cuboid([inner_w+2*rim, inner_h+2*rim, t], anchor = BOTTOM, rounding = rim, edges = [FWD+LEFT,FWD+RIGHT,BACK+LEFT,BACK+RIGHT]);
    down(eps) {
        cuboid([inner_w, inner_h, t+2*eps], anchor = BOTTOM, rounding = rim/4, edges = [FWD+LEFT,FWD+RIGHT,BACK+LEFT,BACK+RIGHT]);
        xcopies(inner_w+rim, 2)
        zcyl(l = t+2*eps, d = 3, anchor = BOTTOM);
    }
}