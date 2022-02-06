$fn = 100;

hole_diameter = 11.1;
wall_thickness = 3;
rim = 2.5;
rim_thickness = 1;

cylinder(h=rim_thickness, d=hole_diameter+2*rim);
translate([0,0, rim_thickness])
    cylinder(h=wall_thickness, d=hole_diameter);    