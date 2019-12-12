#include <toolkit/render_window.hpp>
#include <toolkit/opencl.hpp>
#include <array>
#include <SFML/System.hpp>

#define CHUNK_SIZE 64

struct chunk_descriptor
{
    vec4f pos;
};

///Y GOES UP, Z IS INTO SCREEN, X GOES RIGHT
struct chunk_data
{
    std::array<int, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> chunks;
};

struct level
{
    std::vector<chunk_descriptor> descriptors;
    std::vector<chunk_data> chunks;
};

#define IDX(x, y, z) z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x

level make_level()
{
    level ret;

    ret.descriptors.emplace_back();
    chunk_descriptor& dummy = ret.descriptors.back();
    dummy.pos = {0,0,0,0};

    ret.chunks.emplace_back();
    chunk_data& dummydata = ret.chunks.back();

    for(int z=0; z < CHUNK_SIZE; z++)
    {
        for(int y=0; y < CHUNK_SIZE; y++)
        {
            for(int x=0; x < CHUNK_SIZE; x++)
            {
                int val = 0;

                if(y < CHUNK_SIZE / 2)
                    val = 1;

                dummydata.chunks.at(IDX(x, y, z)) = val;
            }
        }
    }

    return ret;
}

int main()
{
    render_settings sett;
    sett.width = 1400;
    sett.height = 1000;
    sett.viewports = true;
    sett.opencl = true;

    render_window window(sett, "hello");

    cl::context& ctx = window.clctx->ctx;

    cl::program prog(ctx, std::vector<std::string>{"./deps/toolkit/kernels.cl", "kernels.cl"});
    prog.build(ctx, "");

    ctx.register_program(prog);

    cl::command_queue cqueue(ctx);

    printf("No?\n");

    level test_level = make_level();

    printf("Here\n");

    std::vector<cl::buffer> buf_descs;
    std::vector<cl::buffer> buf_chunks;

    for(int i=0; i < (int)test_level.chunks.size(); i++)
    {
        buf_descs.emplace_back(ctx);
        buf_chunks.emplace_back(ctx);

        buf_descs.back().alloc(sizeof(chunk_descriptor));
        buf_descs.back().write(cqueue, std::vector<chunk_descriptor>{test_level.descriptors[i]});

        buf_chunks.back().alloc(sizeof(chunk_data));
        buf_chunks.back().write(cqueue, std::vector<chunk_data>{test_level.chunks[i]});
    }

    printf("Hello\n");

    while(!window.should_close())
    {
        window.poll();

        window.display();
    }
}
