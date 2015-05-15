#pragma once

class GameWindow;

class OpenGLBuffer
{
public:
    enum Type
    {
        VertexBuffer        = 0x8892, // GL_ARRAY_BUFFER
        IndexBuffer         = 0x8893, // GL_ELEMENT_ARRAY_BUFFER
        PixelPackBuffer     = 0x88EB, // GL_PIXEL_PACK_BUFFER
        PixelUnpackBuffer   = 0x88EC  // GL_PIXEL_UNPACK_BUFFER
    };
    enum UsagePattern
    {
        StreamDraw          = 0x88E0, // GL_STREAM_DRAW
        StreamRead          = 0x88E1, // GL_STREAM_READ
        StreamCopy          = 0x88E2, // GL_STREAM_COPY
        StaticDraw          = 0x88E4, // GL_STATIC_DRAW
        StaticRead          = 0x88E5, // GL_STATIC_READ
        StaticCopy          = 0x88E6, // GL_STATIC_COPY
        DynamicDraw         = 0x88E8, // GL_DYNAMIC_DRAW
        DynamicRead         = 0x88E9, // GL_DYNAMIC_READ
        DynamicCopy         = 0x88EA  // GL_DYNAMIC_COPY
    };

    enum Access
    {
        ReadOnly            = 0x88B8, // GL_READ_ONLY
        WriteOnly           = 0x88B9, // GL_WRITE_ONLY
        ReadWrite           = 0x88BA  // GL_READ_WRITE
    };

    enum RangeAccessFlag
    {
        RangeRead             = 0x0001, // GL_MAP_READ_BIT
        RangeWrite            = 0x0002, // GL_MAP_WRITE_BIT
        RangeInvalidate       = 0x0004, // GL_MAP_INVALIDATE_RANGE_BIT
        RangeInvalidateBuffer = 0x0008, // GL_MAP_INVALIDATE_BUFFER_BIT
        RangeFlushExplicit    = 0x0010, // GL_MAP_FLUSH_EXPLICIT_BIT
        RangeUnsynchronized   = 0x0020  // GL_MAP_UNSYNCHRONIZED_BIT
    };

    OpenGLBuffer( GameWindow* gl, OpenGLBuffer::Type type );
    ~OpenGLBuffer();

    OpenGLBuffer( const OpenGLBuffer& copy ) = delete;
    OpenGLBuffer& operator=( const OpenGLBuffer& copy ) = delete;

    OpenGLBuffer::Type  type() const;

    OpenGLBuffer::UsagePattern usagePattern() const;
    void                setUsagePattern(OpenGLBuffer::UsagePattern value);

    void                create();
    bool                isCreated() const;

    void                destroy();

    void                bind();
    void                release();

    static void         release(OpenGLBuffer::Type type);

    unsigned int        bufferId() const;

    int                 size() const;

    void                read(int offset, void* data, int count);
    void                write(int offset, const void* data, int count);

    void                allocate(const void* data, int count);
    inline void         allocate(int count) { allocate(nullptr, count); }

    void*               map(OpenGLBuffer::Access access);
    bool                unmap();

private:

    GameWindow*                 m_gl;
    OpenGLBuffer::Type          m_type;
    OpenGLBuffer::UsagePattern  m_usagePattern;
    unsigned int                m_bufferId;
    unsigned int                m_size;
};
