#pragma once

namespace pipgui
{

    template <bool IsUpdate>
    struct TextFluentT : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(TextFluentT);
        int16_t _x, _y;
        std::optional<FontId> _fontId;
        uint16_t _sizePx;
        uint16_t _weight;
        String _text;
        uint16_t _fg565;
        uint16_t _bg565;
        TextAlign _align;
        UiRect _area;
        bool _hasArea;
        TextFluentT(GUI *g)
            : detail::FluentLifetime(g),
              _x(-1), _y(-1),
              _fontId(std::nullopt),
              _sizePx(0),
              _weight(0),
              _text(),
              _fg565(0xFFFF),
              _bg565(0x0000),
              _align(TextAlign::Left),
              _area{0, 0, 0, 0},
              _hasArea(false)
        {
        }

        ~TextFluentT() { draw(); }

        TextFluentT &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }

        TextFluentT &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }

        TextFluentT &font(FontId fontId, uint16_t sizePx)
        {
            PIPGUI_FLUENT_GUARD();
            _fontId = fontId;
            _sizePx = sizePx;
            return *this;
        }

        TextFluentT &weight(uint16_t weight)
        {
            PIPGUI_FLUENT_GUARD();
            _weight = weight;
            return *this;
        }

        TextFluentT &weight(WeightToken weight)
        {
            return this->weight(weight.value);
        }

        TextFluentT &text(const String &t)
        {
            PIPGUI_FLUENT_GUARD();
            _text = t;
            return *this;
        }

        TextFluentT &color(uint16_t fg565)
        {
            PIPGUI_FLUENT_GUARD();
            _fg565 = fg565;
            return *this;
        }

        TextFluentT &bgColor(uint16_t bg565)
        {
            PIPGUI_FLUENT_GUARD();
            _bg565 = bg565;
            return *this;
        }

        TextFluentT &align(TextAlign a)
        {
            PIPGUI_FLUENT_GUARD();
            _align = a;
            return *this;
        }

        void draw();
    };

    struct DrawTextMarqueeFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawTextMarqueeFluent);
        int16_t _x, _y, _maxWidth;
        std::optional<FontId> _fontId;
        uint16_t _sizePx;
        uint16_t _weight;
        String _text;
        uint16_t _fg565;
        TextAlign _align;
        MarqueeTextOptions _opts;
        DrawTextMarqueeFluent(GUI *g)
            : detail::FluentLifetime(g),
              _x(-1), _y(-1), _maxWidth(0),
              _fontId(std::nullopt),
              _sizePx(0),
              _weight(0),
              _text(),
              _fg565(0xFFFF),
              _align(TextAlign::Left),
              _opts()
        {
        }

        ~DrawTextMarqueeFluent() { draw(); }

        DrawTextMarqueeFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }

        DrawTextMarqueeFluent &width(int16_t width)
        {
            PIPGUI_FLUENT_GUARD();
            _maxWidth = width;
            return *this;
        }

        DrawTextMarqueeFluent &font(FontId fontId, uint16_t sizePx)
        {
            PIPGUI_FLUENT_GUARD();
            _fontId = fontId;
            _sizePx = sizePx;
            return *this;
        }

        DrawTextMarqueeFluent &weight(uint16_t weight)
        {
            PIPGUI_FLUENT_GUARD();
            _weight = weight;
            return *this;
        }

        DrawTextMarqueeFluent &weight(WeightToken weight)
        {
            return this->weight(weight.value);
        }

        DrawTextMarqueeFluent &text(const String &t)
        {
            PIPGUI_FLUENT_GUARD();
            _text = t;
            return *this;
        }

        DrawTextMarqueeFluent &color(uint16_t fg565)
        {
            PIPGUI_FLUENT_GUARD();
            _fg565 = fg565;
            return *this;
        }

        DrawTextMarqueeFluent &align(TextAlign a)
        {
            PIPGUI_FLUENT_GUARD();
            _align = a;
            return *this;
        }

        DrawTextMarqueeFluent &options(const MarqueeTextOptions &opts)
        {
            PIPGUI_FLUENT_GUARD();
            _opts = opts;
            return *this;
        }

        DrawTextMarqueeFluent &speed(uint16_t pxPerSec)
        {
            PIPGUI_FLUENT_GUARD();
            _opts.speedPxPerSec = pxPerSec;
            return *this;
        }

        DrawTextMarqueeFluent &holdStart(uint16_t ms)
        {
            PIPGUI_FLUENT_GUARD();
            _opts.holdStartMs = ms;
            return *this;
        }

        DrawTextMarqueeFluent &phaseStart(uint32_t ms)
        {
            PIPGUI_FLUENT_GUARD();
            _opts.phaseStartMs = ms;
            return *this;
        }

        void draw();
    };

    struct DrawTextEllipsizedFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawTextEllipsizedFluent);
        int16_t _x, _y, _maxWidth;
        std::optional<FontId> _fontId;
        uint16_t _sizePx;
        uint16_t _weight;
        String _text;
        uint16_t _fg565;
        TextAlign _align;
        DrawTextEllipsizedFluent(GUI *g)
            : detail::FluentLifetime(g),
              _x(-1), _y(-1), _maxWidth(0),
              _fontId(std::nullopt),
              _sizePx(0),
              _weight(0),
              _text(),
              _fg565(0xFFFF),
              _align(TextAlign::Left)
        {
        }

        ~DrawTextEllipsizedFluent() { draw(); }

        DrawTextEllipsizedFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }

        DrawTextEllipsizedFluent &width(int16_t width)
        {
            PIPGUI_FLUENT_GUARD();
            _maxWidth = width;
            return *this;
        }

        DrawTextEllipsizedFluent &font(FontId fontId, uint16_t sizePx)
        {
            PIPGUI_FLUENT_GUARD();
            _fontId = fontId;
            _sizePx = sizePx;
            return *this;
        }

        DrawTextEllipsizedFluent &weight(uint16_t weight)
        {
            PIPGUI_FLUENT_GUARD();
            _weight = weight;
            return *this;
        }

        DrawTextEllipsizedFluent &weight(WeightToken weight)
        {
            return this->weight(weight.value);
        }

        DrawTextEllipsizedFluent &text(const String &t)
        {
            PIPGUI_FLUENT_GUARD();
            _text = t;
            return *this;
        }

        DrawTextEllipsizedFluent &color(uint16_t fg565)
        {
            PIPGUI_FLUENT_GUARD();
            _fg565 = fg565;
            return *this;
        }

        DrawTextEllipsizedFluent &align(TextAlign a)
        {
            PIPGUI_FLUENT_GUARD();
            _align = a;
            return *this;
        }

        void draw();
    };

    struct DrawTextBoxFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawTextBoxFluent);
        int16_t _x, _y, _w, _h;
        std::optional<FontId> _fontId;
        uint16_t _sizePx;
        uint16_t _weight;
        String _text;
        uint16_t _fg565;
        uint16_t _bg565;
        TextAlign _align;
        int16_t _lineGap;
        UiRect _area;
        bool _hasArea;

        DrawTextBoxFluent(GUI *g)
            : detail::FluentLifetime(g),
              _x(0), _y(0), _w(0), _h(0),
              _fontId(std::nullopt),
              _sizePx(0),
              _weight(0),
              _text(),
              _fg565(0xFFFF),
              _bg565(0x0000),
              _align(TextAlign::Left),
              _lineGap(-1),
              _area{0, 0, 0, 0},
              _hasArea(false)
        {
        }

        ~DrawTextBoxFluent() { draw(); }

        DrawTextBoxFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }

        DrawTextBoxFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }

        DrawTextBoxFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }

        DrawTextBoxFluent &font(FontId fontId, uint16_t sizePx)
        {
            PIPGUI_FLUENT_GUARD();
            _fontId = fontId;
            _sizePx = sizePx;
            return *this;
        }

        DrawTextBoxFluent &weight(uint16_t weight)
        {
            PIPGUI_FLUENT_GUARD();
            _weight = weight;
            return *this;
        }

        DrawTextBoxFluent &weight(WeightToken weight)
        {
            return this->weight(weight.value);
        }

        DrawTextBoxFluent &text(const String &t)
        {
            PIPGUI_FLUENT_GUARD();
            _text = t;
            return *this;
        }

        DrawTextBoxFluent &color(uint16_t fg565)
        {
            PIPGUI_FLUENT_GUARD();
            _fg565 = fg565;
            return *this;
        }

        DrawTextBoxFluent &bgColor(uint16_t bg565)
        {
            PIPGUI_FLUENT_GUARD();
            _bg565 = bg565;
            return *this;
        }

        DrawTextBoxFluent &align(TextAlign a)
        {
            PIPGUI_FLUENT_GUARD();
            _align = a;
            return *this;
        }

        DrawTextBoxFluent &lineGap(int16_t px)
        {
            PIPGUI_FLUENT_GUARD();
            _lineGap = px;
            return *this;
        }

        void draw();
    };

}
