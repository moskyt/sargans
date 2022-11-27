include <bosl2/std.scad>

$fn = 80;

mx_hole_d = 3.3;
mx_hole_s = 9.5;
mx_led_d = 4.6;
mx_hole_h = 0.8;
mx_led_h = 7.3;
mx_total_h = 12.4;


eps = 0.1;

post_d = 10;
channel_d = 8;
post_h = 5;

block_w = 22;
nub_h = 5;
diode_hole_h = nub_h + 7.2;
block_t = 2;
nub_d = 3.2;
nub_spacing = 12.9;
diode_d = 6;

base_t = 5;
base_d = 20;

box_t = 2;
box_h = 25;

groove_d = 2;
groove_eps = 0.2;


back(-50)
color("red") down(base_t)
union() {
difference() {
fwd(box_t/2)
cuboid([block_w + 2*box_t,base_d + box_t, box_h+box_t], anchor=BOTTOM);
down(eps)
fwd(-eps)
cuboid([block_w,base_d, box_h], anchor=BOTTOM);
}
fwd(block_t)
up(base_t/2) {
    xcopies(block_w-groove_d+2*groove_eps,2)
    cuboid([groove_d-groove_eps,base_d,base_t/2-2*groove_eps]);
}
}

color("yellow")
difference() {
union(){
back(base_d/2) {
cuboid([block_w, block_t, box_h-base_t], anchor = BOTTOM + BACK);

up(nub_h)
fwd(block_t)
xcopies(nub_spacing,2)
ycyl(5,d=nub_d,anchor=BACK);
}
cuboid([block_w,base_d,base_t], anchor=TOP);
down(base_t)
zcyl(post_h, d=post_d, anchor=TOP);
}

up(eps)
zcyl(1000,d=channel_d,anchor=TOP);

up(diode_hole_h)
back(base_d/2)
ycyl(4*block_t,d=diode_d);

fwd(block_t+groove_eps)
down(base_t/2) {
    xcopies(block_w-groove_d+2*groove_eps,2)
    cuboid([groove_d+groove_eps,base_d+2*groove_eps,base_t/2-groove_eps]);
}


}