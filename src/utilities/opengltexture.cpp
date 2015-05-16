#include "opengltexture.h"

#include "../gamewindow.h"

#include <QImage>

OpenGLTexture::OpenGLTexture(GameWindow* gl, OpenGLTexture::Target target ) {
    MI_ASSERT( gl != nullptr );
    m_gl = gl;
    m_target = target;

    m_bufferId = 0;

    m_width = 1;
    m_height = 1;
    m_depth = 1;
    m_storageAllocated = false;

    setMinMagFilters(Nearest, Nearest);
    setWrapMode( Repeat );
}

OpenGLTexture::OpenGLTexture(GameWindow* gl, const std::string& filename ) {
    MI_ASSERT( gl != nullptr );

    m_gl = gl;
    m_target = OpenGLTexture::Target2D;

    m_bufferId = 0;

    m_width = 1;
    m_height = 1;
    m_depth = 1;
    m_storageAllocated = false;

    QImage image( filename.c_str() );

    MI_ASSERT( !image.isNull() );

    create();
    bind(0);
    setMinMagFilters(Nearest, Nearest);
    setWrapMode( Repeat );
    setSize( image.width(), image.height() );
    setFormat(OpenGLTexture::RGBA8_UNorm);
    allocateStorage();
    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);

    unsigned char* data = new unsigned char[image.width() * image.height() * 4];
    memset(data, 127, image.width() * image.height() * 4);
    //setData(OpenGLTexture::RGBA, OpenGLTexture::UInt8, 0, data);
    setData(OpenGLTexture::RGBA, OpenGLTexture::UInt8, 0, glImage.constBits());
    delete[] data;
}

OpenGLTexture::~OpenGLTexture() {
    if( m_bufferId != 0 )
        destroy();
}

OpenGLTexture::Target OpenGLTexture::target() const {
    return m_target;
}

// Creation and destruction
bool OpenGLTexture::create() {
    MI_ASSERT( m_gl != nullptr );

    m_gl->glGenTextures(1, &m_bufferId);
    return m_bufferId != 0;
}

void OpenGLTexture::destroy() {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );
    m_gl->glDeleteTextures(1, &m_bufferId);
}

bool OpenGLTexture::isCreated() const {
    return m_bufferId != 0;
}

GLuint OpenGLTexture::textureId() const {
    return m_bufferId;
}

// Binding and releasing
void OpenGLTexture::bind() {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glBindTexture(m_target, m_bufferId);
}

void OpenGLTexture::bind(unsigned int unit, TextureUnitReset /*reset*/) {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glActiveTexture( GL_TEXTURE0 + unit );
    m_gl->glBindTexture(m_target, m_bufferId);
}

void OpenGLTexture::release() {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glBindTexture(m_target, 0);
}

void OpenGLTexture::release(unsigned int unit, TextureUnitReset /*reset*/) {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glActiveTexture( GL_TEXTURE0 + unit );
    m_gl->glBindTexture(m_target, 0);
}

bool OpenGLTexture::isBound() const {
    return false;
}

bool OpenGLTexture::isBound(unsigned int /*unit*/) {
    return false;
}

void OpenGLTexture::setFormat(TextureFormat format) {
    m_format = format;
}

void OpenGLTexture::setSize(int width, int height, int depth) {
    m_width = width;
    m_height = height;
    m_depth = depth;
}

int OpenGLTexture::width() const {
    return m_width;
}

int OpenGLTexture::height() const {
    return m_height;
}

int OpenGLTexture::depth() const {
    return m_depth;
}

static OpenGLTexture::PixelFormat pixelFormatCompatibleWithInternalFormat(OpenGLTexture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case OpenGLTexture::NoFormat:
        return OpenGLTexture::NoSourceFormat;

    case OpenGLTexture::R8_UNorm:
    case OpenGLTexture::RG8_UNorm:
    case OpenGLTexture::RGB8_UNorm:
    case OpenGLTexture::RGBA8_UNorm:
    case OpenGLTexture::R16_UNorm:
    case OpenGLTexture::RG16_UNorm:
    case OpenGLTexture::RGB16_UNorm:
    case OpenGLTexture::RGBA16_UNorm:
    case OpenGLTexture::R8_SNorm:
    case OpenGLTexture::RG8_SNorm:
    case OpenGLTexture::RGB8_SNorm:
    case OpenGLTexture::RGBA8_SNorm:
    case OpenGLTexture::R16_SNorm:
    case OpenGLTexture::RG16_SNorm:
    case OpenGLTexture::RGB16_SNorm:
    case OpenGLTexture::RGBA16_SNorm:
    case OpenGLTexture::R8U:
    case OpenGLTexture::RG8U:
    case OpenGLTexture::RGB8U:
    case OpenGLTexture::RGBA8U:
    case OpenGLTexture::R16U:
    case OpenGLTexture::RG16U:
    case OpenGLTexture::RGB16U:
    case OpenGLTexture::RGBA16U:
    case OpenGLTexture::R32U:
    case OpenGLTexture::RG32U:
    case OpenGLTexture::RGB32U:
    case OpenGLTexture::RGBA32U:
    case OpenGLTexture::R8I:
    case OpenGLTexture::RG8I:
    case OpenGLTexture::RGB8I:
    case OpenGLTexture::RGBA8I:
    case OpenGLTexture::R16I:
    case OpenGLTexture::RG16I:
    case OpenGLTexture::RGB16I:
    case OpenGLTexture::RGBA16I:
    case OpenGLTexture::R32I:
    case OpenGLTexture::RG32I:
    case OpenGLTexture::RGB32I:
    case OpenGLTexture::RGBA32I:
    case OpenGLTexture::R16F:
    case OpenGLTexture::RG16F:
    case OpenGLTexture::RGB16F:
    case OpenGLTexture::RGBA16F:
    case OpenGLTexture::R32F:
    case OpenGLTexture::RG32F:
    case OpenGLTexture::RGB32F:
    case OpenGLTexture::RGBA32F:
    case OpenGLTexture::RGB9E5:
    case OpenGLTexture::RG11B10F:
    case OpenGLTexture::RG3B2:
    case OpenGLTexture::R5G6B5:
    case OpenGLTexture::RGB5A1:
    case OpenGLTexture::RGBA4:
    case OpenGLTexture::RGB10A2:
        return OpenGLTexture::RGBA;

    case OpenGLTexture::D16:
    case OpenGLTexture::D24:
    case OpenGLTexture::D32:
    case OpenGLTexture::D32F:
        return OpenGLTexture::Depth;

    case OpenGLTexture::D24S8:
    case OpenGLTexture::D32FS8X24:
        return OpenGLTexture::DepthStencil;

    case OpenGLTexture::S8:
        return OpenGLTexture::Stencil;

    case OpenGLTexture::RGB_DXT1:
    case OpenGLTexture::RGBA_DXT1:
    case OpenGLTexture::RGBA_DXT3:
    case OpenGLTexture::RGBA_DXT5:
    case OpenGLTexture::R_ATI1N_UNorm:
    case OpenGLTexture::R_ATI1N_SNorm:
    case OpenGLTexture::RG_ATI2N_UNorm:
    case OpenGLTexture::RG_ATI2N_SNorm:
    case OpenGLTexture::RGB_BP_UNSIGNED_FLOAT:
    case OpenGLTexture::RGB_BP_SIGNED_FLOAT:
    case OpenGLTexture::RGB_BP_UNorm:
    case OpenGLTexture::SRGB8:
    case OpenGLTexture::SRGB8_Alpha8:
    case OpenGLTexture::SRGB_DXT1:
    case OpenGLTexture::SRGB_Alpha_DXT1:
    case OpenGLTexture::SRGB_Alpha_DXT3:
    case OpenGLTexture::SRGB_Alpha_DXT5:
    case OpenGLTexture::SRGB_BP_UNorm:
        return OpenGLTexture::RGBA;

    case OpenGLTexture::DepthFormat:
        return OpenGLTexture::Depth;

    case OpenGLTexture::AlphaFormat:
        return OpenGLTexture::Alpha;

    case OpenGLTexture::RGBFormat:
    case OpenGLTexture::RGBAFormat:
        return OpenGLTexture::RGBA;

    case OpenGLTexture::LuminanceFormat:
        return OpenGLTexture::Luminance;

    case OpenGLTexture::LuminanceAlphaFormat:
        return OpenGLTexture::LuminanceAlpha;
    }

    Q_UNREACHABLE();
    return OpenGLTexture::NoSourceFormat;
}

static OpenGLTexture::PixelType pixelTypeCompatibleWithInternalFormat(OpenGLTexture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case OpenGLTexture::NoFormat:
        return OpenGLTexture::NoPixelType;

    case OpenGLTexture::R8_UNorm:
    case OpenGLTexture::RG8_UNorm:
    case OpenGLTexture::RGB8_UNorm:
    case OpenGLTexture::RGBA8_UNorm:
    case OpenGLTexture::R16_UNorm:
    case OpenGLTexture::RG16_UNorm:
    case OpenGLTexture::RGB16_UNorm:
    case OpenGLTexture::RGBA16_UNorm:
    case OpenGLTexture::R8_SNorm:
    case OpenGLTexture::RG8_SNorm:
    case OpenGLTexture::RGB8_SNorm:
    case OpenGLTexture::RGBA8_SNorm:
    case OpenGLTexture::R16_SNorm:
    case OpenGLTexture::RG16_SNorm:
    case OpenGLTexture::RGB16_SNorm:
    case OpenGLTexture::RGBA16_SNorm:
    case OpenGLTexture::R8U:
    case OpenGLTexture::RG8U:
    case OpenGLTexture::RGB8U:
    case OpenGLTexture::RGBA8U:
    case OpenGLTexture::R16U:
    case OpenGLTexture::RG16U:
    case OpenGLTexture::RGB16U:
    case OpenGLTexture::RGBA16U:
    case OpenGLTexture::R32U:
    case OpenGLTexture::RG32U:
    case OpenGLTexture::RGB32U:
    case OpenGLTexture::RGBA32U:
    case OpenGLTexture::R8I:
    case OpenGLTexture::RG8I:
    case OpenGLTexture::RGB8I:
    case OpenGLTexture::RGBA8I:
    case OpenGLTexture::R16I:
    case OpenGLTexture::RG16I:
    case OpenGLTexture::RGB16I:
    case OpenGLTexture::RGBA16I:
    case OpenGLTexture::R32I:
    case OpenGLTexture::RG32I:
    case OpenGLTexture::RGB32I:
    case OpenGLTexture::RGBA32I:
    case OpenGLTexture::R16F:
    case OpenGLTexture::RG16F:
    case OpenGLTexture::RGB16F:
    case OpenGLTexture::RGBA16F:
    case OpenGLTexture::R32F:
    case OpenGLTexture::RG32F:
    case OpenGLTexture::RGB32F:
    case OpenGLTexture::RGBA32F:
    case OpenGLTexture::RGB9E5:
    case OpenGLTexture::RG11B10F:
    case OpenGLTexture::RG3B2:
    case OpenGLTexture::R5G6B5:
    case OpenGLTexture::RGB5A1:
    case OpenGLTexture::RGBA4:
    case OpenGLTexture::RGB10A2:
        return OpenGLTexture::UInt8;

    case OpenGLTexture::D16:
    case OpenGLTexture::D24:
    case OpenGLTexture::D32:
    case OpenGLTexture::D32F:
        return OpenGLTexture::UInt8;

    case OpenGLTexture::D24S8:
        return OpenGLTexture::UInt32_D24S8;

    case OpenGLTexture::D32FS8X24:
        return OpenGLTexture::Float32_D32_UInt32_S8_X24;

    case OpenGLTexture::S8:
        return OpenGLTexture::UInt8;

    case OpenGLTexture::RGB_DXT1:
    case OpenGLTexture::RGBA_DXT1:
    case OpenGLTexture::RGBA_DXT3:
    case OpenGLTexture::RGBA_DXT5:
    case OpenGLTexture::R_ATI1N_UNorm:
    case OpenGLTexture::R_ATI1N_SNorm:
    case OpenGLTexture::RG_ATI2N_UNorm:
    case OpenGLTexture::RG_ATI2N_SNorm:
    case OpenGLTexture::RGB_BP_UNSIGNED_FLOAT:
    case OpenGLTexture::RGB_BP_SIGNED_FLOAT:
    case OpenGLTexture::RGB_BP_UNorm:
    case OpenGLTexture::SRGB8:
    case OpenGLTexture::SRGB8_Alpha8:
    case OpenGLTexture::SRGB_DXT1:
    case OpenGLTexture::SRGB_Alpha_DXT1:
    case OpenGLTexture::SRGB_Alpha_DXT3:
    case OpenGLTexture::SRGB_Alpha_DXT5:
    case OpenGLTexture::SRGB_BP_UNorm:
        return OpenGLTexture::UInt8;

    case OpenGLTexture::DepthFormat:
    case OpenGLTexture::AlphaFormat:
    case OpenGLTexture::RGBFormat:
    case OpenGLTexture::RGBAFormat:
    case OpenGLTexture::LuminanceFormat:
    case OpenGLTexture::LuminanceAlphaFormat:
        return OpenGLTexture::UInt8;
    }

    Q_UNREACHABLE();
    return OpenGLTexture::NoPixelType;
}

void OpenGLTexture::allocateStorage()
{
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    const OpenGLTexture::PixelFormat pixelFormat = pixelFormatCompatibleWithInternalFormat(m_format);
    const OpenGLTexture::PixelType pixelType = pixelTypeCompatibleWithInternalFormat(m_format);

    switch (m_target) {
    case OpenGLTexture::TargetBuffer:
        return;
    case OpenGLTexture::Target1D:
        m_gl->glTexImage1D(m_target, 0, m_format, m_width, 0, pixelFormat, pixelType, 0);
        break;
    case OpenGLTexture::Target2D:
    case OpenGLTexture::TargetRectangle:
        m_gl->glTexImage2D(m_target, 0, m_format, m_width, m_height, 0, pixelFormat, pixelType, 0);
        break;
    case OpenGLTexture::Target3D:
        m_gl->glTexImage3D(m_target, 0, m_format, m_width, m_height, m_depth, 0, pixelFormat, pixelType, 0);
        break;
    }

    m_storageAllocated = true;
}


bool OpenGLTexture::isStorageAllocated() const {
    return m_storageAllocated;
}

void OpenGLTexture::setData(OpenGLTexture::PixelFormat sourceFormat, OpenGLTexture::PixelType sourceType, int layer, const void *data)
{
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) && m_storageAllocated );

    switch (m_target) {
    case OpenGLTexture::Target1D:
        m_gl->glTexSubImage1D(m_target, 0, 0, m_width, sourceFormat, sourceType, data);
        break;
    case OpenGLTexture::Target2D:
        m_gl->glTexSubImage2D(m_target, 0, 0, 0, m_width, m_height, sourceFormat, sourceType, data);
        break;
    case OpenGLTexture::Target3D:
        m_gl->glTexSubImage3D(m_target, 0, 0, 0, layer, m_width, m_height, m_depth, sourceFormat, sourceType, data);
        break;
    }
}

void OpenGLTexture::setMinificationFilter(OpenGLTexture::Filter filter) {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, filter);
}

void OpenGLTexture::setMagnificationFilter(OpenGLTexture::Filter filter) {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, filter);
}

void OpenGLTexture::setMinMagFilters(OpenGLTexture::Filter minificationFilter,
                                    OpenGLTexture::Filter magnificationFilter) {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, minificationFilter);
    m_gl->glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, magnificationFilter);
}

void OpenGLTexture::setWrapMode(OpenGLTexture::WrapMode mode) {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glTexParameteri( m_target, OpenGLTexture::DirectionS, mode );
    m_gl->glTexParameteri( m_target, OpenGLTexture::DirectionT, mode );
    m_gl->glTexParameteri( m_target, OpenGLTexture::DirectionR, mode );
}

void OpenGLTexture::setWrapMode(CoordinateDirection direction, WrapMode mode) {
    MI_ASSERT( (m_gl != nullptr) && (m_bufferId != 0) );

    m_gl->glTexParameteri( m_target, direction, mode );
}
