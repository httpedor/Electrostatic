#include "raylib.h"
#include "simulation.hpp"
#include "raymath.h"
#include <stdio.h>
#ifdef __EMSCRIPTEN__ 
#include <emscripten/emscripten.h>

EM_JS(int, canvas_get_width, (), {
    return document.getElementById("canvas").width;
});

EM_JS(int, canvas_get_height, (), {
    return document.getElementById("canvas").height;
});

EM_JS(void, resizeCanvas, (), {
    var canvas = document.getElementById("canvas");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
});
#endif

int lastHeight = 0;
int lastWidth = 0;
int chargeSize = 1;

Charge* dragging = nullptr;

void UpdateDrawFrame(void)
{

    int width;
    int height;
    #ifdef __EMSCRIPTEN__ 
        width = canvas_get_width();
        height = canvas_get_height();
    #else
        width = GetScreenWidth();
        height = GetScreenHeight();
    #endif
    if (width != lastWidth || height != lastHeight) {
        SimResize({(float)lastWidth, (float)lastHeight}, {(float)width, (float)height});
        lastWidth = width;
        lastHeight = height;
        SetWindowSize(width, height);
    }

    int chargeRadius = 20;
    int previousSize = chargeSize;
    chargeSize -= (int)GetMouseWheelMove();
    if (chargeSize == 0){
        if (previousSize < 0)
            chargeSize = 1;
        else
            chargeSize = -1;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (dragging == nullptr) {
            Vector2 mousePos = GetMousePosition();
            for (size_t i = 0; i < MAX_CHARGES; i++)
            {
                if (!charges[i].active)
                    continue;
                if (Vector2Distance(mousePos, charges[i].position) < chargeRadius)
                {
                    dragging = &charges[i];
                    break;
                }
            }
        }
    }

    if (dragging != nullptr){
        dragging->position = GetMousePosition();
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        dragging = nullptr;
    }

    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)){
        SimAddCharge(GetMousePosition(), 0);
    }

    if (IsKeyReleased(KEY_C)){
        SimAddCharge(GetMousePosition(), chargeSize);
    }
    if (IsKeyReleased(KEY_R)){
        SimClear();
    }
    if (IsKeyReleased(KEY_SPACE)){
        isPaused = !isPaused;
    }
    if (IsKeyReleased(KEY_BACKSPACE)){
        for (int i = 0; i < MAX_CHARGES; i++)
        {
            if (charges[i].charge == 0)
                continue;
            if (CheckCollisionPointCircle(GetMousePosition(), charges[i].position, chargeRadius))
            {
                SimRemoveCharge(i);
                break;
            }
        }
    }

    SimStep();

    BeginDrawing();
        ClearBackground({48, 44, 52, 255});
        DrawFPS(10, height - 25);
        
        int fontSize = 20;
        char text[12];
        sprintf(text, "Charge: %c%d", chargeSize > 0 ? '+' : '-', abs(chargeSize));
        DrawText(text, width - MeasureText(text, fontSize) - 10, fontSize/2, fontSize, WHITE);
        DrawText(isPaused ? "PAUSED" : "SIMULATING", 10, 10, fontSize, WHITE);

        for(int i = 0; i < MAX_CHARGES; i++){
            Charge& charge = charges[i];
            if (charge.active){
                fontSize = 20;
                DrawCircleV(charge.position, chargeRadius, charge.charge >= 0 ? RED : BLUE);
                char buf[4];
                snprintf(buf, 4, charge.charge < 0 ? "%d" : "+%d", charge.charge);
                int textSize = MeasureText(buf, fontSize);
                DrawText(buf, charge.position.x - textSize/2, charge.position.y - fontSize/2, fontSize, WHITE);
            }
        }

        int arrowRadius = 35;
        for (int i = 0; i < MAX_ARROWS; i++){
            FieldArrow& arrow = fieldArrows[i];
            if (arrow.magnitude > 0){
                Color color = {255, 255, 255, (unsigned char)((arrow.magnitude / biggestMagnitude) * 255)};

                Vector2 arrowStart = Vector2Subtract(arrow.position, Vector2Scale(arrow.direction, arrowRadius/2));
                Vector2 arrowEnd = Vector2Add(arrowStart, Vector2Scale(arrow.direction, arrowRadius));
                DrawLineEx(arrowStart, arrowEnd, arrowRadius/4, color);

                Vector2 arrowTriangle[3] = {
                    Vector2Add(arrowEnd, Vector2Scale({-arrow.direction.y, arrow.direction.x}, arrowRadius/4)),
                    Vector2Add(arrowEnd, Vector2Scale(arrow.direction, arrowRadius/4)),
                    Vector2Add(arrowEnd, Vector2Scale({arrow.direction.y, -arrow.direction.x}, arrowRadius/4))
                };
                DrawTriangle(arrowTriangle[0], arrowTriangle[1], arrowTriangle[2], color);
            }
        }

    EndDrawing();
}
int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 450, "Electrostatic");
    SimInit();
    SetTargetFPS(60);

    #ifdef __EMSCRIPTEN__
    resizeCanvas();
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    #else
    while (!WindowShouldClose()){
        UpdateDrawFrame();
    }
    #endif

    CloseWindow();

    return 0;
}
