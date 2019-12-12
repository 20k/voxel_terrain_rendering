#define CHUNK_SIZE 64

struct chunk_descriptor
{
    float4 pos;
};

#define IDX(x, y, z) z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x

__kernel
void render_chunk(__global struct chunk_descriptor* my_chunk, __global int* chunks, float4 camera_pos, float4 camera_quat, __write_only image2d_t screen)
{
    int3 pos = (int3){get_global_id(0), get_global_id(1), get_global_id(2)};

    if(any(pos >= CHUNK_SIZE))
        return;


}
