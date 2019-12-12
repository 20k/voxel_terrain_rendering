#define CHUNK_SIZE 64

struct chunk_descriptor
{
    float4 pos;
};

__kernel
void render_chunk(__global struct chunk_descriptor* my_chunk, __global int* chunks, float4 camera_pos, float4 camera_quat, __write_only image2d_t screen)
{

}
