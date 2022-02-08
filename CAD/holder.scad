base_height = 9;
pinheader_gap = 2;
pinheader_distance = 18.3;
solder_distance = 17.0;
grip = 1.6;
pcb_cutaway = 0; // set this if the pcb was shortend
pcb_thickness = 1.8;
pcb_length = 52.81 +  0.09 - pcb_cutaway; //Taken from original drawing
pcb_width = 20.78 + 0.12; //Taken from original drawing

debug_cutout_width = 11.5;
debug_cutout = false;

rail = true;
rail_height = 4;
rail_depth = 4;

difference()
{
    //Outer holder dimensions
    cube([pcb_width + grip * 2, pcb_length + grip, base_height + pinheader_gap + pcb_thickness+ grip]);
    union()
    {
        // PCB
        translate([grip, 0, base_height + pinheader_gap])
            cube([pcb_width, pcb_length, pcb_thickness]);

        // Space for pinheader on top
        translate([(pcb_width + grip * 2 - pinheader_distance)/2, 0, base_height + pinheader_gap + pcb_thickness])
          cube([pinheader_distance, pcb_length, grip]);

        // Space for solder of pin header on bottom
        translate([(pcb_width + grip * 2 - solder_distance)/2, 0, base_height])
           cube([solder_distance, pcb_length, pinheader_gap]);

        if(debug_cutout == true) // Cutout for SWD connector
        {
            translate([(pcb_width + grip * 2 - debug_cutout_width)/2, pcb_length, base_height + pinheader_gap + pcb_thickness])
                cube([debug_cutout_width, grip, grip]);
        }

        if(rail == true) // Cutout for rail in case
        {
            cube([pcb_width + grip * 2, rail_depth, rail_height]);
        }
    }
}