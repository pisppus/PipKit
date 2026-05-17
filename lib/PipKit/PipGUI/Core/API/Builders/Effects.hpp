#pragma once

namespace pipgui
{

    template <bool IsUpdate>
    struct BlurRegionFluentT : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(BlurRegionFluentT);
        int16_t _x, _y, _w, _h;
        uint8_t _radius;
        std::optional<BlurDirection> _dir;
        uint8_t _materialStrength;
        std::optional<uint16_t> _materialColor;
        BlurRegionFluentT(GUI *g)
            : detail::FluentLifetime(g),
              _x(0),
              _y(0),
              _w(0),
              _h(0),
              _radius(0),
              _dir(std::nullopt),
              _materialStrength(32),
              _materialColor(std::nullopt)
        {
        }
        ~BlurRegionFluentT() { draw(); }
        BlurRegionFluentT &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        BlurRegionFluentT &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        BlurRegionFluentT &radius(uint8_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _radius = r;
            return *this;
        }
        BlurRegionFluentT &direction(BlurDirection d)
        {
            PIPGUI_FLUENT_GUARD();
            _dir = d;
            return *this;
        }
        BlurRegionFluentT &material(uint8_t strength, int32_t color)
        {
            PIPGUI_FLUENT_GUARD();
            _materialStrength = strength;
            detail::assignOptionalColor(_materialColor, color);
            return *this;
        }
        void draw();
    };

    template <bool IsUpdate>
    struct GlowCircleFluentT : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(GlowCircleFluentT);
        int16_t _x, _y;
        int16_t _r;
        uint16_t _fillColor;
        std::optional<uint16_t> _bgColor;
        std::optional<uint16_t> _glowColor;
        uint8_t _glowSize, _glowStrength;
        GlowAnim _anim;
        uint16_t _pulsePeriodMs;
        GlowCircleFluentT(GUI *g)
            : detail::FluentLifetime(g),
              _x(0),
              _y(0),
              _r(0),
              _fillColor(0),
              _bgColor(std::nullopt),
              _glowColor(std::nullopt),
              _glowSize(12),
              _glowStrength(220),
              _anim(None),
              _pulsePeriodMs(1000)
        {
        }
        ~GlowCircleFluentT() { draw(); }
        GlowCircleFluentT &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        GlowCircleFluentT &radius(int16_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _r = r;
            return *this;
        }
        GlowCircleFluentT &fillColor(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _fillColor = c;
            return *this;
        }
        GlowCircleFluentT &bgColor(int16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            detail::assignOptionalColor(_bgColor, c);
            return *this;
        }
        GlowCircleFluentT &bgColor(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _bgColor = c;
            return *this;
        }
        GlowCircleFluentT &glowColor(int16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            detail::assignOptionalColor(_glowColor, c);
            return *this;
        }
        GlowCircleFluentT &glowColor(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _glowColor = c;
            return *this;
        }
        GlowCircleFluentT &glowSize(uint8_t s)
        {
            PIPGUI_FLUENT_GUARD();
            _glowSize = s;
            return *this;
        }
        GlowCircleFluentT &glowStrength(uint8_t s)
        {
            PIPGUI_FLUENT_GUARD();
            _glowStrength = s;
            return *this;
        }
        GlowCircleFluentT &anim(GlowAnim a)
        {
            PIPGUI_FLUENT_GUARD();
            _anim = a;
            return *this;
        }
        GlowCircleFluentT &pulseMs(uint16_t ms)
        {
            PIPGUI_FLUENT_GUARD();
            _pulsePeriodMs = ms;
            return *this;
        }
        void draw();
    };

}
