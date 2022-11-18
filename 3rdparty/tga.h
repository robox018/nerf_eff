#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>
#include <iostream>
#pragma pack(push,1)
namespace myimg {
	struct TGA_header {//tga文件头，18字节
		char idlength;
		char colormaptype;
		char datatypecode;//图像类型,（10 - 行程编码，真彩图像）
		short colormaporigin;
		short colormaplength;
		char colormapdepth;
		short x_origin;//图像左下角的水平坐标
		short y_origin;//图像左下角的垂直坐标
		short width;//图像宽度
		short height;//图像宽度
		char  bitsperpixel;//每个像素占用的位数，通常的值为8、16、24、32位
		char  imagedescriptor;//0-3位：规定每个像素属性位的数量; 4-5位：表示像素数据从文件发送到屏幕的顺序
	};
#pragma pack(pop)

	struct color {
		union {
			struct {
				unsigned char b, g, r, a;
			};
			unsigned char raw[4];
		};
		int bytespp;
		color() :raw(), bytespp(1) {
			for (int i = 0; i < 4; i++) raw[i] = 0;
		}
		color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) :b(B), g(G), r(R), a(A), bytespp(4) {}

		color& operator =(const color& c) {//第一个&，返回时不创建新对象,提高效率；第二个&避免在函数调用时对实参的一次拷贝，提高效率
			if (this != &c) {
				bytespp = c.bytespp;//这里颜色的赋值操作不改变rgba，只改变bytespp
			}
			return *this;//自我赋值的话直接返回
			//return *this返回的是当前对象的克隆或者本身（若返回类型为A， 则是克隆， 若返回类型为A&， 则是本身 ）
		}
		color operator -(const color& c) {
			return color(r - c.r, g - c.g, b - c.b, a);
		}
		color operator +(const color& c) {
			return color(r + c.r, g + c.g, b + c.b, a);
		}
		color operator *(float c) {
			return color(int(r * c), int(g * c), int(b * c), a);
		}

		color(const unsigned char* p, unsigned char bpp) : raw(), bytespp(bpp) {
			for (int i = 0; i < (int)bpp; i++) {
				raw[i] = p[i];
			}
		}

	};

	class TGAimage {
	private:
		unsigned char* data;//指针变量，占8个字节(64位系统)，用来保存一个地址。
		int width;
		int height;
		int bytespp;//bytes_per_pixel 
		bool unload_tga_file(std::ofstream& out);
		bool load_rle_data(std::ifstream& in);

	public:
		enum format {
			GRAYSCALE = 1, RGB = 3, RGBA = 4
		};
		TGAimage();
		TGAimage(int w, int h, int bpp);
		TGAimage(int w, int h, int bpp, unsigned char* mdata);
		~TGAimage();
		bool write_tga_file(const char* filename, bool rle = true);//值传递时，为节省内存常用指针作为函数参数；为防止修改元数据，加const
		bool read_tga_file(const char* filename);
		color get(int x, int y);
		bool setpoint(int x, int y, color c);
		int get_width();
		int get_height();
		int get_bytespp();

	};

}
#endif //__IMAGE_H__