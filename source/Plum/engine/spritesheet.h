#pragma once

#include "../video/color.h"
#include "../video/blending.h"

namespace plum
{
    class Canvas;
    class Image;
    class Spritesheet
    {
        private:
            Image* image;
        public:
            int frameWidth, frameHeight;
            int padding;
            int columns;

            Spritesheet(Canvas* canvas, int w, int h);
            Spritesheet(Image* img, int w, int h);
            ~Spritesheet();

        private:
            void init(Canvas* canvas, int w, int h);

        public:
            Image* getImage();
            void bind();
            Color getFramePixel(int f, int x, int y);
            void blitFrame(int x, int y, int f, BlendMode mode = BlendUnspecified, Color tint = Color::White);
            void rawBlitFrame(int x, int y, int f, double angle, double scale);
    };
}