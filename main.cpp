#include <toolkit/render_window.hpp>
#include <toolkit/opencl.hpp>
#include <array>
#include <SFML/System.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

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

    vec4f camera_pos = {0,0,0,0};
    quaternion camera_quat = quaternion().identity();

    sf::Clock clk;

    cl::gl_rendertexture rtex(ctx);
    rtex.create(window.get_window_size().x(), window.get_window_size().y());

    while(!window.should_close())
    {
        window.poll();

        double dt_s = clk.restart().asMicroseconds() / 1000. / 1000.;

        if(ImGui::IsKeyDown(GLFW_KEY_W))
        {
            camera_pos.z() -= 1000 * dt_s;
        }

        if(ImGui::IsKeyDown(GLFW_KEY_S))
        {
            camera_pos.z() += 1000 * dt_s;
        }

        if(ImGui::IsKeyDown(GLFW_KEY_D))
        {
            camera_pos.x() += 1000 * dt_s;
        }

        if(ImGui::IsKeyDown(GLFW_KEY_A))
        {
            camera_pos.x() -= 1000 * dt_s;
        }

        if(ImGui::IsKeyDown(GLFW_KEY_E))
        {
            camera_pos.y() += 1000 * dt_s;
        }

        if(ImGui::IsKeyDown(GLFW_KEY_Q))
        {
            camera_pos.y() -= 1000 * dt_s;
        }

        if(ImGui::IsKeyDown(GLFW_KEY_N))
        {
            std::cout << dt_s << std::endl;
        }

        quaternion quat;
        quat.load_from_axis_angle({0, 1, 0, -M_PI/5 * dt_s});

        if(ImGui::IsKeyDown(GLFW_KEY_RIGHT))
        {
            camera_quat = camera_quat * quat;
        }

        if(ImGui::IsKeyDown(GLFW_KEY_LEFT))
        {
            camera_quat = camera_quat * quat.inverse();
        }


        rtex.acquire(cqueue);
        rtex.clear(cqueue);
        //rtex.clear(). Unify image and cl gl textures

        for(int i=0; i < (int)buf_descs.size(); i++)
        {
            cl::args args;
            args.push_back(buf_descs[i]);
            args.push_back(buf_chunks[i]);
            args.push_back(camera_pos);
            args.push_back(camera_quat);
            args.push_back(rtex);

            cqueue.exec("render_chunk", args, {CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE}, {16, 16, 1});
        }

        rtex.unacquire(cqueue);

        cqueue.block();

        vec2f screen_pos = {window.get_window_position().x(), window.get_window_position().y()};

        vec2f window_dim = {window.get_window_size().x(), window.get_window_size().y()};

        window.render_texture(rtex.texture_id, screen_pos, screen_pos + window_dim);

        window.display();
    }
}
