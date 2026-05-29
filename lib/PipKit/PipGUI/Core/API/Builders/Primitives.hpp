#pragma once

namespace pipgui
{

    struct DrawRectFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawRectFluent);
        int16_t _x, _y, _w, _h;
        uint8_t _radius;
        uint8_t _radiusTL, _radiusTR, _radiusBR, _radiusBL;
        uint16_t _fillColor;
        uint16_t _borderColor;
        uint8_t _borderWidth;
        bool _perCorner;
        bool _hasFill;
        UiRect _area;
        bool _hasArea;
        DrawRectFluent(GUI *g)
            : detail::FluentLifetime(g),
              _x(0), _y(0), _w(0), _h(0),
              _radius(0), _radiusTL(0), _radiusTR(0), _radiusBR(0), _radiusBL(0),
              _fillColor(0), _borderColor(0), _borderWidth(0),
              _perCorner(false), _hasFill(false),
              _area{0, 0, 0, 0}, _hasArea(false) {}
        ~DrawRectFluent() { draw(); }
        DrawRectFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        DrawRectFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        DrawRectFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        DrawRectFluent &fill(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _fillColor = c;
            _hasFill = true;
            return *this;
        }
        DrawRectFluent &border(uint8_t width, uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _borderWidth = width;
            _borderColor = c;
            return *this;
        }
        DrawRectFluent &radius(uint8_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _radius = r;
            _perCorner = false;
            return *this;
        }
        DrawRectFluent &radius(uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl)
        {
            PIPGUI_FLUENT_GUARD();
            _radiusTL = tl;
            _radiusTR = tr;
            _radiusBR = br;
            _radiusBL = bl;
            _perCorner = true;
            return *this;
        }
        void draw();
    };

    struct GradientVerticalFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(GradientVerticalFluent);
        int16_t _x, _y, _w, _h;
        uint32_t _topColor, _bottomColor;
        UiRect _area;
        bool _hasArea;
        GradientVerticalFluent(GUI *g) : detail::FluentLifetime(g), _x(0), _y(0), _w(0), _h(0), _topColor(0), _bottomColor(0), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~GradientVerticalFluent() { draw(); }
        GradientVerticalFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        GradientVerticalFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        GradientVerticalFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        GradientVerticalFluent &TColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _topColor = c;
            return *this;
        }
        GradientVerticalFluent &TColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _topColor = detail::color565To888(c565);
            return *this;
        }
        GradientVerticalFluent &BColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _bottomColor = c;
            return *this;
        }
        GradientVerticalFluent &BColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _bottomColor = detail::color565To888(c565);
            return *this;
        }
        void draw();
    };

    struct GradientHorizontalFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(GradientHorizontalFluent);
        int16_t _x, _y, _w, _h;
        uint32_t _leftColor, _rightColor;
        UiRect _area;
        bool _hasArea;
        GradientHorizontalFluent(GUI *g) : detail::FluentLifetime(g), _x(0), _y(0), _w(0), _h(0), _leftColor(0), _rightColor(0), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~GradientHorizontalFluent() { draw(); }
        GradientHorizontalFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        GradientHorizontalFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        GradientHorizontalFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        GradientHorizontalFluent &LColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _leftColor = c;
            return *this;
        }
        GradientHorizontalFluent &LColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _leftColor = detail::color565To888(c565);
            return *this;
        }
        GradientHorizontalFluent &RColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _rightColor = c;
            return *this;
        }
        GradientHorizontalFluent &RColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _rightColor = detail::color565To888(c565);
            return *this;
        }
        void draw();
    };

    struct GradientCornersFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(GradientCornersFluent);
        int16_t _x, _y, _w, _h;
        uint32_t _c00, _c10, _c01, _c11;
        UiRect _area;
        bool _hasArea;
        GradientCornersFluent(GUI *g) : detail::FluentLifetime(g), _x(0), _y(0), _w(0), _h(0), _c00(0), _c10(0), _c01(0), _c11(0), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~GradientCornersFluent() { draw(); }
        GradientCornersFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        GradientCornersFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        GradientCornersFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        GradientCornersFluent &TLColor(uint32_t color)
        {
            PIPGUI_FLUENT_GUARD();
            _c00 = color;
            return *this;
        }
        GradientCornersFluent &TLColor(uint16_t color565)
        {
            PIPGUI_FLUENT_GUARD();
            _c00 = detail::color565To888(color565);
            return *this;
        }
        GradientCornersFluent &TRColor(uint32_t color)
        {
            PIPGUI_FLUENT_GUARD();
            _c10 = color;
            return *this;
        }
        GradientCornersFluent &TRColor(uint16_t color565)
        {
            PIPGUI_FLUENT_GUARD();
            _c10 = detail::color565To888(color565);
            return *this;
        }
        GradientCornersFluent &BLColor(uint32_t color)
        {
            PIPGUI_FLUENT_GUARD();
            _c01 = color;
            return *this;
        }
        GradientCornersFluent &BLColor(uint16_t color565)
        {
            PIPGUI_FLUENT_GUARD();
            _c01 = detail::color565To888(color565);
            return *this;
        }
        GradientCornersFluent &BRColor(uint32_t color)
        {
            PIPGUI_FLUENT_GUARD();
            _c11 = color;
            return *this;
        }
        GradientCornersFluent &BRColor(uint16_t color565)
        {
            PIPGUI_FLUENT_GUARD();
            _c11 = detail::color565To888(color565);
            return *this;
        }
        void draw();
    };

    struct GradientDiagonalFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(GradientDiagonalFluent);
        int16_t _x, _y, _w, _h;
        uint32_t _tlColor, _brColor;
        UiRect _area;
        bool _hasArea;
        GradientDiagonalFluent(GUI *g) : detail::FluentLifetime(g), _x(0), _y(0), _w(0), _h(0), _tlColor(0), _brColor(0), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~GradientDiagonalFluent() { draw(); }
        GradientDiagonalFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        GradientDiagonalFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        GradientDiagonalFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        GradientDiagonalFluent &TLColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _tlColor = c;
            return *this;
        }
        GradientDiagonalFluent &TLColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _tlColor = detail::color565To888(c565);
            return *this;
        }
        GradientDiagonalFluent &BRColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _brColor = c;
            return *this;
        }
        GradientDiagonalFluent &BRColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _brColor = detail::color565To888(c565);
            return *this;
        }
        void draw();
    };

    struct GradientRadialFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(GradientRadialFluent);
        int16_t _x, _y, _w, _h;
        int16_t _cx, _cy;
        int16_t _radius;
        uint32_t _innerColor, _outerColor;
        UiRect _area;
        bool _hasArea;
        GradientRadialFluent(GUI *g) : detail::FluentLifetime(g), _x(0), _y(0), _w(0), _h(0), _cx(0), _cy(0), _radius(0), _innerColor(0), _outerColor(0), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~GradientRadialFluent() { draw(); }
        GradientRadialFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        GradientRadialFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        GradientRadialFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        GradientRadialFluent &center(int16_t cx, int16_t cy)
        {
            PIPGUI_FLUENT_GUARD();
            _cx = cx;
            _cy = cy;
            return *this;
        }
        GradientRadialFluent &radius(int16_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _radius = r;
            return *this;
        }
        GradientRadialFluent &innerColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _innerColor = c;
            return *this;
        }
        GradientRadialFluent &innerColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _innerColor = detail::color565To888(c565);
            return *this;
        }
        GradientRadialFluent &outerColor(uint32_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _outerColor = c;
            return *this;
        }
        GradientRadialFluent &outerColor(uint16_t c565)
        {
            PIPGUI_FLUENT_GUARD();
            _outerColor = detail::color565To888(c565);
            return *this;
        }
        void draw();
    };

    struct DrawLineFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawLineFluent);
        int16_t _x0, _y0, _x1, _y1;
        uint8_t _thickness;
        uint16_t _color;
        UiRect _area;
        bool _hasArea;
        DrawLineFluent(GUI *g) : detail::FluentLifetime(g), _x0(0), _y0(0), _x1(0), _y1(0), _thickness(1), _color(0), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~DrawLineFluent() { draw(); }
        DrawLineFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        DrawLineFluent &from(int16_t x0, int16_t y0)
        {
            PIPGUI_FLUENT_GUARD();
            _x0 = x0;
            _y0 = y0;
            return *this;
        }
        DrawLineFluent &to(int16_t x1, int16_t y1)
        {
            PIPGUI_FLUENT_GUARD();
            _x1 = x1;
            _y1 = y1;
            return *this;
        }
        DrawLineFluent &thickness(uint8_t t)
        {
            PIPGUI_FLUENT_GUARD();
            _thickness = t;
            return *this;
        }
        DrawLineFluent &color(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _color = c;
            return *this;
        }
        void draw();
    };

    struct DrawCircleFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawCircleFluent);
        int16_t _cx, _cy;
        int16_t _r;
        uint16_t _fillColor;
        uint16_t _borderColor;
        uint8_t _borderWidth;
        bool _hasFill;
        UiRect _area;
        bool _hasArea;
        DrawCircleFluent(GUI *g) : detail::FluentLifetime(g), _cx(0), _cy(0), _r(0), _fillColor(0), _borderColor(0), _borderWidth(0), _hasFill(false), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~DrawCircleFluent() { draw(); }
        DrawCircleFluent &pos(int16_t cx, int16_t cy)
        {
            PIPGUI_FLUENT_GUARD();
            _cx = cx;
            _cy = cy;
            return *this;
        }
        DrawCircleFluent &radius(int16_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _r = r;
            return *this;
        }
        DrawCircleFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        DrawCircleFluent &fill(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _fillColor = c;
            _hasFill = true;
            return *this;
        }
        DrawCircleFluent &border(uint8_t width, uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _borderWidth = width;
            _borderColor = c;
            return *this;
        }
        void draw();
    };

    struct DrawArcFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawArcFluent);
        int16_t _cx, _cy;
        int16_t _r;
        uint8_t _thickness;
        float _startDeg, _endDeg;
        uint16_t _color;
        UiRect _area;
        bool _hasArea;
        DrawArcFluent(GUI *g) : detail::FluentLifetime(g), _cx(0), _cy(0), _r(0), _thickness(1), _startDeg(0), _endDeg(360), _color(0), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~DrawArcFluent() { draw(); }
        DrawArcFluent &pos(int16_t cx, int16_t cy)
        {
            PIPGUI_FLUENT_GUARD();
            _cx = cx;
            _cy = cy;
            return *this;
        }
        DrawArcFluent &radius(int16_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _r = r;
            return *this;
        }
        DrawArcFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        DrawArcFluent &thickness(uint8_t t)
        {
            PIPGUI_FLUENT_GUARD();
            _thickness = t;
            return *this;
        }
        DrawArcFluent &start(float d)
        {
            PIPGUI_FLUENT_GUARD();
            _startDeg = d;
            return *this;
        }
        DrawArcFluent &end(float d)
        {
            PIPGUI_FLUENT_GUARD();
            _endDeg = d;
            return *this;
        }
        DrawArcFluent &color(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _color = c;
            return *this;
        }
        void draw();
    };

    struct DrawEllipseFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawEllipseFluent);
        int16_t _cx, _cy;
        int16_t _rx, _ry;
        uint16_t _fillColor;
        uint16_t _borderColor;
        uint8_t _borderWidth;
        bool _hasFill;
        UiRect _area;
        bool _hasArea;
        DrawEllipseFluent(GUI *g) : detail::FluentLifetime(g), _cx(0), _cy(0), _rx(0), _ry(0), _fillColor(0), _borderColor(0), _borderWidth(0), _hasFill(false), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~DrawEllipseFluent() { draw(); }
        DrawEllipseFluent &pos(int16_t cx, int16_t cy)
        {
            PIPGUI_FLUENT_GUARD();
            _cx = cx;
            _cy = cy;
            return *this;
        }
        DrawEllipseFluent &radiusX(int16_t rx)
        {
            PIPGUI_FLUENT_GUARD();
            _rx = rx;
            return *this;
        }
        DrawEllipseFluent &radiusY(int16_t ry)
        {
            PIPGUI_FLUENT_GUARD();
            _ry = ry;
            return *this;
        }
        DrawEllipseFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        DrawEllipseFluent &fill(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _fillColor = c;
            _hasFill = true;
            return *this;
        }
        DrawEllipseFluent &border(uint8_t width, uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _borderWidth = width;
            _borderColor = c;
            return *this;
        }
        void draw();
    };

    struct DrawTriangleFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawTriangleFluent);
        int16_t _tx, _ty;
        UiPoint _p0, _p1, _p2;
        int16_t _w, _h;
        uint8_t _radius;
        TriangleDirection _dir;
        uint16_t _fillColor;
        uint16_t _borderColor;
        uint8_t _borderWidth;
        bool _hasFill;
        bool _usePreset;
        UiRect _area;
        bool _hasArea;
        DrawTriangleFluent(GUI *g) : detail::FluentLifetime(g), _tx(0), _ty(0), _p0{0, 0}, _p1{0, 0}, _p2{0, 0}, _w(0), _h(0), _radius(0), _dir(TriangleDirection::Up), _fillColor(0), _borderColor(0), _borderWidth(0), _hasFill(false), _usePreset(false), _area{0, 0, 0, 0}, _hasArea(false) {}
        ~DrawTriangleFluent() { draw(); }
        DrawTriangleFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _tx = x;
            _ty = y;
            return *this;
        }
        DrawTriangleFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        DrawTriangleFluent &vertex0(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _p0 = UiPoint{x, y};
            _usePreset = false;
            return *this;
        }
        DrawTriangleFluent &vertex1(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _p1 = UiPoint{x, y};
            _usePreset = false;
            return *this;
        }
        DrawTriangleFluent &vertex2(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _p2 = UiPoint{x, y};
            _usePreset = false;
            return *this;
        }
        DrawTriangleFluent &vertices(int16_t x0, int16_t y0,
                                     int16_t x1, int16_t y1,
                                     int16_t x2, int16_t y2)
        {
            PIPGUI_FLUENT_GUARD();
            _p0 = UiPoint{x0, y0};
            _p1 = UiPoint{x1, y1};
            _p2 = UiPoint{x2, y2};
            _usePreset = false;
            return *this;
        }
        DrawTriangleFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            _usePreset = true;
            return *this;
        }
        DrawTriangleFluent &direction(TriangleDirection dir)
        {
            PIPGUI_FLUENT_GUARD();
            _dir = dir;
            return *this;
        }
        DrawTriangleFluent &fill(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _fillColor = c;
            _hasFill = true;
            return *this;
        }
        DrawTriangleFluent &border(uint8_t width, uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _borderWidth = width;
            _borderColor = c;
            return *this;
        }
        DrawTriangleFluent &radius(uint8_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _radius = r;
            return *this;
        }
        void draw();
    };

    struct DrawSquircleRectFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(DrawSquircleRectFluent);
        int16_t _x, _y, _w, _h;
        uint8_t _radius;
        uint8_t _radiusTL, _radiusTR, _radiusBR, _radiusBL;
        bool _perCorner;
        uint16_t _fillColor;
        uint16_t _borderColor;
        uint8_t _borderWidth;
        bool _hasFill;
        UiRect _area;
        bool _hasArea;
        DrawSquircleRectFluent(GUI *g)
            : detail::FluentLifetime(g),
              _x(0), _y(0), _w(0), _h(0),
              _radius(0), _radiusTL(0), _radiusTR(0), _radiusBR(0), _radiusBL(0),
              _perCorner(false), _fillColor(0), _borderColor(0), _borderWidth(0), _hasFill(false),
              _area{0, 0, 0, 0}, _hasArea(false) {}
        ~DrawSquircleRectFluent() { draw(); }
        DrawSquircleRectFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            return *this;
        }
        DrawSquircleRectFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            return *this;
        }
        DrawSquircleRectFluent &in(const UiRect &area)
        {
            PIPGUI_FLUENT_GUARD();
            _area = area;
            _hasArea = true;
            return *this;
        }
        DrawSquircleRectFluent &radius(uint8_t r)
        {
            PIPGUI_FLUENT_GUARD();
            _radius = r;
            _perCorner = false;
            return *this;
        }
        DrawSquircleRectFluent &radius(uint8_t tl, uint8_t tr, uint8_t br, uint8_t bl)
        {
            PIPGUI_FLUENT_GUARD();
            _radiusTL = tl;
            _radiusTR = tr;
            _radiusBR = br;
            _radiusBL = bl;
            _perCorner = true;
            return *this;
        }
        DrawSquircleRectFluent &fill(uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _fillColor = c;
            _hasFill = true;
            return *this;
        }
        DrawSquircleRectFluent &border(uint8_t width, uint16_t c)
        {
            PIPGUI_FLUENT_GUARD();
            _borderWidth = width;
            _borderColor = c;
            return *this;
        }
        void draw();
    };
}
