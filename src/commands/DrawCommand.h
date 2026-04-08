// Added 'public' so the Manager can actually call the methods
class DrawCommand : public Command {
private:
    SDL_Surface* beforeSnapshot;
    SDL_Surface* afterSnapshot;

public:
    DrawCommand(SDL_Surface* before, SDL_Surface* after) {
        // We make deep copies so the Command owns its own data
        beforeSnapshot = SDL_DuplicateSurface(before);
        afterSnapshot = SDL_DuplicateSurface(after);
    }

    ~DrawCommand() {
        if (beforeSnapshot) SDL_DestroySurface(beforeSnapshot);
        if (afterSnapshot) SDL_DestroySurface(afterSnapshot);
    }

    void execute(Canvas& canvas) override {
        // Blit the 'after' state for Redo
        SDL_BlitSurface(afterSnapshot, NULL, canvas.drawingSurface, NULL);
    }

    void undo(Canvas& canvas) override {
        // Blit the 'before' state for Undo
        SDL_BlitSurface(beforeSnapshot, NULL, canvas.drawingSurface, NULL);
    }
};
