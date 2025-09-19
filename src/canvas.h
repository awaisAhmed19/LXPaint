
#include "raylib.h"
struct canvas {
    float width;
    float height;

    string name;
};

struct RGBA {
    int alpha;
    int red;
    int green;
    int blue;
};

struct ctx {
    int lineWidth;
    int fillStyle;

    RGBA rgba;
};
