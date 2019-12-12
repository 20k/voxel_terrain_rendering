#include <toolkit/render_window.hpp>
#include <toolkit/opencl.hpp>

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


    while(!window.should_close())
    {
        window.poll();

        window.display();
    }
}
