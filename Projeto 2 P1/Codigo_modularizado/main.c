#include "Diamond_Blocks_visual.h"

int main(void)
{
    Visual_Init();

    while (!WindowShouldClose())
    {
        Visual_Update();
        Visual_Draw();
    }

    Visual_Unload();
    return 0;
}

