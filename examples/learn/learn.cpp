#include "common.h"
#include "bgfx_utils.h"

#include "n3tree.hpp"
#include "renderer.hpp"
#include "imgui/imgui.h"
#include <cstdio>


namespace volrend {

	namespace {
		class exvolrend : public entry::AppI
		{
		public:
			exvolrend(const char* _name, const char* _description, const char* _url)
				: entry::AppI(_name, _description, _url)
			{
			}

			void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
			{
				Args args(_argc, _argv);

				bool init_loaded = true;
				std::string fname = "F:\\mic.npz";
				tree.open(fname);
				width = _width;
				height = _height;
				float fx = -1.0;
				float fy = -1.0;

				if (fx > 0.f) {
					rend.camera.fx = fx;
				}
				if (fy <= 0.f) {
					rend.camera.fy = rend.camera.fx;
				}

				m_debug = BGFX_DEBUG_NONE;
				m_reset = BGFX_RESET_VSYNC;

				bgfx::Init init;
				init.type = args.m_type;
				init.vendorId = args.m_pciId;
				init.platformData.nwh = entry::getNativeWindowHandle(entry::kDefaultWindowHandle);
				init.platformData.ndt = entry::getNativeDisplayHandle();
				init.resolution.width = _width;
				init.resolution.height =_height;
				init.resolution.reset = m_reset;
				bgfx::init(init);

				// Enable debug text.
				bgfx::setDebug(m_debug);

				// Set view 0 clear state.
				bgfx::setViewClear(0
					, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
					, 0x443355FF
					, 1.0f
					, 0
				);
				bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
				imguiCreate();

				rend.set(tree);

			}

			int shutdown() override
			{
				imguiDestroy();
				rend.destroy();
				bgfx::shutdown();

				return 0;
			}

			bool update() override
			{
				if (!entry::processEvents(width, height, m_debug, m_reset, &m_mouseState))
				{

					imguiBeginFrame(m_mouseState.m_mx
						, m_mouseState.m_my
						, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
						| (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
						| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
						, m_mouseState.m_mz
						, uint16_t(width)
						, uint16_t(height)
					);

					showExampleDialog(this);

					imguiEndFrame();
				    //bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
					bgfx::touch(0);
					//const bx::Vec3 at = { .0f, 0.0f,  0.0f };
					//const bx::Vec3 eye = { 0.0f, 0.0f, -5.0f };
					//float view[16];
					//bx::mtxLookAt(view, eye, at);
					//float proj[16];
					//bx::mtxProj(proj, 60.0f, float(800) / float(800), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
					//bgfx::setViewTransform(0, view, proj);
					
					
					rend.render();
					
					bgfx::frame();
					return true;
				}
				return false;
			}

			entry::MouseState m_mouseState;
			N3Tree tree;
			VolumeRenderer rend;
			uint32_t m_debug;

			uint32_t m_reset;
			uint32_t width;
			uint32_t height;

		};

	} // namespace


}
#if 0
	int _main_(int _argc, char** _argv) {
		volrend::exvolrend app("learn", "how to do.", "null");
		app.init(_argc, _argv, 800, 800);
		bgfx::setViewRect(0, 0, 0, uint16_t(800), uint16_t(800));
		bgfx::touch(0);


		while(app.update())
		{
		}

		return app.shutdown();

	}
#endif

//#if 0
ENTRY_IMPLEMENT_MAIN(
	volrend::exvolrend
	, "volrend"
	, "Initialization"
	, "null"
);
//#endif
