///////////////////////////////////////////////////
/// @file CHEEPROM_2026_PROTO_CASE
/// @brief case for the prototype
/// @copyright William R Cooke 2026
///////////////////////////////////////////////////
$fa = 1.0;
$fs = 0.2;

thick = 2.0;
width = 118;
depth = 93;

post_size = 8;
half_post_sz = post_size / 2;
post_height = 6;
height = post_height + 19;


// bottom
cube([width + 2 * thick, depth + 2 * thick, thick]);
// sides
cube([width + 2 * thick, thick, height + thick]);
translate([0, depth + thick, 0])
  cube([width + 2 * thick, thick, height + thick]);
  
// ends
difference()
{
  cube([thick, depth + 2 * thick, height + thick]);
  // removeusb connector
  translate([0,depth/2, height])
    cube([10,10,10], center = true);
}
difference()
{
  translate([width + thick, 0, 0])
    cube([thick, depth + 2* thick, height + thick]);
  // remove socket lever
    translate([width,48,post_height + 6])
    cube([8, 8, 16]);
}



// mounting posts
difference()
{
  translate([thick - 0.1, thick - 0.1, thick - 0.1]) 
    cube([post_size, post_size, post_height]);
  translate([thick -0.1 + half_post_sz, 
             thick - 0.1 + half_post_sz,
             thick]) 
    cylinder(post_height + 2,1,1);
}

difference()
{
  translate([103 + thick + 0.1, thick - 0.1, thick - 0.1]) 
    cube([post_size, post_size, post_height]);
  translate([103 + thick + 0.1 + half_post_sz, 
             thick - 0.1 + half_post_sz, 
             thick - 0.1]) 
    cylinder(post_height + 2, 1, 1);
}

difference()
{
  translate([103 + thick + 0.1, 
             depth - post_size + thick + 0.1,
             thick - 0.1])
    cube([post_size, post_size, post_height]);
  translate([103 + thick + 0.1 + half_post_sz, 
             depth - post_size + thick + 0.1 + half_post_sz,
             thick])
    cylinder(post_height + 2, 1, 1);
}

difference()
{
  translate([thick - 0.1, depth - post_size + thick + 0.1, 
             thick - 0.1]) 
    cube([post_size, post_size, post_height]);
  translate([thick -0.1 + half_post_sz, 
             depth - post_size + thick - 0.1 + half_post_sz,
             thick ]) 
    cylinder(post_height + 2,1,1);
}
  