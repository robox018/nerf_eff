#include <math.h>
#include <string.h>//memset
#include <iostream>
#include <fstream>
#include "tga.h"
namespace myimg {
	TGAimage::TGAimage() : data(NULL), width(0), height(0), bytespp(0) {}

	TGAimage::TGAimage(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp) {
		unsigned long nbytes = width * height * bytespp;
		data = new unsigned char[nbytes];
		memset(data, 100, nbytes);
	}
	TGAimage::TGAimage(int w, int h, int bpp, unsigned char*mdata) : data(mdata), width(w), height(h), bytespp(bpp) {
		unsigned long nbytes = width * height * bytespp;
	}

	TGAimage::~TGAimage() {
		if (data) delete[] data;
	}

	bool TGAimage::write_tga_file(const char* filename, bool rle) {
		std::ofstream out;//从内存到硬盘,输出流对象out
		out.open(filename, std::ios::binary);//换行符是单字符
		if (!out.is_open()) {
			std::cerr << "can't open file " << filename << "\n";
			out.close();
			return false;
		}
		TGA_header header;
		memset((void*)&header, 0, sizeof(header));
		header.datatypecode = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
		header.width = width;
		header.height = height;
		header.bitsperpixel = bytespp << 3;//乘以8
		//header.imagedescriptor = 0x20;//00000010，top-left
		out.write((char*)&header, sizeof(header));
		if (!out.good()) {
			std::cerr << "can't write header\n";
			out.close();
		}
		if (!rle) {
			out.write((char*)data, width * height * bytespp);
			if (!out.good()) {
				std::cerr << "can't write the image\n";
				out.close();
				return false;
			}
		}
		else {
			if (!unload_tga_file(out)) {
				std::cerr << "can't unload the image\n";
				out.close();
				return false;
			}
		}

		unsigned char extention_area[4] = { 0,0,0,0 };
		unsigned char developer_area[4] = { 0,0,0,0 };
		//unsigned char signature[16] = { 'T','R','}
		std::string signatrue = "TRUEVISION-XFILE";
		unsigned char foot[2] = { '.','\0' };
		out.write((char*)extention_area, sizeof(extention_area));//在写入单个变量时，可以将变量本身视为缓冲区并传递它的地址(&)。
		//但是，在使用数组作为缓冲区时，只要传递数组就可以了，因为数组已经是一个地址。
		if (!out.good()) {
			std::cerr << "can't write the extention_area\n";
			out.close();
			return false;
		}
		out.write((char*)developer_area, sizeof(developer_area));
		if (!out.good()) {
			std::cerr << "can't write the developer_area\n";
			out.close();
			return false;
		}
		out.write((char*)signatrue.c_str(), signatrue.size());//c_str()生成一个const char*指针
		if (!out.good()) {
			std::cerr << "can't write the signature\n";
			out.close();
			return false;
		}
		out.write((char*)foot, sizeof(foot));
		if (!out.good()) {
			std::cerr << "can't write the foot\n";
			out.close();
			return false;
		}
		out.close();

		return true;
	}

	bool TGAimage::unload_tga_file(std::ofstream& out) {
		const unsigned char max_length = 128;
		unsigned long npixels = width * height;
		unsigned long nowpix = 0;
		while (nowpix < npixels) {
			bool consistency = true;//记录当前队列是同一个数还是不同的数
			unsigned char nowlength = 1;//用char，之后写入内存时刚好占一字节;=1使接下来的循环条件有效
			unsigned long nowbit = nowpix * bytespp;
			unsigned long bytestart = nowpix * bytespp;
			while (nowpix + nowlength < npixels && nowlength < max_length) {
				bool same2 = true;
				for (int i = 0; same2 && i < bytespp; i++) {
					same2 = (data[nowbit + i] == data[nowbit + i + bytespp]);
				}
				nowbit += bytespp;
				if (nowlength == 1) {
					consistency = same2;
					nowlength++;
					continue;
				}
				if (same2 && !consistency) {//不同数的队列结束
					nowlength--;
					break;
				}
				if (!same2 && consistency) {//相同数的队列结束
					break;
				}
				nowlength++;
			}
			nowpix += nowlength;
			out.put(consistency ? nowlength + 127 : nowlength - 1);//参考load函数，以128为界
			if (!out.good()) {
				std::cerr << "can't unload the count\n";
				return false;
			}
			out.write((char*)(data + bytestart), (consistency ? bytespp : nowlength * bytespp));
			if (!out.good()) {
				std::cerr << "can't unload the data\n";
				return false;
			}
		}
		return true;
	}

	bool TGAimage::load_rle_data(std::ifstream& in) {
		unsigned long pixelcount = width * height;
		unsigned long currentpixel = 0;
		unsigned long currentbyte = 0;
		color colorbuffer;
		do {
			unsigned char head = 0;
			head = in.get();
			if (!in.good()) {
				std::cerr << "an error occured while reading the data\n";
				return false;
			}
			if (head < 128) {
				head++;
				for (int i = 0; i < head; i++) {
					in.read((char*)colorbuffer.raw, bytespp);
					if (!in.good()) {
						std::cerr << "an error occured while reading the header\n";
						return false;
					}
					for (int t = 0; t < bytespp; t++)
						data[currentbyte++] = colorbuffer.raw[t];
					currentpixel++;
					if (currentpixel > pixelcount) {
						std::cerr << "Too many pixels read\n";
						return false;
					}
				}
			}
			else {
				head -= 127;
				in.read((char*)colorbuffer.raw, bytespp);
				if (!in.good()) {
					std::cerr << "an error occured while reading the header\n";
					return false;
				}
				for (int i = 0; i < head; i++) {
					for (int t = 0; t < bytespp; t++)
						data[currentbyte++] = colorbuffer.raw[t];
					currentpixel++;
					if (currentpixel > pixelcount) {
						std::cerr << "Too many pixels read\n";
						return false;
					}
				}
			}
		} while (currentpixel < pixelcount);
		return true;
	}

	bool TGAimage::setpoint(int x, int y, color c) {
		if (!data || x < 0 || y < 0 || x >= width || y >= height) return false;
		memcpy(data + (x + y * width) * bytespp, c.raw, bytespp);
		return true;
	}

	bool TGAimage::read_tga_file(const char* filename) {
		if (data) delete[] data;
		data = NULL;
		std::ifstream in;
		in.open(filename, std::ios::binary);
		if (!in.is_open()) {
			std::cerr << "can't open file " << filename << "\n";
			in.close();
			return false;
		}
		TGA_header header;
		in.read((char*)&header, sizeof(header));
		if (!in.good()) {
			in.close();
			std::cerr << "an error occured while reading the header\n";
			return false;
		}
		width = header.width;
		height = header.height;
		bytespp = header.bitsperpixel >> 3;
		if (width <= 0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA)) {
			in.close();
			std::cerr << "bad bpp (or width/height) value\n";
			return false;
		}
		unsigned long nbytes = bytespp * width * height;
		data = new unsigned char[nbytes];
		if (3 == header.datatypecode || 2 == header.datatypecode) {
			in.read((char*)data, nbytes);
			if (!in.good()) {
				in.close();
				std::cerr << "an error occured while reading the data\n";
				return false;
			}
		}
		else if (10 == header.datatypecode || 11 == header.datatypecode) {
			if (!load_rle_data(in)) {
				in.close();
				std::cerr << "an error occured while reading the data\n";
				return false;
			}
		}
		else {
			in.close();
			std::cerr << "unknown file format " << (int)header.datatypecode << "\n";
			return false;
		}
		std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
		in.close();
		return true;
	}

	color TGAimage::get(int x, int y) {
		if (!data || x < 0 || y < 0 || x >= width || y >= height) {
			return color();
		}
		return color(data + (x + y * width) * bytespp, bytespp);
	}
	int TGAimage::get_bytespp() {
		return bytespp;
	}

	int TGAimage::get_width() {
		return width;
	}

	int TGAimage::get_height() {
		return height;
	}
}
