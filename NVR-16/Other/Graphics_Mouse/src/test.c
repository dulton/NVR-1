
#include "APIs/Mouse.h"
#include "APIs/Graphics.h"

void test_mouse(void);
void test_graphics(void);


int main(int argc, char *argv[])
{
    //test_mouse();
    test_graphics();
}

void test_mouse(void)
{
    MOUSE_DATA mouse_data;
    
    if (0 != MouseCreate())
    {
        printf("MouseCreate failed");
        
        return;
    }


    while (1)
    {
        if (0 == MouseGetData(&mouse_data))
        {
            printf("mouse_data.key =0x%02x x=0x%02x y=0x%02x z=0x%02x  ax=0x%02x  zy=0x%02x\n"
                , (char)mouse_data.key, (char)mouse_data.x, (char)mouse_data.y, (char)mouse_data.z, mouse_data.ax, mouse_data.ay);

        }
        else
        {
            printf("MouseGetData failed\n");
            
            return;
        }
    }    
}


void fillcolor(GRAPHICS_SURFACE graph_surface, uchar r, uchar g, uchar b)
{
    int i, j;
    ushort color = 0;
    ushort *pmem = (ushort *)(graph_surface.mem);// + graph_surface.pitch * graph_surface.height);

   // printf("graph_surface.mem =%p, pmem=%p\n"
   //     , graph_surface.mem, pmem);

    color = 0x8000 | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
    
    for (i = 0; i < graph_surface.height; i++)
    {
        for (j = 0; j < graph_surface.width; j++)
        {
            pmem[j] = color;
        }
 
        pmem += graph_surface.height;//graph_surface.pitch;
    }
}

void test_graphics(void)
{
    GRAPHICS_SURFACE graph_surface;
    
    if (0 != GraphicsCreate())
    {
        return;
    }

    GraphicsSetVstd(0);//VIDEO_STANDARD_PAL

    //GraphicsSetAlpha(0xff, 0);


    if (0 == GraphicsGetBuffer(&graph_surface))
    {
        printf("GraphicsGetBuffer success graph_surface.format=%d mem=%p width=%d height=%d pitch=%d\n"
            , graph_surface.format, graph_surface.mem, graph_surface.width,  graph_surface.height,  graph_surface.pitch);

        while (1)
        {
            fillcolor(graph_surface, 0xff, 0, 0);
            usleep(5 * 1000 * 1000);
            fillcolor(graph_surface, 0, 0xff, 0);
            usleep(5 * 1000 * 1000);
            fillcolor(graph_surface, 0, 0, 0xff);
            usleep(5 * 1000 * 1000);
        }
    }
    else
    {
        printf("GraphicsGetBuffer failed\n");
    }
}


