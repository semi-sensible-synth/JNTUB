/*
 * Eurorack front panel, based on measurements from:
 *  http://www.doepfer.de/a100_man/a100m_e.htm
*/

// Resolution - higher = slower render.
$fn=48;

/* [Hidden] */
// Constant for 1U (rack unit) - leave unchanged
__oneU = 44.45;
// Constant for HP (horizontal pitch) - leave unchanged
__oneHP = 5.08;


/* [Panel Params] */

panelHeight = (3 * __oneU) - 4.85;  // 128.5, as per Doepfer guide

// 110mm typically gives a safe distance from rails (~9mm from top and bottom of panel)
panelInnerHeight = 110; // panelHeight - 18.5;

// Doepfer panels aren't exactly multiples of 1HP (5.08mm) but have some rounding (usually -0.3 mm tolerance, rounded to the nearest tenth of a millimeter)
panelWidth = 20.0; // [5.0:1HP, 9.80:2HP, 20.0:4HP, 30.0:6HP, 40.3:8HP, 50.5:10HP, 60.60:12HP, 70.8:14HP, 80.9:16HP, 91.3:18HP, 101.3:20HP, 106.3:21HP, 111.40:22HP, 141.90:28HP, 213.00:42HP]
panelThickness = 2.0;
hole_width = __oneHP + 2;
// Doepfer uses 7.5mm - can be less
hole_xinset = 3.5;
// Doepfer uses 3mm so holes line up with 3U rail spacing
hole_yinset = 3.0;

m3_hole_loose_d=3.2;    // easy fit of M3 screw, non-tapping
m3_hole_tapping_d=2.95; // should engage screw thread

m3_r = (m3_hole_loose_d/2);

// Larger potentiometer hole radius - for a T18 knurled shaft (6mm diameter) metal Alpha pot, 3.9mm is a loose fit, can be attached with a nut.
potHoleRadius=3.9;

// Smaller potentiometer hole radius - 3.3mm is good for plastic RV09 style pots
potHoleRadiusSmall=3.3;

// 3.5mm jack hole radius. 3.25mm is a snug fit for a typical 'Thonkiconn' or simiar, secured with nut
socketRadius=3.25;

// Round LED hole - 2.65mm radius is a snug/tight fit for a 5mm LED. You may need to tune this for your specific LEDs if there is variation
ledRoundHoleWidth = 2.65;

led3mmRoundHoleWidth = 1.6;

// Square/rectangular LED hole, WxH - default is for a 5x2mm rectangular face [5.2, 2.2] is a little too tight without post-processing
ledSquareHole = [5.4, 2.4];


// If 3D printing, this can be tuned based on the layer height
// (eg 0.6 is 3 layers for a typical 0.2mm layer height - you might
//  want to change this to 0.4 or 0.2mm, especially if you are doing
//  two color faceplaces with a filament change)
//textEmbossDepth=0.6;
textEmbossDepth=0.4;
    
/* [PoCoBrick] */

// Units that may be compatible with your favorite plastic brick format
pocobrick_unit=1.6;
pocobrick_stud_height=1.8;

// Change by ~0.1mm increments to get better snap fit on studs
pocobrick_stud_diameter_tweak=0.0;
pocobrick_stud_d=(3*pocobrick_unit)+0.2 + pocobrick_stud_diameter_tweak;


module pocobrick_stud() {
    cylinder(d1=pocobrick_stud_d, d2=pocobrick_stud_d+0.1, h=pocobrick_stud_height, center=false, $fn=20);
}

module pocobrick_stud_array(pocobrickxdim, pocobrickydim) {
    for (ix=[0:pocobrickxdim-1]) {
        for (iy=[0:pocobrickydim-1]) {
            translate([ix*5*pocobrick_unit, iy*5*pocobrick_unit, 0]) pocobrick_stud();
        }
    }
}

// Round HP values to Doepfer panel widths in mm
function hp_to_mm_width (hp) = floor(((hp*__oneHP)-0.3)*10)/10;

module mountingHole(depth=panelThickness, hole_width=hole_width) {
    hw = (hole_width/2)-m3_r;
    echo(hw, hole_width, m3_r);
    linear_extrude(height = (depth*2)+0.01, 
                   center = true) {
        translate([0, hw, 0]) {
            circle(r = m3_r);
        }
        translate([0, -hw, 0]) {
            circle(r = m3_r);
        }
        square([m3_hole_loose_d, hw*2], center=true);
    }
}

// inset_topbottom should generally be 10mm or more to safely clear the rails
module reinforcementRails(w=3, depth=2, inset_sides=2,
                          panel_height=panelHeight, 
                          panel_innerHeight=panelInnerHeight) {
    inset_topbottom = (panelHeight - panelInnerHeight) / 2;
    translate([0, 0, -panelThickness]) {
        translate([inset_topbottom, inset_sides, 0]) {
            cube([panelHeight-(inset_topbottom*2), w, depth]);
        }
        translate([inset_topbottom, panelWidth-(inset_sides+w), 0]) {
            cube([panelHeight-(inset_topbottom*2), w, depth]);
        }
    }
}

module reinforcementRailCenter(w=1.5, depth=2, inset_sides=2) {
    inset_topbottom = ((panelHeight - panelInnerHeight) / 2) + 6;
    translate([0, 0, -panelThickness]) {
        translate([inset_topbottom, (panelWidth/2) - (w/2), 0]) {
            cube([panelHeight-(inset_topbottom*2), w, depth]);
        }
    }
}

module frontPanel(width, height, thickness, n_holes=2, hole_width=hole_width) {
  difference() {
    union() {
        cube([width, height, thickness]);
        //reinforcementRails();
        //reinforcementRailCenter();
    }
    
    // Screw holes top and bottom
    translate([hole_yinset, hole_xinset+m3_r, 0]) {
        mountingHole(panelThickness, hole_width);
    }
    translate([panelHeight - hole_yinset, panelWidth - hole_xinset - m3_r, 0]) {
        mountingHole(panelThickness, hole_width);
    }
    
    translate([-3, 0, 0]) { // shift holes and markings higher on panel
        panelHoles();
        panelText();
    }
  }
}

module panelHoles() {
    /* Define your own panel holes (pots, LEDs etc) in here */
    linear_extrude(height = (panelThickness*2)+0.01, 
                   center = true) {
                 
      
        /* Pot holes */
        pot_xstart=22.1;
        pot_spacing=20.32;
        translate([pot_xstart, panelWidth/2, 0]) {
            circle(r = potHoleRadiusSmall);
        }
        translate([pot_xstart+(pot_spacing*1), panelWidth/2, 0]) {
            circle(r = potHoleRadiusSmall);
        }
        translate([pot_xstart+(pot_spacing*2), panelWidth/2, 0]) {
            circle(r = potHoleRadiusSmall);
        }
        
        
        /* Jack holes */
        jack_xstart=81.5;
        jack_yspacing=5;
        jack_xspacing=12.8;
        for (row = [0: 2]) {
            translate([jack_xstart+(jack_xspacing*row), jack_yspacing, 0]) {
                circle(r = socketRadius);
            }
            translate([jack_xstart+(jack_xspacing*row), panelWidth-jack_yspacing, 0]) {
                circle(r = socketRadius);
            }
        }
        
        
        /* Trigger input LED */
        // Square LED mod
        translate([88, panelWidth/2, 0]) {
            square([ledSquareHole[1], ledSquareHole[0]], center=true);
        }
        
        // Original design 3mm round trigger LED
        /*
        translate([88, panelWidth/2, 0]) {
            circle(r = led3mmRoundHoleWidth);
        }
        */
        
        
        /* Bottom output LED(s) */

        // Single 5mm bipolar LED mod
        translate([panelHeight-11.4, panelWidth/2, 0]) {
            circle(r = ledRoundHoleWidth);
        }
        
        // Original 2x3mm LED design
        /*
        translate([panelHeight-15.2, panelWidth/2, 0]) {
            circle(r = led3mmRoundHoleWidth);
        }
        translate([panelHeight-11.4, panelWidth/2, 0]) {
            circle(r = led3mmRoundHoleWidth);
        }
        */
        
    }
}

/*
// Simple text labels
module panelText() {
    fontSize=6;
    smallFontSize=4;
    largeFontSize=18;
    textYOffset=120;
    textEmbossDepth=0.6;
    rotate([0, 0, 90]) {
        translate([panelWidth/2, -12, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("JNTUB", size=4, 
                     font="Arial:style=Bold", 
                     spacing=0.9, 
                     halign = "center", $fn=32);
            }
        }
        
        translate([panelWidth/2, -25, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("3      ", size=6, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", $fn=32);
            }
        }
        
        translate([panelWidth/2, -45.5, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("2      ", size=6, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", $fn=32);
            }
        }
        
        translate([panelWidth/2, -66, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("1      ", size=6, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", $fn=32);
            }
        }
        
        translate([panelWidth/2, -89.5, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("trig           ", size=3, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", $fn=32);
            }
        }
        translate([panelWidth/2, -91, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("    →", size=6, 
                     font="Consolas:style=Bold", 
                     spacing=0.6, 
                     halign = "center", $fn=32);
            }
        }
        
        translate([(panelWidth/2)-0.25, -77, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("1      2", size=4, 
                     font="Arial:style=Bold", 
                     spacing=0.87, 
                     halign = "center", $fn=32);
            }
        }
        
        translate([panelWidth/2, -103, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("out      ", size=4, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", $fn=32);
            }
        }
        translate([panelWidth/2, -104, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("   →", size=6, 
                     font="Consolas:style=Bold", 
                     spacing=0.6, 
                     halign = "center", $fn=32);
            }
        }
        translate([panelWidth/2, -118.5, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("           sig", size=3.5, 
                     font="Arial:style=Bold", 
                     spacing=0.8, 
                     halign = "center", $fn=32);
            }
        }
    }
}
*/


module panelText() {
    fontSize=6;
    smallFontSize=4;
    largeFontSize=18;
    textYOffset=120;
    //titleFont="Source Code Pro:style=Light";
    titleFont="Rubik:style=Bold";
    //titleFont="Rubik:style=Regular";
    //titleFont="Arial:style=Bold";
    
    rotate([0, 0, 90]) {
        translate([(panelWidth/2)+4, -4, panelThickness-textEmbossDepth+0.01]) {
            rotate([0, 0, -90])
            linear_extrude(textEmbossDepth) {
                text("JNTUB", size=10, 
                     font=titleFont, 
                     spacing=1.2, 
                     //halign = "center",
                     valign = "center",
                     $fn=32);
            }
        }
        
        /** Param pots **/
        translate([panelWidth/2, -22.8, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("≡   ", size=12, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([panelWidth/2, -44.2, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("=   ", size=12, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([panelWidth/2, -65, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("−   ", size=12, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        /***************************/
        
        /*
        translate([(panelWidth/2), -78, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("∙ ∙∙", size=16, 
                     font="Arial:style=Bold", 
                     spacing=0.68, 
                     halign = "center", 
                     valign = "center", 
                     $fn=32);
            }
        }
        */
        
        /** CV inputs **/
        translate([panelWidth/2, -83.8, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("−   ", size=12, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([panelWidth/2, -83.2, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("   =", size=12, 
                     font="Arial:style=Bold", 
                     spacing=1, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([(panelWidth/2)-3.6, -72, panelThickness-textEmbossDepth+0.01]) {
            rotate([0, 0, -15])
            linear_extrude(textEmbossDepth) {
                text("⇣", size=10, 
                     font="Code2003:style=Bold", 
                     spacing=1, 
                     halign = "center", $fn=32);
            }
        }
        translate([(panelWidth/2) + 6, -74.8, panelThickness-textEmbossDepth+0.01]) {
            rotate([0, 0, -15])
            linear_extrude(textEmbossDepth) {
                text("⇓", size=10, 
                     font="Code2003:style=Bold", 
                     spacing=1, 
                     halign = "center", $fn=32);
            }
        }
        /*******************/
        
        /* Trigger input and ditto */
        translate([(panelWidth/2) - 5, -99.4, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("⎍", size=8, 
                     font="Code2003:style=Bold", 
                     spacing=0.95, 
                     halign = "center", $fn=32);
            }
        }
        translate([panelWidth/2 - 3, -95, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("→", size=12, 
                     font="Consolas:style=Bold", 
                     spacing=0.6, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([panelWidth/2 +4, -95, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("→", size=12, 
                     font="Consolas:style=Bold", 
                     spacing=0.6, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([-4, -95, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("→", size=12, 
                     font="Consolas:style=Bold", 
                     spacing=0.6, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        /********************/
        
        
        /** Outputs **/
        translate([5, -115.8, panelThickness-textEmbossDepth+0.01]) {
            rotate([0, 0, 90])
            linear_extrude(textEmbossDepth) {
                text("⇜", size=10, 
                     font="Code2003:style=Bold", 
                     spacing=0.6, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([panelWidth - 5, -109, panelThickness-textEmbossDepth+0.01]) {
            rotate([0, 0, -90])
            linear_extrude(textEmbossDepth) {
                text("⇝", size=10, 
                     font="Code2003:style=Bold", 
                     spacing=0.6, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        translate([panelWidth/2, -109.6, panelThickness-textEmbossDepth+0.01]) {
            linear_extrude(textEmbossDepth) {
                text("−", size=12, 
                     font="Arial:style=Bold", 
                     spacing=0.95, 
                     halign = "center", 
                     valign = "center",
                     $fn=32);
            }
        }
        /*********************/
    }
}

frontPanel(panelHeight, panelWidth, panelThickness);
