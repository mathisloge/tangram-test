#include <imgui.h>

#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/ImGuiIntegration/Widgets.h>
#include <Magnum/Math/Color.h>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

#include <platform_magnum.hpp>

namespace Magnum
{
    namespace Examples
    {

        using namespace Math::Literals;

        class ImGuiExample : public Platform::Application
        {
        public:
            explicit ImGuiExample(const Arguments &arguments);

            void drawEvent() override;

            void viewportEvent(ViewportEvent &event) override;

            void keyPressEvent(KeyEvent &event) override;
            void keyReleaseEvent(KeyEvent &event) override;

            void mousePressEvent(MouseEvent &event) override;
            void mouseReleaseEvent(MouseEvent &event) override;
            void mouseMoveEvent(MouseMoveEvent &event) override;
            void mouseScrollEvent(MouseScrollEvent &event) override;
            void textInputEvent(TextInputEvent &event) override;

        private:
            ImGuiIntegration::Context _imgui{NoCreate};

            bool _showDemoWindow = true;
            bool _showAnotherWindow = true;
            Color4 _clearColor = 0x72909aff_rgbaf;
            Float _floatValue = 0.0f;

            std::unique_ptr<Tangram::MagnumTexture> tangram_;
        };

        ImGuiExample::ImGuiExample(const Arguments &arguments) : Platform::Application{arguments,
                                                                                       Configuration{}.setTitle("Magnum ImGui Example").setWindowFlags(Configuration::WindowFlag::Resizable)}
        {
            ImGui::CreateContext();

            const Vector2 scaling{Vector2{framebufferSize()} * dpiScaling() / Vector2{windowSize()}};
            const Vector2 size{Vector2{windowSize()} / dpiScaling()};
            const auto dpi_scaling = framebufferSize().x() / size.x();
            _imgui = ImGuiIntegration::Context(*ImGui::GetCurrentContext(), size, windowSize(), framebufferSize());

            /* Set up proper blending to be used by ImGui. There's a great chance
       you'll need this exact behavior for the rest of your scene. If not, set
       this only for the drawFrame() call. */
            GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
                                           GL::Renderer::BlendEquation::Add);
            GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                           GL::Renderer::BlendFunction::OneMinusSourceAlpha);

#if !defined(MAGNUM_TARGET_WEBGL) && !defined(CORRADE_TARGET_ANDROID)
            /* Have some sane speed, please */
            setMinimalLoopPeriod(16);
#endif

            Tangram::setContext(Magnum::GL::Context::current());

            tangram_ = std::make_unique<Tangram::MagnumTexture>(512, 512, "file://D:/dev/tangram-test/build/scene.yaml", "NEXTZEN_API_KEY", "global.sdk_api_key");
        }

        void ImGuiExample::drawEvent()
        {
            GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

            _imgui.newFrame();

            /* Enable text input, if needed */
            if (ImGui::GetIO().WantTextInput && !isTextInputActive())
                startTextInput();
            else if (!ImGui::GetIO().WantTextInput && isTextInputActive())
                stopTextInput();

            /* 1. Show a simple window.
       Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appear in
       a window called "Debug" automatically */
            {
                ImGui::Text("Hello, world!");
                ImGui::SliderFloat("Float", &_floatValue, 0.0f, 1.0f);
                if (ImGui::ColorEdit3("Clear Color", _clearColor.data()))
                    GL::Renderer::setClearColor(_clearColor);
                if (ImGui::Button("Test Window"))
                    _showDemoWindow ^= true;
                if (ImGui::Button("Another Window"))
                    _showAnotherWindow ^= true;
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0 / Double(ImGui::GetIO().Framerate), Double(ImGui::GetIO().Framerate));
            }

            /* 2. Show another simple window, now using an explicit Begin/End pair */
            if (_showAnotherWindow)
            {
                ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
                ImGui::Begin("Another Window", &_showAnotherWindow, ImGuiWindowFlags_NoMove);

                const auto size = ImGui::GetWindowSize();

                ImGuiIntegration::image(tangram_->texture(), {static_cast<float>(size.x), static_cast<float>(size.y)});
                if (ImGui::IsItemHovered())
                {
                    static bool is_dragging = false;
                    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                    {
                        const auto mpos = ImGui::GetMousePos();
                        if (!is_dragging)
                        {
                            tangram_->handleStartDrag(mpos.x, mpos.y);
                            is_dragging = true;
                        }
                        const auto dmpos = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                        tangram_->handleDrag(mpos.x, mpos.y);
                    }
                    else
                    {
                        if (is_dragging)
                        {
                            tangram_->handleEndDrag();
                        }
                        is_dragging = false;
                    }
                }
                ImGui::SetCursorPos(ImGui::GetCursorStartPos());
                if (ImGui::Button("+"))
                    tangram_->zoomDelta(1.f);
                if (ImGui::Button("-"))
                    tangram_->zoomDelta(-1.f);

                tangram_->resizeScene(size.x * 2, size.y * 2);

                ImGui::End();
            }

            /* 3. Show the ImGui demo window. Most of the sample code is in
       ImGui::ShowDemoWindow() */
            if (_showDemoWindow)
            {
                ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
                ImGui::ShowDemoWindow();
            }

            /* Update application cursor */
            _imgui.updateApplicationCursor(*this);

            /* Set appropriate states. If you only draw ImGui, it is sufficient to
       just enable blending and scissor test in the constructor. */
            GL::Renderer::enable(GL::Renderer::Feature::Blending);
            GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
            GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
            GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

            _imgui.drawFrame();

            /* Reset state. Only needed if you want to draw something else with
            different state after. */
            GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
            GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
            GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
            GL::Renderer::disable(GL::Renderer::Feature::Blending);

            tangram_->render(ImGui::GetTime());

            swapBuffers();
            redraw();
        }

        void ImGuiExample::viewportEvent(ViewportEvent &event)
        {
            GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

            _imgui.relayout(Vector2{event.windowSize()} / event.dpiScaling(),
                            event.windowSize(), event.framebufferSize());
        }

        void ImGuiExample::keyPressEvent(KeyEvent &event)
        {
            if (_imgui.handleKeyPressEvent(event))
                return;
        }

        void ImGuiExample::keyReleaseEvent(KeyEvent &event)
        {
            if (_imgui.handleKeyReleaseEvent(event))
                return;
        }

        void ImGuiExample::mousePressEvent(MouseEvent &event)
        {
            if (_imgui.handleMousePressEvent(event))
                return;
        }

        void ImGuiExample::mouseReleaseEvent(MouseEvent &event)
        {
            if (_imgui.handleMouseReleaseEvent(event))
                return;
        }

        void ImGuiExample::mouseMoveEvent(MouseMoveEvent &event)
        {
            if (_imgui.handleMouseMoveEvent(event))
                return;
        }

        void ImGuiExample::mouseScrollEvent(MouseScrollEvent &event)
        {
            if (_imgui.handleMouseScrollEvent(event))
            {
                /* Prevent scrolling the page */
                event.setAccepted();
                return;
            }
        }

        void ImGuiExample::textInputEvent(TextInputEvent &event)
        {
            if (_imgui.handleTextInputEvent(event))
                return;
        }

    }
}

MAGNUM_APPLICATION_MAIN(Magnum::Examples::ImGuiExample)
