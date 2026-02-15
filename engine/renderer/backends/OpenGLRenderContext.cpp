#include "renderer/backends/OpenGLRenderContext.h"

#if defined(FRESH_OPENGL_SUPPORT) && defined(FRESH_GLEW_AVAILABLE)

    #include <iostream>

#ifdef _WIN32
    // Win32 doesn't use GLFW for OpenGL in this implementation
#else
    #include <GLFW/glfw3.h>
#endif

    #include "core/Logger.h"
#ifdef _WIN32
    #include "core/Win32Window.h"
#endif
    #include "core/Window.h"

namespace fresh
{

// Helper struct to abstract window operations (OpenGL context is handled by window)
struct WindowAdapter {
    static uint32_t getWidth(void* win) {
#ifdef _WIN32
        auto* w = static_cast<Win32Window*>(win);
#else
        auto* w = static_cast<Window*>(win);
#endif
        return w->getWidth();
    }
    
    static uint32_t getHeight(void* win) {
#ifdef _WIN32
        auto* w = static_cast<Win32Window*>(win);
#else
        auto* w = static_cast<Window*>(win);
#endif
        return w->getHeight();
    }
};

// Helper classes for OpenGL resources
class GLBuffer : public RenderBuffer
{
public:
    GLBuffer(GLuint buf, GLenum type, size_t sz) : buffer(buf), bufferType(type), size(sz) {}

    ~GLBuffer()
    {
        if (buffer) {
            glDeleteBuffers(1, &buffer);
        }
    }

    void bind() override
    {
        glBindBuffer(bufferType, buffer);
    }

    void unbind() override
    {
        glBindBuffer(bufferType, 0);
    }

    void updateData(const void* data, size_t dataSize, size_t offset) override
    {
        glBindBuffer(bufferType, buffer);
        glBufferSubData(bufferType, offset, dataSize, data);
        glBindBuffer(bufferType, 0);
    }

    size_t getSize() const override
    {
        return size;
    }
    void* getNativeHandle() override
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(buffer));
    }

private:
    GLuint buffer;
    GLenum bufferType;
    size_t size;
};

class GLTexture : public RenderTexture
{
public:
    GLTexture(GLuint tex, int w, int h) : texture(tex), width(w), height(h) {}

    ~GLTexture()
    {
        if (texture) {
            glDeleteTextures(1, &texture);
        }
    }

    void bind(int unit) override
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void unbind() override
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    int getWidth() const override
    {
        return width;
    }
    int getHeight() const override
    {
        return height;
    }
    void* getNativeHandle() override
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(texture));
    }

private:
    GLuint texture;
    int width;
    int height;
};

class GLShader : public RenderShader
{
public:
    GLShader(GLuint prog) : program(prog) {}

    ~GLShader()
    {
        if (program) {
            glDeleteProgram(program);
        }
    }

    void bind() override
    {
        glUseProgram(program);
    }

    void unbind() override
    {
        glUseProgram(0);
    }

    void setUniformInt(const std::string& name, int value) override
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUseProgram(program);
            glUniform1i(location, value);
        }
    }

    void setUniformFloat(const std::string& name, float value) override
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUseProgram(program);
            glUniform1f(location, value);
        }
    }

    void setUniformVec2(const std::string& name, const glm::vec2& value) override
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUseProgram(program);
            glUniform2fv(location, 1, &value[0]);
        }
    }

    void setUniformVec3(const std::string& name, const glm::vec3& value) override
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUseProgram(program);
            glUniform3fv(location, 1, &value[0]);
        }
    }

    void setUniformVec4(const std::string& name, const glm::vec4& value) override
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUseProgram(program);
            glUniform4fv(location, 1, &value[0]);
        }
    }

    void setUniformMat4(const std::string& name, const glm::mat4& value) override
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location != -1) {
            glUseProgram(program);
            glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
        }
    }

    void* getNativeHandle() override
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(program));
    }

private:
    GLuint program;
};

OpenGLRenderContext::OpenGLRenderContext() : window(nullptr), width(0), height(0), defaultVAO(0) {}

OpenGLRenderContext::~OpenGLRenderContext()
{
    shutdown();
}

bool OpenGLRenderContext::initialize(void* win)
{
    LOG_INFO_C("Initializing OpenGL render context...", "OpenGLRenderContext");

    if (!win) {
        LOG_ERROR_C("Invalid window pointer", "OpenGLRenderContext");
        return false;
    }

    window = win;
    width = WindowAdapter::getWidth(window);
    height = WindowAdapter::getHeight(window);

    // Initialize GLEW for OpenGL extension loading
    if (!initializeGLEW()) {
        LOG_ERROR_C("Failed to initialize GLEW", "OpenGLRenderContext");
        return false;
    }

    // Print OpenGL information
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    LOG_INFO_C("OpenGL Vendor: " + std::string(reinterpret_cast<const char*>(vendor)),
               "OpenGLRenderContext");
    LOG_INFO_C("OpenGL Renderer: " + std::string(reinterpret_cast<const char*>(renderer)),
               "OpenGLRenderContext");
    LOG_INFO_C("OpenGL Version: " + std::string(reinterpret_cast<const char*>(version)),
               "OpenGLRenderContext");
    LOG_INFO_C("GLSL Version: " + std::string(reinterpret_cast<const char*>(glslVersion)),
               "OpenGLRenderContext");

    // Create default VAO (required for core profile)
    glGenVertexArrays(1, &defaultVAO);
    glBindVertexArray(defaultVAO);

    // Set up initial OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    checkGLErrors("OpenGL initialization");

    LOG_INFO_C("OpenGL render context initialized successfully", "OpenGLRenderContext");
    return true;
}

void OpenGLRenderContext::shutdown()
{
    LOG_INFO_C("Shutting down OpenGL render context...", "OpenGLRenderContext");

    if (defaultVAO) {
        glDeleteVertexArrays(1, &defaultVAO);
        defaultVAO = 0;
    }

    LOG_INFO_C("OpenGL render context shutdown complete", "OpenGLRenderContext");
}

bool OpenGLRenderContext::beginFrame()
{
    // Update viewport size from actual framebuffer size (handles window resize)
    if (window) {
        int framebufferWidth, framebufferHeight;
#ifdef _WIN32
        auto* w = static_cast<Win32Window*>(window);
        framebufferWidth = static_cast<int>(w->getWidth());
        framebufferHeight = static_cast<int>(w->getHeight());
#else
        auto* w = static_cast<Window*>(window);
        glfwGetFramebufferSize(w->getHandle(), &framebufferWidth, &framebufferHeight);
#endif
        width = framebufferWidth;
        height = framebufferHeight;
    }

    // Bind the default framebuffer (required in core profile)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGLErrors("Bind default framebuffer");

    // Set clear values and clear the framebuffer
    glClearColor(clearColorValue.r, clearColorValue.g, clearColorValue.b, clearColorValue.a);
    glClearDepth(clearDepthValue);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGLErrors("Clear framebuffer");

    return true;
}

void OpenGLRenderContext::endFrame()
{
    // Swap the front and back buffers to display the rendered frame
    if (window) {
#ifdef _WIN32
        auto* w = static_cast<Win32Window*>(window);
#else
        auto* w = static_cast<Window*>(window);
#endif
        w->swapBuffers();
    }
    checkGLErrors("End frame");
}

void OpenGLRenderContext::waitIdle()
{
    // OpenGL is synchronous by default
    glFinish();
}

void OpenGLRenderContext::setViewport(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);
    width = w;
    height = h;
    checkGLErrors("Set viewport");
}

void OpenGLRenderContext::setScissor(int x, int y, int w, int h)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, w, h);
    checkGLErrors("Set scissor");
}

void OpenGLRenderContext::clearColor(float r, float g, float b, float a)
{
    // Store the clear color value for use in beginFrame()
    clearColorValue = glm::vec4(r, g, b, a);
}

void OpenGLRenderContext::clearDepth(float depth)
{
    // Store the clear depth value for use when clearing
    clearDepthValue = depth;
}

std::shared_ptr<RenderBuffer> OpenGLRenderContext::createVertexBuffer(const void* data, size_t size)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    checkGLErrors("Create vertex buffer");
    return std::make_shared<GLBuffer>(buffer, GL_ARRAY_BUFFER, size);
}

std::shared_ptr<RenderBuffer> OpenGLRenderContext::createIndexBuffer(const void* data, size_t size)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    checkGLErrors("Create index buffer");
    return std::make_shared<GLBuffer>(buffer, GL_ELEMENT_ARRAY_BUFFER, size);
}

std::shared_ptr<RenderBuffer> OpenGLRenderContext::createUniformBuffer(size_t size)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    checkGLErrors("Create uniform buffer");
    return std::make_shared<GLBuffer>(buffer, GL_UNIFORM_BUFFER, size);
}

std::shared_ptr<RenderTexture> OpenGLRenderContext::createTexture(int w, int h, const void* data)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    checkGLErrors("Create texture");
    return std::make_shared<GLTexture>(texture, w, h);
}

std::shared_ptr<RenderShader> OpenGLRenderContext::createShader(const std::string& vertexCode,
                                                                const std::string& fragmentCode)
{
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexSrc = vertexCode.c_str();
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    // Check compilation
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        LOG_ERROR_C("Vertex shader compilation failed: " + std::string(infoLog),
                    "OpenGLRenderContext");
        glDeleteShader(vertexShader);
        return nullptr;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSrc = fragmentCode.c_str();
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        LOG_ERROR_C("Fragment shader compilation failed: " + std::string(infoLog),
                    "OpenGLRenderContext");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return nullptr;
    }

    // Link shaders into program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        LOG_ERROR_C("Shader program linking failed: " + std::string(infoLog),
                    "OpenGLRenderContext");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return nullptr;
    }

    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    checkGLErrors("Create shader");
    return std::make_shared<GLShader>(program);
}

bool OpenGLRenderContext::initializeGLEW()
{
    // Set glewExperimental for core profile compatibility
    // This is needed for OpenGL 3.2+ core/compatibility profiles
    glewExperimental = GL_TRUE;

    // Make sure we have a current GL context
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        LOG_ERROR_C("GLEW initialization failed: " +
                        std::string(reinterpret_cast<const char*>(glewGetErrorString(err))),
                    "OpenGLRenderContext");
        return false;
    }

    // Clear any OpenGL errors that may have been generated by glewInit()
    // glewInit() can cause GL_INVALID_ENUM on some drivers, which is a known issue
    glGetError();

    // Check for required OpenGL version (4.5+ preferred, 4.0+ minimum)
    if (!GLEW_VERSION_4_5) {
        LOG_WARNING_C("OpenGL 4.5 not supported, trying 4.0+", "OpenGLRenderContext");
        if (!GLEW_VERSION_4_0) {
            LOG_ERROR_C("OpenGL 4.0+ required but not available", "OpenGLRenderContext");
            return false;
        }
    }

    return true;
}

void OpenGLRenderContext::checkGLErrors(const char* context)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (err) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            error = "UNKNOWN";
            break;
        }
        LOG_ERROR_C("OpenGL error in " + std::string(context) + ": " + error,
                    "OpenGLRenderContext");
    }
}

} // namespace fresh

#endif // FRESH_OPENGL_SUPPORT && FRESH_GLEW_AVAILABLE
