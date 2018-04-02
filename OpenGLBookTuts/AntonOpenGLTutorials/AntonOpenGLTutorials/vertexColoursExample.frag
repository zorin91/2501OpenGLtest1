
# version 410

in vec3 colour;
out vec4 frag_colour;
uniform sampler2D basic_texture;

void main() 
{
 frag_colour = vec4(colour, 1.0);
}
