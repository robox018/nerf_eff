
#include "bgfx_utils.h"
#include "n3tree.hpp"

#include "renderer.hpp"
#include "tga.h"

#include <cstdint>
#include <string>


namespace volrend{

	namespace
	{
		struct PosVertex
		{
			float m_x;
			float m_y;
			float m_z;

			static void init()
			{
				ms_layout
					.begin()
					.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
					.end();
			}

			static bgfx::VertexLayout ms_layout;
		};

		bgfx::VertexLayout PosVertex::ms_layout;

		static PosVertex quad_verts[] =
		{
		{-1.0f,  1.0f, 0.5f },
		{ 1.0f,  1.0f, 0.5f},
		{-1.0f, -1.0f, 0.5f },
		{ 1.0f, -1.0f, 0.5f},

		};

		static const uint16_t IndexList[] =
		{
			0, 2, 1,
			1,2,3,
		};

	}  // namespace

	struct VolumeRenderer::Impl {
		Impl(Camera& camera, RenderOptions& options)
			: camera(camera), options(options) {
		}

		~Impl() {
		
		}

		void destroy()
		{
			bgfx::destroy(vbh);
			bgfx::destroy(ibh);
			bgfx::destroy(program);
			bgfx::destroy(m_tex_child);
			bgfx::destroy(m_tex_data);
			bgfx::destroy(cam_transform);
			bgfx::destroy(cam_fx);
			bgfx::destroy(cam_fy);
			bgfx::destroy(cam_width);
			bgfx::destroy(cam_height);
			bgfx::destroy(opt_step_size);
			
			bgfx::destroy(opt_stop_thresh);
			bgfx::destroy(opt_sigma_thresh);
			bgfx::destroy(opt_render_bbox_min);
			bgfx::destroy(opt_render_bbox_max);
			bgfx::destroy(opt_basis_minmax);
			bgfx::destroy(opt_rot_dirs);
			bgfx::destroy(tree_data_dim);
			bgfx::destroy(tree_child_dim);
			
			bgfx::destroy(tree_dim);
			bgfx::destroy(tree_data_format);
			bgfx::destroy(tree_basis_dim);
			bgfx::destroy(tree_offset);
			bgfx::destroy(tree_scale);
			bgfx::destroy(tree_ndc_width);
			bgfx::destroy(tree_data_tex);
			bgfx::destroy(tree_child_tex);
			bgfx::destroy(m_tex_data);
			bgfx::destroy(m_tex_child);
		}

		void start() {
			if (started_) return;

			resize(1280.f, 720.f);

			quad_init();
			shader_init();
			started_ = true;
		}

		void render() {

			if (!started_) return;

			camera._update();

			bgfx::setUniform(cam_transform, &camera.transform);
			bgfx::setUniform(cam_fx, &camera.fx);
			bgfx::setUniform(cam_fy, &camera.fy);
			bgfx::setUniform(cam_cen, &camera.cen);
			bgfx::setUniform(cam_width, &camera.width);
			bgfx::setUniform(cam_height, &camera.height);
			bgfx::setUniform(opt_step_size, &options.step_size);
			bgfx::setUniform(opt_render_bbox_min, &options.render_bbox_min);
			bgfx::setUniform(opt_render_bbox_max, &options.render_bbox_max);
			bgfx::setUniform(opt_stop_thresh, &options.stop_thresh);
			bgfx::setUniform(opt_sigma_thresh, &options.sigma_thresh);
			bgfx::setUniform(opt_basis_minmax, &options.basis_minmax);
			bgfx::setUniform(opt_rot_dirs, &options.rot_dirs);
			bgfx::setUniform(tree_basis_dim, &Tbasis_dim);
			bgfx::setUniform(tree_dim, &Tdata_dim);
			bgfx::setUniform(tree_data_format, &Tformat);
			bgfx::setUniform(tree_offset, offtem);
			bgfx::setUniform(tree_scale, scatem);
			bgfx::setUniform(tree_ndc_width, &ndc);
			bgfx::setUniform(tree_child_dim, &texCwid);
			bgfx::setUniform(tree_data_dim, &texDwid);

			

			bgfx::setState(BGFX_STATE_DEFAULT);
			bgfx::setVertexBuffer(0, vbh);
			bgfx::setIndexBuffer(ibh);

			bgfx::setTexture(0, tree_child_tex, m_tex_child);
			//bgfx::setTexture(0, tree_child_tex, m_textureColor);
			bgfx::setTexture(1, tree_data_tex, m_tex_data);
			//bgfx::setTexture(1, tree_data_tex, m_textureNormal);

			bgfx::submit(0, program);

			
		}


		void set(N3Tree& tree) {
			start();

			if (tree.capacity > 0) {
				this->tree = &tree;
				upload_data();
				upload_child_links();
				upload_tree_spec();
			}
			options.basis_minmax[0] = 0;
			options.basis_minmax[1] = std::max(tree.data_format.basis_dim - 1, 0);
			//m_tex_child= loadTexture("textures/child.dds");
			//m_tex_child = loadTexture("textures/fieldstone-rgba.dds");
		}


		void clear() { this->tree = nullptr; }

		void resize(const float width, const float height) {
			if (camera.width == width && camera.height == height) return;
			if (width <= 0 || height <= 0) return;
			camera.width = width;
			camera.height = height;
			//bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
		}

	private:
		void auto_size_2d(size_t size, size_t& width, size_t& height,
			int base_dim = 1) {
			if (size == 0) {
				width = height = 0;
				return;
			}
			width = std::sqrt(size);
			if (width % base_dim) {
				width += base_dim - width % base_dim;
			}
			height = (size - 1) / width + 1;
			
		}

		void upload_data() {
			const uint32_t data_size =
				tree->capacity * tree->N * tree->N * tree->N * tree->data_dim;
			size_t width, height;
			auto_size_2d(data_size, width, height, tree->data_dim);
			const size_t pad = width * height - data_size;
			texDwid = width;

		
			std::vector<half> tmp(data_size + pad);
			std::copy(tree->data_.data<half>(),tree->data_.data<half>() + data_size, tmp.begin());

			const bgfx::Memory* dmem = bgfx::copy((void*)tmp.data(), width * height*2);
			//std::vector<half>().swap(tmp);

			m_tex_data = bgfx::createTexture2D(
				uint16_t(width)
				, uint16_t(height)
				, false
				, 1
				, bgfx::TextureFormat::R16F
				, BGFX_TEXTURE_RT|BGFX_SAMPLER_MAG_POINT|BGFX_SAMPLER_MIN_POINT
				, dmem
			);

		}

		void upload_child_links() {
			const size_t child_size =
				size_t(tree->capacity) * tree->N * tree->N * tree->N;
			size_t width, height;
			auto_size_2d(child_size, width, height);
			const size_t pad = width * height - child_size;
			tree->child_.data_holder.resize((child_size + pad) * sizeof(int32_t));
			texCwid = width;
			//std::vector<int> tmp(child_size + pad);
			//std::vector<float> tmp2(child_size + pad);
			//std::copy(tree->child_.data<int>(), tree->data_.data<int>() + child_size, tmp.begin());
			//for (int k=0; k<child_size; k++)
			//{
			//	tmp2[k] = float(tmp[k]);
			//}
		
			int size = tree->child_.data_holder.size();
			int *bitcolor= new int[size / 4];
			for (int i = 0; i < size/4; ++i) {
				bitcolor[i] = (tree->child_.data_holder[i * 4] << 24 & 0xFF000000) |
					(tree->child_.data_holder[i * 4 + 1] << 16 & 0x00FF0000) |
					(tree->child_.data_holder[i * 4 + 2] << 8 & 0x0000FF00) |
					(tree->child_.data_holder[i * 4 + 3] & 0x000000FF);

			}

			//myimg::TGAimage childTGA(width, height, 4, (unsigned char*) & tree->child_.data_holder[0]);
			//childTGA.write_tga_file("mic_child.tga", false);
			
			//const bgfx::Memory* dmem = bgfx::copy(&(tree->child_.data_holder[0]), width * height * 4);

			const bgfx::Memory* dmem = bgfx::copy(tree->child_.data<int32_t>(), width * height * 4);
			//const bgfx::Memory* dmem = bgfx::copy(bitcolor, width * height * 4);
//#if 0
			 m_tex_child = bgfx::createTexture2D(
				uint16_t(width)
				, uint16_t(height)
				, false
				, 1
				, bgfx::TextureFormat::R32U
				, BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIN_POINT
				, dmem
			);
//#endif

		}

		void upload_tree_spec() {


			Tdata_dim = float(tree->data_dim);
			Tformat = float(tree->data_format.format);
			Tbasis_dim = float(tree->data_format.basis_dim);
			ndc = -1.f;
			
			for(int i=0;i<3;i++)
			{
				offtem[i] = tree->offset[i];
				scatem[i] = tree->scale[i];
			}
			
		}

		void shader_init() {

			cam_transform = bgfx::createUniform("Ctransform", bgfx::UniformType::Mat3);
			cam_fx = bgfx::createUniform("Cfx", bgfx::UniformType::Vec4);
			cam_fy = bgfx::createUniform("Cfy", bgfx::UniformType::Vec4);
			cam_cen = bgfx::createUniform("Ccen", bgfx::UniformType::Vec4);
			cam_width = bgfx::createUniform("Cwidth", bgfx::UniformType::Vec4);
			cam_height = bgfx::createUniform("Cheight", bgfx::UniformType::Vec4);
			opt_step_size = bgfx::createUniform("Ostep_size", bgfx::UniformType::Vec4);
			opt_stop_thresh = bgfx::createUniform("Ostop_thresh", bgfx::UniformType::Vec4);
			opt_sigma_thresh = bgfx::createUniform("Osigma_thresh", bgfx::UniformType::Vec4);
			opt_render_bbox_min = bgfx::createUniform("Orender_bbox_min", bgfx::UniformType::Vec4);
			opt_render_bbox_max = bgfx::createUniform("Orender_bbox_max", bgfx::UniformType::Vec4);
			opt_basis_minmax = bgfx::createUniform("Obasis_minmax", bgfx::UniformType::Vec4);
			opt_rot_dirs = bgfx::createUniform("Orot_dirs", bgfx::UniformType::Vec4);
			tree_data_dim = bgfx::createUniform("tree_data_dim", bgfx::UniformType::Vec4);
			tree_child_dim = bgfx::createUniform("tree_child_dim", bgfx::UniformType::Vec4);
			tree_dim = bgfx::createUniform("Tdata_dim", bgfx::UniformType::Vec4);
			tree_data_format = bgfx::createUniform("Tformat", bgfx::UniformType::Vec4);
			tree_basis_dim = bgfx::createUniform("Tbasis_dim", bgfx::UniformType::Vec4);
			tree_offset = bgfx::createUniform("Tcenter", bgfx::UniformType::Vec4);
			tree_scale = bgfx::createUniform("Tscale", bgfx::UniformType::Vec4);
			tree_ndc_width = bgfx::createUniform("Tndc_width", bgfx::UniformType::Vec4);
			tree_child_tex = bgfx::createUniform("tree_child_tex", bgfx::UniformType::Sampler);
			tree_data_tex = bgfx::createUniform("tree_data_tex", bgfx::UniformType::Sampler);

			program = loadProgram("vs_volrend", "fs_volrend");

		}

		void quad_init() {
			PosVertex::init();
			vbh = bgfx::createVertexBuffer(
				// Static data can be passed with bgfx::makeRef
				bgfx::makeRef(quad_verts, sizeof(quad_verts))
				, PosVertex::ms_layout
			);

			// Create static index buffer for triangle list rendering.
			ibh = bgfx::createIndexBuffer(
				// Static data can be passed with bgfx::makeRef
				bgfx::makeRef(IndexList, sizeof(IndexList))
			);


		}

		Camera& camera;
		RenderOptions& options;

		N3Tree* tree;

		
		float Tdata_dim;
		float Tformat;
		float Tbasis_dim;
		float ndc;
		float offtem[3];
		float scatem[3];
		float texDwid;
		float texCwid;

		bgfx::ProgramHandle program;
		bgfx::VertexBufferHandle vbh;
		bgfx::IndexBufferHandle ibh;

		bgfx::UniformHandle cam_transform;
		bgfx::UniformHandle cam_fx;
		bgfx::UniformHandle cam_fy;
		bgfx::UniformHandle cam_cen;
		bgfx::UniformHandle cam_width;
		bgfx::UniformHandle cam_height;
		bgfx::UniformHandle opt_step_size;
		bgfx::UniformHandle opt_stop_thresh;
		bgfx::UniformHandle opt_sigma_thresh;
		bgfx::UniformHandle opt_render_bbox_min;
		bgfx::UniformHandle opt_render_bbox_max;
		bgfx::UniformHandle opt_basis_minmax;
		bgfx::UniformHandle opt_rot_dirs;
		bgfx::UniformHandle tree_data_dim;
		bgfx::UniformHandle tree_child_dim;
		bgfx::UniformHandle tree_dim;
		bgfx::UniformHandle tree_data_format;
		bgfx::UniformHandle tree_basis_dim;
		bgfx::UniformHandle tree_offset;
		bgfx::UniformHandle tree_scale;
		bgfx::UniformHandle tree_ndc_width;
		bgfx::UniformHandle tree_data_tex;
		bgfx::UniformHandle tree_child_tex;
		bgfx::TextureHandle m_tex_data;
		bgfx::TextureHandle m_tex_child;

		

		bool started_ = false;
	};

	VolumeRenderer::VolumeRenderer()
		: impl_(std::make_unique<Impl>(camera, options)) {}

	VolumeRenderer::~VolumeRenderer() {}
	void VolumeRenderer::destroy() { impl_->destroy(); }

	void VolumeRenderer::render() { impl_->render(); }

	void VolumeRenderer::set(N3Tree& tree) { impl_->set(tree); }
	void VolumeRenderer::clear() { impl_->clear(); }

	void VolumeRenderer::resize(float width, float height) {
		impl_->resize(width, height);
	}

}



