#define CHUNK_SIZE 64
#define FOV_CONST 600.f

float3 depth_project_singular(float3 rotated, float width, float height, float fovc)
{
    float2 rxy = mad(rotated.xy, fovc / rotated.z, (float2){width/2.f, height/2.f});

    float3 ret;

    ret.xy = rxy;
    ret.z = rotated.z;

    return ret;
}

float3 rot_quat(const float3 point, float4 quat)
{
    quat = fast_normalize(quat);

    float3 t = 2.f * cross(quat.xyz, point);

    return point + quat.w * t + cross(quat.xyz, t);
}


struct chunk_descriptor
{
    float4 pos;
};

#define IDX(x, y, z) z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x

bool is_visible(__global int* chunks, int3 pos)
{
    int3 tl = pos - 1;
    int3 br = pos + 1;

    tl = clamp(tl, 0, CHUNK_SIZE-1);
    br = clamp(br, 0, CHUNK_SIZE-1);

    if(chunks[IDX(pos.x, pos.y, pos.z)] == 0)
        return false;

    //float4 chunk_pos = my_chunk.pos;

    ///optimisation is to only look in camera direction
    //float4 to_camera = camera_pos - chunk_pos;

    for(int z=tl.z; z <= br.z; z++)
    {
        for(int y=tl.y; y <= br.y; y++)
        {
            for(int x=tl.x; x <= br.x; x++)
            {
                if(chunks[IDX(x, y, z)] == 0)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

__kernel
void render_chunk(__global struct chunk_descriptor* my_chunk, __global int* chunks, float4 camera_pos, float4 camera_quat, __write_only image2d_t screen)
{
    int3 pos = (int3){get_global_id(0), get_global_id(1), get_global_id(2)};

    if(any(pos >= CHUNK_SIZE))
        return;

    bool is_vis = is_visible(chunks, pos);

    float3 chunk_pos = my_chunk->pos.xyz;

    if(!is_vis)
        return;

    float screen_width = get_image_width(screen);
    float screen_height = get_image_height(screen);

    float3 world_pos = (float3)(pos.x, pos.y, pos.z) + chunk_pos;

    float3 screen_pos = depth_project_singular(rot_quat(chunk_pos - camera_pos.xyz, camera_quat), screen_width, screen_height, FOV_CONST);

    if(screen_pos.z < 1 || any(screen_pos.xy < 0) || any(screen_pos.xy >= (float2)(screen_width, screen_height)))
       return;
}
