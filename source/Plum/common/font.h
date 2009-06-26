#ifndef PLUM_FONT_H
#define PLUM_FONT_H
namespace Plum
{
	class Font
	{
		public:
			static const int FONT_COLUMNS = 20;
			static const int FONT_ROWS = 5;

			Texture* texture;
			int width, height;
			int letterSpacing;
			int glyphWidth[FONT_COLUMNS * FONT_ROWS];

			Font(std::string filename)
			{
				init(filename.c_str());
			}

			Font(const char* filename)
			{
				init(filename);
			}

			~Font()
			{
				delete texture;
			}
		private:
			void init(const char* filename);
			bool isColumnEmpty(int cell, int column);
		public:
			void enableVariableWidth();
			void printChar(int x, int y, char c, BlendMode mode, Color tint);
			void print(int x, int y, const std::string s, BlendMode mode = BlendUnspecified, Color tint = Color::White);
			void printRight(int x, int y, const std::string s, BlendMode mode = BlendUnspecified, Color tint = Color::White);
			void printCenter(int x, int y, const std::string s, BlendMode mode = BlendUnspecified, Color tint = Color::White);
			int lineWidth(std::string s, int lineIndex);
			int lineCount(std::string s);
			int textWidth(std::string s);
			int textHeight(std::string s);
			
	};
}
#endif