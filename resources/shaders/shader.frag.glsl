#version 450

layout(location = 0) in vec4 v_Color;

layout(location = 0) out vec4 o_Color;

void main() {
  
  /* First we test for lower range */
  if(v_Color.x < 0.5) {
      o_Color = vec4(0,1,0,0); /* Green if in lower range*/
  } else if(v_Color.x >= 0.5) { /*Then for the higher range */
      o_Color = vec4(1,0,0,0); /* Red if in higher range */
  } else {
      /* Now we have tested for all real values.
         If we end up here we know that the value must be undefined */
      o_Color = vec4(0,1,0,0); /* Blue if it's undefined */
  }

  o_Color = vec4( 0.0, 0.4, 1.0, 1.0 );
    o_Color = v_Color;
}
