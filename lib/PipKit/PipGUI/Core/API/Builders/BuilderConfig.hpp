#pragma once

namespace pipgui
{
    struct ConfigDisplayFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(ConfigDisplayFluent);
        pipcore::DisplayConfig _cfg;
        bool _touched;

        ConfigDisplayFluent(GUI *g)
            : detail::FluentLifetime(g), _cfg(detail::defaultDisplayConfig()), _touched(false)
        {
        }

        ~ConfigDisplayFluent() { apply(); }

        ConfigDisplayFluent &pins(const DisplayPins &p)
        {
            PIPGUI_FLUENT_GUARD();
            _cfg.mosi = p.mosi;
            _cfg.sclk = p.sclk;
            _cfg.cs = p.cs;
            _cfg.dc = p.dc;
            _cfg.rst = p.rst;
            _touched = true;
            return *this;
        }

        ConfigDisplayFluent &pins(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc, int8_t rst)
        {
            PIPGUI_FLUENT_GUARD();
            _cfg.mosi = mosi;
            _cfg.sclk = sclk;
            _cfg.cs = cs;
            _cfg.dc = dc;
            _cfg.rst = rst;
            _touched = true;
            return *this;
        }

        ConfigDisplayFluent &size(uint16_t w, uint16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _cfg.width = w;
            _cfg.height = h;
            _touched = true;
            return *this;
        }

        ConfigDisplayFluent &hz(uint32_t v)
        {
            PIPGUI_FLUENT_GUARD();
            _cfg.hz = v;
            _touched = true;
            return *this;
        }

        ConfigDisplayFluent &order(const char *v)
        {
            PIPGUI_FLUENT_GUARD();
            if (!v)
                return *this;

            if ((v[0] == 'B' || v[0] == 'b') && (v[1] == 'G' || v[1] == 'g') && (v[2] == 'R' || v[2] == 'r') && v[3] == 0)
                _cfg.order = 1;
            else
                _cfg.order = 0;
            _touched = true;
            return *this;
        }

        ConfigDisplayFluent &invert(bool v = true)
        {
            PIPGUI_FLUENT_GUARD();
            _cfg.invert = v;
            _touched = true;
            return *this;
        }

        ConfigDisplayFluent &swap(bool v = true)
        {
            PIPGUI_FLUENT_GUARD();
            _cfg.swap = v;
            _touched = true;
            return *this;
        }

        ConfigDisplayFluent &offset(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _cfg.xOffset = x;
            _cfg.yOffset = y;
            _touched = true;
            return *this;
        }

        void apply();
    };

    struct ConfigureBacklightFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(ConfigureBacklightFluent);
        uint8_t _pin = 255;
        uint8_t _channel = 0;
        uint32_t _freqHz = 5000;
        uint8_t _resolutionBits = 12;
        bool _touched = false;

        explicit ConfigureBacklightFluent(GUI *g)
            : detail::FluentLifetime(g)
        {
        }

        ~ConfigureBacklightFluent() { apply(); }

        ConfigureBacklightFluent &pin(uint8_t v)
        {
            PIPGUI_FLUENT_GUARD();
            _pin = v;
            _touched = true;
            return *this;
        }

        ConfigureBacklightFluent &channel(uint8_t v)
        {
            PIPGUI_FLUENT_GUARD();
            _channel = v;
            _touched = true;
            return *this;
        }

        ConfigureBacklightFluent &freq(uint32_t v)
        {
            PIPGUI_FLUENT_GUARD();
            _freqHz = v;
            _touched = true;
            return *this;
        }

        ConfigureBacklightFluent &resolution(uint8_t v)
        {
            PIPGUI_FLUENT_GUARD();
            _resolutionBits = v;
            _touched = true;
            return *this;
        }

        void apply();
    };

    struct SetClipFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(SetClipFluent);
        int16_t _x = 0;
        int16_t _y = 0;
        int16_t _w = 0;
        int16_t _h = 0;
        bool _touched = false;

        explicit SetClipFluent(GUI *g)
            : detail::FluentLifetime(g)
        {
        }

        ~SetClipFluent() { apply(); }

        SetClipFluent &pos(int16_t x, int16_t y)
        {
            PIPGUI_FLUENT_GUARD();
            _x = x;
            _y = y;
            _touched = true;
            return *this;
        }

        SetClipFluent &size(int16_t w, int16_t h)
        {
            PIPGUI_FLUENT_GUARD();
            _w = w;
            _h = h;
            _touched = true;
            return *this;
        }

        void apply();
    };

    struct ShowLogoFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(ShowLogoFluent);
        String _title;
        String _subtitle;
        BootAnimation _anim = None;
        bool _touched = false;

        explicit ShowLogoFluent(GUI *g)
            : detail::FluentLifetime(g)
        {
        }

        ~ShowLogoFluent() { apply(); }

        ShowLogoFluent &text(const String &title, const String &subtitle = "")
        {
            PIPGUI_FLUENT_GUARD();
            _title = title;
            _subtitle = subtitle;
            _touched = true;
            return *this;
        }

        ShowLogoFluent &anim(BootAnimation v)
        {
            PIPGUI_FLUENT_GUARD();
            _anim = v;
            _touched = true;
            return *this;
        }

        void apply();
    };

    struct ConfigStatusBarFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(ConfigStatusBarFluent);
        uint8_t _height;
        StatusBarPosition _pos;
        StatusBarStyle _style;

        explicit ConfigStatusBarFluent(GUI *g)
            : detail::FluentLifetime(g), _height(0), _pos(Top), _style(Solid)
        {
        }

        ~ConfigStatusBarFluent() { apply(); }

        ConfigStatusBarFluent &height(uint8_t v)
        {
            PIPGUI_FLUENT_GUARD();
            _height = v;
            return *this;
        }

        ConfigStatusBarFluent &pos(StatusBarPosition v)
        {
            PIPGUI_FLUENT_GUARD();
            _pos = v;
            return *this;
        }

        ConfigStatusBarFluent &style(StatusBarStyle v)
        {
            PIPGUI_FLUENT_GUARD();
            _style = v;
            return *this;
        }

        void apply();
    };

    struct SetStatusBarTextFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(SetStatusBarTextFluent);
        String _left;
        String _center;
        String _right;

        explicit SetStatusBarTextFluent(GUI *g)
            : detail::FluentLifetime(g)
        {
        }

        ~SetStatusBarTextFluent() { apply(); }

        SetStatusBarTextFluent &left(const String &v)
        {
            PIPGUI_FLUENT_GUARD();
            _left = v;
            return *this;
        }

        SetStatusBarTextFluent &center(const String &v)
        {
            PIPGUI_FLUENT_GUARD();
            _center = v;
            return *this;
        }

        SetStatusBarTextFluent &right(const String &v)
        {
            PIPGUI_FLUENT_GUARD();
            _right = v;
            return *this;
        }

        void apply();
    };

    struct SetStatusBarIconFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(SetStatusBarIconFluent);
        TextAlign _side;
        IconId _iconId;
        std::optional<uint16_t> _color565;
        uint16_t _sizePx;
        bool _hasSide;
        bool _hasIcon;

        explicit SetStatusBarIconFluent(GUI *g)
            : detail::FluentLifetime(g), _side(Left), _iconId(static_cast<IconId>(0xFFFF)), _sizePx(0), _hasSide(false), _hasIcon(false)
        {
        }

        ~SetStatusBarIconFluent() { apply(); }

        SetStatusBarIconFluent &side(TextAlign v)
        {
            PIPGUI_FLUENT_GUARD();
            _side = v;
            _hasSide = true;
            return *this;
        }

        SetStatusBarIconFluent &icon(IconId v)
        {
            PIPGUI_FLUENT_GUARD();
            _iconId = v;
            _hasIcon = true;
            return *this;
        }

        SetStatusBarIconFluent &color(int32_t v)
        {
            PIPGUI_FLUENT_GUARD();
            detail::assignOptionalColor(_color565, v);
            return *this;
        }

        SetStatusBarIconFluent &size(uint16_t v)
        {
            PIPGUI_FLUENT_GUARD();
            _sizePx = v;
            return *this;
        }

        void apply();
    };

    struct ListInputFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(ListInputFluent);
        bool _nextDown;
        bool _prevDown;
        bool _selectDown;
        bool _hasSelect;

        ListInputFluent(GUI *g)
            : detail::FluentLifetime(g), _nextDown(false), _prevDown(false), _selectDown(false), _hasSelect(false)
        {
        }

        ListInputFluent &nextDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _nextDown = v;
            return *this;
        }

        ListInputFluent &prevDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _prevDown = v;
            return *this;
        }

        ListInputFluent &selectDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _selectDown = v;
            _hasSelect = true;
            return *this;
        }

        ~ListInputFluent() { apply(); }

    private:
        void apply();
    };

    struct PopupMenuInputFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(PopupMenuInputFluent);
        bool _nextDown;
        bool _prevDown;
        bool _selectDown;
        bool _hasSelect;

        PopupMenuInputFluent(GUI *g)
            : detail::FluentLifetime(g), _nextDown(false), _prevDown(false), _selectDown(false), _hasSelect(false)
        {
        }

        PopupMenuInputFluent &nextDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _nextDown = v;
            return *this;
        }

        PopupMenuInputFluent &prevDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _prevDown = v;
            return *this;
        }

        PopupMenuInputFluent &selectDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _selectDown = v;
            _hasSelect = true;
            return *this;
        }

        ~PopupMenuInputFluent() { apply(); }

        void apply();
    };

    struct UpdateListFluent : detail::FluentLifetime
    {
        const ListItemDef *_items;
        uint8_t _itemCount;
        detail::OwnedHeapArray<ListItemDef> _ownedItems;
        uint16_t _cardColor;
        uint16_t _cardActiveColor;
        uint8_t _radius;
        int16_t _cardWidth;
        int16_t _cardHeight;
        ListMode _mode;
        uint8_t _checkedIndex;

        UpdateListFluent(GUI *g)
            : detail::FluentLifetime(g), _items(nullptr), _itemCount(0), _ownedItems(detail::resolvePlatform(g)),
              _cardColor(0), _cardActiveColor(0), _radius(0),
              _cardWidth(0), _cardHeight(0), _mode(Cards), _checkedIndex(0xFF)
        {
        }

        UpdateListFluent(UpdateListFluent &&other) noexcept
            : detail::FluentLifetime(std::move(other)),
              _items(std::exchange(other._items, nullptr)),
              _itemCount(std::exchange(other._itemCount, 0)),
              _ownedItems(std::move(other._ownedItems)),
              _cardColor(other._cardColor),
              _cardActiveColor(other._cardActiveColor),
              _radius(other._radius),
              _cardWidth(other._cardWidth),
              _cardHeight(other._cardHeight),
              _mode(other._mode),
              _checkedIndex(other._checkedIndex)
        {
        }

        UpdateListFluent &operator=(UpdateListFluent &&other) noexcept
        {
            if (this != &other)
            {
                detail::FluentLifetime::operator=(std::move(other));
                _items = std::exchange(other._items, nullptr);
                _itemCount = std::exchange(other._itemCount, 0);
                _ownedItems = std::move(other._ownedItems);
                _cardColor = other._cardColor;
                _cardActiveColor = other._cardActiveColor;
                _radius = other._radius;
                _cardWidth = other._cardWidth;
                _cardHeight = other._cardHeight;
                _mode = other._mode;
                _checkedIndex = other._checkedIndex;
            }
            return *this;
        }

        UpdateListFluent derive()
        {
            UpdateListFluent copy(_gui);
            copy._itemCount = _itemCount;
            copy._cardColor = _cardColor;
            copy._cardActiveColor = _cardActiveColor;
            copy._radius = _radius;
            copy._cardWidth = _cardWidth;
            copy._cardHeight = _cardHeight;
            copy._mode = _mode;
            copy._checkedIndex = _checkedIndex;

            if (_ownedItems.data && _itemCount > 0)
            {
                ListItemDef *itemsCopy = copy._ownedItems.copyFromPtr(_ownedItems.data, _itemCount);
                copy._items = itemsCopy;
            }
            else
            {
                copy._items = _items;
            }

            _consumed = true;
            return copy;
        }

        template <typename... Rest>
        UpdateListFluent &items(ListItemDef first, Rest... rest)
        {
            PIPGUI_FLUENT_GUARD();
            const ListItemDef packed[] = {first, static_cast<ListItemDef>(rest)...};
            const uint8_t count = static_cast<uint8_t>(1 + sizeof...(rest));
            ListItemDef *copy = _ownedItems.copyFromPtr(packed, count);
            if (!copy)
                return *this;
            _items = copy;
            _itemCount = count;
            return *this;
        }

        UpdateListFluent &inactive(uint16_t color565)
        {
            PIPGUI_FLUENT_GUARD();
            _cardColor = color565;
            return *this;
        }

        UpdateListFluent &active(uint16_t color565)
        {
            PIPGUI_FLUENT_GUARD();
            _cardActiveColor = color565;
            return *this;
        }

        UpdateListFluent &radius(uint8_t px)
        {
            PIPGUI_FLUENT_GUARD();
            _radius = px;
            return *this;
        }

        UpdateListFluent &cardSize(int16_t width, int16_t height)
        {
            PIPGUI_FLUENT_GUARD();
            _cardWidth = width;
            _cardHeight = height;
            return *this;
        }

        UpdateListFluent &mode(ListMode m)
        {
            PIPGUI_FLUENT_GUARD();
            _mode = m;
            return *this;
        }

        UpdateListFluent &checked(uint8_t idx)
        {
            PIPGUI_FLUENT_GUARD();
            _checkedIndex = idx;
            return *this;
        }

        ~UpdateListFluent() { apply(); }

    private:
        void apply();
    };

    struct TileInputFluent : detail::FluentLifetime
    {
        PIPGUI_DEFAULT_FLUENT_MOVE(TileInputFluent);
        bool _nextDown;
        bool _prevDown;
        bool _selectDown;
        bool _hasSelect;

        TileInputFluent(GUI *g)
            : detail::FluentLifetime(g), _nextDown(false), _prevDown(false), _selectDown(false), _hasSelect(false)
        {
        }

        TileInputFluent &nextDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _nextDown = v;
            return *this;
        }

        TileInputFluent &prevDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _prevDown = v;
            return *this;
        }

        TileInputFluent &selectDown(bool v)
        {
            PIPGUI_FLUENT_GUARD();
            _selectDown = v;
            _hasSelect = true;
            return *this;
        }

        ~TileInputFluent() { apply(); }

    private:
        void apply();
    };

    struct UpdateTileFluent : detail::FluentLifetime
    {
        uint8_t _itemCount;
        detail::OwnedHeapArray<TileItemDef> _ownedItems;
        uint8_t _itemCapacity;
        detail::OwnedHeapArray<TilePlacementDef> _ownedPlacements;
        uint8_t _placementCapacity;
        uint8_t _lastTileIndex;
        uint8_t _gridCols;
        uint8_t _gridRows;
        bool _customLayout;
        uint32_t _cardColor;
        uint32_t _cardActiveColor;
        uint8_t _radius;
        uint8_t _spacing;
        uint8_t _columns;
        int16_t _tileWidth;
        int16_t _tileHeight;
        TileMode _contentMode;

        UpdateTileFluent(GUI *g)
            : detail::FluentLifetime(g), _itemCount(0), _ownedItems(detail::resolvePlatform(g)), _itemCapacity(0),
              _ownedPlacements(detail::resolvePlatform(g)), _placementCapacity(0), _lastTileIndex(0xFF),
              _gridCols(0), _gridRows(0), _customLayout(false),
              _cardColor(0), _cardActiveColor(0), _radius(0), _spacing(8),
              _columns(2), _tileWidth(0), _tileHeight(0),
              _contentMode(TextSubtitle)
        {
        }

        UpdateTileFluent(UpdateTileFluent &&other) noexcept
            : detail::FluentLifetime(std::move(other)),
              _itemCount(std::exchange(other._itemCount, 0)),
              _ownedItems(std::move(other._ownedItems)),
              _itemCapacity(std::exchange(other._itemCapacity, 0)),
              _ownedPlacements(std::move(other._ownedPlacements)),
              _placementCapacity(std::exchange(other._placementCapacity, 0)),
              _lastTileIndex(std::exchange(other._lastTileIndex, 0xFF)),
              _gridCols(std::exchange(other._gridCols, 0)),
              _gridRows(std::exchange(other._gridRows, 0)),
              _customLayout(std::exchange(other._customLayout, false)),
              _cardColor(other._cardColor),
              _cardActiveColor(other._cardActiveColor),
              _radius(other._radius),
              _spacing(other._spacing),
              _columns(other._columns),
              _tileWidth(other._tileWidth),
              _tileHeight(other._tileHeight),
              _contentMode(other._contentMode)
        {
        }

        UpdateTileFluent &operator=(UpdateTileFluent &&other) noexcept
        {
            if (this != &other)
            {
                detail::FluentLifetime::operator=(std::move(other));
                _itemCount = std::exchange(other._itemCount, 0);
                _ownedItems = std::move(other._ownedItems);
                _itemCapacity = std::exchange(other._itemCapacity, 0);
                _ownedPlacements = std::move(other._ownedPlacements);
                _placementCapacity = std::exchange(other._placementCapacity, 0);
                _lastTileIndex = std::exchange(other._lastTileIndex, 0xFF);
                _gridCols = std::exchange(other._gridCols, 0);
                _gridRows = std::exchange(other._gridRows, 0);
                _customLayout = std::exchange(other._customLayout, false);
                _cardColor = other._cardColor;
                _cardActiveColor = other._cardActiveColor;
                _radius = other._radius;
                _spacing = other._spacing;
                _columns = other._columns;
                _tileWidth = other._tileWidth;
                _tileHeight = other._tileHeight;
                _contentMode = other._contentMode;
            }
            return *this;
        }

        UpdateTileFluent derive()
        {
            UpdateTileFluent copy(_gui);
            copy._itemCount = _itemCount;
            copy._itemCapacity = _itemCount;
            copy._placementCapacity = _itemCount;
            copy._lastTileIndex = _lastTileIndex;
            copy._gridCols = _gridCols;
            copy._gridRows = _gridRows;
            copy._customLayout = _customLayout;
            copy._cardColor = _cardColor;
            copy._cardActiveColor = _cardActiveColor;
            copy._radius = _radius;
            copy._spacing = _spacing;
            copy._columns = _columns;
            copy._tileWidth = _tileWidth;
            copy._tileHeight = _tileHeight;
            copy._contentMode = _contentMode;

            if (_ownedItems.data && _itemCount > 0)
            {
                TileItemDef *itemsCopy = copy._ownedItems.copyFromPtr(_ownedItems.data, _itemCount);
                (void)itemsCopy;
            }

            if (_ownedPlacements.data && _itemCount > 0)
            {
                TilePlacementDef *placementsCopy = copy._ownedPlacements.copyFromPtr(_ownedPlacements.data, _itemCount);
                (void)placementsCopy;
            }

            _consumed = true;
            return copy;
        }

        UpdateTileFluent &grid(uint8_t cols, uint8_t rows)
        {
            PIPGUI_FLUENT_GUARD();
            _itemCount = 0;
            _lastTileIndex = 0xFF;
            _gridCols = cols;
            _gridRows = rows;
            _customLayout = true;
            return *this;
        }

        template <typename... Rest>
        UpdateTileFluent &items(TileItemDef first, Rest... rest)
        {
            PIPGUI_FLUENT_GUARD();
            const TileItemDef packed[] = {first, static_cast<TileItemDef>(rest)...};
            const uint8_t count = static_cast<uint8_t>(1 + sizeof...(rest));
            TileItemDef *copy = _ownedItems.copyFromPtr(packed, count);
            if (!copy)
                return *this;
            _itemCapacity = count;
            _itemCount = count;
            _lastTileIndex = 0xFF;
            _customLayout = false;
            (void)copy;
            return *this;
        }

        UpdateTileFluent &tile(const char *title, const char *subtitle, uint8_t target)
        {
            PIPGUI_FLUENT_GUARD();
            if (!_customLayout)
            {
                _itemCount = 0;
                _gridCols = 0;
                _gridRows = 0;
                _customLayout = true;
            }
            if (!detail::ensureCapacity(_ownedItems.plat, _ownedItems.data, _itemCapacity, static_cast<uint8_t>(_itemCount + 1)))
                return *this;
            if (!detail::ensureCapacity(_ownedPlacements.plat, _ownedPlacements.data, _placementCapacity, static_cast<uint8_t>(_itemCount + 1)))
                return *this;
            _ownedItems.data[_itemCount] = TileItemDef(title, subtitle, target);
            _ownedPlacements.data[_itemCount] = TilePlacementDef();
            _lastTileIndex = _itemCount;
            ++_itemCount;
            return *this;
        }

        UpdateTileFluent &tile(uint16_t icon, const char *title, const char *subtitle, uint8_t target)
        {
            PIPGUI_FLUENT_GUARD();
            if (!_customLayout)
            {
                _itemCount = 0;
                _gridCols = 0;
                _gridRows = 0;
                _customLayout = true;
            }
            if (!detail::ensureCapacity(_ownedItems.plat, _ownedItems.data, _itemCapacity, static_cast<uint8_t>(_itemCount + 1)))
                return *this;
            if (!detail::ensureCapacity(_ownedPlacements.plat, _ownedPlacements.data, _placementCapacity, static_cast<uint8_t>(_itemCount + 1)))
                return *this;
            _ownedItems.data[_itemCount] = TileItemDef(icon, title, subtitle, target);
            _ownedPlacements.data[_itemCount] = TilePlacementDef();
            _lastTileIndex = _itemCount;
            ++_itemCount;
            return *this;
        }

        UpdateTileFluent &at(uint8_t col, uint8_t row)
        {
            if (!canMutate() || _lastTileIndex == 0xFF || !_ownedPlacements.data)
                return *this;
            _ownedPlacements.data[_lastTileIndex].col = col;
            _ownedPlacements.data[_lastTileIndex].row = row;
            return *this;
        }

        UpdateTileFluent &span(uint8_t cols = 1, uint8_t rows = 1)
        {
            if (!canMutate() || _lastTileIndex == 0xFF || !_ownedPlacements.data)
                return *this;
            _ownedPlacements.data[_lastTileIndex].colSpan = cols;
            _ownedPlacements.data[_lastTileIndex].rowSpan = rows;
            return *this;
        }

        UpdateTileFluent &inactive(uint32_t color888)
        {
            PIPGUI_FLUENT_GUARD();
            _cardColor = color888;
            return *this;
        }

        UpdateTileFluent &active(uint32_t color888)
        {
            PIPGUI_FLUENT_GUARD();
            _cardActiveColor = color888;
            return *this;
        }

        UpdateTileFluent &radius(uint8_t px)
        {
            PIPGUI_FLUENT_GUARD();
            _radius = px;
            return *this;
        }

        UpdateTileFluent &spacing(uint8_t px)
        {
            PIPGUI_FLUENT_GUARD();
            _spacing = px;
            return *this;
        }

        UpdateTileFluent &columns(uint8_t value)
        {
            PIPGUI_FLUENT_GUARD();
            _columns = value;
            return *this;
        }

        UpdateTileFluent &tileSize(int16_t width, int16_t height)
        {
            PIPGUI_FLUENT_GUARD();
            _tileWidth = width;
            _tileHeight = height;
            return *this;
        }

        UpdateTileFluent &mode(TileMode value)
        {
            PIPGUI_FLUENT_GUARD();
            _contentMode = value;
            return *this;
        }

        ~UpdateTileFluent() { apply(); }

    private:
        void apply();
    };
}
