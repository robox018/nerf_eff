#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>
#include <iostream>
#pragma pack(push,1)
namespace myimg {
	struct TGA_header {//tga�ļ�ͷ��18�ֽ�
		char idlength;
		char colormaptype;
		char datatypecode;//ͼ������,��10 - �г̱��룬���ͼ��
		short colormaporigin;
		short colormaplength;
		char colormapdepth;
		short x_origin;//ͼ�����½ǵ�ˮƽ����
		short y_origin;//ͼ�����½ǵĴ�ֱ����
		short width;//ͼ����
		short height;//ͼ����
		char  bitsperpixel;//ÿ������ռ�õ�λ����ͨ����ֵΪ8��16��24��32λ
		char  imagedescriptor;//0-3λ���涨ÿ����������λ������; 4-5λ����ʾ�������ݴ��ļ����͵���Ļ��˳��
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

		color& operator =(const color& c) {//��һ��&������ʱ�������¶���,���Ч�ʣ��ڶ���&�����ں�������ʱ��ʵ�ε�һ�ο��������Ч��
			if (this != &c) {
				bytespp = c.bytespp;//������ɫ�ĸ�ֵ�������ı�rgba��ֻ�ı�bytespp
			}
			return *this;//���Ҹ�ֵ�Ļ�ֱ�ӷ���
			//return *this���ص��ǵ�ǰ����Ŀ�¡���߱�������������ΪA�� ���ǿ�¡�� ����������ΪA&�� ���Ǳ��� ��
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
		unsigned char* data;//ָ�������ռ8���ֽ�(64λϵͳ)����������һ����ַ��
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
		bool write_tga_file(const char* filename, bool rle = true);//ֵ����ʱ��Ϊ��ʡ�ڴ泣��ָ����Ϊ����������Ϊ��ֹ�޸�Ԫ���ݣ���const
		bool read_tga_file(const char* filename);
		color get(int x, int y);
		bool setpoint(int x, int y, color c);
		int get_width();
		int get_height();
		int get_bytespp();

	};

}
#endif //__IMAGE_H__