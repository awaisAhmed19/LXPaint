#pragma once
#include <stack>

#include "command.h"
#include "raylib.h"

class Canvas {
    RenderTexture2D mainTexture;
    RenderTexture2D bufferTexture;

    std::stack<Command*> undoStack;

   public:
    void init(int width, int height) {
        mainTexture = LoadRenderTexture(width, height);
        BeginTextureMode(mainTexture);
        ClearBackground(WHITE);
        EndTextureMode();

        bufferTexture = LoadRenderTexture(width, height);
        BeginTextureMode(bufferTexture);
        ClearBackground(BLANK);  // transparent
        EndTextureMode();
    }
    void applyCommand(Command* cmd) {
        cmd->execute(*this);
        undoStack.push(cmd);
    }

    void undo() {
        if (undoStack.empty()) return;
        Command* cmd = undoStack.top();
        undoStack.pop();
        cmd->undo(*this);
        delete cmd;
    }

    // draw into the canvasmainTexture
    void beginDrawMain() { BeginTextureMode(mainTexture); }
    void beginDrawBuffer() { BeginTextureMode(bufferTexture); }
    void endDraw() { EndTextureMode(); }

    // render canvasmainTexture to screen
    void render() {
        // RenderTexture is flipped vertically in raylib
        DrawTextureRec(mainTexture.texture,
                       {0, 0, (float)mainTexture.texture.width, -(float)mainTexture.texture.height},
                       {0, 0}, WHITE);
        DrawTextureRec(
            bufferTexture.texture,
            {0, 0, (float)bufferTexture.texture.width, -(float)bufferTexture.texture.height},
            {0, 0}, WHITE);
    }
    void clearBuffer() {
        BeginTextureMode(bufferTexture);
        ClearBackground(BLANK);  // {0,0,0,0} — fully transparent
        EndTextureMode();
    }

    void commitBuffer() {
        // copy buffer onto main canvas then clear buffer
        BeginTextureMode(mainTexture);
        DrawTextureRec(
            bufferTexture.texture,
            {0, 0, (float)bufferTexture.texture.width, -(float)bufferTexture.texture.height},
            {0, 0}, WHITE);
        EndTextureMode();
        clearBuffer();
    }

    Image captureSnapshot() { return LoadImageFromTexture(mainTexture.texture); }

    void restoreSnapshot(Image& snapshot) { UpdateTexture(mainTexture.texture, snapshot.data); }

    int getWidth() { return mainTexture.texture.width; }
    int getHeight() { return mainTexture.texture.height; }
};
