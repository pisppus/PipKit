#include "Internal.hpp"

namespace pipgui
{
    template <bool IsUpdate>
    void TextFluentT<IsUpdate>::draw()
    {
        if (_text.length() == 0 || !beginCommit())
            return;

        detail::TextFontGuard guard(_gui);

        if (_fontId && !_gui->setFont(*_fontId))
            return;
        if (_sizePx)
            _gui->setFontSize(_sizePx);
        if (_weight)
            _gui->setFontWeight(_weight);

        int16_t x = _x;
        int16_t y = _y;
        if (_hasArea)
        {
            int16_t tw = 0;
            int16_t th = 0;
            const bool measured = detail::GuiAccess::measureText(*_gui, _text, tw, th);
            x = (_x == center) ? (int16_t)(_area.x + _area.w / 2) : (int16_t)(_area.x + _x);
            y = (_y == center)
                    ? (measured ? static_cast<int16_t>(_area.y + (_area.h - th) / 2) : _area.y)
                    : static_cast<int16_t>(_area.y + _y);
        }

        detail::callByMode<IsUpdate>(
            [&]
            { detail::GuiAccess::updateText(*_gui, _text, x, y, _fg565, _bg565, _align); },
            [&]
            { detail::GuiAccess::drawText(*_gui, _text, x, y, _fg565, _bg565, _align); });
    }
    template void TextFluentT<false>::draw();
    template void TextFluentT<true>::draw();

    void DrawTextMarqueeFluent::draw()
    {
        if (_text.length() == 0 || _maxWidth <= 0 || !beginCommit())
            return;

        detail::TextFontGuard guard(_gui);

        if (_fontId && !_gui->setFont(*_fontId))
            return;
        if (_sizePx)
            _gui->setFontSize(_sizePx);
        if (_weight)
            _gui->setFontWeight(_weight);
        detail::GuiAccess::drawTextMarquee(*_gui, _text, _x, _y, _maxWidth, _fg565, _align, _opts);
    }

    void DrawTextEllipsizedFluent::draw()
    {
        if (_text.length() == 0 || _maxWidth <= 0 || !beginCommit())
            return;

        detail::TextFontGuard guard(_gui);

        if (_fontId && !_gui->setFont(*_fontId))
            return;
        if (_sizePx)
            _gui->setFontSize(_sizePx);
        if (_weight)
            _gui->setFontWeight(_weight);
        detail::GuiAccess::drawTextEllipsized(*_gui, _text, _x, _y, _maxWidth, _fg565, _align);
    }

    void DrawTextBoxFluent::draw()
    {
        if (_text.length() == 0 || !beginCommit())
            return;

        detail::TextFontGuard guard(_gui);

        if (_fontId && !_gui->setFont(*_fontId))
            return;
        if (_sizePx)
            _gui->setFontSize(_sizePx);
        if (_weight)
            _gui->setFontWeight(_weight);

        int16_t x = _x;
        int16_t y = _y;
        int16_t w = _w;
        int16_t h = _h;
        if (_hasArea)
        {
            if (w <= 0)
                w = _area.w;
            if (h <= 0)
                h = _area.h;
            x = (_x == center) ? (int16_t)(_area.x + (_area.w - w) / 2) : (int16_t)(_area.x + _x);
            y = (_y == center) ? (int16_t)(_area.y + (_area.h - h) / 2) : (int16_t)(_area.y + _y);
        }

        detail::GuiAccess::drawTextBox(*_gui, _text, x, y, w, h, _fg565, _bg565, _align, _lineGap);
    }
}
